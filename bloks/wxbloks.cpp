/*
 * wxbloks.cpp
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

// ============================================================================
// declarations
// ============================================================================

// compiled in picture set
//#define PURIST_50
//#define HEX_50
//#define HEX_70
//#define HEX_100
#define EINS

#ifndef DEFAULT_DATA_DIR
#define DEFAULT_DATA_DIR "./"	// will be overridden by Makefile.am
#endif
#ifndef DEFAULT_DATA_DIR2
#define DEFAULT_DATA_DIR2 "./"
#endif
#ifndef DEFAULT_SKIN_FILE
#define DEFAULT_SKIN_FILE DEFAULT_DATA_DIR "skins/bloks20.zip"
#endif
#ifndef DEFAULT_BEEP_FILE
#define DEFAULT_BEEP_FILE DEFAULT_DATA_DIR "sounds/beep.wav"
#endif
#ifndef DEFAULT_SKIN_FILE2
#define DEFAULT_SKIN_FILE2 DEFAULT_DATA_DIR2 "skins/bloks20.zip"
#endif
#ifndef DEFAULT_BEEP_FILE2
#define DEFAULT_BEEP_FILE2 DEFAULT_DATA_DIR2 "sounds/beep.wav"
#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#define MY_WX_MAKING_DLL // for WX MSW using mingw cross-compile
#include "wxbloks.hpp"
#include "bloks.hpp"
#include "dialogs.hpp"
#include "ai.hpp"
#include "util.hpp"
#include "wxmain.hpp"
#include "wxutil.hpp"

#include <wx/zipstrm.h>
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/dcmemory.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/html/helpctrl.h>
#include <wx/cshelp.h>
/*
#ifndef __OLD_GCC__
  #include <sstream>
#endif
*/
#include <fstream>
#include <list>
#include <assert.h>

// wx html help
#if !wxUSE_HTML
#error "wxHTML is required for the wxWindows HTML help system."
#endif

// define custom event type wx_EVT_BLOKS_NOTIFY
DEFINE_EVENT_TYPE(wxEVT_BLOKS_NOTIFY)

namespace bloks
{
  using namespace holtz;

  // ----------------------------------------------------------------------------
  // resources
  // ----------------------------------------------------------------------------

#ifdef PURIST_50
  // --------------------
  // purist pictures 50px

  // field pictures
#include "skins/purist50/field_removed.xpm"
#include "skins/purist50/field_empty.xpm"
#include "skins/purist50/stone_white.xpm"
#include "skins/purist50/stone_grey.xpm"
#include "skins/purist50/stone_black.xpm"
#include "skins/purist50/click_mark.xpm"
#include "skins/purist50/field_mark.xpm"
#include "skins/purist50/stone_mark.xpm"
#include "skins/purist50/background.xpm"
  // field dimensions
#include "skins/purist50/skin.hpp"
#endif

#ifdef HEX_50
  // -----------------------
  // hexagonal pictures 50px

  // field pictures
#include "skins/hex50/field_removed.xpm"
#include "skins/hex50/field_empty.xpm"
#include "skins/hex50/stone_white.xpm"
#include "skins/hex50/stone_grey.xpm"
#include "skins/hex50/stone_black.xpm"
#include "skins/hex50/click_mark.xpm"
#include "skins/hex50/field_mark.xpm"
#include "skins/hex50/stone_mark.xpm"
#include "skins/hex50/background.xpm"
  // field dimensions
#include "skins/hex50/skin.hpp"
#endif

#ifdef HEX_70
  // -----------------------
  // hexagonal pictures 70px

  // field pictures
#include "skins/hex70/field_removed.xpm"
#include "skins/hex70/field_empty.xpm"
#include "skins/hex70/stone_white.xpm"
#include "skins/hex70/stone_grey.xpm"
#include "skins/hex70/stone_black.xpm"
#include "skins/hex70/click_mark.xpm"
#include "skins/hex70/field_mark.xpm"
#include "skins/hex70/stone_mark.xpm"
#include "skins/hex70/background.xpm"
  // field dimensions
#include "skins/hex70/skin.hpp"
#endif

#ifdef HEX_100
  // -----------------------
  // hexagonal pictures 100px

  // field pictures
#include "skins/hex100/field_removed.xpm"
#include "skins/hex100/field_empty.xpm"
#include "skins/hex100/stone_white.xpm"
#include "skins/hex100/stone_grey.xpm"
#include "skins/hex100/stone_black.xpm"
#include "skins/hex100/click_mark.xpm"
#include "skins/hex100/field_mark.xpm"
#include "skins/hex100/stone_mark.xpm"
#include "skins/hex100/background.xpm"
  // field dimensions
#include "skins/hex100/skin.hpp"
#endif

#ifdef EINS
  // -----------------------
  // hexagonal pictures 100px

  // field pictures
#include "skins/eins/field_removed.xpm"
#include "skins/eins/field_empty.xpm"
#include "skins/eins/field_player1.xpm"
#include "skins/eins/field_player2.xpm"
#include "skins/eins/field_player3.xpm"
#include "skins/eins/field_player4.xpm"
#include "skins/eins/click_mark.xpm"
#include "skins/eins/field_mark.xpm"
#include "skins/eins/stone_mark.xpm"
#include "skins/eins/background.xpm"
  // field dimensions
#include "skins/eins/skin.hpp"
#endif

  // ============================================================================
  // utility functions
  // ============================================================================

  static void display_sequence_generator_state
  ( Sequence_Generator *sequence_generator, Sequence_Generator::Sequence_State state,
    WX_GUI_Manager &gui_manager, Game_Manager &game_manager )
  {
    wxString msg;
    Game &game = gui_manager.get_display_game();
    switch( state )
    {
      case Sequence_Generator::finished:
	gui_manager.set_mark(-1,-1);
	gui_manager.clear_stone_marks();
	gui_manager.clear_field_marks();
	sequence_generator = 0;
	game_manager.continue_game();
	break;
      case Sequence_Generator::hold1:
      {
	const Player_Panel* player_panel
	  = gui_manager.get_game_panel().get_player_panel(sequence_generator->get_picked_player_ID());
	if(player_panel!=0)
	{
	  const Stone_Panel& stone_panel = player_panel->get_stone_panel();
	  int picked_stone_type_ID = sequence_generator->get_picked_stone_type_ID();
	  int picked_stone_index = sequence_generator->get_picked_stone_index();
	  std::pair<int,int> field 
	    = stone_panel.get_field_pos( picked_stone_index, picked_stone_type_ID, 
					 Stone_Panel::TOP_LEFT );
	  gui_manager.set_mark( field.first, field.second );
	  gui_manager.clear_stone_marks();
	  gui_manager.clear_field_marks();
	  gui_manager.show_user_information( true );
	} 
	else 
	{
	  msg.Printf( wxT("%s"), _("Oops, internal error occurred! Unable to determine player panel that holds a piece.") );
	  gui_manager.report_error( msg, _("Fatal Error") );
	}
      }
      break;
      case Sequence_Generator::hold2:
      {
	const Player_Panel* player_panel
	  = gui_manager.get_game_panel().get_player_panel(sequence_generator->get_picked_player_ID());
	const Board_Panel &board_panel = gui_manager.get_game_panel().get_board_panel();
	if(player_panel!=0)
	{
	  const Stone_Panel& stone_panel = player_panel->get_stone_panel();
	  int picked_stone_type_ID = sequence_generator->get_picked_stone_type_ID();
	  int picked_stone_index = sequence_generator->get_picked_stone_index();
	  std::pair<int,int> field 
	    = stone_panel.get_field_pos( picked_stone_index, picked_stone_type_ID, 
					 Stone_Panel::BOTTOM_RIGHT );
	  gui_manager.set_mark( field.first, field.second );
	  std::pair<int,int> field2
	    = stone_panel.get_field_pos( picked_stone_index, picked_stone_type_ID, 
					 Stone_Panel::TOP_LEFT );
	  gui_manager.add_stone_mark(field2);
	  Field_Pos set_pos = sequence_generator->get_set_pos();
	  std::pair<int,int> field3 = board_panel.get_field_pos(set_pos.x,set_pos.y);
	  gui_manager.add_stone_mark(field3);
	  if( game.current_player->help_mode != Player::show_possible_moves )
	  {
	    // mark all empty fields that would serve as second click point
	    // (help mode == show possible moves does this already)
	    assert(does_include(game.board.id_stone_types,picked_stone_type_ID));
	    const Stone_Type &stone_type = game.board.id_stone_types[picked_stone_type_ID];
	    std::list<std::pair<int/*dx*/,int/*dy*/> >::const_iterator it;
	    bool is_flipped = stone_panel.get_is_flipped();
	    for( int d=(is_flipped?Field_Iterator::right_flipped : Field_Iterator::right); 
		 d<=(is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom); ++d )
	    {
	      Field_Iterator::Direction dir = Field_Iterator::Direction(d);
	      std::pair<int/*dx*/,int/*dy*/> diff 
		= stone_type.query_sub_field_diff(Stone_Type::TOP_LEFT,Stone_Type::BOTTOM_RIGHT,dir);
	      Field_Pos set_pos2(set_pos.x+diff.first,set_pos.y+diff.second);
	      Field_Iterator p2(set_pos2,&game.board);
	      if( p2.is_valid_field() )
	      {
		if( *p2 == field_empty )
		{
		  std::pair<int,int> field4 = board_panel.get_field_pos(set_pos2.x,set_pos2.y);
		  gui_manager.add_field_mark(field4);
		}
	      }
	    }
	  }
	  gui_manager.show_user_information( true );
	} 
	else 
	{
	  msg.Printf( wxT("%s"), _("Oops, internal error occurred! Unable to determine player panel that holds a piece.") );
	  gui_manager.report_error( msg, _("Fatal Error") );
	}
      }
      break;
      case Sequence_Generator::another_click:
	gui_manager.set_mark(-1,-1);
	gui_manager.clear_stone_marks();
	gui_manager.clear_field_marks();
	gui_manager.show_user_information( true );
	break;
      case Sequence_Generator::fatal_error:
	msg.Printf( wxT("%s"), _("Fatal Error!") );
	gui_manager.report_error( msg, _("Click Error") );
	break;
      case Sequence_Generator::error_require_set:
	gui_manager.show_status_text(_("You have to set a piece!"));
	break;
      case Sequence_Generator::error_can_t_set_here:
	gui_manager.show_status_text(_("Please set the piece on an empty field!"));
	break;
      case Sequence_Generator::error_wrong_player:
	gui_manager.show_status_text(_("Its not the turn of that player!"));
	break;
      case Sequence_Generator::error_impossible_yet:
	gui_manager.show_status_text(_("You can't do that at the moment!"));
	break;
      case Sequence_Generator::error_invalid_orientation:
	gui_manager.show_status_text(_("The two click points on the board must match the marked points on the piece for any valid orientation!"));
	break;
      case Sequence_Generator::error_rule_violation:
	switch(sequence_generator->get_rule_violation()) 
	{
	  case Set_Move::NO_VIOLATION:
	    gui_manager.show_status_text(_("You can't place the piece here: Oops, I am confused whether setting to this position is an error or not! Please inform windiana@sf.net."));
	    break;
	  case Set_Move::STONE_NOT_AVAILABLE:
	    gui_manager.show_status_text(_("You can't place the piece here: Player does not have any more pieces of that kind!"));
	    break;
	  case Set_Move::OUTSIDE:
	    gui_manager.show_status_text(_("You can't place the piece here: Some fields of the piece would go outside of the board!"));
	    break;
	  case Set_Move::OVERLAP:
	    gui_manager.show_status_text(_("You can't place the piece here: Some fields would overlap with other pieces on the board!"));
	    break;
	  case Set_Move::FIRST_NO_CORNER:
	    gui_manager.show_status_text(_("You can't place the piece here: First piece must be placed on one of the four corner fields!"));
	    break;
	  case Set_Move::OWN_ADJACENT:
	    gui_manager.show_status_text(_("You can't place the piece here: The piece must not lie adjacent to another piece of the same color!"));
	    break;
	  case Set_Move::OWN_NO_DIAGONAL:
	    gui_manager.show_status_text(_("You can't place the piece here: The piece must be diagonally adjacent to another piece of the same color!"));
	    break;
	}
	break;
	/*
	  default:
	  msg.Printf( _("Click impossible: %d"), state );
	  
	  wxMessageBox( msg, _("Click"), wxOK | wxICON_INFORMATION, 0 );
	*/
    }
  }

  // ============================================================================
  // class implementation
  // ============================================================================

  // ----------------------------------------------------------------------------
  // Bitmap_Handler
  // ----------------------------------------------------------------------------

  void Bitmap_Handler::setup_field_stone_bitmaps()
  {
    // consider prerendering bitmaps for each stone of player's stones
  }

  void Bitmap_Handler::setup_rotated_bitmaps()
  {
    if( !dimensions.rotation_symmetric )
    {
      // rotate field_bitmaps
      rotated.field_bitmaps.clear();
      std::map<Field_State_Type,wxBitmap>::iterator field_bitmap;
      for( field_bitmap =  normal.field_bitmaps.begin();
	   field_bitmap != normal.field_bitmaps.end(); ++field_bitmap )
      {
	wxBitmap &normal_bitmap = field_bitmap->second;
	const Field_State_Type &type = field_bitmap->first;
	wxImage normal_image 
	  = normal_bitmap.ConvertToImage().Rotate90( false /*counter clockwise*/ );
	rotated.field_bitmaps.insert
	  (std::pair<Field_State_Type,wxBitmap>(type, wxBitmap(normal_image)));
      }

      // rotate stone_bitmaps
      rotated.stone_bitmaps.clear();
      std::map<int/*stone_type_ID*/,wxBitmap>::iterator stone_bitmap;
      for( stone_bitmap =  normal.stone_bitmaps.begin();
	   stone_bitmap != normal.stone_bitmaps.end(); ++stone_bitmap )
      {
	wxBitmap &normal_bitmap = stone_bitmap->second;
	const int &type = stone_bitmap->first;
	wxImage normal_image = normal_bitmap.ConvertToImage().Rotate90( false/*counter clockwise*/ );
	rotated.stone_bitmaps.insert
	  (std::pair<int,wxBitmap>(type, wxBitmap(normal_image)));
      }

      // rotate marks
      rotated.click_mark 
	= wxBitmap(normal.click_mark.ConvertToImage().Rotate90(false/*counter clockwise*/));
      rotated.field_mark 
	= wxBitmap(normal.field_mark.ConvertToImage().Rotate90(false/*counter clockwise*/));
      rotated.stone_mark 
	= wxBitmap(normal.stone_mark.ConvertToImage().Rotate90(false/*counter clockwise*/));
      rotated.background 
	= wxBitmap(normal.background.ConvertToImage().Rotate90(false/*counter clockwise*/));
    }
  }
  
  // ----------------------------------------------------------------------------
  // Board_Panel
  // ----------------------------------------------------------------------------

  Board_Panel::Settings::Settings( bool rotate_board, bool show_coordinates, wxFont coord_font )
    : rotate_board(rotate_board), show_coordinates(show_coordinates), coord_font(coord_font)
  {
    if( coord_font == wxNullFont )
    {
      this->coord_font = wxFont(10,wxDEFAULT,wxNORMAL,wxNORMAL);
    }
  }

  Board_Panel::Board_Panel( Settings &settings, Game_Manager &game_manager, 
			    WX_GUI_Manager &gui_manager, Sequence_Generator* &sg )
    : settings(settings), game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      board_x(bitmap_handler.dimensions.board_x_offset), 
      board_y(bitmap_handler.dimensions.board_y_offset),
      sequence_generator( sg )     
  {
    if( settings.show_coordinates )
    {
      if( settings.rotate_board )
      {
	if( bitmap_handler.dimensions.rotation_symmetric )
	  board_y += bitmap_handler.dimensions.field_width;
	else
	  board_y += bitmap_handler.dimensions.field_height;
      }
      else
	board_x += bitmap_handler.dimensions.field_width;
    }
  }

  void Board_Panel::calc_dimensions()
  {
    Game &game = gui_manager.get_display_game();
    board_x = bitmap_handler.dimensions.board_x_offset;
    board_y = bitmap_handler.dimensions.board_y_offset;
    if( settings.show_coordinates )
    {
      if( settings.rotate_board )
      {
	if( bitmap_handler.dimensions.rotation_symmetric )
	{
	  board_y += bitmap_handler.dimensions.field_width;
	}
	else
	{
	  board_y += bitmap_handler.dimensions.field_height;
	}
      }
      else
      {
	board_x += bitmap_handler.dimensions.field_width;
      }
    }

    if( settings.rotate_board )
    {
      if( bitmap_handler.dimensions.rotation_symmetric )
      {
	width = bitmap_handler.dimensions.field_width * game.board.get_y_size() + board_x; 
	height = bitmap_handler.dimensions.field_height * game.board.get_x_size() + board_y; 
	if( settings.show_coordinates )
	{
	  height += bitmap_handler.dimensions.field_width;
	}
      }
      else
      {
	width = bitmap_handler.dimensions.field_height * game.board.get_y_size() + board_x; 
	height = bitmap_handler.dimensions.field_width * game.board.get_x_size() + board_y; 
	if( settings.show_coordinates )
	{
	  height += bitmap_handler.dimensions.field_height;
	}
      }
    }
    else
    {
      width = bitmap_handler.dimensions.field_width * game.board.get_x_size() + board_x;
      height = bitmap_handler.dimensions.field_height * game.board.get_y_size() + board_y;
      if( settings.show_coordinates )
      {
	width  += bitmap_handler.dimensions.field_width;
      }
    }
  }

  void Board_Panel::draw( wxDC &dc ) const
  {
    Game &game = gui_manager.get_display_game();
    Bitmap_Set &bitmap_set = get_bitmap_set();

    for( int fy = 0; fy < game.board.get_y_size(); ++fy )
    {
      for( int fx = 0; fx < game.board.get_x_size(); ++fx )
      {
	std::pair<int,int> pos = get_field_pos( fx, fy );

	dc.DrawBitmap( bitmap_set.field_bitmaps[ game.board.field[fx][fy] ], 
		       pos.first,  pos.second, true );
      }
    }
  }
  void Board_Panel::draw_text( wxDC &dc ) const 
  {
    if( settings.show_coordinates )
    {
      Game &game = gui_manager.get_display_game();
      dc.SetTextForeground(*wxBLACK/*settings.coordinate_font_colour*/);
      dc.SetFont( settings.coord_font );
      for( int fy = 0; fy < game.board.get_y_size(); ++fy )
      {
	for( int fx = 0; fx < game.board.get_x_size(); ++fx )
	{
	  if( !Board::is_removed(game.board.field[fx][fy]) )
	  {
	    Field_Pos pos;
	    pos.x = fx;
	    pos.y = fy;
	    std::string coord_name 
	      = game.ruleset->get_coordinate_translator()->get_field_name( pos );
	    wxCoord w,h;
	    dc.GetTextExtent( str_to_wxstr(coord_name), &w, &h );
	    std::pair<int,int> field = get_field_pos( fx, fy );
	    int coord_x, coord_y;
	    if( settings.rotate_board )
	    {
	      if( bitmap_handler.dimensions.rotation_symmetric )
	      {
		coord_x = field.first  + (bitmap_handler.dimensions.field_width      - w) / 2 +
		  bitmap_handler.dimensions.field_x;
		coord_y = field.second + (bitmap_handler.dimensions.field_height * 3 - h) / 2 +
		  bitmap_handler.dimensions.field_y;
	      }
	      else
	      {
		coord_x = field.first  + (bitmap_handler.dimensions.field_height    - w) / 2 +
		  bitmap_handler.dimensions.field_y;
		coord_y = field.second + (bitmap_handler.dimensions.field_width * 3 - h) / 2 +
		  bitmap_handler.dimensions.field_x;
	      }
	    }
	    else
	    {
	      coord_x = field.first  - (bitmap_handler.dimensions.field_width  + w) / 2 +
		bitmap_handler.dimensions.field_x;
	      coord_y = field.second + (bitmap_handler.dimensions.field_height - h) / 2 +
		bitmap_handler.dimensions.field_y;
	    }
	    dc.DrawText( str_to_wxstr(coord_name), coord_x, coord_y  );
	    break;
	  }
	}      
	for( int fx2 = game.board.get_x_size() - 1; fx2 >= 0; --fx2 )
	{
	  if( !Board::is_removed(game.board.field[fx2][fy]) )
	  {
	    Field_Pos pos;
	    pos.x = fx2;
	    pos.y = fy;
	    std::string coord_name = game.ruleset->get_coordinate_translator()->get_field_name( pos );
	    wxCoord w,h;
	    dc.GetTextExtent( str_to_wxstr(coord_name), &w, &h );
	    std::pair<int,int> field = get_field_pos( fx2, fy );
	    int coord_x, coord_y;
	    if( settings.rotate_board )
	    {
	      if( bitmap_handler.dimensions.rotation_symmetric )
	      {
		coord_x = field.first  + (bitmap_handler.dimensions.field_width  - w) / 2 +
		  bitmap_handler.dimensions.field_x;
		coord_y = field.second - (bitmap_handler.dimensions.field_height + h) / 2 +
		  bitmap_handler.dimensions.field_y;
	      }
	      else
	      {
		coord_x = field.first  + (bitmap_handler.dimensions.field_height - w) / 2 +
		  bitmap_handler.dimensions.field_y;
		coord_y = field.second - (bitmap_handler.dimensions.field_width  + h) / 2 +
		  bitmap_handler.dimensions.field_x;
	      }
	    }
	    else
	    {
	      coord_x = field.first  + (bitmap_handler.dimensions.field_width * 3  - w) / 2 +
		bitmap_handler.dimensions.field_x;
	      coord_y = field.second + (bitmap_handler.dimensions.field_height     - h) / 2 +
		bitmap_handler.dimensions.field_y;
	    }
	    dc.DrawText( str_to_wxstr(coord_name), coord_x, coord_y  );
	    break;
	  }
	}
      }
    }
  }

  void Board_Panel::on_click( int click_x, int click_y ) const
  {
    if( sequence_generator )
    {
      Field_Pos pos = get_field( click_x, click_y );
      if( pos.x >= 0 )		// valid position
      {
	// determine whether stones are flipped for player
	bool is_flipped=false; // set preliminary

	const Player_Panel* player_panel
	  = gui_manager.get_game_panel().get_player_panel(sequence_generator->get_picked_player_ID());
	if(player_panel!=0)
	{
	  is_flipped = player_panel->get_is_flipped();
	} 
	else 
	{
	  wxString msg;
	  msg.Printf( wxT("%s"), _("Oops, internal error occurred! Unable to determine player panel that holds a piece.") );
	  gui_manager.report_error( msg, _("Fatal Error") );
	}

	Sequence_Generator::Sequence_State state;
	state = sequence_generator->add_click( pos, is_flipped );
	display_sequence_generator_state(sequence_generator, state, gui_manager, game_manager);
      }
    }
  }

  Field_Pos Board_Panel::get_field( int abs_x, int abs_y ) const
  {
    Game &game = gui_manager.get_display_game();

    int offset, field_width, row;
    int rel_x, rel_y;

    if( settings.rotate_board )
    {
      // swap x-y coordinates
      rel_x = abs_y - y - board_y;
      rel_y = abs_x - x - board_x;
      // invert y-axis => rotated board
      int board_size_y;
      if( bitmap_handler.dimensions.rotation_symmetric )
      {
	board_size_y = bitmap_handler.dimensions.field_height * game.board.get_x_size();
	row = (rel_y) / bitmap_handler.dimensions.field_width;
	field_width = bitmap_handler.dimensions.field_height;
      }
      else
      {
	board_size_y = bitmap_handler.dimensions.field_width * game.board.get_x_size();
	row = (rel_y) / bitmap_handler.dimensions.field_height;
	field_width = bitmap_handler.dimensions.field_width;
      }
      rel_x = board_size_y - rel_x - 1;
    }
    else
    {
      rel_x = abs_x - x - board_x;
      rel_y = abs_y - y - board_y;
      row = (rel_y) / bitmap_handler.dimensions.field_height;
      field_width = bitmap_handler.dimensions.field_width;
    }
    
    offset = 0;
    if( (rel_x >= offset) && (rel_y >= 0 ) )
    {
      int col = (rel_x - offset) / field_width;
      
      if( (col >= 0) && (row >= 0) && 
	  (col < game.board.get_x_size()) && 
	  (row < game.board.get_y_size()) )
      {
	return Field_Pos(col,row);
      }
    }
    return Field_Pos();
  }

  std::pair<int,int> Board_Panel::get_field_pos( int col, int row ) const
  {
    Game &game = gui_manager.get_display_game();
    int offset;

    if( settings.rotate_board )
    {
      if( bitmap_handler.dimensions.rotation_symmetric )
      {
	offset = 0;
	int anf_x = x + board_x - bitmap_handler.dimensions.field_x;
	int anf_y = y + board_y - bitmap_handler.dimensions.field_y + offset;

	// swap x-y and invert y-axis => rotated board
	int field_y = anf_y
	  + bitmap_handler.dimensions.field_height * (game.board.get_x_size() - 1 - col);
	int field_x = anf_x + row*bitmap_handler.dimensions.field_width;
	return std::pair<int,int>( field_x, field_y );
      }
      else
      {
	offset = 0;
	int anf_x = x + board_x - bitmap_handler.dimensions.field_y;
	int anf_y = y + board_y - bitmap_handler.dimensions.field_x + offset;

	// swap x-y and invert y-axis => rotated board
	int field_y = anf_y
	  + bitmap_handler.dimensions.field_width * (game.board.get_x_size() - 1 - col);
	int field_x = anf_x + row*bitmap_handler.dimensions.field_height;
	return std::pair<int,int>( field_x, field_y );
      }
    }
    else
    {
      offset = 0;
      int anf_x = x + board_x - bitmap_handler.dimensions.field_x + offset;
      int anf_y = y + board_y - bitmap_handler.dimensions.field_y;

      return std::pair<int,int>( anf_x + col*bitmap_handler.dimensions.field_width, 
				 anf_y + row*bitmap_handler.dimensions.field_height );
    }
  }

  Bitmap_Set &Board_Panel::get_bitmap_set() const
  {
    if( settings.rotate_board && !bitmap_handler.dimensions.rotation_symmetric )
      return bitmap_handler.rotated;
    else
      return bitmap_handler.normal;
  }

  // ----------------------------------------------------------------------------
  // Stone Panel
  // ----------------------------------------------------------------------------

  Stone_Panel::Settings::Settings( bool rotate_stones, bool multiple_stones, bool horizontal, 
				   int max_stones, wxFont stone_font )
    : rotate_stones(rotate_stones), multiple_stones(multiple_stones), horizontal(horizontal), 
      max_stones(max_stones), stone_font(stone_font)
  {
    if( stone_font == wxNullFont )
    {
      this->stone_font = wxFont(16,wxDEFAULT,wxNORMAL,wxBOLD);
    }
  }

  Stone_Panel::Stone_Panel( Settings &settings, Field_State_Type player_field, Stones &stones, 
			    Game_Manager &game_manager, WX_GUI_Manager &gui_manager,
			    Sequence_Generator* &sg )
    : settings(settings), player_field(player_field), stones(stones),
      game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      sequence_generator(sg),
      is_flipped(false)
  {
  }

  void Stone_Panel::calc_dimensions()
  {
    Field_Iterator::Direction dir = is_flipped?Field_Iterator::right_flipped : Field_Iterator::right;
    if( settings.rotate_stones ) 
      dir = is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom;
    Game &game = gui_manager.get_display_game();
    // calculate max_height and max width based on common_stones
    int max_x=0;
    int max_y=0;
    const std::map<int/*stone_ID*/, int/*count*/>& stone_counts = game.common_stones.get_stone_counts();
    std::map<int/*stone_ID*/, int/*count*/>::const_iterator stone_it;
    for( stone_it=stone_counts.begin(); stone_it!=stone_counts.end(); ++stone_it )
      {
	int stone_ID = stone_it->first;
	int count = stone_it->second;
	if(count <= 0 ) continue;
	assert(does_include(game.board.id_stone_types,stone_ID));
	const Stone_Type &stone_type = game.board.id_stone_types[stone_ID];

	const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields = stone_type.get_sub_fields(dir);
	for( int i=0; i<count; ++i )
	{
	  int stone_x = stone_type.get_max_x();
	  int stone_y = stone_type.get_max_y();
	  if(settings.rotate_stones) 
	  {
	    stone_x = stone_type.get_max_y();
	    stone_y = stone_type.get_max_x();
	  }
	  if(stone_y > max_y) max_y = stone_y;
	  max_x += stone_x+2;
	  if( !settings.multiple_stones ) break;
	}
      }
    // calculate dimensions
    width  = bitmap_handler.dimensions.field_width * max_x; 
    height = bitmap_handler.dimensions.field_height * (max_y+1);
  }
  
  void Stone_Panel::draw( wxDC &dc ) const
  {
    Field_Iterator::Direction dir = is_flipped?Field_Iterator::right_flipped : Field_Iterator::right;
    if( settings.rotate_stones ) 
      dir = is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom;
    unsigned cur_x = 0;
    unsigned cur_y = 0;
    Game &game = gui_manager.get_display_game();
    const std::map<int/*stone_ID*/, int/*count*/>& stone_counts = stones.get_stone_counts();
    std::map<int/*stone_ID*/, int/*count*/>::const_iterator stone_it;
    for( stone_it=stone_counts.begin(); stone_it!=stone_counts.end(); ++stone_it )
    {
      int stone_ID = stone_it->first;
      int count = stone_it->second;
      if(count <= 0 ) continue;
      assert(does_include(game.board.id_stone_types,stone_ID));
      const Stone_Type &stone_type = game.board.id_stone_types[stone_ID];
      const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields = stone_type.get_sub_fields(dir);
      for( int i=0; i<count; ++i )
      {
	std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
	for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
	{
	  unsigned rx = it->first;
	  unsigned ry = it->second;
	  int x_pos = x + (cur_x+rx)*bitmap_handler.dimensions.field_width;
	  int y_pos = y + (cur_y+ry)*bitmap_handler.dimensions.field_height;
	  dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ player_field ], x_pos, y_pos, true );
	}
	int stone_max_x = stone_type.get_max_x();
	if(settings.rotate_stones) stone_max_x = stone_type.get_max_y();
	cur_x += stone_max_x+2;
	if( !settings.multiple_stones ) break;
      }
    }
  }

  void Stone_Panel::draw_text( wxDC &dc ) const
  {
    if( !settings.multiple_stones )
    {
      // Field_Iterator::Direction dir = Field_Iterator::right;
      // if( settings.rotate_stones ) 
      // 	dir = Field_Iterator::bottom;
      unsigned cur_x = 0;
      unsigned cur_y = 0;
      Game &game = gui_manager.get_display_game();
      const std::map<int/*stone_ID*/, int/*count*/>& stone_counts = stones.get_stone_counts();
      std::map<int/*stone_ID*/, int/*count*/>::const_iterator stone_it;
      for( stone_it=stone_counts.begin(); stone_it!=stone_counts.end(); ++stone_it )
      {
	int stone_ID = stone_it->first;
	int count = stone_it->second;
	if(count <= 0 ) continue;
	assert(does_include(game.board.id_stone_types,stone_ID));
	const Stone_Type &stone_type = game.board.id_stone_types[stone_ID];

    	wxString str;
    	str.Printf(wxT("%d"), count);

    	dc.SetTextForeground(*wxBLACK/*settings.stone_font_colour*/); // TODO: choose correct color
    	dc.SetFont( settings.stone_font );
    	wxCoord w,h;
    	dc.GetTextExtent(str,&w,&h);

	double rx = (stone_type.get_max_x())/2.;
	double ry = (stone_type.get_max_y())/2.;
	int stone_max_x = stone_type.get_max_x();
	if(settings.rotate_stones)
	{
	  rx = (stone_type.get_max_y())/2;
	  ry = (stone_type.get_max_x())/2;
	  stone_max_x = stone_type.get_max_y();
	}
	int x_pos = x + int((cur_x+rx)*bitmap_handler.dimensions.field_width);
	int y_pos = y + int((cur_y+ry)*bitmap_handler.dimensions.field_height);

	if(count > 1 )
	{
	  dc.DrawText( str, 
		       x_pos + (bitmap_handler.dimensions.field_width  - w) / 2 +
		       bitmap_handler.dimensions.field_x, 
		       y_pos + (bitmap_handler.dimensions.field_height - h) / 2 +
		       bitmap_handler.dimensions.field_y );
	}

	cur_x += stone_max_x+2;
      }
    }
  }
  
  void Stone_Panel::on_click( int /*click_x*/, int /*click_y*/ ) const
  {
    // there is no common stone panel for bloks (see player_panel)
  }

  void Stone_Panel::on_right_click( int /*cl_x*/, int /*cl_y*/ ) const
  {
    is_flipped = !is_flipped; // toggle is_flipped
    gui_manager.show_user_information( true, true ); // refresh
  }

  // stone_index: index within all stones of type stone_type_ID
  // returns <x,y>: <-1,-1>: error
  std::pair<int,int> Stone_Panel::get_field_pos
  ( int stone_index, int stone_type_ID, Field_Pos_Type type ) const
  {
    Field_Iterator::Direction dir = is_flipped?Field_Iterator::right_flipped : Field_Iterator::right;
    if( settings.rotate_stones ) 
      dir = is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom;
    unsigned cur_x = 0;
    unsigned cur_y = 0;
    Game &game = gui_manager.get_display_game();
    const std::map<int/*stone_ID*/, int/*count*/>& stone_counts = stones.get_stone_counts();
    std::map<int/*stone_ID*/, int/*count*/>::const_iterator stone_it;
    for( stone_it=stone_counts.begin(); stone_it!=stone_counts.end(); ++stone_it )
    {
      int stone_ID = stone_it->first;
      int count = stone_it->second;
      if(count <= 0 ) continue;
      assert(does_include(game.board.id_stone_types,stone_ID));
      const Stone_Type &stone_type = game.board.id_stone_types[stone_ID];
      int stone_max_x = stone_type.get_max_x();
      int stone_max_y = stone_type.get_max_y();
      if(settings.rotate_stones) {
	stone_max_x = stone_type.get_max_y();
	stone_max_y = stone_type.get_max_x();
      }
      if(stone_ID<stone_type_ID) // choose insert point even if no stones of that type are present, yet
      {
	cur_x += (stone_max_x+2)*(settings.multiple_stones?count:1);
      }
      else
      {
	for( int i=0; i<count; ++i )
	{
	  if( i==stone_index )
	  {
	    std::pair<unsigned,unsigned> match;
	    std::pair<double,double> match2;
	    bool use_match2 = false;
	    switch(type)
	    {
	    case ORIGIN: 
	      match.first = 0;
	      match.second = 0;
	      break;
	    case EXACT_CENTER:
	      match2.first = (stone_max_x+1)/2;
	      match2.second = (stone_max_y+1)/2;
	      use_match2 = true;
	      break;
	    case EXTENT:
	      match.first = stone_max_x+1;
	      match.second = stone_max_y+1;
	      break;
	    case CENTER: 
	      match = stone_type.query_sub_field(Stone_Type::CENTER,dir);
	      break;
	    case TOP_LEFT:
	      match = stone_type.query_sub_field(Stone_Type::TOP_LEFT,dir);
	      break;
	    case BOTTOM_RIGHT:
	      match = stone_type.query_sub_field(Stone_Type::BOTTOM_RIGHT,dir);
	      break;
	    }
	    int x_pos=-1, y_pos=-1;
	    if(use_match2)
	    {
	      x_pos = x + int((cur_x+match2.first)*bitmap_handler.dimensions.field_width);
	      y_pos = y + int((cur_y+match2.second)*bitmap_handler.dimensions.field_height);
	    }
	    else
	    {
	      x_pos = x + (cur_x+match.first)*bitmap_handler.dimensions.field_width;
	      y_pos = y + (cur_y+match.second)*bitmap_handler.dimensions.field_height;
	    } 
	    return std::pair<int,int>(x_pos,y_pos);
	  }
	  cur_x += stone_max_x+2;
	  if( !settings.multiple_stones ) break;
	}
      }
    }
    return std::pair<int,int>(-1,-1);
  }

  std::pair<int,int> Stone_Panel::get_field_pos( std::pair<int/*stone_type_ID*/,int/*stone_index*/> stone,
						 Field_Pos_Type type ) const
  {
    return get_field_pos( stone.second, stone.first, type );
  }

  std::pair<int/*stone_type_ID*/,int/*stone_index*/> Stone_Panel::get_stone( int cl_x, int cl_y ) const
  {
    if( !is_in(cl_x,cl_y) )
      return std::pair<int/*stone_type_ID*/,int>(-1,-1);

    Field_Iterator::Direction dir = is_flipped?Field_Iterator::right_flipped : Field_Iterator::right;
    if( settings.rotate_stones ) 
      dir = is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom;
    unsigned cur_x = 0;
    //unsigned cur_y = 0;
    Game &game = gui_manager.get_display_game();
    const std::map<int/*stone_ID*/, int/*count*/>& stone_counts = stones.get_stone_counts();
    std::map<int/*stone_ID*/, int/*count*/>::const_iterator stone_it;
    for( stone_it=stone_counts.begin(); stone_it!=stone_counts.end(); ++stone_it )
    {
      int stone_ID = stone_it->first;
      int count = stone_it->second;
      if(count <= 0 ) continue;
      assert(does_include(game.board.id_stone_types,stone_ID));
      const Stone_Type &stone_type = game.board.id_stone_types[stone_ID];
      const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields = stone_type.get_sub_fields(dir);
      for( int i=0; i<count; ++i )
      {
	int stone_max_x = stone_type.get_max_x();
	if(settings.rotate_stones)
	{
	  stone_max_x = stone_type.get_max_y();
	}
	int x_pos1 = x + (cur_x)*bitmap_handler.dimensions.field_width;
	int x_pos2 = x + (cur_x+stone_max_x+1)*bitmap_handler.dimensions.field_width;
	if(cl_x < x_pos1) break;
	if(cl_x < x_pos2)
	{
	  return std::pair<int/*stone_type_ID*/,int>(stone_ID,i);
	}

	cur_x += stone_max_x+2;
	if( !settings.multiple_stones ) break;
      }
    }
    return std::pair<int/*stone_type_ID*/,int>(-1,-1);
  }

  // ----------------------------------------------------------------------------
  // Player Panel
  // ----------------------------------------------------------------------------

  Player_Panel::Settings::Settings( const bloks::Stone_Panel::Settings &stone_settings,
				    wxFont player_font )
    : stone_settings(stone_settings), player_font( player_font )
  {
    if( player_font == wxNullFont )
    {
      this->player_font = wxFont(18,wxDEFAULT,wxNORMAL,wxBOLD);
    }
  }
				   
  
  Player_Panel::Player_Panel( Settings &settings, Player &player, Game_Manager &game_manager,
			      WX_GUI_Manager &gui_manager, Sequence_Generator* &sg )
    : settings(settings), player(player), 
      game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      sequence_generator(sg), 
      stone_panel( settings.stone_settings, player.field_type, player.stones, game_manager, gui_manager, sg ),
      header_panel( settings, player )
  {
    add( &header_panel );
    add( new Spacer(bitmap_handler.dimensions.stones_player_spacing,0), true /*deleted by sizer*/ );
    Horizontal_Sizer *stone_sizer = new Horizontal_Sizer();
    stone_sizer->add( new Spacer(bitmap_handler.dimensions.stone_offset,0), true /*deleted by sizer*/ );
    stone_sizer->add( &stone_panel );
    add( stone_sizer, true /*deleted by sizer*/ );
  }

  void Player_Panel::on_click( int cl_x, int cl_y ) const
  {
    wxString msg;
    if( sequence_generator )
    {
      std::pair<int/*stone_type_ID*/,int> clicked_stone = stone_panel.get_stone( cl_x, cl_y );
      if( clicked_stone.first >= 1 )
      {
	int stone_type_ID = clicked_stone.first;
	int stone_index = clicked_stone.second;

	if( clicked_stone.second < player.stones.get_stone_count(stone_type_ID) )
	{
	  // determine whether stones are flipped for player
	  bool is_flipped=get_is_flipped();

	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click_player_stone( player.id, stone_type_ID, stone_index, is_flipped );
	  display_sequence_generator_state(sequence_generator, state, gui_manager, game_manager);
	}
      }
    }
  }

  Player_Panel::Header_Panel::Header_Panel( Player_Panel::Settings &settings, Player &player )
    : settings(settings), player(player)
  {
  }

  void Player_Panel::Header_Panel::draw_text( wxDC &dc ) const
  {
    std::string str = player.name;
    if( player.host != "" )
      str = str + " (" + player.host + ")";
    wxString wxstr = str_to_wxstr(str);
    if( player.type == Player::ai )
      wxstr = _("[AI] ") + wxstr;

    dc.SetTextForeground(*wxBLACK/*settings.player_font_colour*/);
    dc.SetFont( settings.player_font );
    wxCoord w,h;
    dc.GetTextExtent(wxstr,&w,&h);

    //dc.SetBackgroundMode(wxSOLID);
    dc.SetPen( *wxTRANSPARENT_PEN );
    if( player.is_active )
    {
      //dc.SetTextBackground(*wxRED);
      //caption_text->SetBackgroundColour(*wxRED);

      dc.SetBrush( *wxRED_BRUSH );
      dc.DrawRectangle( x, y, w, h );
    }
    else
    {
      //dc.SetTextBackground(gui_manager.get_background_colour());
      //caption_text->SetBackgroundColour(gui_manager.get_background_colour());
#ifndef DRAW_BACKGROUND
      dc.SetBrush( wxBrush(gui_manager.get_background_colour(), wxSOLID) );
      dc.DrawRectangle( x, y, w, h );
#endif
    }
    dc.DrawText( wxstr, x, y );
  }

  void Player_Panel::Header_Panel::calc_dimensions()
  {
    std::string str = player.name;
    if( player.host != "" )
      str = str + " (" + player.host + ")";
    wxString wxstr = str_to_wxstr(str);
    if( player.type == Player::ai )
      wxstr = _("[AI] ") + wxstr;

    wxFrame frame(0,-1,str_to_wxstr(""));
    wxClientDC dc(&frame);
    //wxScreenDC dc;
    dc.SetFont( settings.player_font );
    wxCoord w,h;
    dc.GetTextExtent(wxstr,&w,&h);
    dc.DrawText( wxstr, 0, 0 );

    width  = w;
    height = h;
  }

  // ----------------------------------------------------------------------------
  // Mouse Handler
  // ----------------------------------------------------------------------------
  
  Mouse_Handler::Mouse_Handler( Game_Manager &game_manager, WX_GUI_Manager &gui_manager,
				Sequence_Generator * &sg )
    : Generic_Mouse_Input(gui_manager.get_display_game()), 
      game_manager(game_manager), gui_manager(gui_manager), 
      sequence_generator_hook(sg), ai(0)
  {
  }
  
  void Mouse_Handler::init_mouse_input() 
  {
    Game &game = gui_manager.get_display_game();

    Variant *target = gui_manager.get_target_variant();
    Variant *prev = 0;
    bool found = false;
    while( target )
    {
      if( target == game.variant_tree.get_current_variant() && prev != 0 )
      {
	std::cout << "Found target variant: id=" << gui_manager.get_target_variant()->id 
		  << ", sequence=" << prev->move_sequence << std::endl;
	found = true;
	sequence_generator.reset();
	sequence_generator.set_sequence( prev->move_sequence );
	gui_manager.do_move_slowly( prev->move_sequence, this, 
				    -1 /*done event id*/, -1 /*abort event id*/ );
	break;
      }
      if( target == game.variant_tree.get_root_variant() ) break;
      prev = target;
      target = target->prev;
    }
    if( !found )
    {
      if( gui_manager.get_target_variant() )
	std::cout << "Target variant is off-track: id=" << gui_manager.get_target_variant()->id 
		  << ", current id=" << game.variant_tree.get_current_variant()->id << std::endl;

      sequence_generator.reset();
      sequence_generator_hook = &sequence_generator;
      gui_manager.clear_target_variant();
      stop_watch.Start();
      gui_manager.allow_user_activity();
      if( game.current_player->help_mode == Player::show_hint )
      {
	ai = game_manager.get_hint_ai();
	ai->determine_hints();
      }
    }
  }
  void Mouse_Handler::disable_mouse_input() 
  {
    Game &game = gui_manager.get_display_game();
    if( sequence_generator_hook )
      sequence_generator_hook->reset();
    sequence_generator_hook = 0;

    used_time = stop_watch.Time();
    if( game.players.size() )
    {
      if( game.current_player->help_mode == Player::show_hint )
      {
	if( ai )
	  ai->abort();
	gui_manager.remove_hint();
      }
    }

    gui_manager.show_user_information(false,false);
  }

  long Mouse_Handler::get_used_time()
  {
    return used_time;
  }

  void Mouse_Handler::on_animation_done( wxCommandEvent& )
  {
    game_manager.continue_game();
  }

  BEGIN_EVENT_TABLE(Mouse_Handler, wxEvtHandler)				
    EVT_COMMAND(-1, wxEVT_BLOKS_NOTIFY, Mouse_Handler::on_animation_done)	//**/
  END_EVENT_TABLE()					//**/

  // ----------------------------------------------------------------------------
  // Game Panel
  // ----------------------------------------------------------------------------

  Game_Panel::Settings::Settings( const Board_Panel::Settings &board_settings, 
				  const Player_Panel::Settings &player_settings,
				  const Stone_Panel::Settings &common_stone_settings,
				  Arrangement_Type arrangement,
				  wxString skin_file, wxString beep_file, bool play_sound )
    : board_settings(board_settings), player_settings(player_settings), 
      common_stone_settings(common_stone_settings ), arrangement(arrangement ),
      skin_file(skin_file), beep_file(beep_file), play_sound(play_sound)
  {
  }

  Game_Panel::Game_Panel( Settings &settings, Game_Manager &game_manager, 
			  WX_GUI_Manager &gui_manager, Sequence_Generator* &sequence_generator )
    : settings(settings), game_manager(game_manager), gui_manager(gui_manager),
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      sequence_generator(sequence_generator),
      board_panel( settings.board_settings, game_manager, gui_manager, sequence_generator )
  {
    //rearrange_panels();		// will be done with calc_dimensions
  }

  Game_Panel::~Game_Panel()
  {
    if( player_panels.size() )
    {
      std::list<Player_Panel*>::iterator panel;
      for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
      {
	delete *panel;
      }
      player_panels.clear();
    }
  }

  void Game_Panel::calc_dimensions()
  {
    rearrange_panels();
    Horizontal_Sizer::calc_dimensions();
  }

  void Game_Panel::on_right_click( int cl_x, int cl_y ) const
  {
    if( sequence_generator )
    {
      if( sequence_generator->get_sequence_state() != Sequence_Generator::another_click )
      {
	Sequence_Generator::Sequence_State state;
	state = sequence_generator->undo_click();
	display_sequence_generator_state(sequence_generator, state, gui_manager, game_manager);
      } else {
	Horizontal_Sizer::on_right_click(cl_x,cl_y);
      }
    }
  }

  void Game_Panel::rearrange_panels()
  {
    clear();
    switch( settings.arrangement )
    {
      case Settings::arrange_standard:
      {
	Vertical_Sizer *vertical = new Vertical_Sizer();
	vertical->add( &board_panel );
	vertical->add( new Spacer( 0, bitmap_handler.dimensions.board_stones_spacing ), true /*destroy on remove*/ );
	vertical->add( &player_panel_sizer );						     
	add( vertical, true /* destroy on remove */ );
      }
      break;
      case Settings::arrange_player_stones_right:
      {
	add( &board_panel );
	add( new Spacer( bitmap_handler.dimensions.board_stones_spacing, 0 ), true );
	Vertical_Sizer *vertical = new Vertical_Sizer();
	vertical->add( &player_panel_sizer );						     
	add( vertical, true /* destroy on remove */ );
      }
      break;
    }
  }

  void Game_Panel::remove_players()
  {
    player_panel_sizer.clear();
    if( player_panels.size() )
    {
      std::list<Player_Panel*>::iterator panel;
      for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
      {
	delete *panel;
      }
      player_panels.clear();
    }
  }
  void Game_Panel::add_player( Player &player )
  {
    bool first = player_panels.empty();
    Player_Panel *player_panel = new Player_Panel( settings.player_settings, player, game_manager, 
						   gui_manager, sequence_generator );
    player_panels.push_back( player_panel );
    
    if( !first ) 
      player_panel_sizer.add( new Spacer(0,bitmap_handler.dimensions.player_player_spacing), true );
    player_panel_sizer.add( player_panel );
  }

  const Player_Panel *Game_Panel::get_player_panel( int id ) const
  {
    std::list<Player_Panel*>::const_iterator i;
    for( i = player_panels.begin(); i != player_panels.end(); ++i )
    {
      if( (*i)->get_id() == id )
	return *i;
    }
    return 0;
  }

  const wxBitmap &Game_Panel::get_background() const
  {
    if( bitmap_handler.dimensions.rotate_background )
      return get_board_panel().get_bitmap_set().background;
    else
      return bitmap_handler.normal.background;
  }

  // ----------------------------------------------------------------------------
  // WX_GUI_Manager
  // ----------------------------------------------------------------------------

  WX_GUI_Manager::WX_GUI_Manager( Game_Manager& game_manager, Game_Window &game_window, 
				  Variants_Display_Manager& variants_display_manager )
    : Game_UI_Manager( game_manager.get_game() ),
      game_manager(game_manager), game_window(game_window),
      variants_display_manager(variants_display_manager),
      game_settings( Board_Panel::Settings(),
		     Player_Panel::Settings(Stone_Panel::Settings()),
		     Stone_Panel::Settings() ),
      game_panel( game_settings, game_manager, *this, sequence_generator ), 
      sequence_generator(0),
      click_mark_x(-1), 
      move_animation( new Move_Sequence_Animation(*this,game_window) ),
      mouse_handler( game_manager, *this, sequence_generator ),
      user_activity_allowed(false)
  {
    // self register
    game_manager.set_ui_manager( this );
    variants_display_manager.set_notifiee( this );

    load_settings();

    add( &game_panel );

    x = 0; y = 0;
    calc_dimensions();
  }

  WX_GUI_Manager::~WX_GUI_Manager()
  {
    delete move_animation;
  }

  void WX_GUI_Manager::calc_dimensions()
  {
    Horizontal_Sizer::calc_dimensions();
    game_window.init_scrollbars();
  }

  void WX_GUI_Manager::selected_variant( std::list<unsigned> variant_id_path )
  {
    target_variant = display_game.variant_tree.get_variant(variant_id_path);
    std::list<unsigned> cur_id_path = display_game.variant_tree.get_variant_id_path
      (display_game.variant_tree.get_current_variant());

    std::list<unsigned>::iterator it1, it2;
    it1=variant_id_path.begin(); 
    it2=cur_id_path.begin();
    unsigned n=0;
    for( ; it1!=variant_id_path.end() && it2 !=cur_id_path.end(); ++it1, ++it2 )
    {
      if( *it1 == *it2 )
	++n;			// identical prefix
      else
	break;
    }
    // debug
    std::cout << "Root Variant: " << display_game.variant_tree.get_root_variant() << std::endl;
    {
      std::cout << "Selected Variant: ";
      std::list<unsigned>::iterator dit; 
      Variant *cur_variant = display_game.variant_tree.get_root_variant();
      for( dit=variant_id_path.begin(); dit != variant_id_path.end(); ++dit )
      {
	cur_variant = cur_variant->id_variant_map[*dit];
	std::cout << cur_variant << "(" << cur_variant->id << "); ";
      }
      std::cout << std::endl;
    }
    {
      std::cout << "Current Variant: ";
      std::list<unsigned>::iterator dit; 
      Variant *cur_variant = display_game.variant_tree.get_root_variant();
      for( dit=cur_id_path.begin(); dit != cur_id_path.end(); ++dit )
      {
	cur_variant = cur_variant->id_variant_map[*dit];
	std::cout << cur_variant << "(" << cur_variant->id << "); ";
      }
      std::cout << std::endl;
    }
    std::cout << "Target Variant = " << get_target_variant() << ", undo " 
	      << cur_id_path.size() - n << " moves" <<  std::endl;
    if( variant_id_path.size() == n ) // exact match
    {
      std::cout << "Exact Target Variant Match!" << std::endl;
      target_variant = 0;
    }
    // perform undo first
    int n_undo = cur_id_path.size() - n;
    if( n_undo > 0 )
      game_manager.undo_moves( n_undo );
    else
      if( target_variant !=0 )
	game_manager.continue_game();
  }

  void WX_GUI_Manager::setup_game_display() // setup all windows to display game
  {
    game_panel.remove_players();
    std::vector<Player>::iterator i;
    for( i = get_display_game().players.begin(); i != get_display_game().players.end(); ++i )
    {
      game_panel.add_player( *i );
    }
    calc_dimensions();
    show_user_information(true,true /*do_refresh*/);
  }

  void WX_GUI_Manager::set_board( const Game &new_game )
  {
    variants_display_manager.reset();
    update_board( new_game );
  }

  static bool check_player_references_changed(const Game &g1, const Game &g2)
  {
    if( g1.players.size() != g2.players.size() ) return true;
    std::vector<Player>::const_iterator it1,it2;
    for(it1=g1.players.begin(), it2=g2.players.begin(); 
	it1!=g1.players.end() && it2!=g2.players.end(); ++it1, ++it2)
    {
      const Player &player1 = *it1;
      const Player &player2 = *it2;
      if( &player1 != &player2 ) return true;
    }
    if( it1!=g1.players.end() ) return true;
    if( it2!=g2.players.end() ) return true;
    
    return false;
  }

  void WX_GUI_Manager::update_board( const Game &new_game )
  {
    std::list<unsigned> variant_id_path;
    if( target_variant )
      variant_id_path = display_game.variant_tree.get_variant_id_path(target_variant);
    bool player_references_changed = check_player_references_changed(display_game,new_game);
    display_game = new_game; // attention: make sure that setup_game_display is called whenever players change
    if( target_variant )
      target_variant = display_game.variant_tree.get_variant(variant_id_path);

    if( player_references_changed )
      setup_game_display();
    else
      refresh();
  }

  void WX_GUI_Manager::report_scores( const Game*, 
				      std::multimap<int/*score*/,const Player*> scores )
  {
    wxString msg, line;
    std::multimap<int/*score*/,const Player*>::iterator it;
    for( it=scores.begin(); it!=scores.end(); ++it )
      {
	int score = it->first;
	const Player *player = it->second;
	line.Printf( _("%3d points: %s\n"), score, str_to_wxstr(player->name).c_str() );
	msg += line;
      }
    report_information( msg, _("Scores") );
  }

  void WX_GUI_Manager::report_winner( Player *player )
  {
    if( player )
    {
      wxString msg;
      msg.Printf( _("And the winner is: %s"), str_to_wxstr(player->name).c_str() );
      report_information( msg, _("Winner") );
    }
    else
    {
      report_information( _("Nobody could win the game - This is a Tie!"), _("No Winner") );
    }
  }

  void WX_GUI_Manager::report_error( wxString msg, wxString caption )
  {
    wxMessageBox( msg, caption, wxOK | wxICON_ERROR, &game_window );
  }

  void WX_GUI_Manager::report_information( wxString msg, wxString caption )
  {
    wxMessageBox( msg, caption, wxOK | wxICON_INFORMATION, &game_window );
  }

  void WX_GUI_Manager::reset()
  {
    set_mark( -1, -1 );
    clear_stone_marks();
    clear_field_marks();
    show_status_text( wxT("") );
  }
  
  void WX_GUI_Manager::show_user_information( bool visible, bool do_refresh )
  {
    Game &game = get_display_game();
    stone_mark_positions.clear();
    field_mark_positions.clear();
    if( visible && game.players.size() )
    {
      // determine whether stones are flipped for player
      bool is_flipped=false; // set preliminary

      const Player_Panel* player_panel
	= get_game_panel().get_player_panel(game.current_player->id);
      if(player_panel!=0)
      {
	is_flipped = player_panel->get_is_flipped();
      } 
      else 
      {
	wxString msg;
	msg.Printf( wxT("%s"), _("Oops, internal error occurred! Unable to determine player panel that holds a piece.") );
	report_error( msg, _("Fatal Error") );
      }

      switch( game.current_player->help_mode )
      {
	case Player::no_help: break;
	case Player::show_possible_moves: 
	{
	  if( sequence_generator )
	  {
	    if( sequence_generator->get_required_move_type() == Move::set_move )
	    {
	      Sequence_Generator::Sequence_State state = sequence_generator->get_sequence_state();

	      switch( state )
	      {
		case Sequence_Generator::hold1:
		case Sequence_Generator::hold2:
		{
		  std::list<Field_Pos> clicks = sequence_generator->get_possible_clicks(is_flipped);
		  std::list<Field_Pos>::iterator click;
		  for( click = clicks.begin(); click != clicks.end(); ++click )
		  {
		    field_mark_positions.push_back
		      ( game_panel.get_board_panel().get_field_pos( click->x, click->y ) );
		  }
		}
		break;
		default:
		{
		  std::list<std::pair<int/*playerID*/,int/*stoneID*/> > 
		    stone_clicks = sequence_generator->get_possible_stone_clicks(is_flipped); 
		  std::list<std::pair<int/*playerID*/,int/*stoneID*/> >::const_iterator it;
		  for( it=stone_clicks.begin(); it!=stone_clicks.end(); ++it )
		  {
		    int player_ID = it->first;
		    int stone_ID = it->second;
		    const Player_Panel* player_panel
		      = get_game_panel().get_player_panel(player_ID);
		    if(player_panel!=0)
		    {
		      const Stone_Panel& stone_panel = player_panel->get_stone_panel();
		      int stone_index = 0;
		      std::pair<int,int> field 
			= stone_panel.get_field_pos( stone_index, stone_ID, 
						     Stone_Panel::TOP_LEFT );
		      stone_mark_positions.push_back( field );
		    } 
		    else 
		    {
		      wxString msg;
		      msg.Printf( wxT("%s"), 
				  _("Oops, internal error occurred! Unable to determine player panel that holds a piece.") );
		      report_error( msg, _("Fatal Error") );
		    }
		  }
		}
		break;
	      }
	    }
	  }
	}
	break;
	case Player::show_hint:
	{
	  if( current_hint.valid )
	  {
	    const std::list<bloks::Move*> &moves = current_hint.sequence.get_moves();
	    std::list<bloks::Move*>::const_iterator move;
	    for( move = moves.begin(); move != moves.end(); ++move )
	    {
	      switch( (*move)->get_type() )
	      {
		case Move::no_move:
		case Move::finish_move:
		  break;
		case Move::set_move:
		  {
		    Set_Move *set_move = dynamic_cast<Set_Move *>(*move);
		    std::list<Player_Panel*>::const_iterator panel;
		    for( panel = game_panel.get_player_panels().begin(); 
			 panel != game_panel.get_player_panels().end(); ++panel )
		    {
		      if( (*panel)->get_id() == game.current_player->id )
		      {
			int stone_count = 
			  game.current_player->stones.get_stone_count( set_move->stone_type_ID );
			if( stone_count > 0 )
			{
			  stone_mark_positions.push_back
			    ( (*panel)->get_stone_panel().
			      get_field_pos( 0, set_move->stone_type_ID, /*type=*/Stone_Panel::TOP_LEFT ) );

			  std::pair<int,int> field 
			    = (*panel)->get_stone_panel().get_field_pos
			    ( 0, set_move->stone_type_ID, /*type=*/Stone_Panel::BOTTOM_RIGHT );
			  field_mark_positions.push_back( field );
			  //set_mark( field.first, field.second );

			  const Stone_Type &stone_type = game.board.get_stone_type(set_move->stone_type_ID);
			  std::pair<Field_Pos /*top_left*/, Field_Pos /*bottom_right*/> click_points
			    = stone_type.get_click_points(set_move->dir,/*origin=*/set_move->pos); 
			  Field_Pos top_left = click_points.first;
			  std::pair<int,int> field3 = game_panel.get_board_panel().get_field_pos(top_left.x,top_left.y);
			  stone_mark_positions.push_back( field3 );
			  Field_Pos bottom_right = click_points.second;
			  std::pair<int,int> field4 = game_panel.get_board_panel().get_field_pos(bottom_right.x,bottom_right.y);
			  field_mark_positions.push_back( field4 );
			}
			break;
		      }
		    }
		    // std::pair<unsigned,unsigned> offset1 
		    //   = stone_type.query_sub_field(Stone_Type::TOP_LEFT);
		    // std::pair<unsigned,unsigned> offset2 
		    //   = stone_type.query_sub_field(Stone_Type::BOTTOM_RIGHT);
		    // //TODO: modify offset1/offset2 according to dir
		    // stone_mark_positions.push_back
		    //   ( game_panel.get_board_panel().get_field_pos( set_move->pos.x + offset1.first, 
		    // 						    set_move->pos.y + offset1.second ) );
		    // stone_mark_positions.push_back
		    //   ( game_panel.get_board_panel().get_field_pos( set_move->pos.x + offset2.first, 
		    // 						    set_move->pos.y + offset2.second ) );
		  }
		break;
	      }
	    }
	  }
	}
	break;
      }

      if( sequence_generator )
      {
	switch ( sequence_generator->get_required_move_type() )
	{
	  case Move::set_move:
	    show_status_text(_("You should set a piece")); // remove old status text message
	    break;
	  case Move::no_move:
	  case Move::finish_move:
	    break;
	}
      }
    }
    else
    {
      show_status_text(wxT("")); // remove old status text message
    }
    if( do_refresh )
      refresh();
  }

  void WX_GUI_Manager::give_hint( AI_Result ai_result )
  {
    current_hint = ai_result;
    show_user_information( true, true );
  }

  void WX_GUI_Manager::remove_hint()
  {
    current_hint = AI_Result();
    show_user_information( true, false );
  }

  void WX_GUI_Manager::set_mark( int x, int y )
  {
    click_mark_x = x;
    click_mark_y = y;

    wxClientDC dc(&game_window);
    //dc.BeginDrawing(); depricated
    game_window.PrepareDC(dc);
    draw_mark(dc);
    //dc.EndDrawing(); depricated
  }

  void WX_GUI_Manager::draw_mark( wxDC &dc )
  {
    std::list< std::pair<int,int> >::iterator position;
    for( position = static_stone_mark_positions.begin();
	 position != static_stone_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_board_panel().get_bitmap_set().stone_mark, x, y, true );
    }

    for( position = stone_mark_positions.begin();
	 position != stone_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_board_panel().get_bitmap_set().stone_mark, x, y, true );
    }

    for( position = static_field_mark_positions.begin();
	 position != static_field_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_board_panel().get_bitmap_set().field_mark, x, y, true );
    }

    for( position = field_mark_positions.begin();
	 position != field_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_board_panel().get_bitmap_set().field_mark, x, y, true );
    }

    if( click_mark_x >= 0 )
      dc.DrawBitmap( get_game_panel().get_board_panel().get_bitmap_set().click_mark, 
		     click_mark_x, click_mark_y, true );
  }

  void WX_GUI_Manager::allow_user_activity()
  {
    show_user_information( true );
    beep();			// tell user that his activity is recommended
    variants_display_manager.set_allow_selection(true);
    user_activity_allowed = true;
  }

  void WX_GUI_Manager::stop_user_activity()
  {
    variants_display_manager.set_allow_selection(false);
    show_user_information( false, false );
    if( user_activity_allowed )
    {
      user_activity_allowed = false;
      mouse_handler.disable_mouse_input();
    }
  }

  void WX_GUI_Manager::stop_animations()
  {
    move_animation->abort();
  }

  void WX_GUI_Manager::abort_all_activity()
  {
    stop_animations();
    stop_user_activity();
    refresh();
  }

  void WX_GUI_Manager::do_move_slowly( Move_Sequence sequence, wxEvtHandler *done_handler, 
				       int event_id, int abort_id  ) // show user how move is done
  {
    Game &game = get_display_game();
    assert( sequence.check_sequence( game ) );
    bool ret = move_animation->start( sequence, game, done_handler, event_id, abort_id );

    if( !ret && done_handler )
    {
      wxCommandEvent event( wxEVT_BLOKS_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  void WX_GUI_Manager::undo_move_slowly( wxEvtHandler *done_handler, 
					 int event_id, int abort_id  ) // show user how move is done
  {
    Game &game = get_display_game();
    bool ret;

    if( game.variant_tree.is_first() )
      ret = false;
    else
    {
      Move_Sequence sequence = game.variant_tree.get_current_variant()->move_sequence;

      ret = move_animation->start_undo( sequence, game, done_handler, event_id, abort_id );
    }

    if( !ret && done_handler )
    {
      wxCommandEvent event( wxEVT_BLOKS_NOTIFY, event_id );
      done_handler->ProcessEvent( event );
    }
  }

  void WX_GUI_Manager::show_status_text( wxString text ) // shows text in status bar
  {
    game_window.show_status_text( text );
  }

  void WX_GUI_Manager::beep()
  {
#if wxUSE_SOUND
    if( game_settings.play_sound )
      sound_beep.Play();
#endif
  }

  Player_Input *WX_GUI_Manager::get_user_input()
  {
    return &mouse_handler;
  }

  void WX_GUI_Manager::load_settings()
  {
    bool ok = false;
    wxConfigBase* cfg = wxConfig::Get();
    wxString buf;
    if( cfg->Read( wxT("/bloks/skin_file"), &buf) )
    {
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  game_settings.skin_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      buf = str_to_wxstr(DEFAULT_SKIN_FILE);
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  cfg->Write( wxT("/bloks/skin_file"), buf);
	  cfg->Flush();
	  game_settings.skin_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok && (std::string(DEFAULT_SKIN_FILE) != DEFAULT_SKIN_FILE2) )
    {
      buf = str_to_wxstr(DEFAULT_SKIN_FILE2);
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  cfg->Write( wxT("/bloks/skin_file"), buf);
	  cfg->Flush();
	  game_settings.skin_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      buf = wxFileSelector( _("Choose a skin File"), wxT(""), 
			    wxT(""), wxT(""), _("Skin Files (*.zip)|*.zip"),
			    wxFD_OPEN );
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  cfg->Write( wxT("/bloks/skin_file"), buf);
	  cfg->Flush();
	  game_settings.skin_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      bitmap_handler.normal.field_bitmaps[field_removed]	= wxBitmap(field_removed_xpm);
      bitmap_handler.normal.field_bitmaps[field_empty]		= wxBitmap(field_empty_xpm);
      bitmap_handler.normal.field_bitmaps[field_player1]	= wxBitmap(field_player1_xpm);
      bitmap_handler.normal.field_bitmaps[field_player2]	= wxBitmap(field_player2_xpm);
      bitmap_handler.normal.field_bitmaps[field_player3]	= wxBitmap(field_player3_xpm);
      bitmap_handler.normal.field_bitmaps[field_player4]	= wxBitmap(field_player4_xpm);
      bitmap_handler.normal.click_mark				= wxBitmap(click_mark_xpm);
      bitmap_handler.normal.field_mark				= wxBitmap(field_mark_xpm);
      bitmap_handler.normal.stone_mark				= wxBitmap(stone_mark_xpm);
      bitmap_handler.normal.background				= wxBitmap(background_xpm);
      bitmap_handler.dimensions = Dimensions( field_empty_xpm );

      bitmap_handler.setup_field_stone_bitmaps();
      bitmap_handler.setup_rotated_bitmaps();
    }

    ok = false;
    if( cfg->Read( wxT("/bloks/beep_file"), &buf) )
    {
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_beep(buf) )
	{
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      buf = str_to_wxstr(DEFAULT_BEEP_FILE);
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_beep(buf) )
	{
	  cfg->Write( wxT("/bloks/beep_file"), buf);
	  cfg->Flush();
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok && (std::string(DEFAULT_BEEP_FILE) != DEFAULT_BEEP_FILE2) )
    {
      buf = str_to_wxstr(DEFAULT_BEEP_FILE2);
#ifndef __WXMSW__
      std::cerr << "Trying to load: " << wxstr_to_str(buf) << std::endl;
#endif
      if( wxFileExists(buf) )
      {
	if( load_beep(buf) )
	{
	  cfg->Write( wxT("/bloks/beep_file"), buf);
	  cfg->Flush();
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      buf = wxFileSelector( _("Choose a sound file as move reminder"), wxT(""),
			    wxT(""), wxT(""), _("Sound Files (*.wav)|*.wav"),
			    wxFD_OPEN );
      if( wxFileExists(buf) )
      {
	if( load_beep(buf) )
	{
	  cfg->Write( wxT("/bloks/beep_file"), buf );
	  cfg->Flush();
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )		// disable sound if there is no valid sound file
    {
      game_settings.play_sound = false;
      cfg->Write( wxT("/bloks/play_sound"), false );
      cfg->Flush();
    }
    else
    {
      bool play_sound;
      if( cfg->Read( wxT("/bloks/play_sound"), &play_sound ) )
      {
	game_settings.play_sound = play_sound;
      }
      else
      {
	game_settings.play_sound = true;
	cfg->Write( wxT("/bloks/play_sound"), true );
	cfg->Flush();
      }
    }
  }

  void WX_GUI_Manager::save_settings()
  {
    wxConfigBase* cfg = wxConfig::Get();
    cfg->Write( wxT("/bloks/skin_file"),  game_settings.skin_file );
    cfg->Write( wxT("/bloks/beep_file"),  game_settings.beep_file );
    cfg->Write( wxT("/bloks/play_sound"), game_settings.play_sound );
    cfg->Flush();
  }

  bool WX_GUI_Manager::load_skin( wxString filename )
  {
    show_user_information(false,false);

    wxInputStream *input;
    wxImage image;

    input = get_zip_input_stream( filename, wxT("field_removed.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_removed] = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_empty.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_empty] = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_player1.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_player1] = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_player2.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_player2] = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_player3.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_player3] = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_player4.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_player4] = wxBitmap(image);
    delete input;

    input = get_zip_input_stream( filename, wxT("click_mark.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.click_mark = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("field_mark.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_mark = wxBitmap(image);
    delete input;
    input = get_zip_input_stream( filename, wxT("stone_mark.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.stone_mark = wxBitmap(image);
    delete input;

    input = get_zip_input_stream( filename, wxT("background.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.background = wxBitmap(image);
    delete input;

    input = get_zip_input_stream( filename, wxT("skin.ini") );
    //if( input->Eof() ) { delete input; return false; } // ini file might be empty
    wxFileConfig config( *input );
    bitmap_handler.dimensions 
      = Dimensions( config, bitmap_handler.normal.field_bitmaps[field_empty] );
    delete input;

    bitmap_handler.setup_field_stone_bitmaps();
    bitmap_handler.setup_rotated_bitmaps();

    calc_dimensions();
    
    return true;
  }

  bool WX_GUI_Manager::load_beep( wxString filename )
  {
    if( wxFile::Exists( filename ) )
    {
#if wxUSE_SOUND
      if( sound_beep.Create(filename) )
      {
	sound_beep.Play();
	return true;
      }
#endif
    }
    return false;
  }

  void WX_GUI_Manager::mouse_click_left( int x, int y )
  {
    show_status_text(wxT("")); // remove old status text message
    on_click( x, y );
  }

  void WX_GUI_Manager::mouse_click_right( int x, int y )
  {
    show_status_text(wxT("")); // remove old status text message
    on_right_click( x, y );
  }
  
  void WX_GUI_Manager::refresh()
  {
    game_window.refresh();
    variants_display_manager.show_variant_tree(display_game.variant_tree,
					       display_game.coordinate_translator);
  }
}


