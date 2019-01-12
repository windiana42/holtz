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

#define MY_WX_MAKING_DLL
#include "network.hpp" // workaround for cygwin 

#include "ai.hpp"

#include "util.hpp"
#include "wxdvonn.hpp"
#include "animations.hpp"

#include <assert.h>

#define RATE_MAX	1e13
// #ifndef NDEBUG
// #  warning "disable DEBUG_MIN_MAX in production"
// #  define DEBUG_MIN_MAX  // verify min-max algo is correct !!! never activate this in production !!!
// #  define DEBUG_MIN_MAX_PRINT
// #endif

namespace dvonn
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
    : set_moves( game.board.get_game_state() == Board::set_moves ),
      expanded(0), rating(0), handler(0)
  {
  }

  Position::Position( bool set_moves )
    : set_moves( set_moves ),
      expanded(0), rating(0), handler(0)
  {
  }

  Position::~Position()
  {
    std::list< Branch* >::iterator i;
    for( i = following_positions.begin(); i != following_positions.end(); ++i )
      delete *i;
  }

  bool Position::add_jump_moves( Game &game, Field_Pos from )
				// caller has to ensure that at <from> is a movable stone
  {
    bool ret = true;
    bool set_moves = false;

    Field_Iterator from_field(from, &game.board);
    assert( from_field.is_valid_field() );
    assert( !from_field->empty() );
    assert( Board::is_stone(from_field->back()) );
    int stack_height = from_field->size();
    for( int d=Field_Iterator::right; d<=Field_Iterator::bottom_right; ++d )
    {
      Field_Iterator::Direction dir = Field_Iterator::Direction(d);
      bool valid = true;
      Field_Iterator to_field = from_field;
      for( int i=0; i<stack_height; ++i )
      {
	to_field.Go(dir);
	if( !to_field.is_valid_field() ) 
	{
	  valid = false;
	  break;
	}
      }
      if( !valid ) continue;
      if( !Board::is_stone(*to_field) ) continue;
      // valid jump move
      Move_Sequence sequence;
      sequence.add_move( new Jump_Move( from, to_field.get_pos() ) );
      sequence.add_move( new Finish_Move() );
      assert( sequence.check_sequence( game ) );
      sequence.do_sequence( game );
      ret = add_branch( game, new Branch( sequence, Position(set_moves) ) );
      sequence.undo_sequence( game );
      if( !ret ) break;
    }
    return ret;
  }
  bool Position::add_set_move( Game &game, Field_Pos to )
  {
    bool ret = true;
    bool set_moves = game.board.num_empty_fields > 2;
    Move_Sequence sequence;
    Move *move = new Set_Move( to );
    sequence.add_move( move );
    assert( sequence.check_sequence( game ) );
    sequence.do_sequence( game );
    ret = add_branch( game, new Branch( sequence, Position(set_moves) ) );
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

  void Position::calc_following_positions( Game &game, Field_Permutation &field_permutation )
  {
    Move_Sequence current_sequence;
    following_positions.clear();
    sorted_positions.clear();
    
    bool go_on = true;
    if( set_moves )
    {
      Field_Pos pos;
      Field_Iterator p1(&game.board);
     
      field_permutation.new_context();
      for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	   pos = field_permutation.get_next() )
      {
	p1.set_pos( pos );
	if( Board::is_empty(*p1) )
	{
	  go_on = add_set_move( game, pos );
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
      for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	   pos = field_permutation.get_next() )
      {
	p1.set_pos( pos );
	assert( !p1->empty() );
	if( p1->back() != Field_State_Type(game.current_player->stone_type) ) continue;

	// *******************************
	// check whether stone is movable
	bool movable = false;
	for( int d=Field_Iterator::right; d<=Field_Iterator::bottom_right; ++d )
	{
	  Field_Iterator::Direction dir = Field_Iterator::Direction(d);
	  Field_Iterator p2 = p1;
	  p2.Go(dir);
	  if(!p2.is_valid_field())
	  {
	    movable = true;
	    break;
	  }
	  if( !Board::is_stone(*p2) )
	  {
	    movable = true;
	    break;
	  }
	}
	if( movable )
	  go_on = add_jump_moves( game, pos );

	if( !go_on ) 
	  break;
      }
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
  // Distance_Map
  // ----------------------------------------------------------------------------

  Distance_Map::Distance_Map( Board &board )
    : board(board)
  {
    field.resize( board.field.size() );
    std::vector< std::vector< Field > >::iterator x_it1;
    std::vector< std::vector< std::deque<Field_State_Type> > >::iterator x_it2;
    for( x_it1 = field.begin(), x_it2 = board.field.begin(); 
	 x_it2 != board.field.end(); ++x_it1, ++x_it2 )
    {
      std::vector< Field > &col1 = *x_it1;
      std::vector< std::deque<Field_State_Type> > &col2 = *x_it2;
      col1.resize(col2.size());
    }
  }

  void Distance_Map::find_closest_distances( bool across_empty )
  {
    std::multimap<int,Field_Pos> priority_list;
    red_stones.clear();
    white_control.clear();
    black_control.clear();
    int x,y;
    for( x=0; x<board.get_x_size(); ++x )
    {
      for( y=0; y<board.get_y_size(); ++y )
      {
	Field_Pos pos(x,y);
	Field &fld = field[x][y];
	std::deque<Field_State_Type> &stack = board.field[x][y];
	if( Board::is_stone(stack) )
	{
	  if( stack.back() == field_white )
	    white_control.push_back(pos);
	  else if ( stack.back() == field_black )
	    black_control.push_back(pos);

	  if( Board::includes_red_stone(stack) )
	  {
	    red_stones.push_back( pos );
	    fld.distance = 0;
	    priority_list.insert( std::pair<int,Field_Pos>(fld.distance,pos) );
	  }
	}
      }
    }
    while( !priority_list.empty() )
    {
      // remove element with highest priority (closest distance)
      int distance = priority_list.begin()->first;
      Field_Pos front = priority_list.begin()->second;
      priority_list.erase(priority_list.begin());
      Field &fld = field[front.x][front.y];
      if(!fld.optimal)
      {
	fld.distance = distance;
	fld.optimal = true;
	// insert neighboring fields in priority list
	Field_Iterator p1(front,&board);
	for( int d=Field_Iterator::right; d<=Field_Iterator::bottom_right; ++d )
	{
	  Field_Iterator::Direction dir = Field_Iterator::Direction(d);
	  Field_Iterator p2 = p1;
	  p2.Go(dir);
	  if( !p2.is_valid_field() ) continue;
	  if( Board::is_removed(*p2) ) continue;
	  if( !across_empty && Board::is_empty(*p2) ) continue;
	  Field_Pos pos2 = p2.get_pos();
	  Field &fld2 = field[pos2.x][pos2.y];
	  if( fld2.distance == -1 || fld2.distance > distance + 1 )
	  {
	    assert(!fld2.optimal);
	    fld2.distance = distance + 1;
	    priority_list.insert(std::pair<int,Field_Pos>(fld2.distance,pos2));
	  }
	}
      }
    }
  }
  
  // ----------------------------------------------------------------------------
  // AI
  // ----------------------------------------------------------------------------

  AI::AI( const Game &game )
    : max_time(30000), average_time(4000), num_measures(0), max_positions_expanded(150000),
      max_depth(12),
      alloc_max_depth(12+3),
      rate_alpha(0.85),		// exponential value degradation with distance to red stone
      rate_beta_no_good(0.3), 	// factor if only move on own stones possible
      rate_beta_not_now(0.85), 	// factor if stack is blocked by surrounding stones
      rate_beta_red(1.2), 	// factor if stack can move on red stone
      rate_beta_add_bonus(0.05), // adds to factor 1.0 for bonus conditions
      rate_current_player_bonus(0),  // this feature is not well implemented currently
      min_depth(2), 
      deep_jumping_possible(false/*not implemented,yet*/), while_deep_jumping(false),
      field_permutation( game.board )
  {
    real_average_time = 0;
  }

  AI_Result AI::get_move( const Game &game )
  {
    AI_Result result;

    Game save_game(game);
    Position current_position( save_game );

    if( need_stop_watch() )
      stop_watch.Start();

    aborted = false;
    don_t_abort = true;
    deep_jumping = false;
    if( deep_jumping_possible )
      deep_jumping = true;

    ai_player = save_game.current_player;
    unsigned depth, end_depth = max_depth;
    max_ratings.resize(2);
    max_ratings[0].resize(alloc_max_depth);
    max_ratings[1].resize(alloc_max_depth);
    if( save_game.common_stones.stone_count[Stones::red_stone] > 1 )
      end_depth = 1;		// don't waste time for placing the first two red stones
    for( depth = 1; depth <= end_depth; ++depth )
    {
      positions_checked = 0;
      max_depth_expanded = 0;
      expanded_calls = 0;
      cur_depth = depth;

#ifndef __WXMSW__
      std::cerr << std::endl;
      std::cerr << "AI: depth = " << depth << (deep_jumping?" (deep jumping)":"") << " player=" 
		<< ai_player->name << std::endl;
#endif

      for( unsigned d = 0; d < alloc_max_depth; ++d )
      {
	max_ratings[0/*min_max_vz=-1*/][d/*depth*/] = RATE_MAX * 2;
	max_ratings[1/*min_max_vz=1*/] [d/*depth*/] = -RATE_MAX * 2;
      }
      int new_min_max_vz = -1;
      depth_search( save_game, current_position, 0, max_ratings[new_min_max_vz>0?1:0][0], 
		    new_min_max_vz, /*jump-only=*/false, /*debug_disable_min_max=*/false );
      assert( ai_player == save_game.current_player );

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
		<< "; player=" << ai_player->name
		<< "; positions = " << positions_checked 
		<< "; max_depth = " << max_depth_expanded
		<< "; expanded = " << expanded_calls
		<< "; duration = " << stop_watch.Time() / 1000.0 << "s"
		<< "; current best move = " << result.sequence 
		<< "(" << DEBUG_translate_move(result.sequence) << ")" << std::endl;
      if( current_position.following_positions.size() > 1 )
      {
	Branch *second_branch = *(++current_position.sorted_positions.begin())->second;
	std::cerr << "AI: rating = " << second_branch->position.rating
		  << "; rating2 = " << (++current_position.sorted_positions.begin())->first
		  << "; second best move = " << second_branch->sequence 
		  << "(" << DEBUG_translate_move(second_branch->sequence) << ")" << std::endl;
      }

      Branch *worst_branch = *(--current_position.sorted_positions.end())->second;
      std::cerr << "AI: rating = " << worst_branch->position.rating
		<< "; rating2 = " << (--current_position.sorted_positions.end())->first
		<< "; worst move = " << worst_branch->sequence 
		<< "(" << DEBUG_translate_move(worst_branch->sequence) << ")" << std::endl;
      std::cerr << "AI: expect:" << std::endl; 
      std::cerr << "  best sequence: ";
      Position *pos = &current_position;
      while( pos->is_expanded() )
      {
	std::cerr << DEBUG_translate_move((*pos->sorted_positions.begin()->second)->sequence)
		  << "; ";
	pos = &(*pos->sorted_positions.begin()->second)->position;
      }
      std::cerr << std::endl;
#endif

#ifdef DEBUG_MIN_MAX
      double rating = current_position.rating;
      Move_Sequence best_move = result.sequence;
      std::string best_move_str = DEBUG_translate_move(result.sequence);
      Game save_game2(game);
      Position current_position2( save_game2 );
      new_min_max_vz = -1;
      depth_search( save_game2, current_position2, 0, 0, 
		    new_min_max_vz, /*jump-only=*/false, /*debug_disable_min_max=*/true );
      if( !aborted )
	{
	  Move_Sequence &sequence2 = (*current_position.sorted_positions.begin()->second)->sequence;
	  assert( sequence2.check_sequence( save_game2 ) );
	  double rating2 = current_position2.rating;
	  Move_Sequence &best_move2 = sequence2;
	  std::string best_move_str2 = DEBUG_translate_move(sequence2);
	  if( rating != rating2 )
	    {
	      std::cerr << "  best sequence2: ";
	      Position *pos = &current_position2;
	      while( !pos->sorted_positions.empty() )
		{
		  std::cerr << DEBUG_translate_move((*pos->sorted_positions.begin()->second)->sequence)
			    << "; ";
		  pos = &(*pos->sorted_positions.begin()->second)->position;
		}
	      std::cerr << std::endl;
	    }
	  assert(fabs(rating - rating2) < epsilon);
	  assert(best_move == best_move2);
	}
#endif

      if( (current_position.following_positions.size() == 1) || // if I have no choice
	  (current_position.rating ==  RATE_MAX) ||		// or if I can win
	  (current_position.rating == -RATE_MAX) )		// or if I can give up
      {
	std::cerr << "AI: aborting due to having no choice or having a guaranteed winner: " 
		  << "#choices=" << current_position.following_positions.size() << std::endl;
	break;
      }

      if( depth >= min_depth )	// after depth 2
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
	      << "; last_average_time = " << last_average_time / 1000.0 << "s"
	      << std::endl;
#endif


    result.used_time    = time;
    result.average_time = real_average_time;
    result.num_measures = num_measures;

    return result;
  }

  // static std::string prefix(int depth)
  // {
  //   std::string ret = "d" + long_to_string(depth) + ":";
  //   // indent 
  //   for( int i=0; i<depth; ++i )
  //   {
  //     ret += "  ";
  //   }
  //   return ret;
  // }

  double AI::rate_player( Game &game, Player &player, Distance_Map &distance_map )
  {
    double rate = 0;

    std::list<Field_Pos> *controlled_fields = 0;
    Field_State_Type own = Field_State_Type(player.stone_type);
    Field_State_Type opposite;
    if( own == field_white )
    {
      opposite = field_black;
      controlled_fields = &distance_map.white_control;
    }
    else
    {
      opposite = field_white;
      controlled_fields = &distance_map.black_control;
    }

    std::list<Field_Pos>::iterator it;
    for( it = controlled_fields->begin(); it != controlled_fields->end(); ++it )
    {
      Field_Pos pos = *it;
      int distance = distance_map.field[pos.x][pos.y].distance;
      int stack_height = game.board.field[pos.x][pos.y].size();
      bool movable;		// whether stone is currently(or after setting) movable
      bool never_movable;	// whether stone will never be able to move again
      // check potential destinations of moves
      std::list<Jump_Move> moves1 = game.board.get_jump_moves( pos, false /*don't check blocked*/ );
      if( game.board.get_game_state() == Board::set_moves )
      {
	if( game.board.is_border(pos) ) movable = true;
	else movable = false;
	never_movable = false;
      }
      else
      {
	movable = !moves1.empty() && !game.board.is_blocked(pos);
	never_movable = moves1.empty();
      }
      bool can_move_opposite = false;
      bool can_move_red = false;
      bool can_move_opposite_opposite = false;
      bool can_move_opposite_red = false;
      bool can_block_opposite_own = false;
      bool can_block_opposite_red = false;
      std::list<Jump_Move>::iterator it2, it3;
      for( it2 = moves1.begin(); it2 != moves1.end(); ++it2 )
      {
	Field_Pos dest1 = it2->to;
	Field_Iterator dest1_it( dest1, &game.board );
	if( Board::includes_red_stone(*dest1_it) )
	  can_move_red = true;
	if( dest1_it->back() == opposite )
	{
	  can_move_opposite = true;
	  std::list<Jump_Move> moves2 
	    = game.board.get_jump_moves( dest1, false /*don't check blocked*/ );
	  for( it3 = moves2.begin(); it3 != moves2.end(); ++it3 )
	  {
	    Field_Pos dest2 = it3->to;
	    Field_Iterator dest2_it( dest2, &game.board );
	    if( Board::includes_red_stone(*dest2_it) )
	      can_block_opposite_red = true;
	    if( dest2_it->back() == own )
	      can_block_opposite_own = true;
	  }
	  std::list<Jump_Move> moves3 
	    = game.board.get_jump_moves( dest1, false /*don't check blocked*/, 
					 stack_height /* additional height */ );
	  for( it3 = moves3.begin(); it3 != moves3.end(); ++it3 )
	  {
	    Field_Pos dest2 = it3->to;
	    Field_Iterator dest2_it( dest2, &game.board );
	    if( Board::includes_red_stone(*dest2_it) )
	      can_move_opposite_red = true;
	    if( dest2_it->back() == opposite )
	      can_move_opposite_opposite = true;
	  }
	}
      }
      // determine points for field
      if( !never_movable )
      {
	int boni = 0;
	double field_rate = (1. / stack_height) * pow(rate_alpha,distance);
	if( !can_move_opposite && !can_move_red )
	  field_rate *= rate_beta_no_good;
	if( !movable )
	  field_rate *= rate_beta_not_now;
	if( can_move_red )
	  field_rate *= rate_beta_red;
	if( can_move_red && can_move_opposite )
	  boni++;
	if( can_move_opposite_opposite )
	  boni++;
	if( can_move_opposite_red )
	  boni++;
	if( can_block_opposite_own )
	  boni++;
	if( can_block_opposite_red )
	  boni++;
	field_rate *= 1.0 + boni * rate_beta_add_bonus;
	rate += field_rate;
#ifndef __WXMSW__
	// DEBUG!!!
	/*
	std::cerr << "AI DEBUG: " << (own == field_white ? "[white] " : "[black] ")
		  << rate << "(+" << field_rate << ") " 
		  << pos.x << "/" << pos.y << " - " << stack_height << " "
		  << (can_move_red ? "move_red ":"")
		  << (can_move_opposite ? "move_opposite ":"")
		  << (can_move_opposite_opposite ? "move_opposite_opposite ":"")
		  << (can_move_opposite_red ? "move_opposite_red ":"")
		  << (can_block_opposite_own ? "block_opposite_own ":"")
		  << (can_block_opposite_red ? "block_opposite_red ":"")
		  << std::endl;
	*/
#endif
      }
    }
    return rate;
  }

  double AI::rate_position( Game &game, Position &position )
  {
    double rating = 0;
    // deep jumping is a feature of Zertz not Dvonn AI
//     if( deep_jumping && !position.set_moves && !while_deep_jumping )
//     {
//       while_deep_jumping = true;

//       unsigned save_cur_depth = cur_depth;
//       // store recursion parameters
//       unsigned save_depth = depth; 
//       int save_min_max_vz = min_max_vz; 

//       cur_depth += 4;		// additional depth
//       if( cur_depth > max_depth )
// 	cur_depth = max_depth;
//       max_ratings[depth+2] = -(RATE_MAX * 2) * min_max_vz;
//       // search for more jump moves
//       int new_min_max_vz = (game.current_player == game.prev_player ? min_max_vz : -min_max_vz);
//       rating = depth_search( game, position, depth+1, max_ratings[depth+1], new_min_max_vz, true );

//       if( (rating != RATE_MAX) && (rating != -RATE_MAX) )
// 	if( ai_player->id == game.get_current_player().id )
// 	  rating -= rate_current_player_bonus; // all other positions on this level "get the bonus"

//       // restore recursion parameters
//       depth = save_depth; 
//       min_max_vz = save_min_max_vz; 

//       if( rating == RATE_MAX || rating == -RATE_MAX )
// 	std::cout << "Deep-Jump Rating: depth=" << depth << " rating=" << rating 
// 		  << " min_max_vz=" << min_max_vz << std::endl;
      
//       cur_depth = save_cur_depth;

//       while_deep_jumping = false;
//     }
//     else
    {
      Distance_Map distance_map(game.board);
      if( game.board.get_game_state() == Board::set_moves )
	distance_map.find_closest_distances( true /*across empty fields*/);
      else
	distance_map.find_closest_distances( false /*across empty fields*/);

      std::vector<Player>::iterator player;
      for( player = game.players.begin(); player != game.players.end(); ++player )
      {
	assert( !game.win_condition->did_player_win(game, *player) );
	if( player->id == ai_player->id )
	{
	  rating += rate_player(game, *player, distance_map);
	}
	else
	{
	  rating -= rate_player(game, *player, distance_map);
	}
      }
      ++positions_checked;
#ifndef __WXMSW__
      /*
      // !!! DEBUG!!!
      std::cerr << "AI DEBUG: " << rating << "(" 
		<< (ai_player->stone_type == Stones::white_stone ? "white" : "black") 
		<< "), last player: " 
		<< (game.current_player->stone_type == Stones::white_stone ? "white" : "black") 
		<< std::endl;
      */
      // progress report
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
    double save_max = max_val;
    int save_min_max_vz = min_max_vz; 
    bool save_jump_only = jump_only;
    bool save_debug_disable_min_max = debug_disable_min_max;

    double pos_rating;

    // test if expanded move caused player to win:
    bool game_finished=false;
    int winner_id=-1;
    std::string winner_name;
    std::vector<Player>::iterator player;
    for( player = game.players.begin(); player != game.players.end(); ++player )
    {
      if( game.win_condition->did_player_win(game,*player) )
      {
	game_finished = true;
	winner_id = player->id;
	winner_name = player->name;
	break;
      }
    }

    if( game_finished )
    {
      if( ai_player->id == winner_id )
	pos_rating = RATE_MAX;
      else
	pos_rating = -RATE_MAX;
//       std::cout << "Winner: depth=" << depth << " win-id=" << winner_id << " rating=" << pos_rating 
// 		<< " min_max_vz=" << min_max_vz << " jump-only=" << jump_only << std::endl;
      /** /
      // DEBUG!!!
      std::cerr << "AI DEBUG: " << prefix(depth) << "   Winner: win-id=" << winner_id << "(" << winner_name << ")" 
		<< " rating=" << pos_rating << "(" << ai_player->id << ")" << " min_max_vz=" << min_max_vz 
		<< " jump-only=" << jump_only << std::endl;
      / **/
    }
    else
    {
      int prev_player_index = game.prev_player_index;
      bool ret = game.choose_next_player();
      if(!ret)			// no player can move -> tie
      {
	pos_rating = 0;
      }
      else
      {
	if( depth < cur_depth - 1 )
	{
	  // reset max_ratings for the depth where prev player may choose again
	  if( game.prev_player->id == game.current_player->id )
	  {
	    max_ratings[0/*min_max_vz=-1*/][depth+1] = RATE_MAX * 2;
	    max_ratings[1/*min_max_vz=1*/] [depth+1] = -RATE_MAX * 2;
	  }
	  max_ratings[0/*min_max_vz=-1*/][depth+2] = RATE_MAX * 2;
	  max_ratings[1/*min_max_vz=1*/] [depth+2] = -RATE_MAX * 2;
	  int new_min_max_vz = (game.current_player->id == game.prev_player->id ? min_max_vz : -min_max_vz);
	  pos_rating = depth_search( game, (*branch)->position, depth+1, 
				     max_ratings[new_min_max_vz>0?1:0][depth+1], new_min_max_vz, jump_only,
				     debug_disable_min_max);
	  // attention: avoid using depth/min_max_vz/etc., use save_depth/... from now on
	}
	else
	{
	  pos_rating = rate_position( game, (*branch)->position );
	}
      }
      game.choose_prev_player( prev_player_index );
    }

    // restore recursion parameters
    position = save_position;
    depth = save_depth; 
    max_val = save_max;
    min_max_vz = save_min_max_vz; 
    jump_only = save_jump_only;
    debug_disable_min_max = save_debug_disable_min_max;
	  
    if( (position->rating - pos_rating) * min_max_vz > 0 )
      {
	position->rating = pos_rating;
	/** /
	// DEBUG !!!
	std::cerr << "AI DEBUG: " << prefix(depth) << "   Finished(New best rating): move=" 
		  << DEBUG_translate_move((*branch)->sequence)
		  << " rating=" << pos_rating 
		  << " min_max_vz=" << min_max_vz << " player=" << game.current_player->name
		  << " jump-only=" << jump_only << std::endl;
	/ **/
      }
    else
      {
	/** /
	// DEBUG !!!
	std::cerr << "AI DEBUG: " << prefix(depth) << "   Finished: move=" << DEBUG_translate_move((*branch)->sequence) 
		  << " rating=" << pos_rating 
		  << " min_max_vz=" << min_max_vz << " player=" << game.current_player->name
		  << " jump-only=" << jump_only << std::endl;
	/ **/
      }
    
    position->sorted_positions.insert
      ( Position::sorted_positions_element_type( pos_rating * min_max_vz, branch ) );


    if(!debug_disable_min_max)
      {
	// bound tree branches according to min-max-algorithm
	//assert( max_val == max_ratings[depth] );
	if( depth == 0 )
	  assert( (max_val == RATE_MAX * 2) && (min_max_vz == -1) );
	assert( (pos_rating + epsilon >= -RATE_MAX) && (pos_rating - epsilon <= RATE_MAX) );
      
	if( (max_val - pos_rating) * min_max_vz >= 0 )
	  {
	    assert( depth != 0 );
	    return false;
	  }
      }

    if( should_stop(false) )
      return false;

    return true;
  }

  double AI::depth_search( Game &game, Position &position, unsigned depth, double max_val, 
			   int min_max_vz, bool jump_only/*unused*/, bool debug_disable_min_max )
  {
    // save recursive parameters for expanded()
    this->position = &position;
    this->depth = depth;
    this->max_val = max_val;
    this->min_max_vz = min_max_vz;
    this->jump_only = jump_only;
    this->debug_disable_min_max = debug_disable_min_max;

    position.rating = RATE_MAX * 1.5 * min_max_vz;
    bool go_on = true, stop = false;

    if( !position.is_expanded() )
    {
#ifndef __WXMSW__
      /** /
      // DEBUG!!!
      std::cerr << "AI DEBUG: " << prefix(depth) << " expanding new...; min_max_vz=" << min_max_vz 
		<< "; max=" << max_val << std::endl;
      / **/
#endif
      position.set_expanded_handler(this);
      position.calc_following_positions( game, field_permutation );
      if( !aborted && (max_depth_expanded < depth) )
	max_depth_expanded = depth;

#ifndef __WXMSW__
      if( while_deep_jumping )
      {
	/*
	  std::cout << "AI: expanded " << position.sorted_positions.size() << " positions" 
	  << "; rating = " << position.rating
	  << "; max = " << max_val
	  << "; depth = " << depth << std::endl;
	*/
      }
      /** /
      // DEBUG!!!
      std::cerr << "AI DEBUG: " << prefix(depth) << " expanded(new) " 
		<< position.sorted_positions.size() << " positions" 
		<< "; rating = " << position.rating
		<< "; min_max_vz=" << min_max_vz
		<< "; max = " << max_val
		<< std::endl;
      / **/
#endif
    }
    else
    {
#ifndef __WXMSW__
      /** /
      // DEBUG!!!
      std::cerr << "AI DEBUG: " << prefix(depth) << " expanding old... " 
		<< position.sorted_positions.size() << " positions"
		<< "; min_max_vz=" << min_max_vz
		<< "; max = " << max_val
		<< std::endl;
      / **/
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
      if( position.is_expanded() )
      {
	assert( position.sorted_positions.size() == position.following_positions.size() );
      }
      assert( position.sorted_positions.size() > 0 ); // any move should be possible
      /** /
      // DEBUG!!!
      std::cerr << "AI DEBUG: " << prefix(depth) << " expanded(old) " 
		<< position.sorted_positions.size() << " positions" 
		<< "; rating = " << position.rating
		<< "; min_max_vz=" << min_max_vz
		<< "; max = " << max_val
		<< std::endl;
      / **/
    }
    assert( (position.rating + epsilon >= -RATE_MAX) && (position.rating - epsilon <= RATE_MAX) );

    // update max_ratings if new maximum/minimum reached 
    if( (position.rating - max_ratings[min_max_vz>0?1:0][depth]) * min_max_vz > 0 )
      max_ratings[min_max_vz>0?1:0][depth] = position.rating;

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
#ifndef __WXMSW__
      if( depth_finished )
      {
	std::cerr << "AI: aborting due to maximum expanded positions reached: #expanded=" 
		  << expanded_calls << " max=" << max_positions_expanded << std::endl;
      }
#endif
      return true;
    }

    long time = stop_watch.Time();
    if( time > max_time )
    {
      aborted = true;
#ifndef __WXMSW__
      if( depth_finished )
      {
	std::cerr << "AI: aborting due to maximum time reached: time=" << (time/1000.0) << "s"
		  << " max=" << (max_time/1000.0) << "s" << std::endl;
      }
#endif
      return true;
    }

    if( (time == 0) && need_stop_watch() )
    {
      stop_watch.Start();
#ifndef __WXMSW__
      std::cerr << "AI: Error: stop watch failed!" << std::endl;
#endif
    }

    if( num_measures > 0 )
      time += (real_average_time - average_time) * 2;
    if( depth_finished )
    {
      if( time > average_time/2 ) 
      {
	aborted = true;
#ifndef __WXMSW__
	if( depth_finished )
	{
	  std::cerr << "AI: aborting due to target average time reached (end of depth): time=" << (time/1000.0) << "s"
		    << " target=" << (average_time/2/1000.0) << "s" << std::endl;
	}
#endif
	return true;
      }
    }
    else
    {
      if( time > average_time * 2 ) 
      {
	aborted = true;
#ifndef __WXMSW__
	if( depth_finished )
	{
	  std::cerr << "AI: aborting due to target average time reached: time=" << (time/1000.0) << "s"
		    << " target=" << (average_time*2/1000.0) << "s" << std::endl;
	}
#endif
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

  IMPLEMENT_DYNAMIC_CLASS( dvonn::AI_Event, wxEvent ) //**/

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

    Connect( -1, wxEVT_DVONN_NOTIFY, 
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

  void AI_Input::on_animation_done( wxCommandEvent & /*event*/ )
  {
    move_done = true;
    game_manager.continue_game();
  }
  
}

