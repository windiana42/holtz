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

#include "wxbloks.hpp"
#include "util.hpp"
#include "animations.hpp"

#include <assert.h>
#include <math.h>

#define RATE_MAX	1e13
// #ifndef NDEBUG
// #  warning "disable DEBUG_MIN_MAX in production"
// #  define DEBUG_MIN_MAX  // verify min-max algo is correct !!! never activate this in production !!!
// #  define DEBUG_MIN_MAX_PRINT
// #endif

namespace bloks
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

  Position::Position( Game & /*game*/ )
    : expanded(0), rating(0), handler(0)
  {
  }

  Position::Position( bool /*set_moves*/ )
    : expanded(0), rating(0), handler(0)
  {
  }

  Position::~Position()
  {
    std::list< Branch* >::iterator i;
    for( i = following_positions.begin(); i != following_positions.end(); ++i )
      delete *i;
  }
  bool Position::add_set_move( Game &game, const Set_Move &move )
  {
    bool ret = true;
    bool set_moves = true; // never changes for bloks as opposed to dvonn
    Move_Sequence sequence;
    sequence.add_move( new Set_Move(move) );
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

  void Position::calc_following_positions( Game &game, Field_Permutation & /*field_permutation*/ )
  {
    Move_Sequence current_sequence;
    following_positions.clear();
    sorted_positions.clear();

    std::list<Set_Move> possible_moves 
      = game.get_possible_moves( *game.current_player, /*only_one_flip_direction=*/false, /*is_flipped=*/false, 
				 /*just_one=*/false, /*just_one_per_stone=*/false, 
				 /*just_stone_ID=*/-1, /*just_one_rotation_symmetric=*/true,
				 /*random_order=*/true );
    std::list<Set_Move>::iterator it;

    bool go_on = true;
    for( it=possible_moves.begin(); it!=possible_moves.end(); ++it )
    {
      Set_Move &move = *it;
      go_on = add_set_move( game, move );
      if(!go_on)
	break;
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

  // // ----------------------------------------------------------------------------
  // // Distance_Map
  // // ----------------------------------------------------------------------------

  // Distance_Map::Distance_Map( Board &board )
  //   : board(board)
  // {
  //   field.resize( board.field.size() );
  //   std::vector< std::vector< Field > >::iterator x_it1;
  //   std::vector< std::vector< std::deque<Field_State_Type> > >::iterator x_it2;
  //   for( x_it1 = field.begin(), x_it2 = board.field.begin(); 
  // 	 x_it2 != board.field.end(); ++x_it1, ++x_it2 )
  //   {
  //     std::vector< Field > &col1 = *x_it1;
  //     std::vector< std::deque<Field_State_Type> > &col2 = *x_it2;
  //     col1.resize(col2.size());
  //   }
  // }

  // void Distance_Map::find_closest_distances( bool across_empty )
  // {
  //   std::multimap<int,Field_Pos> priority_list;
  //   red_stones.clear();
  //   white_control.clear();
  //   black_control.clear();
  //   int x,y;
  //   for( x=0; x<board.get_x_size(); ++x )
  //   {
  //     for( y=0; y<board.get_y_size(); ++y )
  //     {
  // 	Field_Pos pos(x,y);
  // 	Field &fld = field[x][y];
  // 	std::deque<Field_State_Type> &stack = board.field[x][y];
  // 	if( Board::is_stone(stack) )
  // 	{
  // 	  if( stack.back() == field_white )
  // 	    white_control.push_back(pos);
  // 	  else if ( stack.back() == field_black )
  // 	    black_control.push_back(pos);

  // 	  if( Board::includes_red_stone(stack) )
  // 	  {
  // 	    red_stones.push_back( pos );
  // 	    fld.distance = 0;
  // 	    priority_list.insert( std::pair<int,Field_Pos>(fld.distance,pos) );
  // 	  }
  // 	}
  //     }
  //   }
  //   while( !priority_list.empty() )
  //   {
  //     // remove element with highest priority (closest distance)
  //     int distance = priority_list.begin()->first;
  //     Field_Pos front = priority_list.begin()->second;
  //     priority_list.erase(priority_list.begin());
  //     Field &fld = field[front.x][front.y];
  //     if(!fld.optimal)
  //     {
  // 	fld.distance = distance;
  // 	fld.optimal = true;
  // 	// insert neighboring fields in priority list
  // 	Field_Iterator p1(front,&board);
  // 	for( int d=Field_Iterator::right; d<=Field_Iterator::bottom_right; ++d )
  // 	{
  // 	  Field_Iterator::Direction dir = Field_Iterator::Direction(d);
  // 	  Field_Iterator p2 = p1;
  // 	  p2.Go(dir);
  // 	  if( !p2.is_valid_field() ) continue;
  // 	  if( Board::is_removed(*p2) ) continue;
  // 	  if( !across_empty && Board::is_empty(*p2) ) continue;
  // 	  Field_Pos pos2 = p2.get_pos();
  // 	  Field &fld2 = field[pos2.x][pos2.y];
  // 	  if( fld2.distance == -1 || fld2.distance > distance + 1 )
  // 	  {
  // 	    assert(!fld2.optimal);
  // 	    fld2.distance = distance + 1;
  // 	    priority_list.insert(std::pair<int,Field_Pos>(fld2.distance,pos2));
  // 	  }
  // 	}
  //     }
  //   }
  // }
  
  // ----------------------------------------------------------------------------
  // AI
  // ----------------------------------------------------------------------------

  AI::AI( const Game &game )
// #warning "undo increasing max time and average time"
//     : max_time(300000), average_time(40000), num_measures(0), max_positions_expanded(150000),
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
    //player_group_command.resize(2);
    //player_group_command[0].resize(alloc_max_depth);
    //player_group_command[1].resize(alloc_max_depth);
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
	//player_group_command[0][d] = PG_RESET;
	//player_group_command[1][d] = PG_RESET;
	max_ratings[0/*min_max_vz=-1*/][d/*group_depth*/] = RATE_MAX * 2;
	max_ratings[1/*min_max_vz=1*/] [d/*group_depth*/] = -RATE_MAX * 2;
      }
      int new_min_max_vz = -1;
      int prev_min_max_vz = -1;
      bool group_depth_leaf = false;
      // bool bounded_branch = false;
      // double bounded_max_rating = - 3 * RATE_MAX;
      depth_search( save_game, current_position, /*depth=*/0, /*group_depth=*/0, /*max=*/max_ratings[new_min_max_vz>0?1:0][0],
		    new_min_max_vz, prev_min_max_vz, /*jump-only=*/false, /*debug_disable_min_max=*/false,
		    group_depth_leaf /*in-out*/
		    //bounded_branch /*in-out*/, bounded_max_rating /*in-out*/ 
		    );
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
      while( !pos->sorted_positions.empty() )
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
      prev_min_max_vz = -1;
      bool group_depth_leaf2 = false;
      // bool bounded_branch2 = false;
      // double bounded_max_rating2 = - 3 * RATE_MAX;
      depth_search( save_game2, current_position2, /*depth=*/0, /*group_depth=*/0, /*max=*/0,
		    new_min_max_vz, prev_min_max_vz, /*jump-only=*/false, /*debug_disable_min_max=*/true,
		    group_depth_leaf2 /*in-out*/
		    //bounded_branch2 /*in-out*/, bounded_max_rating2 /*in-out*/ 
		    );
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

#ifdef DEBUG_MIN_MAX_PRINT
  static std::string prefix(int depth)
  {
    std::string ret = "d" + long_to_string(depth) + ":";
    // indent 
    for( int i=0; i<depth; ++i )
    {
      ret += "  ";
    }
    return ret;
  }
#endif

  static std::list<Field_Pos> transpose( const std::list<Field_Pos> &fields )
  {
    std::list<Field_Pos> ret;
    std::list<Field_Pos>::const_iterator it;
    for( it=fields.begin(); it!=fields.end(); ++it )
      {
	Field_Pos pos = *it;
	ret.push_back( Field_Pos(pos.y,pos.x) );
      }
    return ret;
  }

  static int get_area_dir( const std::list<Field_Pos> &fields )
  {
    int area = 0;
    // sort free corners for increasing y-values
    std::multimap<int/*y*/,Field_Pos> sorted_fields;
    std::list<Field_Pos>::const_iterator it;
    for( it=fields.begin(); it!=fields.end(); ++it )
      {
	Field_Pos pos = *it;
	sorted_fields.insert( std::make_pair(pos.y,pos) );
      }
    // determine min/max x-coordinate
    int min_x=-1, max_x=-1;
    int min_x_y=0, max_x_y=0;
    for( it=fields.begin(); it!=fields.end(); ++it )
      {
	Field_Pos pos = *it;
	if( min_x<0 || pos.x < min_x ) { min_x = pos.x; min_x_y = pos.y; }
	if( max_x<0 || pos.x > max_x ) { max_x = pos.x; max_x_y = pos.y; }
      }
    int mid_x = (min_x+max_x) / 2;
    // reset min/max x-coordinate
    min_x=-1, max_x=-1;
    // determine area between mid_x and convex min_x shape / convex max_x shape
    std::multimap<int/*y*/,Field_Pos>::const_iterator it2;
    for( it2=sorted_fields.begin(); it2!=sorted_fields.end(); ++it2 )
      {
	Field_Pos pos = it2->second;
	if( pos.y <= min_x_y )
	  {
	    // decrease min_x
	    if( min_x<0 || pos.x < min_x ) { min_x = pos.x; }
	  }
	else
	  {
	    // increase min_x
	    if( min_x<0 || pos.x > min_x ) { min_x = pos.x; }
	  }
	if( pos.y <= max_x_y )
	  {
	    // increase max_x
	    if( max_x<0 || pos.x > max_x ) { max_x = pos.x; }
	  }
	else
	  {
	    // decrease max_x
	    if( max_x<0 || pos.x < max_x ) { max_x = pos.x; }
	  }
	if( mid_x >= min_x ) area += mid_x - min_x + 1;
	if( max_x > mid_x) area += max_x - mid_x;
      }
    return area;
  }

  static int get_area( const std::list<Field_Pos> &fields )
  {
    // make sure the area measure is direction independent
    return get_area_dir(fields)
      + get_area_dir( transpose(fields) );
  }
  
  double AI::rate_player( Game &game, Player &player )
  {
    double rate = 0;

    // determine number of fields placed on the board
    int num_fields = 0;
    const std::map<Field_State_Type,int> &field_counts = game.board.count_fields();
    if( does_include(field_counts,player.field_type) )
      num_fields += field_counts.find(player.field_type)->second;
    assert(num_fields >= 0);

    //TODO: FIX
    // assume that the player could move now, determine the number of possible moves
    std::list<int> save = game.AI_set_current_player(player);
    int num_player_moves = 0;
    if( game.get_current_player().id == player.id ) num_player_moves = game.get_num_possible_moves();
    game.AI_unset_current_player(save);
    assert(num_player_moves >= 0);

    // determine area spanned by free corners 
    // (TODO: only count free corners that enable a move, or that enable a move that cannot be blocked)
    std::list<Field_Pos> free_corners = game.board.get_free_corners(player);
    int area = get_area( free_corners );
    assert(area >= 0);

    // use some kind of geometric combination with fixed offsets
    rate = ::pow(double(num_fields + 5) * double(num_player_moves + 5) * double(area + 5),1./3.);

    return rate;
  }

  double AI::rate_position( Game & game, Position & position, Branch* /*branch*/ )
  {
    double rating = 0;
    // deep jumping is a feature of Zertz not Bloks AI
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
      assert( game.win_condition->get_winner(game) < 0 );
      std::vector<Player>::iterator player;
      for( player = game.players.begin(); player != game.players.end(); ++player )
	{
	  // assume all players are playing against ai_player, and greater rating is good for ai_player
	  if( player->id == ai_player->id )
	    {
	      rating += rate_player(game, *player);
	    }
	  else
	    {
	      rating -= rate_player(game, *player) / (game.players.size()-1); // for 4 players: rate / 3
	    }
	}
      ++positions_checked;
#ifndef __WXMSW__
      /** /
      // !!! DEBUG!!!
      std::cerr << "AI DEBUG:             rated: move=" << DEBUG_translate_move(branch->sequence) << " rating=" << rating << "(" 
		<< ai_player->name << "), last player: " 
		<< game.current_player->name << std::endl;
      / **/
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

    // initialize in-out parameters of depth_search()
    *inout_group_depth_leaf = false;
    // *inout_bounded_branch = false;
    // *inout_bounded_max_rating = -3 * RATE_MAX;
	
    // store recursion parameters
    Position *save_position = position;
    unsigned save_depth = depth; 
    unsigned save_group_depth = group_depth; 
    double save_max = max_val;
    int save_min_max_vz = min_max_vz; 
    int save_prev_min_max_vz = prev_min_max_vz; 
    bool save_jump_only = jump_only;
    bool save_debug_disable_min_max = debug_disable_min_max;
    bool *save_inout_group_depth_leaf = inout_group_depth_leaf;
    // bool *save_inout_bounded_branch = inout_bounded_branch;
    // double *save_inout_bounded_max_rating = inout_bounded_max_rating;

    double pos_rating = -3*RATE_MAX; // invalid rating
    int prev_player_group = (game.current_player->id == ai_player->id? 0 : 1); 
    int prev_player_index = game.prev_player_index;

#ifdef DEBUG_MIN_MAX_PRINT
    std::string prev_player_name = game.current_player->name;
#endif

    bool ret = game.choose_next_player();
    // check player group (current AI player or any opponent => pessimistic AI) 
    // attention: player_group reflects current_player after expanded move whereas min_max_vz
    // reflects current_player before move
    int player_group = (game.current_player->id == ai_player->id? 0 : 1); 
    bool group_depth_leaf = player_group != prev_player_group || depth >= cur_depth - 1;
				// whether this is a leaf within a group-subtree
				// important: min-max algorithm only acts on group_depth_leafs

#ifdef DEBUG_MIN_MAX_PRINT
    std::string current_player_name = game.current_player->name;

    std::string indent; // just for debug output
    for( unsigned i=0; i<depth; ++i ) indent += "  ";
    std::cout << "**AI-DEBUG** " << depth << ":" << group_depth << ":" << indent << ">>BRANCH VZ=" << min_max_vz 
	      << " G" << player_group << " pG" << prev_player_group << "(" << prev_player_name << ")"
	      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
#endif

    // if player group is again the same player, min/max values must be reset
//     if( player_group_command[player_group][depth] == PG_RESET )
//       {
// 	max_ratings[min_max_vz>0?1:0][depth] = -min_max_vz * RATE_MAX * 2;
// 	player_group_command[player_group][depth] = PG_KEEP;
// #ifdef DEBUG_MIN_MAX_PRINT
// 	std::cout << "**AI-DEBUG** " << depth << ":" << indent << "RESET VZ=" << min_max_vz 
// 		  << " G" << player_group << "(" << prev_player_name << ")" << " max=" 
// 		  << max_ratings[min_max_vz>0?1:0][depth] << std::endl;
// #endif
//       }
//     // if player group is still opponent, min/max values must be copied from previous depth
//     if( player_group_command[player_group][depth] == PG_COPY )
//       {
// 	max_ratings[min_max_vz>0?1:0][depth] = max_ratings[min_max_vz>0?1:0][depth];
// 	player_group_command[player_group][depth] = PG_KEEP;
// #ifdef DEBUG_MIN_MAX_PRINT
// 	std::cout << "**AI-DEBUG** " << depth << ":" << indent << "COPY VZ=" << min_max_vz 
// 		  << " G" << player_group << "(" << prev_player_name << ")" << " max=" 
// 		  << max_ratings[min_max_vz>0?1:0][depth] << std::endl;
// #endif
//       }

    // test if expanded move caused player to win:
    bool game_finished=false;
    int winner_index = game.win_condition->get_winner(game);
#ifdef DEBUG_MIN_MAX_PRINT
    int winner_id = -1;
#endif
    std::string winner_name;
    if( winner_index >= 0 )
    {
      //std::cerr << "AI DEBUG: found winner index: " << winner_index << std::endl;
      int index = 0;
      std::vector<Player>::iterator player;
      for( player = game.players.begin(); player != game.players.end(); ++player )
      {
	if( index == winner_index )
	{
	  game_finished = true;
#ifdef DEBUG_MIN_MAX_PRINT
	  winner_id = player->id;
#endif
	  winner_name = player->name;
	  break;
	}
	++index;
      }
    }

    bool new_group_depth_leaf = false;
    // bool bounded_branch = false; // will be set within recursive depth_search call
    // double bounded_max_rating = -3 * RATE_MAX; // will be set within recursive depth_search call

    if( game_finished )
    {
      float ai_player_rank = game.win_condition->get_player_rank(game,*ai_player);
      if( ai_player_rank <= 1.02 || ai_player_rank >= game.players.size()-0.02 )
      {
	// scale pos_rating between -RATE_MAX and +RATE_MAX according to rank of ai_player
	pos_rating = -((ai_player_rank - 1) * 2 * RATE_MAX / (game.players.size()-1) - RATE_MAX);
      }
      else
      {
	float rank_weight = 10; // corresponds to +3.3/-3.3 in case of rank 2/3 of a 4 player game
	// scale pos_rating between -RATE_MAX and +RATE_MAX according to rank of ai_player
	pos_rating = -((ai_player_rank - 1) * 2 * rank_weight / (game.players.size()-1) - rank_weight);
      }
      // set RATE_MAX values to exactly RATE_MAX
      if( !(pos_rating < 1.02 * RATE_MAX && pos_rating > 1.02 * -RATE_MAX) )
      {
	std::cerr << "Error: pos_rating=" << pos_rating << " rank=" << ai_player_rank
		  << " #players=" << game.players.size() << std::endl;
      }
      assert( pos_rating < 1.02 * RATE_MAX && pos_rating > 1.02 * -RATE_MAX );
      if( pos_rating > 0.98 *  RATE_MAX ) pos_rating =  RATE_MAX;
      if( pos_rating < 0.98 * -RATE_MAX ) pos_rating = -RATE_MAX;
#ifdef DEBUG_MIN_MAX_PRINT
      std::cerr << "AI DEBUG: " << prefix(depth) << "   Winner: win-id=" << winner_id << "(" << winner_name << ")" 
		<< " rating=" << pos_rating << "(" << ai_player->id << ")" << " min_max_vz=" << min_max_vz 
		<< " jump-only=" << jump_only << std::endl;
#endif
    }
    else
    {
      if(!ret)			// no player can move -> tie
      {
	pos_rating = 0;
      }
      else
      {
	if( depth < cur_depth - 1 )
	{
// 	  if( new_min_max_vz != min_max_vz ) {
// 	    // on any group change, reset min-max ratings for NEXT upcoming group change which changes back to min_max_vz
// 	    std::string inc_indent = indent + "  "; // start with inc_depth=2
// 	    for( int inc_depth=2; depth+inc_depth<alloc_max_depth; ++inc_depth )
// 	      {
// 		inc_indent += "  ";
// #ifdef DEBUG_MIN_MAX_PRINT
// 		if( fabs(max_ratings[min_max_vz>0?1:0][depth+inc_depth] - (-min_max_vz * RATE_MAX * 2)) > epsilon )
// 		  {
// 		    std::cout << "**AI-DEBUG** " << (depth+inc_depth) << ":" << inc_indent 
// 			      << "  RESET VZ=" << min_max_vz 
// 			      << " oG" << player_group << "(" << prev_player_name << ")"
// 			      << " max=" << max_ratings[min_max_vz>0?1:0][depth+inc_depth] 
// 			      << "->" << (-min_max_vz * RATE_MAX * 2)
// 			      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
// 		  }
// #endif
// 		max_ratings[min_max_vz>0?1:0][depth+inc_depth] = -min_max_vz * RATE_MAX * 2;
// 		if( depth+inc_depth >= cur_depth - 1 ) break;
// 	      }
// 	  }

	  // depth search for depth+1
	  int new_min_max_vz = (game.current_player->id == ai_player->id? -1 : 1); // assume ai_player against others

	  // reset max_ratings for the depth where prev player may choose again
	  if( group_depth_leaf )
	    {
	      if( game.prev_player->id == game.current_player->id )
		{
		  max_ratings[0/*min_max_vz=-1*/][group_depth+1] = RATE_MAX * 2;
		  max_ratings[1/*min_max_vz=1*/] [group_depth+1] = -RATE_MAX * 2;
		}
	      max_ratings[0/*min_max_vz=-1*/][group_depth+2] = RATE_MAX * 2;
	      max_ratings[1/*min_max_vz=1*/] [group_depth+2] = -RATE_MAX * 2;
	    }
	  unsigned new_group_depth = group_depth + (group_depth_leaf ? 1 : 0);

	  pos_rating = depth_search( game, (*branch)->position, depth+1, new_group_depth,
				     max_ratings[new_min_max_vz>0?1:0][group_depth+1],
				     new_min_max_vz, min_max_vz, jump_only,
				     debug_disable_min_max, new_group_depth_leaf /*in-out*/
				     //bounded_branch /*in-out*/, bounded_max_rating /*in-out*/
				     );
	  // attention: avoid using depth/min_max_vz/etc., use save_depth/... from now on

// 	  // update max_ratings if new maximum/minimum reached 
// 	  if( !bounded_branch && prev_player_group != player_group ) // update only if complete subtree of a "team" is processed
// 	    {
// 	      std::string inc_indent = indent;
// 	      for( int inc_depth=1; save_depth+inc_depth<alloc_max_depth; ++inc_depth )
// 		{
// 		  inc_indent += "  ";
// 		  if( (pos_rating - max_ratings[new_min_max_vz>0?1:0][save_depth+inc_depth]) * new_min_max_vz > 0 )
// 		    {
// 		      max_ratings[new_min_max_vz>0?1:0][save_depth+inc_depth] = pos_rating;
// #ifdef DEBUG_MIN_MAX_PRINT
// 		      std::cout << "**AI-DEBUG** " << save_depth+inc_depth << ":" << inc_indent << "NEW-MAX VZ=" << new_min_max_vz 
// 				<< " oG" << player_group << "(" << prev_player_name << ")"
// 				<< " max=" << max_ratings[new_min_max_vz>0?1:0][save_depth+inc_depth] 
// 				<< " rate=" << pos_rating << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
// #endif
// 		    }
// 		  else
// 		    {
// #ifdef DEBUG_MIN_MAX_PRINT
// 		      std::cout << "**AI-DEBUG** " << save_depth+inc_depth << ":" << inc_indent << "OLD-MAX VZ=" << new_min_max_vz 
// 				<< " oG" << player_group << "(" << prev_player_name << ")"
// 				<< " max=" << max_ratings[new_min_max_vz>0?1:0][save_depth+inc_depth] 
// 				<< " rate=" << pos_rating << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
// #endif
// 		    }
// 		  if( save_depth+inc_depth >= cur_depth - 1 ) break;
// 		}
// 	    }
	}
	else
	{
	  assert(group_depth_leaf);
	  pos_rating = rate_position( game, (*branch)->position, *branch );
	}
      }
    }
    game.choose_prev_player( prev_player_index );

    //** start DEBUG: check that all best moves are expanded up to max_depth
    // if( pos_rating < RATE_MAX * 0.8 && pos_rating > -RATE_MAX * 0.8 )
    // {
    //   Position *pos = &(*branch)->position;
    //   unsigned depth_expanded = 0;
    //   while( !pos->sorted_positions.empty() )
    //   {
    // 	pos = &(*pos->sorted_positions.begin()->second)->position;
    // 	++depth_expanded;
    //   }
    //   if( depth_expanded + depth + 1 < cur_depth )
    // 	std::cerr << "Error: Expansion failed: #expanded=" << depth_expanded << " depth=" << save_depth 
    // 		  << " cur_depth=" << cur_depth << std::endl;
    // }
    //assert(depth_expanded == cur_depth - depth - 1);
    //** end DEBUG

    // restore recursion parameters
    position = save_position;
    depth = save_depth; 
    group_depth = save_group_depth; 
    max_val = save_max;
    min_max_vz = save_min_max_vz; 
    prev_min_max_vz = save_prev_min_max_vz; 
    jump_only = save_jump_only;
    debug_disable_min_max = save_debug_disable_min_max;
    inout_group_depth_leaf = save_inout_group_depth_leaf;
    *inout_group_depth_leaf = group_depth_leaf;
    // inout_bounded_branch = save_inout_bounded_branch;
    // inout_bounded_max_rating = save_inout_bounded_max_rating;
	  
//     double orig_pos_rating = pos_rating;
//     bool continue_bound = false;
//     bool prev_unmodified = position->rating < -1.49 * RATE_MAX || position->rating > 1.49 * RATE_MAX;
//     //bool prev_not_confirmed = fabs(bounded_max_rating - position->rating) > epsilon;
//     bool prev_not_confirmed = 
//       bounded_max_rating * prev_min_max_vz - epsilon >= position->rating * prev_min_max_vz;
//     if( bounded_branch )
//       {
// 	pos_rating = 1.5 * RATE_MAX * prev_min_max_vz;
// 	// reset unconfirmed position ratings
// 	if( prev_not_confirmed /*HACK>>* /&& bounded_max_rating > -RATE_MAX-epsilon/ *<<END*/ )
// 	  {
// #ifdef DEBUG_MIN_MAX_PRINT
// 	    std::cout << "**AI-DEBUG** " << depth << ":" << indent << "**Reset rating** VZ=" << min_max_vz
// 		      << " pVZ=" << prev_min_max_vz
// 		      << " G" << player_group << " pG" << prev_player_group << "(" << prev_player_name << ")"
// 		      << " rate=" << position->rating << "->" << (1.5 * RATE_MAX * prev_min_max_vz)
// 		      << " bound-max=" << bounded_max_rating << " unmod=" << prev_unmodified
// 		      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
// #endif
// 	    position->rating = 1.5 * RATE_MAX * prev_min_max_vz;  
// 	    continue_bound = true;
// 	  }

// // 	// replace initialization value 1.5 * RATE_MAX * min_max_vz to avoid assert
// // 	// and reset unconfirmed position ratings
// // 	if( prev_unmodified || (prev_not_confirmed && /*HACK>>*/bounded_max_rating > -RATE_MAX-epsilon/*<<END*/ ) )
// // 	  {
// // #ifdef DEBUG_MIN_MAX_PRINT
// // 	    std::cout << "**AI-DEBUG** " << depth << ":" << indent << "**Reset rating** VZ=" << min_max_vz 
// // 		      << " G" << player_group << " pG" << prev_player_group << "(" << prev_player_name << ")"
// // 		      << " rate=" << position->rating << "->" << (RATE_MAX * min_max_vz)
// // 		      << " bound-max=" << bounded_max_rating << " unmod=" << prev_unmodified
// // 		      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
// // #endif
// // 	    position->rating = RATE_MAX * min_max_vz;  
// // 	  }
		
//       }
//     else 
    if( (position->rating - pos_rating) * min_max_vz > 0 )
      {
	position->rating = pos_rating;
	/** /
	// DEBUG !!!
	std::cerr << "AI DEBUG: " << prefix(depth) << "   Finished(New best rating): move=" 
		  << DEBUG_translate_move((*branch)->sequence)
		  << " rating=" << pos_rating 
		  << " min_max_vz=" << min_max_vz << " player=" << prev_player_name
		  << " jump-only=" << jump_only << std::endl;
	/ **/
      }
    else
      {
	/** /
	// DEBUG !!!
	std::cerr << "AI DEBUG: " << prefix(depth) << "   Finished: move=" << DEBUG_translate_move((*branch)->sequence) 
		  << " rating=" << pos_rating 
		  << " min_max_vz=" << min_max_vz << " player=" << prev_player_name
		  << " jump-only=" << jump_only << std::endl;
	/ **/
      }
    
    position->sorted_positions.insert
      ( Position::sorted_positions_element_type( pos_rating * min_max_vz, branch ) );


    if( group_depth_leaf && !debug_disable_min_max )
      {
	// bound tree branches according to min-max-algorithm
	//assert( max_val == max_ratings[depth] );
	if( depth == 0 )
	  assert( (max_val == RATE_MAX * 2) && (min_max_vz == -1) );
	assert( (pos_rating + epsilon >= -RATE_MAX) && (pos_rating - epsilon <= RATE_MAX) );
	
	if( (max_val - pos_rating) * min_max_vz >= 0 )
	  {
	    // *inout_bounded_branch = true;
	    // *inout_bounded_max_rating = (bounded_branch&&continue_bound)?bounded_max_rating:max_val;
	    // if(prev_player_group != player_group)
	    //   *inout_bounded_max_rating = -3 * RATE_MAX; // HACK: ensure prev_not_confirmed=false for caller
#ifdef DEBUG_MIN_MAX_PRINT
	    std::cout << "**AI-DEBUG** " << depth << ":" << group_depth << ":" << indent << "--BOUND VZ=" << min_max_vz 
		      << " G" << player_group << " pG" << prev_player_group << "(" << prev_player_name << ")"
		      << " max=" << max_ratings[min_max_vz>0?1:0][group_depth] 
		      << " rate=" << pos_rating << "(" /*<< orig_pos_rating << "/"*/ << position->rating << ")" 
	      //<< " unmod=" << prev_unmodified
	      //<< " bounded=" << bounded_branch << "(" << bounded_max_rating << ")"
		      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
#endif
	    assert( depth != 0 );
	    return false;
	  }
      }

#ifdef DEBUG_MIN_MAX_PRINT
    std::cout << "**AI-DEBUG** " << depth << ":" << group_depth << ":" << indent << "++BRANCH VZ=" << min_max_vz 
	      << " G" << player_group << " pG" << prev_player_group << "(" << prev_player_name << ")"
	      << " max=" << max_ratings[min_max_vz>0?1:0][group_depth] 
	      << " rate=" << pos_rating << "(" /*<< orig_pos_rating << "/"*/ << position->rating << ")" 
      //<< " unmod=" << prev_unmodified
      //<< " bounded=" << bounded_branch << "(" << bounded_max_rating << ")"
	      << " " << DEBUG_translate_move((*branch)->sequence) << std::endl;
#endif

    if( should_stop(false) )
      return false;

    return true;
  }

  double AI::depth_search( Game &game, Position &position, unsigned depth, unsigned group_depth, double max_val, 
			   int min_max_vz, int prev_min_max_vz, 
			   bool jump_only/*not used*/, bool debug_disable_min_max,
			   bool &group_depth_leaf /*in-out*/
			   //bool &bounded_branch /*in-out*/, double &bounded_max_rating /*in-out*/ 
			   )
  {
    // save recursive parameters for expanded()
    this->position = &position;
    this->depth = depth;
    this->group_depth = group_depth;
    this->max_val = max_val;
    this->min_max_vz = min_max_vz;
    this->prev_min_max_vz = prev_min_max_vz;
    this->jump_only = jump_only;
    this->debug_disable_min_max = debug_disable_min_max;
    this->inout_group_depth_leaf = &group_depth_leaf; /*in-out*/
    //this->inout_bounded_branch = &bounded_branch; /*in-out*/
    //this->inout_bounded_max_rating = &bounded_max_rating; /*in-out*/

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
    if( group_depth_leaf )
      {
	if( (position.rating - max_ratings[min_max_vz>0?1:0][group_depth]) * min_max_vz > 0 )
	  max_ratings[min_max_vz>0?1:0][group_depth] = position.rating;
      }

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

  IMPLEMENT_DYNAMIC_CLASS( bloks::AI_Event, wxEvent ) //**/

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

    Connect( -1, wxEVT_BLOKS_NOTIFY, 
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

