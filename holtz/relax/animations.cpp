/*
 * animations.cpp
 * 
 * GUI implementation
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
#include "animations.hpp"

#include "wxmain.hpp"
#include <stdlib.h>

namespace relax
{
  inline int min( int a, int b )
  {
    return a < b ? a : b;
  }

  inline int max( int a, int b )
  {
    return a > b ? a : b;
  }

  inline int abs( int a )
  {
    return a < 0 ? -a : a;
  }

  Bitmap_Move_Animation::Bitmap_Move_Animation( Game_Window &game_window )
    : game_window(game_window), occupied(false)
  {
    background_dc[0] = new wxMemoryDC();
    background_dc[1] = new wxMemoryDC();
    background[0] = 0;
    background[1] = 0;
  }
  Bitmap_Move_Animation::~Bitmap_Move_Animation()
  {
    delete background_dc[0];
    delete background_dc[1];
    if( background[0] ) delete background[0];
    if( background[1] ) delete background[1];
  }

  bool Bitmap_Move_Animation::move( wxBitmap _bitmap, wxPoint _from, wxPoint _to, 
				    unsigned _steps, unsigned _step_rate,
				    wxEvtHandler *_done_handler, 
				    int _event_id, int _abort_id )
  {
    if( occupied )
      return false;
    occupied = true;

    // either from or to should be different from (-1,-1)
    if( _from == wxPoint(-1,-1) )
    {
      int x,y;
      game_window.GetVirtualSize(&x,&y);
      _from.x = x;
      _from.y = _to.y;
    }
    if( _to == wxPoint(-1,-1) )
    {
      int x,y;
      game_window.GetVirtualSize(&x,&y);
      _to.x = x;
      _to.y = _from.y;
    }

    bitmap = _bitmap;
    from = _from;
    to = _to;
    steps = _steps;
    step_rate = _step_rate;
    done_handler = _done_handler;
    event_id = _event_id;
    abort_id = _abort_id;
    current_step = 0;

    int diff_x = abs( from.x - to.x ) / (steps-1) + 1;
    int diff_y = abs( from.y - to.y ) / (steps-1) + 1;
    int width  = diff_x + bitmap.GetWidth();
    int height = diff_y + bitmap.GetHeight();

    if( background[0] ) delete background[0];
    if( background[1] ) delete background[1];
    background[0] = new wxBitmap( width, height );
    background[1] = new wxBitmap( width, height );
    background_dc[0]->SelectObject( *background[0] );
    background_dc[1]->SelectObject( *background[1] );
    //background_dc[0]->SelectObject( wxBitmap( width, height ) );
    //background_dc[1]->SelectObject( wxBitmap( width, height ) );
    old_bg = 0;

    return step();
  }

  // abort a running animation
  void Bitmap_Move_Animation::abort()
  {
    Stop();
    if(occupied)
    {
      occupied = false;
    
      if( done_handler )
      {
	wxCommandEvent event( wxEVT_RELAX_NOTIFY, abort_id );
	done_handler->ProcessEvent( event );
      }
    }
  }

  bool Bitmap_Move_Animation::step()
  {
    wxDC *dc = game_window.get_client_dc();
    //dc->BeginDrawing(); depricated

    int width  = bitmap.GetWidth();
    int height = bitmap.GetHeight();
    // calc new position
    if( current_step == 0 )
    {
      pos_x = (to.x*current_step + from.x*(steps - current_step - 1)) / (steps - 1);
      pos_y = (to.y*current_step + from.y*(steps - current_step - 1)) / (steps - 1);
    }

    // calc future position
    int future_step = current_step + 1;
    int new_x = (to.x*future_step + from.x*(steps - future_step - 1)) / (steps - 1);
    int new_y = (to.y*future_step + from.y*(steps - future_step - 1)) / (steps - 1);

    // calc new background position
    int bg = old_bg ^ 1;
    int bg_x = min( pos_x, new_x );
    int bg_y = min( pos_y, new_y );
    bg_width[bg]  = abs( pos_x - new_x ) + width;
    bg_height[bg] = abs( pos_y - new_y ) + height;

    // store new background
    if( current_step == 0 )
      background_dc[bg]->Blit( 0, 0, bg_width[bg], bg_height[bg], dc, bg_x, bg_y );
    else
      //if( current_step > 0 )
    {
      if( pos_x == bg_x )
	background_dc[bg]->Blit( width, 0, bg_width[bg] - width, bg_height[bg], 
				 dc, bg_x + width, bg_y );
      else
	background_dc[bg]->Blit( 0, 0, bg_width[bg] - width, bg_height[bg], dc, bg_x, bg_y );
      
      if( pos_y == bg_y )
	background_dc[bg]->Blit( 0, height, bg_width[bg], bg_height[bg] - height, 
				 dc, bg_x, bg_y + height);
      else
	background_dc[bg]->Blit( 0, 0, bg_width[bg], bg_height[bg] - height, dc, bg_x, bg_y );

      // assert that position of new bitmap is within both backgrounds
      assert( (pos_x >= bg_x) && (pos_y >= bg_y) );
      assert( (pos_x >= old_bg_x) && (pos_y >= old_bg_y) );
      // restore overlapping area from old background
      background_dc[bg]->Blit( pos_x - bg_x, pos_y - bg_y, width, height,
			       background_dc[old_bg], pos_x - old_bg_x, pos_y - old_bg_y );
    }

    if( current_step > 0 )
    {
      // draw bitmap on background
      background_dc[old_bg]->DrawBitmap( bitmap, pos_x - old_bg_x, pos_y - old_bg_y, true );
      // put bitmap + background on screen
      int add_x = 0, add_y = 0;
      if( old_bg_x < 0 ) add_x = -old_bg_x;
      if( old_bg_y < 0 ) add_y = -old_bg_y;
      dc->Blit( old_bg_x + add_x, old_bg_y + add_y, 
	       bg_width[old_bg] - add_x, bg_height[old_bg] - add_y, 
	       background_dc[old_bg], add_x, add_y );
    }
    else
    {
      // draw bitmap on background
      dc->DrawBitmap( bitmap, pos_x, pos_y, true );
    }


    //dc->EndDrawing(); depricated
    old_bg_x = bg_x; old_bg_y = bg_y;
    old_x = pos_x; old_y = pos_y;
    pos_x = new_x; pos_y = new_y;
    old_bg = bg;
    ++current_step;

    if( current_step < steps )
    {
      if( !Start( 1000 / step_rate, true ) )
      {
	finish();
	return false;
      }
      /*
      if( !IsRunning() )
      {
	finish();
	return false;
      }
      */
    }
    else
      finish();

    delete dc;
    return true;
  }

  void Bitmap_Move_Animation::finish()
  {
    occupied = false;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_RELAX_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  void Bitmap_Move_Animation::Notify()
  {
    if(!occupied) 
      Stop();
    else
      step();
  }

  Move_Sequence_Animation::Move_Sequence_Animation( WX_GUI_Manager &gui_manager, 
						    Game_Window &game_window )
    : gui_manager(gui_manager), game_window(game_window), bitmap_move_animation( game_window ),
      done_handler(0), state(finished)
  {
    // connect event functions
    Connect( ANIMATION_DONE, wxEVT_RELAX_NOTIFY, 
	     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) 
	     &Move_Sequence_Animation::on_done );
    Connect( ANIMATION_ABORTED, wxEVT_RELAX_NOTIFY, 
	     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) 
	     &Move_Sequence_Animation::on_aborted );
  }

  Move_Sequence_Animation::~Move_Sequence_Animation()
  {
  }
    
  bool Move_Sequence_Animation::start( Move_Sequence _sequence, Game &_game, 
				       wxEvtHandler *_done_handler, 
				       int _event_id, int _abort_id )
  {
    //!!! todo: implement correct animations
    return false;

    if( state != finished )
      return false;

    // check sequence
    if( !_sequence.check_sequence(_game) )
      return false;

    if( !_sequence.get_moves().size() ) 
      return false;

    sequence = _sequence;
    game = &_game;
    done_handler = _done_handler;
    event_id = _event_id;
    abort_id = _abort_id;
    current_move = sequence.get_moves().begin();
    state = begin;

    current_player = game->current_player;
    undo = false;

    return step();
  }

  bool Move_Sequence_Animation::start_undo( Move_Sequence _sequence, Game &_game, 
					    wxEvtHandler *_done_handler, 
					    int _event_id, int _abort_id )
  {
    //!!! todo: implement correct animations
    return false;

    if( state != finished )
      return false;

    if( !_sequence.get_moves().size() ) 
      return false;

    if( _game.variant_tree.is_first() )	// if there is no move already done in game
      return false;

    sequence = _sequence;
    game = &_game;
    done_handler = _done_handler;
    event_id = _event_id;
    abort_id = _abort_id;
    current_undo_move = sequence.get_moves().rbegin();
    state = begin;

    current_player 
      = game->get_player(game->variant_tree.get_current_variant()->current_player_index); 
				// get player of last move
    undo = true;
    
    return step_undo();
  }

  void Move_Sequence_Animation::abort()
  {
    bitmap_move_animation.abort();    
  }

  // perform animation step
  // - before initiating a new animation: refresh display 
  bool Move_Sequence_Animation::step()
  {
    bool ret = true;
    switch( state )
    {
      case begin:
      {
	assert( current_move != sequence.get_moves().end() );
	switch( (*current_move)->get_type() )
	{
	  case Move::set_move:
	  {
	    // ** initialize variables
	    Set_Move *set_move = dynamic_cast<Set_Move*>(*current_move);
	    assert( set_move );
	    //Stones::Stone_Type stone = set_move->stone_type;

	    // ** calculate animation positions and modify board
	    std::pair<int,int> _from;
	    // get position of last stone of the right type
	    int col = game->common_stones.stone_count[set_move->stone_type] - 1;
	    assert(col >= 0);
	    _from = gui_manager.get_game_panel().get_stone_panel()
	      .get_field_pos( col, set_move->stone_type );
	    // remove stone from reservoir
	    --game->common_stones.stone_count[set_move->stone_type];

	    wxPoint from( _from.first, _from.second );
	    std::pair<int,int> _pos; /*= 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
	      set_move->pos.y );*/
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh();
	    /*
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().
		field_bitmaps[Field_State_Type(stone)],
		from, pos, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
	    */
	  }
	  break;
	  case Move::finish_move:
	  {
	    // ** initialize variables
	    Finish_Move *finish_move = dynamic_cast<Finish_Move*>(*current_move);
	    assert( finish_move );

	    // move has to be done once to calculate removed_stones
	    Game save_game(*game);
	    finish_move->do_move(save_game);

	    // ** change state
	    state = finish_sequence;
	    
	    // ** recursive run
	    ret = step();
	  }
	  break;
	  case Move::no_move:
	    ret = false;
	    break;
	  case Move::select_move:
	    break;
	}
      }
      break;
      case setting_stone:
      {
	// ** initialize variables
	Set_Move *set_move = dynamic_cast<Set_Move*>(*current_move);
	assert( set_move );
	Field_State_Type stone = Field_State_Type(set_move->stone_type);
	assert( Board::is_stone(stone) );

	// ** modify board state
	Mutable_Field_Iterator to_field  ( set_move->pos, &game->current_player->board );
	*to_field = stone;

	// ** begin new step
	++current_move;
	state = begin;
	ret = step();		// next step at once
      }
      break;
      case finish_sequence:
      {
	if( current_removed_stone == end_removed_stones )
	{
	  game->choose_next_player();

	  finish();
	  return true;
	}
	else
	{
	  // ** initialize variables
	  Field_Pos &field_pos = current_removed_stone->first;
	  Stones::Stone_Type &stone = current_removed_stone->second;

	  // ** modify board state
	  Mutable_Field_Iterator field( field_pos, &game->current_player->board );
	  *field = field_removed;

	  // ** calculate animation positions
	  std::pair<int,int> _pos ;/*= 
				     gui_manager.get_game_panel().get_board_panel().get_field_pos( field_pos.x, field_pos.y );*/
	  wxPoint pos( _pos.first, _pos.second );

	  std::pair<int,int> _to;
	  const Player_Panel *panel = 
	    gui_manager.get_game_panel().get_player_panel( current_player->id );
	  int col = current_player->stones.stone_count[stone];
	  assert(col >= 0);
	  _to = panel->get_stone_panel().get_field_pos( col, stone );
	  wxPoint to( _to.first, _to.second );

	  // ** change state
	  state = finish_removes;

	  // ** refresh display and start animation
	  game_window.refresh(); 
	  /*
	  ret = bitmap_move_animation.move
	    ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().
	      field_bitmaps[Field_State_Type(stone)], 
	      pos, to, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
	  */
	}
      }
      break;
      case finish_removes:
      {
	// ** modify board state
	Stones::Stone_Type &stone = current_removed_stone->second;
	++current_player->stones.stone_count[stone];

	// ** begin with next removed stone
	state = finish_sequence;
	++current_removed_stone;
	ret = step();
      }
      break;
      case finished:
	break;
      case knock_out_jump:
      case knock_out_collects:
      case removing:
	break;
    }

    return ret;
  }

  // perform animation step for undo animation
  // - before initiating a new animation: refresh display 
  bool Move_Sequence_Animation::step_undo()
  {
    bool ret = true;
    switch( state )
    {
      case begin:
      {
	if( current_undo_move == sequence.get_moves().rend() )
	{
	  // change variant tree and players 
	  game->variant_tree.move_a_variant_back();
	  if( !game->variant_tree.is_first() )
	  {
	    game->choose_prev_player
	      ( game->variant_tree.get_current_variant()->current_player_index );
	  }
	  else
	    game->choose_prev_player( -1 );

	  finish();
	  return true;
	}

	switch( (*current_undo_move)->get_type() )
	{
	  case Move::set_move:
	  {
	    // ** initialize variables
	    Set_Move *set_move = dynamic_cast<Set_Move*>(*current_undo_move);
	    assert( set_move );
	    //Stones::Stone_Type stone = set_move->stone_type;

	    // ** modify board state
	    Mutable_Field_Iterator field  ( set_move->pos, &game->current_player->board );
	    *field = field_empty;

	    // ** calculate animation positions
	    std::pair<int,int> _from;
	    // get position of last stone of the right type
	    int col = game->common_stones.stone_count[set_move->stone_type];
	    assert(col >= 0);
	    _from = gui_manager.get_game_panel().get_stone_panel()
	      .get_field_pos( col, set_move->stone_type );
	    stones = &game->common_stones;

	    wxPoint from( _from.first, _from.second );

	    std::pair<int,int> _pos /*= 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
	      set_move->pos.y )*/;
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh(); // remove picked stone from screen
	    /*
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().
		field_bitmaps[Field_State_Type(stone)],
		pos, from, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
	    */
				// move in direction: <pos> => <from>
	  }
	  break;
	  case Move::finish_move:
	  {
	    // ** initialize variables
	    Finish_Move *finish_move = dynamic_cast<Finish_Move*>(*current_undo_move);
	    assert( finish_move );

	    // ** change state
	    state = finish_sequence;

	    // ** recursive run
	    ret = step_undo();	
	  }
	  break;
	  case Move::no_move:
	  {
	    // ** begin new step
	    ++current_undo_move;
	    state = begin;
	    ret = step_undo();
	  }
	  break;
	  case Move::select_move:
	  break;
	}
      }
      break;
      case setting_stone:
      {
	// ** initialize variables
	Set_Move *set_move = dynamic_cast<Set_Move*>(*current_undo_move);
	assert( set_move );
	Field_State_Type stone = Field_State_Type(set_move->stone_type);
	assert( Board::is_stone(stone) );

	// ** modify board state
	Mutable_Field_Iterator to_field  ( set_move->pos, &game->current_player->board );
	*to_field = field_empty;

	// readd stone to player or common stones
	++stones->stone_count[set_move->stone_type];

	// ** begin new step
	++current_undo_move;
	state = begin;
	ret = step_undo();
      }
      break;
      case finish_sequence:
      {
	if( current_removed_stone == end_removed_stones )
	{
	  // ** begin new step
	  ++current_undo_move;
	  state = begin;
	  ret = step_undo();
	}
	else
	{
	  // ** initialize variables
	  //Field_Pos &field_pos = current_removed_stone->first;
	  Stones::Stone_Type &stone = current_removed_stone->second;

	  // ** modify board state
	  --current_player->stones.stone_count[stone]; // remove stone from player

	  // ** calculate animation positions
	  std::pair<int,int> _pos /*= 
				    gui_manager.get_game_panel().get_board_panel().get_field_pos( field_pos.x, field_pos.y )*/;
	  wxPoint pos( _pos.first, _pos.second );

	  std::pair<int,int> _to;
	  const Player_Panel *panel = 
	    gui_manager.get_game_panel().get_player_panel( current_player->id );
	  int col = current_player->stones.stone_count[stone];
	  assert(col >= 0);
	  _to = panel->get_stone_panel().get_field_pos( col, stone );
	  wxPoint to( _to.first, _to.second );

	  // ** change state
	  state = finish_removes;

	  // ** refresh display and start animation
	  game_window.refresh(); 
	  /*
	  ret = bitmap_move_animation.move
	    ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().
	      field_bitmaps[Field_State_Type(stone)], 
	      to, pos, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
	  */
				// move from <to> of player => <pos> on board
	}
      }
      break;
      case finish_removes:
      {
	// ** initialize variables
	Field_Pos &field_pos = current_removed_stone->first;
	Stones::Stone_Type &stone = current_removed_stone->second;

	// ** modify board state
	Mutable_Field_Iterator field( field_pos, &game->current_player->board );
	*field = Field_State_Type(stone); // place removed stone

	// ** begin next finish animation
	state = finish_sequence;
	++current_removed_stone;
	ret = step_undo();
      }
      break;
      case finished:
	break;
      case knock_out_jump:
      case knock_out_collects:
      case removing:
	break;
    }

    return ret;
  }

  // current sub-animation done
  void Move_Sequence_Animation::on_done( wxCommandEvent & )
  {
    std::cout << "animation step: done " << std::endl;
    if( !undo )
      step();
    else
      step_undo();
  }

  // current sub-animation aborted
  void Move_Sequence_Animation::on_aborted( wxCommandEvent & )
  {
    std::cout << "animation: aborted " << std::endl;
    state = finished;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_RELAX_NOTIFY, abort_id );
      done_handler->ProcessEvent( event );
    }
  }

  void Move_Sequence_Animation::finish()
  {
    std::cout << "animation: finished " << std::endl;
    state = finished;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_RELAX_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  //BEGIN_EVENT_TABLE(Move_Sequence_Animation, wxEvtHandler)				
  //  EVT_TIMER(ANIMATION_DONE, Move_Sequence_Animation::on_done)	//**/
  //  EVT_TIMER(ANIMATION_ABORTED, Move_Sequence_Animation::on_aborted)	//**/
  //END_EVENT_TABLE()						//**/
}
