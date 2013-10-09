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

namespace zertz
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
	wxCommandEvent event( wxEVT_ZERTZ_NOTIFY, abort_id );
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
      wxCommandEvent event( wxEVT_ZERTZ_NOTIFY, event_id );
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
    Connect( ANIMATION_DONE, wxEVT_ZERTZ_NOTIFY, 
	     (wxObjectEventFunction) (wxEventFunction) (wxCommandEventFunction) 
	     &Move_Sequence_Animation::on_done );
    Connect( ANIMATION_ABORTED, wxEVT_ZERTZ_NOTIFY, 
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
	  case Move::knock_out_move:
	  {
	    // ** initialize variables
	    Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move*>(*current_move);
	    assert( knock_move );
	    Field_State_Type stone_field = game->board.get_field(knock_move->from);
	    assert( Board::is_stone(stone_field) );
	    Stones::Stone_Type stone = Stones::Stone_Type(game->board.get_field(knock_move->from));

	    // ** modify board state
	    Field_Iterator from_field( knock_move->from, &game->board );
	    save_field = *from_field; // store stone which moves
	    *from_field = field_empty; // remove stone which moves

	    // ** calculate animation positions
	    std::pair<int,int> _from = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->from.x, 
									    knock_move->from.y );
	    std::pair<int,int> _to = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->to.x, 
									    knock_move->to.y );

	    wxPoint from( _from.first, _from.second );
	    wxPoint to( _to.first, _to.second );

	    // ** change state
	    state = knock_out_jump;

	    // ** refresh display and start animation
	    game_window.refresh(); 
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone],
		from, to, 12, 35, this, ANIMATION_DONE, ANIMATION_ABORTED );
	  }
	  break;
	  case Move::set_move:
	  {
	    // ** initialize variables
	    Set_Move *set_move = dynamic_cast<Set_Move*>(*current_move);
	    assert( set_move );
	    Stones::Stone_Type stone = set_move->stone_type;

	    // ** calculate animation positions and modify board
	    std::pair<int,int> _from;
	    if( set_move->own_stone ) // is stone owned by current_player
	    {
	      const Player_Panel *panel = 
		gui_manager.get_game_panel().get_player_panel( current_player->id );
	      int col = current_player->stones.stone_count[set_move->stone_type] - 1;
	      assert(col >= 0);
	      _from = panel->get_stone_panel().get_field_pos( col, set_move->stone_type );

	      // remove stone from player
	      --current_player->stones.stone_count[set_move->stone_type];
	    }
	    else
	    {
	      // get position of last stone of the right type
	      int col = game->common_stones.stone_count[set_move->stone_type] - 1;
	      assert(col >= 0);
	      _from = gui_manager.get_game_panel().get_stone_panel()
		.get_field_pos( col, set_move->stone_type );
	      // remove stone from reservoir
	      --game->common_stones.stone_count[set_move->stone_type];
	    }
	    wxPoint from( _from.first, _from.second );

	    std::pair<int,int> _pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
									    set_move->pos.y );
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh();
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone],
		from, pos, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
	  }
	  break;
	  case Move::remove:
	  {
	    // ** initialize variables
	    Remove *remove = dynamic_cast<Remove*>(*current_move);
	    assert( remove );

	    // ** modify board state
	    Field_Iterator field  ( remove->remove_pos,   &game->board );
	    *field = field_removed;

	    // ** calculate animation positions
	    std::pair<int,int> _remove_pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( remove->remove_pos.x, 
							   remove->remove_pos.y );
	    wxPoint remove_pos( _remove_pos.first, _remove_pos.second );

	    // ** stay in begin state after animation
	    ++current_move;
	    state = begin;

	    // ** refresh display and start animation
	    game_window.refresh(); 
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set()
		.field_bitmaps[field_empty], 
		remove_pos, wxPoint(-1,-1), 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
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

	    current_removed_stone = finish_move->removed_stones.begin();
	    end_removed_stones = finish_move->removed_stones.end();

	    // ** change state
	    state = finish_sequence;
	    
	    // ** recursive run
	    ret = step();
	  }
	  break;
	  case Move::no_move:
	    ret = false;
	    break;
	}
      }
      break;
      case knock_out_jump:
      {
	// ** initialize variables
	Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move*>(*current_move);
	assert( knock_move );
	Field_State_Type stone_field = knock_move->knocked_stone;
	assert( Board::is_stone(stone_field) );
	Stones::Stone_Type stone = Stones::Stone_Type(stone_field);

	// ** modify board state
	Field_Iterator to_field  ( knock_move->to,   &game->board );
	Field_Iterator over_field( knock_move->over, &game->board );
	*to_field = save_field;	    // set stone on destination field
	*over_field = field_empty;  // remove stone in the middle

	// ** calculate animation positions
	std::pair<int,int> _over = 
	  gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->over.x, 
									knock_move->over.y );
	wxPoint over( _over.first, _over.second );

	std::pair<int,int> _to;
	const Player_Panel *panel = 
	  gui_manager.get_game_panel().get_player_panel( current_player->id );
	int col = current_player->stones.stone_count[stone];
	assert(col >= 0);
	_to = panel->get_stone_panel().get_field_pos( col, stone );
	wxPoint to( _to.first, _to.second );

	save_field = stone_field;

	// ** change state
	state = knock_out_collects;

	// ** refresh display and start animation
	game_window.refresh(); 
	ret = bitmap_move_animation.move
	  ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone],
	    over, to, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
      }
      break;
      case knock_out_collects:
      {
	// ** modify board state
	assert( Board::is_stone(save_field) );
	++current_player->stones.stone_count[Stones::Stone_Type(save_field)];

	// ** begin new step
	++current_move;
	state = begin;
	ret = step();		// next step at once
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
	Field_Iterator to_field  ( set_move->pos, &game->board );
	*to_field = stone;

	// ** begin new step
	++current_move;
	state = begin;
	ret = step();		// next step at once
      }
      break;
      case removing:
      {
	assert(false);
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
	  Field_Iterator field( field_pos, &game->board );
	  *field = field_removed;

	  // ** calculate animation positions
	  std::pair<int,int> _pos = 
	    gui_manager.get_game_panel().get_board_panel().get_field_pos( field_pos.x, field_pos.y );
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
	  ret = bitmap_move_animation.move
	    ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone], 
	      pos, to, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED );
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
	  case Move::knock_out_move:
	  {
	    // ** initialize variables
	    Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move*>(*current_undo_move);
	    assert( knock_move );

	    Field_State_Type stone_field = knock_move->knocked_stone;
	    assert( Board::is_stone(stone_field) );
	    Stones::Stone_Type stone = Stones::Stone_Type(knock_move->knocked_stone);

	    // ** modify board state
#ifndef __WXMSW__
	    if( current_player->stones.stone_count[stone] <= 0 )
	    {
	      std::cerr << "Error removing stonetype:" << stone << std::endl;
	      std::cerr << current_player->stones.stone_count[Stones::white_stone] << std::endl;
	      std::cerr << current_player->stones.stone_count[Stones::grey_stone] << std::endl;
	      std::cerr << current_player->stones.stone_count[Stones::black_stone] << std::endl;
	      std::cerr 
		<< game->get_next_player(current_player)->stones.stone_count[Stones::white_stone] 
		<< std::endl;
	      std::cerr 
		<< game->get_next_player(current_player)->stones.stone_count[Stones::grey_stone] 
		<< std::endl;
	      std::cerr 
		<< game->get_next_player(current_player)->stones.stone_count[Stones::black_stone] 
		<< std::endl;      
	    }
#endif
	    assert(current_player->stones.stone_count[stone] > 0 );
	    --current_player->stones.stone_count[stone]; // remove stone from player

	    // ** calculate animation positions
	    std::pair<int,int> _over = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->over.x, 
									    knock_move->over.y );
	    wxPoint over( _over.first, _over.second );

	    std::pair<int,int> _to;
	    const Player_Panel *panel = 
	      gui_manager.get_game_panel().get_player_panel( current_player->id );
	    int col = current_player->stones.stone_count[stone];
	    assert(col >= 0);
	    _to = panel->get_stone_panel().get_field_pos( col, stone );
	    wxPoint to( _to.first, _to.second );
	    
	    // ** change state
	    state = knock_out_jump;
	    
	    // ** refresh display and start animation
	    game_window.refresh(); 
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone],
		to, over, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
				// move in direction: <to> => <over>
	  }
	  break;
	  case Move::set_move:
	  {
	    // ** initialize variables
	    Set_Move *set_move = dynamic_cast<Set_Move*>(*current_undo_move);
	    assert( set_move );
	    Stones::Stone_Type stone = set_move->stone_type;

	    // ** modify board state
	    Field_Iterator field  ( set_move->pos, &game->board );
	    *field = field_empty;

	    // ** calculate animation positions
	    std::pair<int,int> _from;
	    if( set_move->own_stone ) // is stone owned by current_player
	    {
	      const Player_Panel *panel = 
		gui_manager.get_game_panel().get_player_panel( current_player->id );
	      int col = current_player->stones.stone_count[set_move->stone_type];
	      assert(col >= 0);
	      _from = panel->get_stone_panel().get_field_pos( col, set_move->stone_type );

	      stones = &current_player->stones;
	    }
	    else
	    {
	      // get position of last stone of the right type
	      int col = game->common_stones.stone_count[set_move->stone_type];
	      assert(col >= 0);
	      _from = gui_manager.get_game_panel().get_stone_panel()
		.get_field_pos( col, set_move->stone_type );
	      stones = &game->common_stones;
	    }
	    wxPoint from( _from.first, _from.second );

	    std::pair<int,int> _pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( set_move->pos.x, 
									    set_move->pos.y );
	    wxPoint pos( _pos.first, _pos.second );

	    // ** change state
	    state = setting_stone;
	    
	    // ** refresh display and start animation
	    game_window.refresh(); // remove picked stone from screen
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone],
		pos, from, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
				// move in direction: <pos> => <from>
	  }
	  break;
	  case Move::remove:
	  {
	    // ** initialize variables
	    Remove *remove = dynamic_cast<Remove*>(*current_undo_move);
	    assert( remove );

	    // ** modify board state
	    Field_Iterator field  ( remove->remove_pos, &game->board );
	    *field = field_removed;

	    // ** calculate animation positions
	    std::pair<int,int> _remove_pos = 
	      gui_manager.get_game_panel().get_board_panel().get_field_pos( remove->remove_pos.x, 
							   remove->remove_pos.y );
	    wxPoint remove_pos( _remove_pos.first, _remove_pos.second );

	    // ** change state
	    state = removing;

	    // ** refresh display and start animation
	    game_window.refresh(); 
	    ret = bitmap_move_animation.move
	      ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set()
		.field_bitmaps[field_empty], wxPoint(-1,-1), remove_pos, 20, 40, 
		this, ANIMATION_DONE, ANIMATION_ABORTED ); // move field from anywhere 
	  }
	  break;
	  case Move::finish_move:
	  {
	    // ** initialize variables
	    Finish_Move *finish_move = dynamic_cast<Finish_Move*>(*current_undo_move);
	    assert( finish_move );

	    current_removed_stone = finish_move->removed_stones.begin();
	    end_removed_stones = finish_move->removed_stones.end();

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
	}
      }
      break;
      case knock_out_jump:
      {
	// ** initialize variables
	Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move*>(*current_undo_move);
	assert( knock_move );
	
	Field_State_Type stone_field = knock_move->knocked_stone;
	assert( Board::is_stone(stone_field) );

	// ** modify board state
	Field_Iterator over_field( knock_move->over, &game->board );
	*over_field = stone_field;  // set stone in the middle
	Field_Iterator to_field( knock_move->to, &game->board );
	save_field = *to_field;
	*to_field = field_empty; // remove stone which moves

	// ** calculate animation positions
	std::pair<int,int> _from = 
	  gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->from.x, 
									knock_move->from.y );
	std::pair<int,int> _to = 
	  gui_manager.get_game_panel().get_board_panel().get_field_pos( knock_move->to.x, 
									knock_move->to.y );

	wxPoint from( _from.first, _from.second );
	wxPoint to( _to.first, _to.second );

	// ** change state
	state = knock_out_collects;

	// ** refresh display and start animation
	game_window.refresh(); 
	ret = bitmap_move_animation.move
	  ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().
	    stone_bitmaps[Stones::Stone_Type(save_field)],
	    to, from, 12, 35, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
				// move stone back: <to> => <from>
      }
      break;
      case knock_out_collects:
      {
	// ** initialize variables
	Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move*>(*current_undo_move);
	assert( knock_move );
	
	Field_State_Type stone_field = knock_move->knocked_stone;
	assert( Board::is_stone(stone_field) );

	// ** modify board state
	Field_Iterator from_field  ( knock_move->from,   &game->board );
	assert( Board::is_stone(save_field) );
	*from_field = save_field;	    // set stone on starting field

	// ** begin new step
	++current_undo_move;
	state = begin;
	ret = step_undo();
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
	Field_Iterator to_field  ( set_move->pos, &game->board );
	*to_field = field_empty;

	// readd stone to player or common stones
	++stones->stone_count[set_move->stone_type];

	// ** begin new step
	++current_undo_move;
	state = begin;
	ret = step_undo();
      }
      break;
      case removing:
      {
	// ** initialize variables
	Remove *remove = dynamic_cast<Remove*>(*current_undo_move);
	assert( remove );

	// ** modify board state
	Field_Iterator field  ( remove->remove_pos, &game->board );
	*field = field_empty;

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
	  Field_Pos &field_pos = current_removed_stone->first;
	  Stones::Stone_Type &stone = current_removed_stone->second;

	  // ** modify board state
	  --current_player->stones.stone_count[stone]; // remove stone from player

	  // ** calculate animation positions
	  std::pair<int,int> _pos = 
	    gui_manager.get_game_panel().get_board_panel().get_field_pos( field_pos.x, field_pos.y );
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
	  ret = bitmap_move_animation.move
	    ( gui_manager.get_game_panel().get_board_panel().get_bitmap_set().stone_bitmaps[stone], 
	      to, pos, 20, 40, this, ANIMATION_DONE, ANIMATION_ABORTED ); 
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
	Field_Iterator field( field_pos, &game->board );
	*field = Field_State_Type(stone); // place removed stone

	// ** begin next finish animation
	state = finish_sequence;
	++current_removed_stone;
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
      wxCommandEvent event( wxEVT_ZERTZ_NOTIFY, abort_id );
      done_handler->ProcessEvent( event );
    }
  }

  void Move_Sequence_Animation::finish()
  {
    state = finished;
    
    if( done_handler )
    {
      wxCommandEvent event( wxEVT_ZERTZ_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  //BEGIN_EVENT_TABLE(Move_Sequence_Animation, wxEvtHandler)				
  //  EVT_TIMER(ANIMATION_DONE, Move_Sequence_Animation::on_done)	//**/
  //  EVT_TIMER(ANIMATION_ABORTED, Move_Sequence_Animation::on_aborted)	//**/
  //END_EVENT_TABLE()						//**/
}
