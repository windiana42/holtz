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

#define MY_WX_MAKING_DLL // for WX MSW using mingw cross-compile
#include "animations.hpp"

#include "wxmain.hpp"
#include <stdlib.h>

namespace bloks
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
	wxCommandEvent event( wxEVT_BLOKS_NOTIFY, abort_id );
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
      wxCommandEvent event( wxEVT_BLOKS_NOTIFY, event_id );
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
    Connect( ANIMATION_DONE, wxEVT_BLOKS_NOTIFY, 
	     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) 
	     &Move_Sequence_Animation::on_done );
    Connect( ANIMATION_ABORTED, wxEVT_BLOKS_NOTIFY, 
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

  wxBitmap generate_stone_bitmap(const wxBitmap &field_bitmap, const Stone_Type &stone_type, Field_Iterator::Direction dir)
  {
    unsigned max_x = stone_type.get_max_x(dir);
    unsigned max_y = stone_type.get_max_y(dir);
    wxBitmap bitmap(field_bitmap.GetWidth()*(max_x+1),field_bitmap.GetHeight()*(max_y+1));
    wxMemoryDC memory_dc;
    memory_dc.SelectObject(bitmap);
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields = stone_type.get_sub_fields(dir);
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned x = it->first;
      unsigned y = it->second;
      memory_dc.DrawBitmap(field_bitmap, field_bitmap.GetWidth()*x, field_bitmap.GetHeight()*y, true);
    }
    wxImage image = bitmap.ConvertToImage();
    image.SetMaskColour(0,0,0);
    return wxBitmap(image);
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
	    int stone_type_ID = set_move->stone_type_ID;
	    const Stone_Type &stone_type = game->board.get_stone_type(stone_type_ID);

	    // ** calculate animation positions and modify board
	    std::pair<int,int> _from;
	    const Player_Panel *panel = 
	      gui_manager.get_game_panel().get_player_panel( current_player->id );
	    int col = current_player->stones.get_stone_count(set_move->stone_type_ID) - 1;
	    assert(col >= 0);
	    _from = panel->get_stone_panel().get_field_pos( col, set_move->stone_type_ID, Stone_Panel::ORIGIN );

	    // remove stone from player
	    current_player->stones.dec_stone_count(set_move->stone_type_ID);

	    wxPoint from( _from.first, _from.second );
	    std::pair<int,int> _pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
									    set_move->pos.y );
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh();
	    wxBitmap &field_bitmap 
	      = gui_manager.get_game_panel().get_board_panel().get_bitmap_set()
	      .field_bitmaps[current_player->field_type];
	    // image = image.Rotate(45,wxPoint(field_bitmap.GetWidth(),field_bitmap.GetHeight()));
	    ret = bitmap_move_animation.move
	      ( generate_stone_bitmap(field_bitmap,stone_type,set_move->dir), 
		from, pos, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
	  }
	  break;
	  case Move::finish_move:
	  case Move::no_move:
	    ret = false;
	    break;
	}
      }
      break;
      case setting_stone:
      {
	// ** initialize variables
	Set_Move *set_move = dynamic_cast<Set_Move*>(*current_move);
	assert( set_move );

	// ** modify board state
	current_player->stones.inc_stone_count(set_move->stone_type_ID); // make sure check succeeds 
	if(set_move->check_move(*game)) 
	{
	  set_move->do_move(*game);
	}
	else
	{
	  std::string reason = to_string(set_move->get_check_fail_reason());	  
	  std::cout << "Animation::step: Error: failed to play move due to: " << reason << std::endl;
	}

	// ** no new step
	++current_move;
	state = finished;

	// ** sequence finished
	game->choose_next_player();
	finish();
	return true;
      }
      break;
      case finished:
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
	    int stone_type_ID = set_move->stone_type_ID;
	    const Stone_Type &stone_type = game->board.get_stone_type(stone_type_ID);

	    // ** modify board state
	    set_move->undo_move(*game,/*no_update_player=*/true); 

	    // ** calculate animation positions
	    std::pair<int,int> _from;
	    const Player_Panel *panel = 
	      gui_manager.get_game_panel().get_player_panel( current_player->id );
	    int col = current_player->stones.get_stone_count(set_move->stone_type_ID);
	    assert(col >= 0);
	    _from = panel->get_stone_panel().get_field_pos( col, set_move->stone_type_ID, Stone_Panel::ORIGIN );

	    stones = &current_player->stones;
	    wxPoint from( _from.first, _from.second );

	    std::pair<int,int> _pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
									    set_move->pos.y );
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh(); // remove picked stone from screen
	    wxBitmap &field_bitmap 
	      = gui_manager.get_game_panel().get_board_panel().get_bitmap_set()
	      .field_bitmaps[current_player->field_type];

	    ret = bitmap_move_animation.move
	      ( generate_stone_bitmap(field_bitmap,stone_type,set_move->dir),
		pos, from, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
				// move in direction: <pos> => <from>
	  }
	  break;
	  case Move::finish_move:
	  case Move::no_move:
	    ret = false;
	    break;
	}
      }
      break;
      case setting_stone:
      {
	// ** initialize variables
	Set_Move *set_move = dynamic_cast<Set_Move*>(*current_undo_move);
	assert( set_move );

	// readd stone to player 
	current_player->stones.inc_stone_count(set_move->stone_type_ID);

	// ** begin new step
	++current_undo_move;
	state = begin;
	    
	// ** recursive run
	ret = step_undo();
      }
      break;
      case finished:
	break;
    }

    return ret;
  }

  // current sub-animation done
  void Move_Sequence_Animation::on_done( wxCommandEvent & )
  {
    if( !undo )
      step();
    else
      step_undo();
  }

  // current sub-animation aborted
  void Move_Sequence_Animation::on_aborted( wxCommandEvent & )
  {
    state = finished;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_BLOKS_NOTIFY, abort_id );
      done_handler->ProcessEvent( event );
    }
  }

  void Move_Sequence_Animation::finish()
  {
    state = finished;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_BLOKS_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  //BEGIN_EVENT_TABLE(Move_Sequence_Animation, wxEvtHandler)				
  //  EVT_TIMER(ANIMATION_DONE, Move_Sequence_Animation::on_done)	//**/
  //  EVT_TIMER(ANIMATION_ABORTED, Move_Sequence_Animation::on_aborted)	//**/
  //END_EVENT_TABLE()						//**/
}
