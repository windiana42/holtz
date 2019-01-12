/*
 * wxrelax.cpp
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
#define DEFAULT_SKIN_FILE DEFAULT_DATA_DIR "skins/relax60.zip"
#endif
#ifndef DEFAULT_BEEP_FILE
#define DEFAULT_BEEP_FILE DEFAULT_DATA_DIR "sounds/beep.wav"
#endif
#ifndef DEFAULT_SKIN_FILE2
#define DEFAULT_SKIN_FILE2 DEFAULT_DATA_DIR2 "skins/relax60.zip"
#endif
#ifndef DEFAULT_BEEP_FILE2
#define DEFAULT_BEEP_FILE2 DEFAULT_DATA_DIR2 "sounds/beep.wav"
#endif

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#define MY_WX_MAKING_DLL // for WX MSW using mingw cross-compile
#include "wxrelax.hpp"
#include "relax.hpp"
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
#include <wx/numdlg.h>
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

// define custom event type wx_EVT_RELAX_NOTIFY
DEFINE_EVENT_TYPE(wxEVT_RELAX_NOTIFY)

namespace relax
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
#include "skins/eins/stone_white.xpm"
#include "skins/eins/stone_grey.xpm"
#include "skins/eins/stone_black.xpm"
#include "skins/eins/click_mark.xpm"
#include "skins/eins/field_mark.xpm"
#include "skins/eins/stone_mark.xpm"
#include "skins/eins/background.xpm"
  // field dimensions
#include "skins/eins/skin.hpp"
#endif

  // ============================================================================
  // implementation
  // ============================================================================

  // ----------------------------------------------------------------------------
  // Bitmap_Handler
  // ----------------------------------------------------------------------------

  void Bitmap_Handler::setup_field_stone_bitmaps()
  {
    Standard_Ruleset ruleset; 	// to convert numbers in fields

    std::map<int/*num*/,wxBitmap>::iterator it1,it2,it3;
    for( it1=normal.num_bitmaps[0].begin(); 
	 it1!=normal.num_bitmaps[0].end(); ++it1 )
      for( it2=normal.num_bitmaps[1].begin(); 
	   it2!=normal.num_bitmaps[1].end(); ++it2 )
	for( it3=normal.num_bitmaps[2].begin(); 
	     it3!=normal.num_bitmaps[2].end(); ++it3 )
	  {
	    std::vector<int> nums; nums.resize(3);
	    nums[0] = it1->first;
	    nums[1] = it2->first;
	    nums[2] = it3->first;
	    std::vector<wxBitmap*> bitmaps; bitmaps.resize(3);
	    bitmaps[0] = &it1->second;
	    bitmaps[1] = &it2->second;
	    bitmaps[2] = &it3->second;
	    Stones::Stone_Type stone_type = ruleset.get_stone(nums[0],nums[1],nums[2]);
	    wxImage field_stone_image (normal.stone_base.ConvertToImage());
	    for( unsigned dir=0; dir<3; ++dir )
	      {
		wxImage stone_image (bitmaps[dir]->ConvertToImage());
		// selfmade alpha-based overlay copy
		unsigned char *field_stone_data  = field_stone_image.GetData();
		unsigned char *field_stone_alpha = field_stone_image.GetAlpha();
		unsigned char *stone_data        = stone_image.GetData();
		unsigned char *stone_alpha       = stone_image.GetAlpha();
		unsigned char mask_colour[3];
		mask_colour[0] = stone_image.GetMaskRed();
		mask_colour[1] = stone_image.GetMaskGreen();
		mask_colour[2] = stone_image.GetMaskBlue();
		int min_width = field_stone_image.GetWidth() < stone_image.GetWidth() ? 
		  field_stone_image.GetWidth() : stone_image.GetWidth();
		int min_height = field_stone_image.GetHeight() < stone_image.GetHeight() ? 
		  field_stone_image.GetHeight() : stone_image.GetHeight();

		for( int x = 0; x < min_width; ++x )
		  for( int y = 0; y < min_height; ++y )
		    {
		      if( stone_alpha && field_stone_alpha )
			{
			  int sa = stone_alpha[x+y*stone_image.GetWidth()];
			  int fa = field_stone_alpha[x+y*stone_image.GetWidth()];
			  int smul = sa;
			  int fmul = int((1-sa/255.)*fa);
			  int na = max(sa,fa);
			  for( int c = 0; c < 3; ++c )
			    {
			      int s = stone_data[(x+y*stone_image.GetWidth())*3 + c];
			      int f = field_stone_data[(x+y*stone_image.GetWidth())*3 + c];
			      if( smul+fmul > 0 )
				field_stone_data[(x+y*stone_image.GetWidth())*3 + c]
				  = ( s * smul + f * fmul ) / (smul+fmul); // apply weightings
			      else
				na = 0;
			    }
			  field_stone_alpha[x+y*stone_image.GetWidth()] = na;
			}
		      else
			{
			  // fall-back implementation for no/partial alpha support
			  bool masked = true;
			  for( int c = 0; c < 3; ++c )
			    {
			      if( mask_colour[c] != stone_data[(x+y*stone_image.GetWidth())*3 + c] )
				{
				  masked = false;
				  break;
				}
			    }
			  if( stone_alpha )
			    if( stone_alpha[x+y*stone_image.GetWidth()] < 255 )
			      masked = true; // consider using alpha as half transparency
			  if( !masked )
			    {
			      for( int c = 0; c < 3; ++c )
				{
				  field_stone_data[(x+y*field_stone_image.GetWidth())*3 + c] = 
				    stone_data[(x+y*stone_image.GetWidth())*3 + c];
				}
			      if( field_stone_alpha )
				field_stone_alpha[(x+y*field_stone_image.GetWidth())] = 255;
			    }
			}
		    }
		stone_image.Destroy();
	      }
	    Field_State_Type field_type = Field_State_Type(stone_type);
	    assert( Board::is_stone( field_type ) );
	    normal.field_bitmaps[field_type] = wxBitmap(field_stone_image);
	    field_stone_image.Destroy();
	  }
    // !! workaround for bitmap->image->bitmap convertion loosing alpha
    wxImage field_empty_image (normal.field_bitmaps[field_empty].ConvertToImage());
    normal.field_bitmaps[field_empty] = wxBitmap(field_empty_image);
    // !! end workaround
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
      this->coord_font = wxFont(16,wxDEFAULT,wxNORMAL,wxBOLD);
    }
  }

  Board_Panel::Board_Panel( Settings &settings, Game_Manager &game_manager, 
			    WX_GUI_Manager &gui_manager, Sequence_Generator* &sg,
			    int player_id )
    : settings(settings), game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      board_x(bitmap_handler.dimensions.board_x_offset), 
      board_y(bitmap_handler.dimensions.board_y_offset),
      sequence_generator( sg ), player_id(player_id)
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

  const Board &Board_Panel::get_board() const 
  { 
    return gui_manager.get_display_game().get_player_by_id(player_id)->board; 
  }

  void Board_Panel::calc_dimensions()
  {
    //Game &game = gui_manager.get_display_game();
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
	width = bitmap_handler.dimensions.field_packed_width * (get_board().get_y_size()-1) + 
	  bitmap_handler.dimensions.field_width + board_x; 
	height = bitmap_handler.dimensions.field_height * get_board().get_x_size() +
	  bitmap_handler.dimensions.field_height / 2 + board_y; 
	
	if( settings.show_coordinates )
	{
	  height += bitmap_handler.dimensions.field_width;
	  width  += bitmap_handler.dimensions.field_packed_width;
	}
      }
      else
      {
	width = bitmap_handler.dimensions.field_packed_height * (get_board().get_y_size()-1) + 
	  bitmap_handler.dimensions.field_height + board_x; 
	height = bitmap_handler.dimensions.field_width * get_board().get_x_size() +
	  bitmap_handler.dimensions.field_width / 2 + board_y; 
	
	if( settings.show_coordinates )
	{
	  height += bitmap_handler.dimensions.field_height;
	  width  += bitmap_handler.dimensions.field_packed_width;
	}
      }
    }
    else
    {
      width = bitmap_handler.dimensions.field_width * get_board().get_x_size() + board_x
	+ bitmap_handler.dimensions.field_width / 2;
      height = bitmap_handler.dimensions.field_packed_height * (get_board().get_y_size()-1)  
	+ bitmap_handler.dimensions.field_height + board_y;

      if( settings.show_coordinates )
      {
	width  += bitmap_handler.dimensions.field_width;
	height += bitmap_handler.dimensions.field_height;
      }
    }
  }

  void Board_Panel::draw( wxDC &dc ) const
  {
    //Game &game = gui_manager.get_display_game();
    Bitmap_Set &bitmap_set = get_bitmap_set();

    for( int fy = 0; fy < get_board().get_y_size(); ++fy )
    {
      for( int fx = 0; fx < get_board().get_x_size(); ++fx )
      {
	std::pair<int,int> pos = get_field_pos( fx, fy );

	dc.DrawBitmap( bitmap_set.field_bitmaps[ get_board().field[fx][fy] ], 
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
      for( int fy = 0; fy < get_board().get_y_size(); ++fy )
      {
	for( int fx = 0; fx < get_board().get_x_size(); ++fx )
	{
	  if( !Board::is_removed(get_board().field[fx][fy]) )
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
	for( int fx2 = get_board().get_x_size() - 1; fx2 >= 0; --fx2 )
	{
	  if( !Board::is_removed(get_board().field[fx2][fy]) )
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
      wxString msg;

      Field_Pos pos = get_field( click_x, click_y );
      if( pos.x >= 0 )		// valid position
      {
	Sequence_Generator::Sequence_State state;
	state = sequence_generator->add_click( player_id, pos );
	
	switch( state )
	{
	  case Sequence_Generator::hold_stone_begin:
	  case Sequence_Generator::hold_stone_end:
	    break;
	  case Sequence_Generator::finished:
	    gui_manager.set_mark(-1,-1);
	    sequence_generator = 0;
	    game_manager.continue_game();
	    break;
	  case Sequence_Generator::another_click:
	    gui_manager.set_mark(-1,-1);
	    gui_manager.show_user_information( true );
	    break;
	  case Sequence_Generator::fatal_error:
	    msg.Printf( wxT("%s"), _("Fatal Error!") );
	    gui_manager.report_error( msg, _("Click Error") );
	    break;
	  case Sequence_Generator::error_require_set:
	    gui_manager.show_status_text(_("You have to set a stone!"));
	    break;
	  case Sequence_Generator::error_can_t_set_here:
	    gui_manager.show_status_text(_("Please set the stone on an empty field!"));
	    break;
	  case Sequence_Generator::error_wrong_player:
	    gui_manager.show_status_text(_("Its not the turn of that player!"));
	    break;
	  case Sequence_Generator::error_impossible_yet:
	    gui_manager.show_status_text(_("You can't do that at the moment!"));
	    break;
	    /*
	      default:
	      msg.Printf( _("Click impossible: %d"), state );
	  
	      wxMessageBox( msg, _("Click"), wxOK | wxICON_INFORMATION, 0 );
	    */
	}
      }
    }
  }

  Field_Pos Board_Panel::get_field( int abs_x, int abs_y ) const
  {
    //Game &game = gui_manager.get_display_game();

    int half_diff, offset, field_width, row;
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
	board_size_y = bitmap_handler.dimensions.field_height * get_board().get_x_size() 
	  + bitmap_handler.dimensions.field_height / 2;

	half_diff = (bitmap_handler.dimensions.field_width - 
		     bitmap_handler.dimensions.field_packed_width) / 2;
	row = (rel_y - half_diff) / bitmap_handler.dimensions.field_packed_width;
	offset = (row & 1) ? 0 : bitmap_handler.dimensions.field_height / 2;
	field_width = bitmap_handler.dimensions.field_height;
      }
      else
      {
	board_size_y = bitmap_handler.dimensions.field_width * get_board().get_x_size() 
	  + bitmap_handler.dimensions.field_width / 2;

	half_diff = (bitmap_handler.dimensions.field_height - 
		     bitmap_handler.dimensions.field_packed_height) / 2;
	row = (rel_y - half_diff) / bitmap_handler.dimensions.field_packed_height;
	offset = (row & 1) ? 0 : bitmap_handler.dimensions.field_width / 2;
	field_width = bitmap_handler.dimensions.field_width;
      }
      rel_x = board_size_y - rel_x - 1;
    }
    else
    {
      rel_x = abs_x - x - board_x;
      rel_y = abs_y - y - board_y;

      half_diff = (bitmap_handler.dimensions.field_height - 
		   bitmap_handler.dimensions.field_packed_height) / 2;
      row = (rel_y - half_diff) / bitmap_handler.dimensions.field_packed_height;
      offset = (row & 1) ? 0 : bitmap_handler.dimensions.field_width / 2;
      field_width = bitmap_handler.dimensions.field_width;
    }
    
    
    if( (rel_x >= offset) && (rel_y >= 0 ) )
    {
      int col = (rel_x - offset) / field_width;
      
      if( (col >= 0) && (row >= 0) && 
	  (col < get_board().get_x_size()) && 
	  (row < get_board().get_y_size()) )
      {
	return Field_Pos(col,row);
      }
    }
    return Field_Pos();
  }

  std::pair<int,int> Board_Panel::get_field_pos( int col, int row ) const
  {
    //Game &game = gui_manager.get_display_game();
    int offset;

    if( settings.rotate_board )
    {
      if( bitmap_handler.dimensions.rotation_symmetric )
      {
	if( row & 1 )		// even row
	  offset = bitmap_handler.dimensions.field_height / 2;
	else
	  offset = 0;
	int anf_x = x + board_x - bitmap_handler.dimensions.field_x;
	int anf_y = y + board_y - bitmap_handler.dimensions.field_y + offset;

	// swap x-y and invert y-axis => rotated board
	int field_y = anf_y
	  + bitmap_handler.dimensions.field_height * (get_board().get_x_size() - 1 - col);
	int field_x = anf_x + row*bitmap_handler.dimensions.field_packed_width;
	return std::pair<int,int>( field_x, field_y );
      }
      else
      {
	if( row & 1 )		// even row
	  offset = bitmap_handler.dimensions.field_width / 2;
	else
	  offset = 0;
	int anf_x = x + board_x - bitmap_handler.dimensions.field_y;
	int anf_y = y + board_y - bitmap_handler.dimensions.field_x + offset;

	// swap x-y and invert y-axis => rotated board
	int field_y = anf_y
	  + bitmap_handler.dimensions.field_width * (get_board().get_x_size() - 1 - col);
	int field_x = anf_x + row*bitmap_handler.dimensions.field_packed_height;
	return std::pair<int,int>( field_x, field_y );
      }
    }
    else
    {
      if( row & 1 )		// even row
	offset = 0;
      else
	offset = bitmap_handler.dimensions.field_width / 2;
      int anf_x = x + board_x - bitmap_handler.dimensions.field_x + offset;
      int anf_y = y + board_y - bitmap_handler.dimensions.field_y;

      return std::pair<int,int>( anf_x + col*bitmap_handler.dimensions.field_width, 
				 anf_y + row*bitmap_handler.dimensions.field_packed_height );
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

  Stone_Panel::Stone_Panel( Settings &settings, Stones &stones, Game_Manager &game_manager, 
			    WX_GUI_Manager &gui_manager,
			    Sequence_Generator* &sg )
    : settings(settings), stones(stones), 
      game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      sequence_generator(sg)
  {
  }

  void Stone_Panel::calc_dimensions()
  {
    if( settings.multiple_stones )  // no difference
    {
      width  = bitmap_handler.dimensions.field_width;
      height = bitmap_handler.dimensions.field_height;
    }
    else
    {
      width  = bitmap_handler.dimensions.field_width;
      height = bitmap_handler.dimensions.field_height;
    }
  }
  
  void Stone_Panel::draw( wxDC &dc ) const
  {
    int y_pos = y, x_pos = x;
    if( stones.current_stone != Stones::invalid_stone )
      {
	if( settings.rotate_stones && !bitmap_handler.dimensions.rotation_symmetric )
	  {
	    dc.DrawBitmap( bitmap_handler.rotated.field_bitmaps[ Field_State_Type(stones.current_stone) ], 
			   x_pos, y_pos, true );
	  }
	else
	  {
	    dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ Field_State_Type(stones.current_stone) ], 
			   x_pos, y_pos, true );
	  }
      }
  }

  void Stone_Panel::draw_text( wxDC & ) const
  {
//     if( stones.current_stone != Stones::invalid_stone )
//       {
// 	int stone_type;
// 	int y_pos = y, x_pos = x;
// 	int num1=(stones.current_stone-Stones::stone_begin)%3;  // todo: user ruleset to determine!!!
// 	int num2=((stones.current_stone-Stones::stone_begin)/3)%3;
// 	int num3=((stones.current_stone-Stones::stone_begin)/9)%3;

// 	wxString str;
// 	str.Printf(wxT("%d/%d/%d"), num1, num2, num3);

// 	dc.SetTextForeground(*wxRED/*settings.stone_font_colour*/);
// 	dc.SetFont( settings.stone_font );
// 	wxCoord w,h;
// 	dc.GetTextExtent(str,&w,&h);

// 	dc.DrawText( str, 
// 		     x_pos + (bitmap_handler.dimensions.field_width  - w) / 2 +
// 		     bitmap_handler.dimensions.field_x, 
// 		     y_pos + (bitmap_handler.dimensions.field_height - h) / 2 +
// 		     bitmap_handler.dimensions.field_y );
//       }
  }
  
  void Stone_Panel::on_click( int /*click_x*/, int /*click_y*/ ) const
  {
	/*
    wxString msg;
    if( sequence_generator )
    {
      std::pair<Stones::Stone_Type,int> clicked_stone = get_stone( click_x, click_y );
      Stones::Stone_Type &type = clicked_stone.first;
      int &col = clicked_stone.second;

      if( type != Stones::invalid_stone )
      {
	if( col < stones.stone_count[type] )
	{
	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click_common_stone( type );
	  
	  switch( state )
	  {
	    case Sequence_Generator::finished:
	      gui_manager.set_mark(-1,-1);
	      sequence_generator = 0;
	      game_manager.continue_game();
	      break;
	    case Sequence_Generator::hold_white:
	    case Sequence_Generator::hold_grey:
	    case Sequence_Generator::hold_black:
	    {
	      std::pair<int,int> coord = get_field_pos( col, type );
	      gui_manager.set_mark( coord.first, coord.second );
	      gui_manager.show_user_information( true );
	    }
	    break;
	    case Sequence_Generator::another_click:
	      gui_manager.set_mark(-1,-1);
	      gui_manager.show_user_information( true );
	      break;
	    case Sequence_Generator::fatal_error:
	      msg.Printf( wxT("%s"), _("Fatal Error!") );
	      gui_manager.report_error( msg, _("Click Error") );
	      break;
	    case Sequence_Generator::error_require_knock_out:
	      gui_manager.show_status_text(_("You must do the knock out move!"));
	      break;
	    case Sequence_Generator::error_require_set:
	      gui_manager.show_status_text(_("You have to set a stone!"));
	      break;
	    case Sequence_Generator::error_require_remove:
	      gui_manager.show_status_text(_("You have to remove an empty border field!"));
	      break;
	    case Sequence_Generator::error_can_t_remove:
	      gui_manager.show_status_text(_("Field can't be removed without moving another stone!"));
	      break;
	    case Sequence_Generator::error_can_t_move_here:
	      gui_manager.show_status_text(_("You can't move to that field!"));
	      break;
	    case Sequence_Generator::error_can_t_set_here:
	      gui_manager.show_status_text(_("Please set the stone on an empty field!"));
	      break;
	    case Sequence_Generator::error_must_pick_common_stone:
	      gui_manager.show_status_text(_("You must pick a common stone!"));
	      break;
	    case Sequence_Generator::error_wrong_player:
	      gui_manager.show_status_text(_("Its not the turn of that player!"));
	      break;
	    case Sequence_Generator::error_impossible_yet:
	      gui_manager.show_status_text(_("You can't do that at the moment!"));
	      break;
	    case Sequence_Generator::error_must_knock_out_with_same_stone:
	      gui_manager.show_status_text(_("You must knock out once more with the same stone!"));
	      break;
	      / *
		default:
		msg.Printf( wxT("%s"), _("Click impossible") );
	    
		wxMessageBox(msg, _("Click"), wxOK | wxICON_INFORMATION, 0);
	      * /
	  }
	}
      }
    }
	*/
  }

  std::pair<int,int> Stone_Panel::get_field_pos( int col, Stones::Stone_Type type ) const
  {
    if( settings.multiple_stones )
      return std::pair<int,int>( x + col * bitmap_handler.dimensions.field_width, 
				 y + (int(type) - 1) * bitmap_handler.dimensions.field_height );
    else
      return std::pair<int,int>( x + (int(type) - 1) * bitmap_handler.dimensions.field_width, y );
  }

  std::pair<int,int> Stone_Panel::get_field_pos( std::pair<Stones::Stone_Type,int> stone ) const
  {
    return get_field_pos( stone.second, stone.first );
  }

  std::pair<Stones::Stone_Type,int> Stone_Panel::get_stone( int cl_x, int cl_y ) const
  {
    if( !is_in(cl_x,cl_y) )
      return std::pair<Stones::Stone_Type,int>(Stones::invalid_stone,-1);

    if( settings.multiple_stones )
    {
      int row = (cl_y - y) / bitmap_handler.dimensions.field_height;
      int col = (cl_x - x) / bitmap_handler.dimensions.field_width;

      if( (row < 0) || (row > 2) || (col < 0) || (col >= settings.max_stones) )
	return std::pair<Stones::Stone_Type,int>(Stones::invalid_stone,-1);

      return std::pair<Stones::Stone_Type,int>( Stones::Stone_Type(row + 1), col );
    }
    else
    {
      int type_num = (cl_x - x) / bitmap_handler.dimensions.field_width;
      return std::pair<Stones::Stone_Type,int>( Stones::Stone_Type(type_num + 1), 0 );
    }
  }

  // ----------------------------------------------------------------------------
  // Player Panel
  // ----------------------------------------------------------------------------

  Player_Panel::Settings::Settings( const relax::Stone_Panel::Settings &stone_settings,
				    const relax::Board_Panel::Settings &board_settings,
				    wxFont player_font )
    : stone_settings(stone_settings), board_settings(board_settings), player_font( player_font )
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
      stone_panel( settings.stone_settings, player.stones, game_manager, gui_manager, sg ),
      board_panel( settings.board_settings, game_manager, gui_manager, sequence_generator, player.id ),
      header_panel( settings, player )
  {
    add( &header_panel );
    Horizontal_Sizer *stone_sizer = new Horizontal_Sizer();
    stone_sizer->add( new Spacer(bitmap_handler.dimensions.stone_offset,0), true /*deleted by sizer*/ );
    stone_sizer->add( &board_panel );
    stone_sizer->add( &stone_panel );
    add( stone_sizer, true /*deleted by sizer*/ );
  }

  void Player_Panel::on_click( int cl_x, int cl_y ) const
  {
    if( board_panel.is_in(cl_x,cl_y) )
      board_panel.on_click( cl_x, cl_y );
    /*
    wxString msg;
    if( sequence_generator )
    {
      std::pair<Stones::Stone_Type,int> clicked_stone = stone_panel.get_stone( cl_x, cl_y );
      if( clicked_stone.first != Stones::invalid_stone )
      {
	Stones::Stone_Type type = clicked_stone.first;
	/ *
	if( clicked_stone.second < player.stones.stone_count[type] )
	{
	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click_player_stone( player.id, type );

	  switch( state )
	  {
	    case Sequence_Generator::finished:
	      gui_manager.set_mark(-1,-1);
	      sequence_generator = 0;
	      game_manager.continue_game();
	      break;
	    case Sequence_Generator::hold_white:
	    case Sequence_Generator::hold_grey:
	    case Sequence_Generator::hold_black:
	    {
	      std::pair<int,int> field = stone_panel.get_field_pos( clicked_stone );
	      gui_manager.set_mark( field.first, field.second );
	      gui_manager.show_user_information( true );
	    }
	    break;
	    case Sequence_Generator::another_click:
	      gui_manager.set_mark(-1,-1);
	      gui_manager.show_user_information( true );
	      break;
	    case Sequence_Generator::fatal_error:
	      msg.Printf( wxT("%s"), _("Fatal Error!") );
	      gui_manager.report_error( msg, _("Click Error") );
	      break;
	    case Sequence_Generator::error_require_knock_out:
	      gui_manager.show_status_text(_("You must do the knock out move!"));
	      break;
	    case Sequence_Generator::error_require_set:
	      gui_manager.show_status_text(_("You have to set a stone!"));
	      break;
	    case Sequence_Generator::error_require_remove:
	      gui_manager.show_status_text(_("You have to remove an empty border field!"));
	      break;
	    case Sequence_Generator::error_can_t_remove:
	      gui_manager.show_status_text(_("Field can't be removed without moving another stone!"));
	      break;
	    case Sequence_Generator::error_can_t_move_here:
	      gui_manager.show_status_text(_("You can't move to that field!"));
	      break;
	    case Sequence_Generator::error_can_t_set_here:
	      gui_manager.show_status_text(_("Please set the stone on an empty field!"));
	      break;
	    case Sequence_Generator::error_must_pick_common_stone:
	      gui_manager.show_status_text(_("You must pick a common stone!"));
	      break;
	    case Sequence_Generator::error_wrong_player:
	      gui_manager.show_status_text(_("Its not the turn of that player!"));
	      break;
	    case Sequence_Generator::error_impossible_yet:
	      gui_manager.show_status_text(_("You can't do that at the moment!"));
	      break;
	    case Sequence_Generator::error_must_knock_out_with_same_stone:
	      gui_manager.show_status_text(_("You must knock out once more with the same stone!"));
	      break;
	      / *
		default:
		msg.Printf( wxT("%s"), _("Click impossible") );
	    
		wxMessageBox(msg, _("Click"), wxOK | wxICON_INFORMATION, 0);
	      * /
	  }
	}
	* /
      }
    }
    */
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
      if( game.get_current_player().help_mode == Player::show_hint )
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
      if( game.get_current_player().help_mode == Player::show_hint )
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
    EVT_COMMAND(-1, wxEVT_RELAX_NOTIFY, Mouse_Handler::on_animation_done)	//**/
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
      //board_panel( settings.board_settings, game_manager, gui_manager, sequence_generator ),
      stone_panel( settings.common_stone_settings, gui_manager.get_display_game().common_stones, 
		   game_manager, gui_manager,sequence_generator )
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

  void Game_Panel::on_right_click( int, int ) const
  {
    if( sequence_generator )
    {
      //Sequence_Generator::Sequence_State state;
      /*state =*/ sequence_generator->undo_click();
      /*	
      switch( state )
      {
	case Sequence_Generator::hold_white:
	case Sequence_Generator::hold_grey:
	case Sequence_Generator::hold_black:
	{
	  Field_Pos pos = sequence_generator->get_selected_pos();
	  std::pair<int,int> coord = board_panel.get_field_pos( pos.x, pos.y );
	  gui_manager.set_mark( coord.first, coord.second );
	  gui_manager.show_user_information( true );
	}
	break;
	case Sequence_Generator::another_click:
	  gui_manager.set_mark(-1,-1);
	  gui_manager.show_user_information( true );
	  break;
	default:
	  assert(false);	// shouldn't be returned by undo_click()
      }
      */
    }
  }

  void Game_Panel::rearrange_panels()
  {
    settings.player_settings.board_settings = settings.board_settings;
    clear();
    switch( settings.arrangement )
    {
      case Settings::arrange_standard:
      {
	Vertical_Sizer *vertical = new Vertical_Sizer();
	//vertical->add( &board_panel );
	vertical->add( new Spacer( 0, bitmap_handler.dimensions.board_stones_spacing ), 
		       true /*destroy on remove*/ );
	vertical->add( &stone_panel );
	add( vertical, true /* destroy on remove */ );
	add( new Spacer( bitmap_handler.dimensions.stones_player_spacing, 0 ), true );
	add( &player_panel_sizer );						     
      }
      break;
      case Settings::arrange_stones_right:
      {
	//add( &board_panel );
	Vertical_Sizer *vertical = new Vertical_Sizer();
	vertical->add( &stone_panel );
	vertical->add( new Spacer( 0, bitmap_handler.dimensions.stones_player_spacing ), true );
	vertical->add( &player_panel_sizer );						     
	add( new Spacer( bitmap_handler.dimensions.board_stones_spacing, 0 ), true );
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
    // !!! todo: maybe we need to update settings.player_settings.board_settings from settings.board_settings
    bool first = player_panels.empty();
    Player_Panel *player_panel = new Player_Panel( settings.player_settings, player, game_manager, 
						   gui_manager, sequence_generator );
    player_panels.push_back( player_panel );
    
    if( !first ) 
      player_panel_sizer.add( new Spacer(0,bitmap_handler.dimensions.player_player_spacing), true );
    player_panel_sizer.add( player_panel );
  }

  void Game_Panel::update_player_panels()
  {
    bool difference = false;
    std::vector<Player>::iterator player = gui_manager.get_display_game().players.begin();
    std::list<Player_Panel*>::iterator panel;
    for( panel = player_panels.begin(); panel != player_panels.end(); ++panel, ++player )
    {
      if( player == gui_manager.get_display_game().players.end() )
      {
	difference = true; break;
      }
      if( &(*player) != &(*panel)->get_player() )
      {
	difference = true; break;
      }
    } 
    if( player != gui_manager.get_display_game().players.end() ) 
      difference = true;
    if( difference )
    {
      // remove and readd player panels
      remove_players();
      for( player = gui_manager.get_display_game().players.begin(); 
	   player != gui_manager.get_display_game().players.end(); ++player )
      {
	add_player( *player );
      }
    }
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
      return get_bitmap_set().background;
    else
      return bitmap_handler.normal.background;
  }

  Bitmap_Set &Game_Panel::get_bitmap_set() const
  {
    if( settings.board_settings.rotate_board && !bitmap_handler.dimensions.rotation_symmetric )
      return bitmap_handler.rotated;
    else
      return bitmap_handler.normal;
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
		     Player_Panel::Settings(Stone_Panel::Settings(),Board_Panel::Settings()),
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

  void WX_GUI_Manager::report_scores( const Game* game, 
				      std::multimap<int/*score*/,const Player*> scores )
  {
    bool activate_math_exercise = true;  // this is a special hack for my friend Samuel
    int math_exercise_max_cnt = 3; // maximum number of questions asked
    wxString msg, line;
    std::multimap<int/*score*/,const Player*>::iterator it;
    for( it=scores.begin(); it!=scores.end(); ++it )
      {
	int score = it->first;
	const Player *player = it->second;
	// math_excercise: ask for help to compute total scores (learn multiplication) 
	if( activate_math_exercise && player->type == Player::user ) 
	{
	  std::list<std::pair<int/*count*/,int/*num*/> > scores 
	    = game->get_max_score_detail(player).second;
	  int cnt = 0;
	  for( std::list<std::pair<int/*count*/,int/*num*/> >::iterator it = scores.begin();
	       it != scores.end(); ++it )
	    {
	      if( ++cnt > math_exercise_max_cnt ) break;
	      int count = it->first;
	      int num = it->second;
	      wxString hint = wxT("");
	      long result;
	      do{
		wxString base_question = _("%s: Please help me calculate %d * %d%s");
		wxString title = _("Calculate Scores");
		wxString question;
		question.Printf( base_question,
				 str_to_wxstr(player->name).c_str(), count, num, 
				 hint.c_str() );
		result = wxGetNumberFromUser( question, /*prompt=*/_("= "), 
					      title,
					      /*value=*/-1, /*min=*/0, /*max=*/1000, 
					      /*parent=*/&game_window );
		if( result != count*num ) 
		  {
		    if( result < count*num ) 
		      {
			hint.Printf( _(" (Hint: the actual result is higher than %d)"), 
				       result);
		      }
		    else
		      {
			hint.Printf( _(" (Hint: the actual result is lower than %d)"), 
				       result);
		      }
		  }
	      }while(result != count*num);
	    }
	}
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
    show_status_text( wxT("") );
  }
  
  void WX_GUI_Manager::show_user_information( bool visible, bool do_refresh )
  {
    Game &game = get_display_game();
    stone_mark_positions.clear();
    field_mark_positions.clear();
    if( visible && game.players.size() )
    {
      switch( game.get_current_player().help_mode )
      {
	case Player::no_help: break;
	case Player::show_possible_moves: 
	{
	  // show possible moves doesn't make sense for relax
	}
	break;
	case Player::show_hint:
	{
	  if( current_hint.valid )
	  {
	    const std::list<relax::Move*> &moves = current_hint.sequence.get_moves();
	    std::list<relax::Move*>::const_iterator move;
	    for( move = moves.begin(); move != moves.end(); ++move )
	    {
	      switch( (*move)->get_type() )
	      {
		case Move::no_move:
		case Move::select_move:
		case Move::finish_move:
		  break;
		case Move::set_move:
		  {
		    Set_Move *set_move = dynamic_cast<Set_Move *>(*move);
		    int player_id = game.get_current_player().id;
		    field_mark_positions.push_back
		      ( game_panel.get_player_panel(player_id)->get_board_panel()
			.get_field_pos( set_move->pos.x, set_move->pos.y ) );
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
	    show_status_text(_("You should set a stone")); // remove old status text message
	    break;
	  case Move::no_move:
	  case Move::select_move:
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
    for( position = stone_mark_positions.begin();
	 position != stone_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_bitmap_set().stone_mark, x, y, true );
    }

    for( position = field_mark_positions.begin();
	 position != field_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( get_game_panel().get_bitmap_set().field_mark, x, y, true );
    }
    if( click_mark_x >= 0 )
      dc.DrawBitmap( get_game_panel().get_bitmap_set().click_mark, 
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
      wxCommandEvent event( wxEVT_RELAX_NOTIFY, event_id );
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
      wxCommandEvent event( wxEVT_RELAX_NOTIFY, event_id );
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
    if( cfg->Read( wxT("/relax/skin_file"), &buf) )
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
	  cfg->Write( wxT("/relax/skin_file"), buf);
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
	  cfg->Write( wxT("/relax/skin_file"), buf);
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
	  cfg->Write( wxT("/relax/skin_file"), buf);
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
      //bitmap_handler.normal.stone_bitmaps[Stones::white_stone]	= wxBitmap(stone_white_xpm);
      //bitmap_handler.normal.stone_bitmaps[Stones::grey_stone]	= wxBitmap(stone_grey_xpm);
      //bitmap_handler.normal.stone_bitmaps[Stones::black_stone]	= wxBitmap(stone_black_xpm);
      bitmap_handler.normal.click_mark				= wxBitmap(click_mark_xpm);
      bitmap_handler.normal.field_mark				= wxBitmap(field_mark_xpm);
      bitmap_handler.normal.stone_mark				= wxBitmap(stone_mark_xpm);
      bitmap_handler.normal.background				= wxBitmap(background_xpm);
      bitmap_handler.dimensions = Dimensions( field_empty_xpm );

      bitmap_handler.setup_field_stone_bitmaps();
      bitmap_handler.setup_rotated_bitmaps();
    }

    ok = false;
    if( cfg->Read( wxT("/relax/beep_file"), &buf) )
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
	  cfg->Write( wxT("/relax/beep_file"), buf);
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
	  cfg->Write( wxT("/relax/beep_file"), buf);
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
	  cfg->Write( wxT("/relax/beep_file"), buf );
	  cfg->Flush();
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )		// disable sound if there is no valid sound file
    {
      game_settings.play_sound = false;
      cfg->Write( wxT("/relax/play_sound"), false );
      cfg->Flush();
    }
    else
    {
      bool play_sound;
      if( cfg->Read( wxT("/relax/play_sound"), &play_sound ) )
      {
	game_settings.play_sound = play_sound;
      }
      else
      {
	game_settings.play_sound = true;
	cfg->Write( wxT("/relax/play_sound"), true );
	cfg->Flush();
      }
    }
  }

  void WX_GUI_Manager::save_settings()
  {
    wxConfigBase* cfg = wxConfig::Get();
    cfg->Write( wxT("/relax/skin_file"),  game_settings.skin_file );
    cfg->Write( wxT("/relax/beep_file"),  game_settings.beep_file );
    cfg->Write( wxT("/relax/play_sound"), game_settings.play_sound );
    cfg->Flush();
  }

  bool WX_GUI_Manager::load_skin( wxString filename )
  {
    show_user_information(false,false);

    Standard_Ruleset ruleset;	// to lookup numbers
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
    for( unsigned dir=0; dir<3; ++dir )
      {
	std::vector<int>::iterator it;
	for( it=ruleset.get_nums(dir).begin(); it!=ruleset.get_nums(dir).end(); ++it )
	  {
	    int num = *it;
	    std::string stone_file = "stone_" + long_to_string(dir) + "_" + 
	      long_to_string(num) + ".png";
	    input = get_zip_input_stream( filename, str_to_wxstr(stone_file) );
	    if( input->Eof() ) { delete input; return false; }
	    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
	    bitmap_handler.normal.num_bitmaps[dir][num] = wxBitmap(image);
	    delete input;
	  }
      }
    input = get_zip_input_stream( filename, wxT("stone_base.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.stone_base = wxBitmap(image);
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


