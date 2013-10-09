/*
 * ai.cpp
 * 
 * GUI declaration
 * 
 * Copyright (c) 2003 by Martin Trautmann (martintrautmann@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#define MY_WX_MAKING_DLL // for WX MSW using mingw cross-compile
#include "network.hpp" // workaround for cygwin 

#include "ai.hpp"

#include "util.hpp"
#include "wxzertz.hpp"
#include "animations.hpp"

#include <assert.h>

#define RATE_MAX	1e13

namespace zertz
{
  // ----------------------------------------------------------------------------
  // Position_Expanded_Handler
  // ----------------------------------------------------------------------------

  Position_Expanded_Handler::~Position_Expanded_Handler() 
  {
  }

  // ----------------------------------------------------------------------------
  // Field_Permutation
  // ----------------------------------------------------------------------------

  Field_Permutation::Field_Permutation( const Board &board )
  {
    position_vector.resize( next_prime( board.get_x_size() * board.get_y_size() ) );
    unsigned num = 0;
    Field_Pos pos;
    for( pos.x = 0; pos.x < board.get_x_size(); ++pos.x )
      for( pos.y = 0; pos.y < board.get_y_size(); ++pos.y )
      {
	position_vector[num] = pos;
	num++;
      }

    num_valid_fields = num;

    for( ; num < position_vector.size(); ++num )
    {
      position_vector[num] = Field_Pos(-1,-1);
    }

    new_context();
  }

  Field_Pos Field_Permutation::get_first( bool new_random )
  {
    /*
    current_context->start = 0;
    current_context->jump = 1;
    */
    if( new_random )
    {
      current_context->start = random( num_valid_fields );
      current_context->jump  = random( 2, position_vector.size() - 2 );
    }
    current_context->cur_pos = current_context->start;

    return position_vector[current_context->cur_pos];
  }
  bool Field_Permutation::is_end()
  {
    return current_context->cur_pos < 0;
  }
  Field_Pos Field_Permutation::get_next()
  {
    if( current_context->cur_pos >= 0 )
    {
      do
      {
	current_context->cur_pos += current_context->jump;
	if( current_context->cur_pos >= int(position_vector.size()) )
	  current_context->cur_pos -= position_vector.size();
      }
      while( current_context->cur_pos >= num_valid_fields );

      if( current_context->cur_pos != current_context->start )
	return position_vector[current_context->cur_pos];
      else
	current_context->cur_pos = -1;
    }

    return Field_Pos(-1,-1);
  }

  void Field_Permutation::new_context()
  {
    contexts.push( Context( 0, 1, -1 ) );
    current_context = &contexts.top();
  }
  void Field_Permutation::restore_context()
  {
    assert( contexts.size() > 0 );
    contexts.pop();
    current_context = &contexts.top();
  }

  Field_Permutation::Context::Context( int start, int jump, int cur_pos )
    : start(start), jump(jump), cur_pos(cur_pos)
  {
  }

  // ----------------------------------------------------------------------------
  // Position
  // ----------------------------------------------------------------------------

  Position::Position( Game &game )
    : knock_out_possible( game.board.is_knock_out_possible() ),
      expanded(0), rating(0), handler(0)
  {
  }

  Position::Position( bool knock_out_possible )
    : knock_out_possible( knock_out_possible ),
      expanded(0), rating(0), handler(0)
  {
  }

  Position::~Position()
  {
    std::list< Branch* >::iterator i;
    for( i = following_positions.begin(); i != following_positions.end(); ++i )
      delete *i;
  }

  bool Position::add_knock_out_moves( Game &game, Move_Sequence &sequence, 
				      Field_Iterator from, Field_Iterator over, Field_Iterator to )
  {
    bool ret = true;

    Move *move = new Knock_Out_Move( from.get_pos(), over.get_pos(), to.get_pos() );
    assert( move->check_move( game ) );
    bool ok = sequence.add_move( game, move );
    assert( ok );
    move->do_move( game );
    from = to;			// destination field is start field for further moves

    bool knock_out = false;
    // search in any direction
    int dir;
    for( dir =  Field_Iterator::right;
	 dir <= Field_Iterator::bottom_right; ++dir )
    {
      over = from.Next( Field_Iterator::Direction(dir) );
      to   = over.Next( Field_Iterator::Direction(dir) );
      if( to.is_valid_field() )
      {
	assert( over.is_valid_field() );
	if( Board::is_stone(*over) )	// middle must be stone
	{
	  if( Board::is_empty(*to) && Board::is_stone(*from) )
	  {
	    knock_out = true;
	    ret = add_knock_out_moves( game, sequence, from, over, to ); // add recursive
	  }
	}
      }
      if( !ret ) 
	break;
    }
    if( !knock_out )		// if no further knock out possible
    {
      // add finish move
      move = new Finish_Move();
      assert( move->check_move( game ) );
      ok = sequence.add_move(game,move);
      assert( ok );
      move->do_move(game);
      
      bool knock_out_possible = game.board.is_knock_out_possible();
      ret = add_branch( game, new Branch( sequence.clone(), Position(knock_out_possible) ) );

      sequence.undo_last_move( game );
    }
    sequence.undo_last_move( game );
    return ret;
  }

  bool Position::add_set_moves( Game &game, Field_Iterator to, Stones::Stone_Type type,
				Field_Permutation &field_permutation, bool expand_first )
  {
    bool ret = true;
    Move_Sequence sequence;

    Move *move = new Set_Move( to.get_pos(), type );
    assert( move->check_move( game ) );
    bool ok = sequence.add_move( game, move );
    assert( ok );
    move->do_move( game );

    field_permutation.new_context();
    bool any_removable = false;
    Field_Pos pos;
    for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	 pos = field_permutation.get_next() )
    {
      if( game.board.is_removable(pos) )
      {
	any_removable = true;

	move = new Remove( pos );
	assert( move->check_move( game ) );
	ok = sequence.add_move( game, move );
	assert( ok );
	move->do_move( game );
	
	move = new Finish_Move();
	assert( move->check_move( game ) );
	ok = sequence.add_move( game, move );
	assert( ok );
	move->do_move( game );
	
	bool knock_out_possible = game.board.is_knock_out_possible();
	ret = add_branch( game, new Branch( sequence.clone(), Position(knock_out_possible) ) );
	sequence.undo_last_move( game );
	sequence.undo_last_move( game );

	if( expand_first )
	  break;		// first is already expanded
      }
      if( !ret )
	break;
    }
    field_permutation.restore_context(); // only restore in second pass
    
    if( !any_removable && expand_first )
    {
      Move *move = new Finish_Move();
      assert( move->check_move( game ) );
      bool ok = sequence.add_move( game, move );
      assert( ok );
      move->do_move( game );

      bool knock_out_possible = game.board.is_knock_out_possible();
      ret = add_branch( game, new Branch( sequence.clone(), Position(knock_out_possible) ) );
    }
    sequence.undo_sequence( game );
    return ret;
  }

  bool Position::add_branch( Game &game, Branch *branch )
  {
    std::list<Branch*>::iterator i = following_positions.insert( following_positions.end(), branch );
    if( handler )
      return handler->expanded( game, i );

    return true;
  }

  void Position::calc_following_positions( Game &game, Field_Permutation &field_permutation, 
					   std::vector<Stones::Stone_Type> &stone_types )
  {
    Move_Sequence current_sequence;
    following_positions.clear();
    sorted_positions.clear();
    
    bool go_on = true;

    if( knock_out_possible )
    {
      Field_Pos pos;
      Field_Iterator p1(&game.board), p2(&game.board), p3(&game.board);
	
      field_permutation.new_context();
      for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	   pos = field_permutation.get_next() )
      {
	p1.set_pos( pos );
	// ******************************************
	// search each pair of three fields in a row:

	// search: horizontal
	p2 = p1.Next_Right();
	p3 = p2.Next_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      go_on = add_knock_out_moves( game, current_sequence, p1, p2, p3 );
	    else
	      go_on = add_knock_out_moves( game, current_sequence, p3, p2, p1 );
	    assert( current_sequence.is_empty() );
	  }
	}

	if( !go_on ) 
	  break;

	// search: bottom left
	p2 = p1.Next_Bottom_Left();
	p3 = p2.Next_Bottom_Left();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      go_on = add_knock_out_moves( game, current_sequence, p1, p2, p3 );
	    else
	      go_on = add_knock_out_moves( game, current_sequence, p3, p2, p1 );
	    assert( current_sequence.is_empty() );
	  }
	}

	if( !go_on ) 
	  break;

	// search: bottom right
	p2 = p1.Next_Bottom_Right();
	p3 = p2.Next_Bottom_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      go_on = add_knock_out_moves( game, current_sequence, p1, p2, p3 );
	    else
	      go_on = add_knock_out_moves( game, current_sequence, p3, p2, p1 );
	    assert( current_sequence.is_empty() );
	  }
	}
	if( !go_on ) 
	  break;
      }
      field_permutation.restore_context();
    }
    else
    {
      Field_Pos pos;
      Field_Iterator p1(&game.board);
     
      field_permutation.new_context();
      bool expand_first = true; // expand only one of each set moves
      do
      {
	for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	     pos = field_permutation.get_next() )
	{
	  p1.set_pos( pos );
	  if( *p1 == field_empty )
	  {
	    bool any = false;
	    int start = random(3);
	    for( int i = 0; i < 3; ++i )
	    {
	      if( !go_on ) 
		break;

	      int type = (start + i) % 3;
	      if( game.common_stones.stone_count[ stone_types[type] ] > 0 )
	      {
		go_on = add_set_moves( game, p1, stone_types[type], field_permutation,
				       expand_first );
		any = true;
	      }
	    }
	    if( !any )
	    {
	      for( int i = 0; i < 3; ++i )
	      {
		if( !go_on ) 
		  break;
	      
		int type = (start + i) % 3;
		if( game.get_current_player().stones.stone_count[ stone_types[type] ] > 0 )
		  go_on = add_set_moves( game, p1, stone_types[type], field_permutation,
					 expand_first );
	      }
	    }
	  }
	  if( !go_on ) 
	    break;
	}
	expand_first = !expand_first;
      }while( !expand_first );
      field_permutation.restore_context();
    }
    if( go_on ) 
      expanded = true;
  }

  void Position::set_expanded_handler( Position_Expanded_Handler *_handler )
  {
    handler = _handler;
  }

  // ----------------------------------------------------------------------------
  // Branch
  // ----------------------------------------------------------------------------

  Branch::Branch( Move_Sequence sequence, Position position )
    : sequence(sequence), position(position)
  {
  }

  // ----------------------------------------------------------------------------
  // AI_Result
  // ----------------------------------------------------------------------------

  AI_Result::AI_Result()
    : rating(0), depth(0), used_time(0), average_time(0), num_measures(0), valid(false)
  {
  }

  AI_Result::AI_Result( Move_Sequence sequence, double rating, int depth, 
			long used_time, long average_time, int num_measures )
    : sequence(sequence), rating(rating), depth(depth), 
      used_time(used_time), average_time(average_time), num_measures(num_measures),
      valid(true)
  {
  }

  // ----------------------------------------------------------------------------
  // AI
  // ----------------------------------------------------------------------------

  AI::AI( const Game &game )
    : max_time(30000), average_time(5000), num_measures(0), max_positions_expanded(150000),
      max_depth(12),
      rate_white(4.5), rate_grey(3.5), rate_black(2.5), rate_current_player_bonus(0.3),
      min_depth(2), deep_knocking_possible(true), while_deep_knocking(false),
      field_permutation( game.board )
  {
    stone_types.resize(3);
    stone_types[0] = Stones::white_stone;
    stone_types[1] = Stones::grey_stone;
    stone_types[2] = Stones::black_stone;
    real_average_time = 0;
  }

  AI_Result AI::get_move( const Game &game )
  {
    AI_Result result;

    Game save_game(game);
    Position current_position( save_game );

    if( need_stop_watch() )
      stop_watch.Start();

    //while( stop_watch.Time() == 0 ); // check for stop watch to be started
    aborted = false;
    don_t_abort = true;
    deep_knocking = false;
    if( deep_knocking_possible )
      deep_knocking = true;

    current_player = save_game.current_player;
    unsigned depth;
    max_ratings.resize(max_depth+3);
    for( depth = 1; depth <= max_depth; ++depth )
    {
      positions_checked = 0;
      max_depth_expanded = 0;
      expanded_calls = 0;
      cur_depth = depth;

#ifndef __WXMSW__
      std::cerr << std::endl;
      std::cerr << "AI: depth = " << depth << (deep_knocking?" (deep knocking)":"") << std::endl;
#endif

      max_ratings[0] = RATE_MAX * 2;
      max_ratings[1] = -RATE_MAX * 2;
      depth_search( save_game, current_position, 0, max_ratings[0], -1, false );
      assert( current_player == save_game.current_player );

      if( aborted )
	break;
      else
      {
	// new best move
	Move_Sequence &sequence = (*current_position.sorted_positions.begin()->second)->sequence;
	assert( sequence.check_sequence( save_game ) );
	result = AI_Result( sequence.clone(), current_position.rating, depth, 
			    stop_watch.Time(), real_average_time, num_measures );
	report_current_hint( result );
      }

      assert( current_position.sorted_positions.size() != 0 );

#ifndef __WXMSW__
      std::cerr << "AI: rating = " << current_position.rating
		<< "; rating2 = " << current_position.sorted_positions.begin()->first
		<< "; positions = " << positions_checked 
		<< "; max_depth = " << max_depth_expanded
		<< "; expanded = " << expanded_calls
		<< "; duration = " << stop_watch.Time() / 1000.0 << "s"
		<< "; current best move = " << result.sequence << std::endl;
      if( current_position.following_positions.size() > 1 )
      {
	Branch *second_branch = *(++current_position.sorted_positions.begin())->second;
	std::cerr << "AI: rating = " << second_branch->position.rating
		  << "; rating2 = " << (++current_position.sorted_positions.begin())->first
		  << "; second best move = " << second_branch->sequence << std::endl;
      }

      Branch *worst_branch = *(--current_position.sorted_positions.end())->second;
      std::cerr << "AI: rating = " << worst_branch->position.rating
		<< "; rating2 = " << (--current_position.sorted_positions.end())->first
		<< "; worst move = " << worst_branch->sequence << std::endl;
      std::cerr << "AI: expect:" << std::endl; 
      Position *pos = &current_position;
      while( pos->is_expanded() )
      {
	std::cerr << "  " << (*pos->sorted_positions.begin()->second)->sequence 
		  << ": " << (*pos->sorted_positions.begin()->second)->position.rating
		  << std::endl; 
	pos = &(*pos->sorted_positions.begin()->second)->position;
      }
#endif

      if( (current_position.following_positions.size() == 1) || // if I have no choice
	  (current_position.rating ==  RATE_MAX) ||		// or if I can win
	  (current_position.rating == -RATE_MAX) )		// or if I can give up
	break;

      if( depth >= min_depth )		// after depth 2
	don_t_abort = false;	// make abort possible

      if( should_stop(true) )
	break;
    }

#ifndef __WXMSW__
    long last_average_time = real_average_time;
#endif

    long time = stop_watch.Time();
    real_average_time = real_average_time * num_measures + time;
    ++num_measures;
    real_average_time /= num_measures;

#ifndef __WXMSW__
    std::cerr << "AI: average_time = " << real_average_time / 1000.0 << "s" 
	      << "; num_measures = " << num_measures 
	      << "; last_average_time = " << last_average_time
	      << std::endl;
#endif


    result.used_time    = time;
    result.average_time = real_average_time;
    result.num_measures = num_measures;

    return result;
  }

  double AI::rate_player( Player &player )
  {
    double rate = 0;
    rate += player.stones.stone_count[Stones::white_stone] * rate_white;
    rate += player.stones.stone_count[Stones::grey_stone]  * rate_grey;
    rate += player.stones.stone_count[Stones::black_stone] * rate_black;
    return rate;
  }

  double AI::rate_position( Game &game, Position &position )
  {
    double rating = 0;
    if( deep_knocking && position.knock_out_possible && !while_deep_knocking )
    {
      while_deep_knocking = true;

      unsigned save_cur_depth = cur_depth;
      // store recursion parameters
      unsigned save_depth = depth; 
      int save_min_max_vz = min_max_vz; 

      cur_depth += 6;		// additional depth
      if( cur_depth > max_depth )
	cur_depth = max_depth;
      max_ratings[depth+2] = -(RATE_MAX * 2) * min_max_vz;
      // search for all knock_out moves
      rating = depth_search( game, position, depth+1, max_ratings[depth+1], -min_max_vz, true );

      if( (rating != RATE_MAX) && (rating != -RATE_MAX) )
	if( current_player->id == game.get_current_player().id )
	  rating -= rate_current_player_bonus; // all other positions on this level "get the bonus"

      // restore recursion parameters
      depth = save_depth; 
      min_max_vz = save_min_max_vz; 
      
      cur_depth = save_cur_depth;

      while_deep_knocking = false;
    }
    else
    {
      std::vector<Player>::iterator player;
      for( player = game.players.begin(); player != game.players.end(); ++player )
      {
	assert( !game.win_condition->did_player_win(game, *player) );
	if( player->id == current_player->id )
	{
	  rating += rate_player(*player);
	}
	else
	{
	  rating -= rate_player(*player);
	}
      }
      ++positions_checked;
#ifndef __WXMSW__
      if( positions_checked % 10000 == 0 ) 
	std::cerr << "AI: positions = " << positions_checked << std::endl;
#endif
    }
    position.rating = rating;
    return rating;
  }

  // position expanded return false: don't continue
  bool AI::expanded( Game &game, std::list<Branch*>::iterator branch )  
  {
    ++expanded_calls;
	
    // store recursion parameters
    Position *save_position = position;
    unsigned save_depth = depth; 
    double save_max = max;
    int save_min_max_vz = min_max_vz; 
    bool save_knock_out_only = knock_out_only;

    double pos_rating;
    // test if expanded move caused player to win:
    if( game.win_condition->did_player_win( game, *game.current_player ) )
    {
      if( game.get_current_player().id == current_player->id )
	pos_rating = RATE_MAX;
      else
	pos_rating = -RATE_MAX;
    }
    else
    {
      int prev_player_index = game.prev_player_index;
      game.choose_next_player();
      if( depth < cur_depth - 1 )
      {
	max_ratings[depth+2] = -(RATE_MAX * 2) * min_max_vz;
	pos_rating = depth_search( game, (*branch)->position, depth+1, 
				   max_ratings[depth+1], -min_max_vz, knock_out_only );
      }
      else
      {
	pos_rating = rate_position( game, (*branch)->position );
      }
      game.choose_prev_player( prev_player_index );
    }

    // restore recursion parameters
    position = save_position;
    depth = save_depth; 
    max = save_max;
    min_max_vz = save_min_max_vz; 
    knock_out_only = save_knock_out_only;
	  
    if( (position->rating - pos_rating) * min_max_vz > 0 )
      position->rating = pos_rating;
    
    position->sorted_positions.insert
      ( Position::sorted_positions_element_type( pos_rating * min_max_vz, branch ) );

    assert( max == max_ratings[depth] );
    if( depth == 0 )
      assert( (max == RATE_MAX * 2) && (min_max_vz == -1) );
    assert( (pos_rating >= -RATE_MAX) && (pos_rating <= RATE_MAX) );
      
    if( (max - pos_rating) * min_max_vz >= 0 )
    {
      assert( depth != 0 );
      return false;
    }

    if( should_stop(false) )
      return false;

    return true;
  }

  double AI::depth_search( Game &game, Position &position, unsigned depth, double max, 
			   int min_max_vz, bool knock_out_only )
  {
    // save recursive parameters for expanded()
    this->position = &position;
    this->depth = depth;
    this->max = max;
    this->min_max_vz = min_max_vz;
    this->knock_out_only = knock_out_only;

    position.rating = RATE_MAX * 1.5 * min_max_vz;
    bool go_on = true, stop = false;

    if( knock_out_only && !position.knock_out_possible )
    {
      position.rating = rate_position( game, position );
      if( (position.rating != RATE_MAX) && (position.rating != -RATE_MAX) )
	if( current_player->id == game.get_current_player().id )
	  position.rating += rate_current_player_bonus;
    }
    else
    {
      if( !position.is_expanded() )
      {
	position.set_expanded_handler(this);
	position.calc_following_positions( game, field_permutation, stone_types );
	if( !aborted && (max_depth_expanded < depth) )
	  max_depth_expanded = depth;

#ifndef __WXMSW__
	if( while_deep_knocking )
	{
	  /*
	  std::cout << "AI: expanded " << position.sorted_positions.size() << " positions" 
		    << "; rating = " << position.rating
		    << "; max = " << max
		    << "; depth = " << depth << std::endl;
	  */
	}
	/*
	if( !knock_out_only && position.sorted_positions.size() > 1 )
	  std::cerr << "AI: expanded " << position.sorted_positions.size() << " positions" 
		    << "; rating = " << position.rating
		    << "; max = " << max
		    << std::endl;
	*/
#endif
      }
      else
      {
#ifndef __WXMSW__
	/*
	if( !knock_out_only )
	  std::cerr << "AI: work on " << position.sorted_positions.size() << " positions" << std::endl;
	*/
#endif
	assert( position.sorted_positions.size() == position.following_positions.size() );
	std::multimap<double,std::list<Branch*>::iterator> copy_sorted_positions;
	copy_sorted_positions = position.sorted_positions;	// copy sorted positions
	position.sorted_positions.clear();

	std::multimap<double,std::list<Branch*>::iterator>::iterator i;
	for( i = copy_sorted_positions.begin(); i != copy_sorted_positions.end(); ++i )
	{
	  std::list<Branch*>::iterator &branch = i->second;
	  (*branch)->sequence.do_sequence(game);
	  go_on = expanded( game, branch );
	  (*branch)->sequence.undo_sequence(game);

	  if( should_stop(false) )
	    stop = true;

	  if( !go_on || stop )
	  {
	    ++i;
	    break;
	  }
	}
	// copy rest of positions with bad rating (RATE_MAX)
	for( ; i != copy_sorted_positions.end(); ++i )
	{
	  std::list<Branch*>::iterator &branch = i->second;
	  position.sorted_positions.insert
	    ( Position::sorted_positions_element_type( RATE_MAX, branch ) );
	}
      }
      if( position.is_expanded() )
      {
	assert( position.sorted_positions.size() == position.following_positions.size() );
      }
      assert( position.sorted_positions.size() > 0 ); // any move should be possible
    }
    assert( (position.rating >= -RATE_MAX) && (position.rating <= RATE_MAX) );

    if( (position.rating - max_ratings[depth]) * min_max_vz > 0 )
      max_ratings[depth] = position.rating;

    return position.rating;
  }

  bool AI::should_stop( bool depth_finished )		// determines whether AI should stop now
  {
    if( don_t_abort )
      return false;		// always calculate depth 2

    if( aborted )
      return true;

    if( expanded_calls > max_positions_expanded )
    {
      aborted = true;
      return true;
    }

    long time = stop_watch.Time();
    if( time > max_time )
    {
      aborted = true;
      return true;
    }

    if( (time == 0) && need_stop_watch() )
    {
      stop_watch.Start();
#ifndef __WXMSW__
      std::cerr << "Error: stop watch failed!" << std::endl;
#endif
    }

    if( num_measures > 0 )
      time += (real_average_time - average_time) * 2;
    if( depth_finished )
    {
      if( time > average_time/2 ) 
      {
	aborted = true;
	return true;
      }
    }
    else
    {
      if( time > average_time * 2 ) 
      {
	aborted = true;
	return true;
      }
    }
    return false;
  }


  // ----------------------------------------------------------------------------
  // AI_Thread
  // ----------------------------------------------------------------------------

  AI_Thread::AI_Thread( wxEvtHandler *handler, const Game &game, 
			long last_average_time, int _num_measures, 
			bool give_hints_only )
    : AI(game), handler(handler), game(game), give_hints_only(give_hints_only)
  {
    if( (average_time >= 0) && (_num_measures > 0) )
      real_average_time = last_average_time;
    num_measures = _num_measures;
  }

  wxThread::ExitCode AI_Thread::Entry()
  {
    // determine move (get_move()) while blocking the AI thread
    if( give_hints_only )
    {
      get_move(game);
    }
    else
    {
      AI_Event event( get_move(game), EVT_AI_REPORT_MOVE, this ); 
      if( TestDestroy() ) return 0;
      handler->AddPendingEvent( event );
    }

    Sleep(200);			// assure that report move event is received earlier

    if( TestDestroy() ) return 0;

    AI_Event event( EVT_AI_FINISHED, this ); 
    handler->AddPendingEvent( event );

    // wait for external delete
    while( !TestDestroy() )
    {
      Sleep(100);
    }

    return 0;
  }

  bool AI_Thread::should_stop( bool depth_finished )	// determines whether AI should stop now
  {
    if( give_hints_only )
    {
      if( expanded_calls > max_positions_expanded )
	aborted = true;
      else
	aborted = TestDestroy();
      return aborted;
    }
    else
      return AI::should_stop( depth_finished );
  }

  void AI_Thread::report_current_hint( AI_Result hint )
  {
    AI_Event event( hint, EVT_AI_REPORT_HINT, this ); 
    if( TestDestroy() ) return;
    handler->AddPendingEvent( event );
  }

  // ----------------------------------------------------------------------------
  // Report_Move_Event
  // ----------------------------------------------------------------------------

  DEFINE_EVENT_TYPE(EVT_AI_REPORT_MOVE) //**/
  DEFINE_EVENT_TYPE(EVT_AI_REPORT_HINT) //**/
  DEFINE_EVENT_TYPE(EVT_AI_FINISHED)    //**/

  IMPLEMENT_DYNAMIC_CLASS( zertz::AI_Event, wxEvent ) //**/

  AI_Event::AI_Event()
    : wxEvent(-1)
  {
    SetEventType(EVT_AI_REPORT_MOVE);
  }

  AI_Event::AI_Event( WXTYPE type, AI_Thread *thread )
    : wxEvent(-1), thread(thread)
  {
    SetEventType(type);
  }

  AI_Event::AI_Event( AI_Result ai_result, WXTYPE type, AI_Thread *thread )
    : wxEvent(-1), ai_result(ai_result), thread(thread)
  {
    SetEventType(type);
  }

  // ----------------------------------------------------------------------------
  // AI_Input
  // ----------------------------------------------------------------------------


  AI_Input::AI_Input( Game_Manager &game_manager, Game_UI_Manager *ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), ai_done(false), move_done(false), 
      thread(0), thread_active(false), give_hints(false)
  {
    Connect( -1, EVT_AI_REPORT_MOVE, 
	     (wxObjectEventFunction) (wxEventFunction) (AI_Event_Function) 
	     &AI_Input::on_report_move );

    Connect( -1, EVT_AI_REPORT_HINT, 
	     (wxObjectEventFunction) (wxEventFunction) (AI_Event_Function) 
	     &AI_Input::on_report_hint );

    Connect( -1, EVT_AI_FINISHED, 
	     (wxObjectEventFunction) (wxEventFunction) (AI_Event_Function) 
	     &AI_Input::on_finished );

    Connect( -1, wxEVT_ZERTZ_NOTIFY, 
	     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) 
	     &AI_Input::on_animation_done );
  }

  AI_Input::~AI_Input()
  {
    abort();
  }

  Player_Input::Player_State AI_Input::determine_move() throw(Exception)
  {
    const Game &game = game_manager.get_game();
    if( game.get_current_player().help_mode == Player::show_hint )
      give_hints = true;
    else
      give_hints = false;

    if( ai_done )
    {
      if( move_done )
	return Player_Input::finished;
    }
    else
    {
      abort();			// abort possibly running thread
      thread_active = true;
      thread = new AI_Thread( this, game, game.get_current_player().average_time, 
			      game.get_current_player().num_measures );
      thread->Create();
      thread->Run();
    }
    return Player_Input::wait_for_event;
  }

  Move_Sequence AI_Input::get_move()
  {
    ai_done = false;
    move_done = false;
    return sequence;
  }

  long AI_Input::get_used_time()
  {
    return used_time;
  }

  void AI_Input::determine_hints()
  {
    const Game &game = game_manager.get_game();
    give_hints = true;
    abort();			// abort possibly running thread
    thread_active = true;
    thread = new AI_Thread( this, game, -1, 0, true );
    //thread->SetPriority( WXTHREAD_MIN_PRIORITY );
    thread->Create();
    thread->Run();
  }

  void AI_Input::abort()
  {
    thread_active = false;
    if( thread )
      thread->Delete();
    thread = 0;
  }

  void AI_Input::destroy_ai()
  {
    abort();
    //!! intentional memory leak since waiting until thread is actually destroyed is not that easy
    //wxPostDelete(this); // initiate deletion after thread was deleted
  }

  void AI_Input::on_report_move( AI_Event &event )
  {
    if( thread_active && (event.thread == thread) )
    {
      assert( event.ai_result.valid );	// assert that a move was found
      ai_done = true;

      used_time = event.ai_result.used_time;
      
      sequence = event.ai_result.sequence.clone();
      if( ui_manager )
      {
	if( give_hints )
	  ui_manager->remove_hint();
	ui_manager->do_move_slowly( sequence, this, 
				    -1 /*done event id*/, -1 /*abort event id*/ );
      }
    }
  }

  void AI_Input::on_report_hint( AI_Event &event )
  {
    if( thread_active && (event.thread == thread) && give_hints && ui_manager )
    {
      ui_manager->give_hint( event.ai_result );
    }
  }

  void AI_Input::on_finished( AI_Event &event )
  {
    if( thread_active && (event.thread == thread) )
    {
      abort();
    }
  }

  void AI_Input::on_animation_done( wxCommandEvent & )
  {
    move_done = true;
    game_manager.continue_game();
  }
  
}

