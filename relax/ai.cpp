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
#include "wxrelax.hpp"
#include "animations.hpp"

#include <assert.h>

#define RATE_MAX	1e13

namespace relax
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

  bool Position::add_set_moves( Game &game, Field_Iterator to, Stones::Stone_Type type,
				Field_Permutation & /*field_permutation*/, bool /*expand_first*/ )
  {
    bool ret = true;
    Move_Sequence sequence;

    Move *move = new Set_Move( to.get_pos(), type );
    assert( move->check_move( game ) );
    bool ok = sequence.add_move( game, move );
    assert( ok );
    move->do_move( game );

    move = new Finish_Move();
    assert( move->check_move( game ) );
    ok = sequence.add_move( game, move );
    assert( ok );
    move->do_move( game );

    bool knock_out_possible = false;//game.board.is_knock_out_possible();
    ret = add_branch( game, new Branch( sequence.clone(), Position(knock_out_possible) ) );

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

    Field_Pos pos;
    Field_Iterator p1(&game.get_current_player().board);

    bool expand_first = false;
    field_permutation.new_context();
    for( pos = field_permutation.get_first(); !field_permutation.is_end(); 
	 pos = field_permutation.get_next() )
      {
	p1.set_pos( pos );
	if( *p1 == field_empty )
	  {
	    go_on = add_set_moves( game, p1, game.common_stones.current_stone, 
				   field_permutation, expand_first );
	  }
	if( !go_on ) 
	  break;
      }
    field_permutation.restore_context();

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
      max_depth(12), min_depth(2), field_permutation( game.get_current_player().board )
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

    //while( stop_watch.Time() == 0 ); // check for stop watch to be started
    aborted = false;
    don_t_abort = true;
    
    // determine number of empty fields after current set-move
    assert(save_game.get_current_player().board.get_empty_fields().size() > 0 );
    unsigned num_empty_fields = save_game.get_current_player().board.get_empty_fields().size()-1;
    // determine sets of available stones
    unsigned num_try_variants;
    unsigned choose_variant;
    // This table configures how optimistic the strategy is for future stone selection.
    // It is a wild guess and not systematically optimized
    if( num_empty_fields > 30 )
      {
	num_try_variants = 2;
	choose_variant = 0;
      }
    else if( num_empty_fields > 16 )
      {
	num_try_variants = 5;
	choose_variant = 1;
      }
    else if( num_empty_fields > 10 )
      {
	num_try_variants = 15;
	choose_variant = 10;
      }
    else if( num_empty_fields > 5 )
      {
	num_try_variants = 25;
	choose_variant = 10;
      }
    else
      {
	num_try_variants = 25;
	choose_variant = 5;
      }
    std::vector<std::vector<std::map<int/*num*/, unsigned /*stones*/> > > stones_availables;
    stones_availables.resize(num_try_variants);
    for( unsigned i=0; i<num_try_variants; ++i )
      {
	std::vector<std::map<int/*num*/, unsigned /*stones*/> > &stones_available 
	  = stones_availables[i];
	stones_available.resize(3);
	// determine random set of stones
	Game save_save_game = save_game;
	for( unsigned j=0; j<num_empty_fields; ++j )
	  {
	    Move_Sequence sequence = save_save_game.initialize_round();
	    assert(sequence.get_moves().size() > 1);
	    assert(sequence.get_moves().front()->get_type() == Move::select_move);
	    const Select_Move *move = 
	      dynamic_cast<const Select_Move*>(sequence.get_moves().front());
	    assert(move);
	    int num0 = save_game.ruleset->get_numbers(move->stone_type).get<0>();
	    int num1 = save_game.ruleset->get_numbers(move->stone_type).get<1>();
	    int num2 = save_game.ruleset->get_numbers(move->stone_type).get<2>();
	    if( !does_include(stones_available[0],num0) ) stones_available[0][num0] = 0;
	    if( !does_include(stones_available[1],num1) ) stones_available[1][num1] = 0;
	    if( !does_include(stones_available[2],num2) ) stones_available[2][num2] = 0;
	    ++stones_available[0][num0];
	    ++stones_available[1][num1];
	    ++stones_available[2][num2];
	    save_save_game.set_selecting_phase();  // first player does select moves
	    sequence.do_sequence(save_save_game);
	  }
      }

    std::cout << "AI Thread: Testmove Scores" << std::endl;
    int max_score=-1;
    Branch *best_branch=0;
    current_position.calc_following_positions(save_game,field_permutation);
    std::list< Branch* >::iterator it;
    for( it = current_position.following_positions.begin(); 
	 it != current_position.following_positions.end(); ++it )
      {
	Branch* branch = *it;
	(branch)->sequence.do_sequence(save_game);

	std::multiset<int> scores;
	for( unsigned i=0; i<num_try_variants; ++i )
	  {
	    std::vector<std::map<int/*num*/, unsigned /*stones*/> > stones_available
	      = stones_availables[i];
	    // get maxscore based on stones_available
	    int cur_score = save_game.get_max_score(0/*current_player*/,&stones_available);
	    scores.insert(cur_score);
	  }
	
	// determine score
	assert(scores.size()==num_try_variants);
	assert(num_try_variants > choose_variant);
	std::multiset<int>::iterator score_it = scores.begin();
	for( unsigned j=0; j < choose_variant && score_it != scores.end(); ++j )
	  ++score_it;
	int score = *score_it;  // choose a low quantile of all scores

	// debug output
	assert(branch->sequence.get_moves().size() > 1);
	assert(branch->sequence.get_moves().front()->get_type() == Move::set_move);
	const Set_Move *move = 
	  dynamic_cast<const Set_Move*>(branch->sequence.get_moves().front());
	std::cout << "  Move: " << move->pos.x << "/" << move->pos.y
		  << " Scores: " << *scores.begin() << "/" << score << "/" << *scores.rbegin() << std::endl;

	(branch)->sequence.undo_sequence(save_game);

	if( score > max_score || best_branch==0 )
	  {
	    max_score = score;
	    best_branch = branch;
	    // report current hint
	    result.sequence = best_branch->sequence;
	    result.rating = score;
	    result.depth = 1;
	    result.valid = true;
	    report_current_hint(result);
	  }

// 	if( should_stop(false) )
// 	  stop = true;

// 	if( stop )
// 	  {
// 	    ++it;
// 	    break;
// 	  }
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

  bool AI::should_stop( bool depth_finished )		// determines whether AI should stop now
  {
    if( don_t_abort )
      return false;		// always calculate depth 2

    if( aborted )
      return true;

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

  AI_Thread::AI_Thread( int ID, wxEvtHandler *handler, const Game &game, 
			long last_average_time, int _num_measures, 
			bool give_hints_only )
    : AI(game), ID(ID), handler(handler), game(game), give_hints_only(give_hints_only)
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
      AI_Result result = get_move(game);
      Sleep(50);      // workaround for timing problem
      AI_Event event( result, EVT_AI_REPORT_MOVE, this ); 
      if( TestDestroy() ) return 0;
      handler->AddPendingEvent( event );
      Sleep(200);			// assure that report move event is received earlier than finished event
    }

    AI_Event event( EVT_AI_FINISHED, this ); 
    if( TestDestroy() ) return 0;
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

  IMPLEMENT_DYNAMIC_CLASS( relax::AI_Event, wxEvent ) //**/

  AI_Event::AI_Event()
    : wxEvent(-1), thread(0), thread_ID(-1)
  {
    SetEventType(EVT_AI_REPORT_MOVE);
  }

  AI_Event::AI_Event( WXTYPE type, AI_Thread *thread )
    : wxEvent(-1), thread(thread)
  {
    thread_ID = thread->get_ID();
    SetEventType(type);
  }

  AI_Event::AI_Event( AI_Result ai_result, WXTYPE type, AI_Thread *thread )
    : wxEvent(-1), ai_result(ai_result), thread(thread)
  {
    thread_ID = thread->get_ID();
    SetEventType(type);
  }

  // ----------------------------------------------------------------------------
  // AI_Input
  // ----------------------------------------------------------------------------

  int AI_Input::unique_ID_counter = 0;

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

    Connect( -1, wxEVT_RELAX_NOTIFY, 
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
      thread = new AI_Thread( next_ID(), this, game, game.get_current_player().average_time, 
			      game.get_current_player().num_measures );
      thread_ID = thread->get_ID();
      std::cout << "AI: Create new thread " << thread << std::endl;
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
    std::cout << "AI: Create new hint thread" << std::endl;
    thread_active = true;
    thread = new AI_Thread( next_ID(), this, game, -1, 0, true );
    thread_ID = thread->get_ID();
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
    thread_ID = -1;
  }

  void AI_Input::destroy_ai()
  {
    abort();
    //!! intentional memory leak since waiting until thread is actually destroyed is not that easy
    //wxPostDelete(this); // initiate deletion after thread was deleted
  }

  void AI_Input::on_report_move( AI_Event &event )
  {
    std::cout << "AI: report move " << event.thread << std::endl;
    if( thread_active && (event.thread_ID == thread_ID) )
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
    std::cout << "AI: report hint " << event.thread << std::endl;
    if( thread_active && (event.thread_ID == thread_ID) && give_hints && ui_manager )
    {
      ui_manager->give_hint( event.ai_result );
    }
  }

  void AI_Input::on_finished( AI_Event &event )
  {
    std::cout << "AI: finished " << event.thread << std::endl;
    if( thread_active && (event.thread_ID == thread_ID) )
    {
      std::cout << "AI: aborting thread" << std::endl;
      abort();
    }
  }

  void AI_Input::on_animation_done( wxCommandEvent & )
  {
    move_done = true;
    game_manager.continue_game();
  }
  
}

