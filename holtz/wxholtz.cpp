/*
 * wxholtz.cpp
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

#ifdef __WXMSW__
#define DRAW_BACKGROUND
#define DOUBLE_BUFFER
#else
#define DRAW_BACKGROUND
#define DOUBLE_BUFFER
#endif

#ifndef DEFAULT_DATA_DIR
#define DEFAULT_DATA_DIR "./"
#endif
#ifndef DEFAULT_SKIN_FILE
#define DEFAULT_SKIN_FILE DEFAULT_DATA_DIR wxT("skins/hex70.zip")
#endif
#ifndef DEFAULT_BEEP_FILE
#define DEFAULT_BEEP_FILE DEFAULT_DATA_DIR wxT("sounds/beep.wav")
#endif

// compiled in picture set
//#define PURIST_50
//#define HEX_50
//#define HEX_70
//#define HEX_100
#define EINS

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wxholtz.hpp"
#include "holtz.hpp"
#include "dialogs.hpp"
#include "ai.hpp"
#include "util.hpp"
#include "pbm.hpp"

#include <wx/zipstrm.h>
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/dcmemory.h>
#include <assert.h>
/*
#ifndef __OLD_GCC__
  #include <sstream>
#endif
*/
#include <fstream>
#include <list>

// wx html help
#if !wxUSE_HTML
#error "wxHTML is required for the wxWindows HTML help system."
#endif
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/html/helpctrl.h>
#include <wx/cshelp.h>

namespace holtz
{
  // ----------------------------------------------------------------------------
  // resources
  // ----------------------------------------------------------------------------

  // the application icon
#ifndef __WXMSW__
#include "icon.xpm"
#endif

#ifdef PURIST_50
  // --------------------
  // purist pictures 50px

  // field pictures
#include "skins/purist/field_removed.xpm"
#include "skins/purist/field_empty.xpm"
#include "skins/purist/field_white.xpm"
#include "skins/purist/field_grey.xpm"
#include "skins/purist/field_black.xpm"
#include "skins/purist/field_mark.xpm"
#include "skins/purist/field_mark2.xpm"
#include "skins/purist/background.xpm"
  // field dimensions
#include "skins/purist/skin.hpp"
#endif

#ifdef HEX_50
  // -----------------------
  // hexagonal pictures 50px

  // field pictures
#include "skins/hex50/field_removed.xpm"
#include "skins/hex50/field_empty.xpm"
#include "skins/hex50/field_white.xpm"
#include "skins/hex50/field_grey.xpm"
#include "skins/hex50/field_black.xpm"
#include "skins/hex50/field_mark.xpm"
#include "skins/hex50/field_mark2.xpm"
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
#include "skins/hex70/field_white.xpm"
#include "skins/hex70/field_grey.xpm"
#include "skins/hex70/field_black.xpm"
#include "skins/hex70/field_mark.xpm"
#include "skins/hex70/field_mark2.xpm"
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
#include "skins/hex100/field_white.xpm"
#include "skins/hex100/field_grey.xpm"
#include "skins/hex100/field_black.xpm"
#include "skins/hex100/field_mark.xpm"
#include "skins/hex100/field_mark2.xpm"
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
#include "skins/eins/field_white.xpm"
#include "skins/eins/field_grey.xpm"
#include "skins/eins/field_black.xpm"
#include "skins/eins/field_mark.xpm"
#include "skins/eins/field_mark2.xpm"
#include "skins/eins/background.xpm"
  // field dimensions
#include "skins/eins/skin.hpp"
#endif

  // ----------------------------------------------------------------------------
  // event tables and other macros for wxWindows
  // ----------------------------------------------------------------------------

  // the event tables connect the wxWindows events with the functions (event
  // handlers) which process them. It can be also done at run-time, but for the
  // simple menu events like this the static method is much simpler.
  BEGIN_EVENT_TABLE(Main_Frame, wxFrame)				//**/
  EVT_MENU(HOLTZ_NEW_GAME,	  Main_Frame::on_new_game)		//**/
  EVT_MENU(HOLTZ_SETTINGS,	  Main_Frame::on_settings)		//**/
  EVT_MENU(HOLTZ_QUIT,		  Main_Frame::on_quit)			//**/
  EVT_MENU(HOLTZ_HELP_CONTENTS,	  Main_Frame::on_help_contents)		//**/
  EVT_MENU(HOLTZ_HELP_LICENSE,	  Main_Frame::on_help_license)		//**/
  EVT_MENU(HOLTZ_ABOUT,		  Main_Frame::on_about)			//**/
  EVT_CLOSE(Main_Frame::on_close)					//**/
  END_EVENT_TABLE()							//**/

  BEGIN_EVENT_TABLE(Game_Window, wxScrolledWindow)			//**/
  EVT_LEFT_DOWN(Game_Window::on_mouse_event)				//**/
  EVT_RIGHT_DOWN(Game_Window::on_mouse_event)				//**/
#ifdef DRAW_BACKGROUND
  EVT_ERASE_BACKGROUND(Game_Window::on_erase_background)		//**/
#endif
  END_EVENT_TABLE()							//**/

  // ============================================================================
  // implementation
  // ============================================================================

  // ----------------------------------------------------------------------------
  // the application class
  // ----------------------------------------------------------------------------

  // 'Main program' equivalent: the program execution "starts" here
  bool wxHoltz::OnInit()
  {
    randomize();		// initialize random

    wxLocale *loc = new wxLocale();
    loc->Init(wxLANGUAGE_DEFAULT); 
    loc->AddCatalog(wxT("holtz"));      

    SetAppName(wxT("Holtz"));
    SetVendorName(wxT("Martin Trautmann"));
    global_config = new wxConfig(GetAppName());
    wxConfig::Set(global_config);

    // this will link all image libraries, also the unused ones
    // wxInitAllImageHandlers(); // make it possible to load PNG images
    // that's better, we don't use other image formats:
    wxImage::AddHandler(new wxXPMHandler);
    wxImage::AddHandler(new wxPNGHandler);
    // for html help
    wxHelpControllerHelpProvider* provider = new wxHelpControllerHelpProvider;
    provider->SetHelpController(&get_help_controller());
    wxHelpProvider::Set(provider);
    // for help: zip files
    wxFileSystem::AddHandler(new wxZipFSHandler);
    if(!init_help(*loc))
      wxLogWarning(_("No help file found."));

    check_config(); // asks for configuration if not yet done

    // create the main application window
    Main_Frame *frame = new Main_Frame( _("Holtz") );

    // and show it (the frames, unlike simple controls, are not shown when
    // created initially)
    frame->Show(true);

    SetTopWindow(frame);
    SetExitOnFrameDelete(true);

    // success: wxApp::OnRun() will be called which will enter the main message
    // loop and the application will run. If we returned FALSE here, the
    // application would exit immediately.
    return TRUE;
  }

  bool wxHoltz::init_help(wxLocale& loc)
  {
	// try to load the HTML help file, in decreasing order:
	// - help/help_la_co.htb
	// - help/help_la.htb
	// - data_dir/help/help_la_co.htb
	// - data_dir/help/help_la.htb
	// - help/help_en.htb
	// - data_dir/help/help_en.htb
	// where 'la' is the language, and 'co' the country of the currently used locale
	// if all fails, return false
	wxString language = loc.GetCanonicalName();
	if(!get_help_controller().Initialize(wxT("help/help_") + language))
	  if(language.Len() <= 2 || !get_help_controller().Initialize(wxT("help/help_") + language.Left(2)))
	    if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
						 + wxT("help/help_") + language))
	      if(language.Len() <= 2 || !get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
									  + wxT("help/help_") 
									  + language.Left(2)))
		if(!get_help_controller().Initialize(wxT("help/help_en")))
		  if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
						       + wxT("help/help_en")))
	  	  return false;
	return true;
  }

  bool wxHoltz::check_config()
  {
    return true;
  }

  wxHoltz::~wxHoltz()
  {
    // doesn't work? seems to be already deleted
    //delete global_config; // writes things back
  }

  // ----------------------------------------------------------------------------
  // Dimensions
  // ----------------------------------------------------------------------------

  Dimensions::Dimensions()
    : field_x(0), field_y(0), field_width(51), field_height(51), 
      field_packed_width(44), field_packed_height(44),
      caption_height(30), stone_offset(20), board_x_offset(10), board_y_offset(10),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotate_symmetric(false)
  {
  }

  Dimensions::Dimensions( char **field_xpm )
    : field_x(0), field_y(0), caption_height(30), stone_offset(20),
      board_x_offset(10), board_y_offset(10),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotate_symmetric(false)
  {
    wxBitmap bitmap(field_xpm);
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)
    field_packed_width  = int(field_width  * 0.866);	// height * cos(30deg)
  }

  Dimensions::Dimensions( wxConfigBase &config, wxBitmap &bitmap )	// load from configuration
    : field_x(0), field_y(0), caption_height(30), stone_offset(20),
      board_x_offset(10), board_y_offset(10),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotate_symmetric(false)
  {
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)
    field_packed_width  = int(field_width  * 0.866);	// height * cos(30deg)

    int buf;
    bool bool_buf;

    if( config.Read( wxT("field_x"), &buf ) ) field_x = buf;
    if( config.Read( wxT("field_y"), &buf ) ) field_y = buf;
    if( config.Read( wxT("field_width"), &buf ) ) field_width = buf;
    if( config.Read( wxT("field_height"), &buf ) ) field_height = buf;
    if( config.Read( wxT("field_packed_width"), &buf ) ) field_packed_width = buf;
    if( config.Read( wxT("field_packed_height"), &buf ) ) field_packed_height = buf;
    if( config.Read( wxT("caption_height"), &buf ) ) caption_height = buf;
    if( config.Read( wxT("stone_offset"), &buf ) ) stone_offset = buf;
    if( config.Read( wxT("board_stones_spacing"), &buf ) ) board_stones_spacing = buf;
    if( config.Read( wxT("player_player_spacing"), &buf ) ) player_player_spacing = buf;
    if( config.Read( wxT("stones_player_spacing"), &buf ) ) stones_player_spacing = buf;
    if( config.Read( wxT("rotate_symmetric"), &bool_buf ) ) rotate_symmetric = bool_buf;
  }

  // ----------------------------------------------------------------------------
  // Bitmap_Handler
  // ----------------------------------------------------------------------------

  void Bitmap_Handler::setup_field_stone_bitmaps()
  {
    std::map<Stones::Stone_Type,wxBitmap>::iterator stone_bitmap;
    std::pair<Field_State_Type,wxBitmap> field_pair;
    for( stone_bitmap =  normal.stone_bitmaps.begin();
	 stone_bitmap != normal.stone_bitmaps.end(); ++stone_bitmap )
    {
      const Stones::Stone_Type &stone_type = stone_bitmap->first;

      wxImage field_stone_image (normal.field_bitmaps[field_empty]);
      wxImage stone_image (stone_bitmap->second);
      unsigned char *field_stone_data = field_stone_image.GetData();
      unsigned char *stone_data       = stone_image.GetData();
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
	  bool masked = true;
	  for( int c = 0; c < 3; ++c )
	  {
	    if( mask_colour[c] != stone_data[(x+y*stone_image.GetWidth())*3 + c] )
	    {
	      masked = false;
	      break;
	    }
	  }
	  if( !masked )
	  {
	    for( int c = 0; c < 3; ++c )
	    {
	      field_stone_data[(x+y*field_stone_image.GetWidth())*3 + c] = 
		stone_data[(x+y*stone_image.GetWidth())*3 + c];
	    }
	  }
	}
      Field_State_Type field_type = Field_State_Type(stone_type);
      assert( Board::is_stone( field_type ) );
      normal.field_bitmaps[field_type] = field_stone_image.ConvertToBitmap();

      field_stone_image.Destroy();
      stone_image.Destroy();
    }
  }

  void Bitmap_Handler::setup_rotated_bitmaps()
  {
    if( !dimensions.rotate_symmetric )
    {
      // rotate field_bitmaps
      rotated.field_bitmaps.clear();
      std::map<Field_State_Type,wxBitmap>::iterator field_bitmap;
      for( field_bitmap =  normal.field_bitmaps.begin();
	   field_bitmap != normal.field_bitmaps.end(); ++field_bitmap )
      {
	wxBitmap &normal_bitmap = field_bitmap->second;
	const Field_State_Type &type = field_bitmap->first;
      wxImage normal_image = wxImage(normal_bitmap).Rotate90( false /*counter clockwise*/ );
      rotated.field_bitmaps.insert
	(std::pair<Field_State_Type,wxBitmap>(type, normal_image.ConvertToBitmap()));
      }
      // rotate stone_bitmaps
      rotated.stone_bitmaps.clear();
      std::map<Stones::Stone_Type,wxBitmap>::iterator stone_bitmap;
      for( stone_bitmap =  normal.stone_bitmaps.begin();
	   stone_bitmap != normal.stone_bitmaps.end(); ++stone_bitmap )
      {
	wxBitmap &normal_bitmap = stone_bitmap->second;
	const Stones::Stone_Type &type = stone_bitmap->first;
	wxImage normal_image = wxImage(normal_bitmap).Rotate90( false /*counter clockwise*/ );
	rotated.stone_bitmaps.insert
	  (std::pair<Stones::Stone_Type,wxBitmap>(type, normal_image.ConvertToBitmap()));
      }
    }
  }
  
  // ----------------------------------------------------------------------------
  // Basic Panel
  // ----------------------------------------------------------------------------

  Basic_Panel::Basic_Panel()
    : x(-1), y(-1), width(-1), height(-1)
  {
  }
  Basic_Panel::~Basic_Panel()
  {
  }

  // ----------------------------------------------------------------------------
  // Spacer
  // ----------------------------------------------------------------------------

  Spacer::Spacer( int _width, int _height )
  {
    width = _width;
    height = _height;
  }

  // ----------------------------------------------------------------------------
  // Horizontal_Sizer
  // ----------------------------------------------------------------------------

  Horizontal_Sizer::Horizontal_Sizer( Align_Type align )
    : align(align)
  {
  }

  Horizontal_Sizer::~Horizontal_Sizer()
  {
    clear();
  }

  void Horizontal_Sizer::add( Basic_Panel *panel, bool destroy_on_remove )
  {
    elements.push_back( std::pair<Basic_Panel*,bool>(panel, destroy_on_remove) );
  }
  void Horizontal_Sizer::erase( Basic_Panel *panel )
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->first == panel )
      {
	if( element->second )
	  delete panel;

	elements.erase(element);
	break;
      }
    }
    calc_dimensions();
  }
  void Horizontal_Sizer::clear()
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->second )
	delete element->first;
    }
    elements.clear();
    width = 0;
    height = 0;
  }

  void Horizontal_Sizer::draw( wxDC &dc ) const
  {
    //wxBrush brush1(*wxBLACK,wxTRANSPARENT);
    //wxBrush brush2(*wxRED,wxTRANSPARENT);
    //dc.SetBrush(brush1);
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      //dc.DrawRectangle( element->first->get_x(), element->first->get_y(), 
      //		element->first->get_width(), element->first->get_height() );
      element->first->draw(dc);
    }
    //dc.SetBrush(brush2);
    //dc.DrawRectangle( get_x(), get_y(), get_width(), get_height() );
  }

  void Horizontal_Sizer::draw_text( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->draw_text(dc);
    }
  }
  
  void Horizontal_Sizer::on_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_x < element->first->get_x() + element->first->get_width() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }
  void Horizontal_Sizer::calc_dimensions()
  {
    int cur_width = 0;
    int cur_height = 0;
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->set_x( x + cur_width );
      element->first->calc_dimensions();
      if( element->first->get_height() > cur_height )
	cur_height = element->first->get_height();
      cur_width += element->first->get_width();
    }
    width = cur_width;
    height = cur_height;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      switch(align)
      {
	case top:    element->first->set_y(y); break;
	case bottom: element->first->set_y(y +  height - element->first->get_height()); break;
	case middle: element->first->set_y(y + (height - element->first->get_height()) / 2); break;
      }
      element->first->calc_dimensions();
    }
  }

  // ----------------------------------------------------------------------------
  // Vertical_Sizer
  // ----------------------------------------------------------------------------

  Vertical_Sizer::Vertical_Sizer( Align_Type align )
    : align(align)
  {
  }

  Vertical_Sizer::~Vertical_Sizer()
  {
    clear();
  }

  void Vertical_Sizer::add( Basic_Panel *panel, bool destroy_on_remove )
  {
    elements.push_back( std::pair<Basic_Panel*,bool>(panel, destroy_on_remove) );
  }
  void Vertical_Sizer::erase( Basic_Panel *panel )
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->first == panel )
      {
	if( element->second )
	  delete panel;

	elements.erase(element);
	break;
      }
    }
    calc_dimensions();
  }
  void Vertical_Sizer::clear()
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->second )
	delete element->first;
    }
    elements.clear();
    width = 0;
    height = 0;
  }

  void Vertical_Sizer::draw( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      //dc.DrawRectangle( element->first->get_x(), element->first->get_y(), 
      //		  element->first->get_width(), element->first->get_height() );
      element->first->draw(dc);
    }
    //dc.DrawRectangle( get_x(), get_y(), get_width(), get_height() );
  }

  void Vertical_Sizer::draw_text( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->draw_text(dc);
    }
  }

  void Vertical_Sizer::on_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_y < element->first->get_y() + element->first->get_height() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }
  void Vertical_Sizer::calc_dimensions()
  {
    int cur_width = 0;
    int cur_height = 0;
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->set_y( y + cur_height );
      element->first->calc_dimensions();
      if( element->first->get_width() > cur_width )
	cur_width = element->first->get_width();
      cur_height += element->first->get_height();
    }
    width = cur_width;
    height = cur_height;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      switch(align)
      {
	case left:   element->first->set_x(x); break;
	case right:  element->first->set_x(x +  width - element->first->get_width()); break;
	case center: element->first->set_x(x + (width - element->first->get_width()) / 2); break;
      }
      element->first->calc_dimensions();
    }
  }

  // ----------------------------------------------------------------------------
  // Board_Panel
  // ----------------------------------------------------------------------------

  wxFont Board_Panel::Settings::default_font(20,wxMODERN,wxNORMAL,wxNORMAL);

  Board_Panel::Settings::Settings( bool rotate_board, bool show_coordinates, wxFont coord_font )
    : rotate_board(rotate_board), show_coordinates(show_coordinates), coord_font(coord_font)
  {
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
	if( bitmap_handler.dimensions.rotate_symmetric )
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
	if( bitmap_handler.dimensions.rotate_symmetric )
	  board_y += bitmap_handler.dimensions.field_width;
	else
	  board_y += bitmap_handler.dimensions.field_height;
      }
      else
	board_x += bitmap_handler.dimensions.field_width;
    }

    if( settings.rotate_board )
    {
      if( bitmap_handler.dimensions.rotate_symmetric )
      {
	width = bitmap_handler.dimensions.field_packed_width * (game.board.get_y_size()-1) + 
	  bitmap_handler.dimensions.field_width + board_x; 
	height = bitmap_handler.dimensions.field_height * game.board.get_x_size() +
	  bitmap_handler.dimensions.field_height / 2 + board_y; 
	
	if( settings.show_coordinates )
	  height += bitmap_handler.dimensions.field_height;
      }
      else
      {
	width = bitmap_handler.dimensions.field_packed_height * (game.board.get_y_size()-1) + 
	  bitmap_handler.dimensions.field_height + board_x; 
	height = bitmap_handler.dimensions.field_width * game.board.get_x_size() +
	  bitmap_handler.dimensions.field_width / 2 + board_y; 
	
	if( settings.show_coordinates )
	  height += bitmap_handler.dimensions.field_width;
      }
    }
    else
    {
      width = bitmap_handler.dimensions.field_width * game.board.get_x_size() + board_x
	+ bitmap_handler.dimensions.field_width / 2;
      height = bitmap_handler.dimensions.field_packed_height * (game.board.get_y_size()-1)  
	+ bitmap_handler.dimensions.field_height + board_y;

      if( settings.show_coordinates )
	width += bitmap_handler.dimensions.field_width;
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
      wxFont font(20,wxDECORATIVE,wxNORMAL,wxNORMAL);
      dc.SetFont( font /*settings.coordinate_font*/ );
      for( int fy = 0; fy < game.board.get_y_size(); ++fy )
      {
	for( int fx = 0; fx < game.board.get_x_size(); ++fx )
	{
	  if( !Board::is_removed(game.board.field[fx][fy]) )
	  {
	    Field_Pos pos;
	    pos.x = fx;
	    pos.y = fy;
	    std::string coord_name = game.ruleset->get_coordinate_translator()->get_field_name( pos );
	    wxCoord w,h;
	    dc.GetTextExtent( str_to_wxstr(coord_name), &w, &h );
	    std::pair<int,int> field = get_field_pos( fx, fy );
	    int coord_x, coord_y;
	    if( settings.rotate_board )
	    {
	      if( bitmap_handler.dimensions.rotate_symmetric )
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
	      if( bitmap_handler.dimensions.rotate_symmetric )
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
	state = sequence_generator->add_click( pos );
	gui_manager.reset();
	
	switch( state )
	{
	case Sequence_Generator::finished:
	  sequence_generator = 0;
	  game_manager.continue_game();
	  break;
	case Sequence_Generator::hold_white:
	case Sequence_Generator::hold_grey:
	case Sequence_Generator::hold_black:
	{
	  std::pair<int,int> coord = get_field_pos( pos.x, pos.y );
	  gui_manager.set_mark( coord.first, coord.second );
	  gui_manager.show_user_information( true );
	}
	break;
	case Sequence_Generator::another_click:
	  gui_manager.show_user_information( true );
	  break;
	case Sequence_Generator::fatal_error:
	  msg.Printf( _("Fatal Error!") );
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
    Game &game = gui_manager.get_display_game();
    int rel_x, rel_y;
    if( settings.rotate_board )
    {
      // swap x-y coordinates
      rel_x = abs_y - y - board_y;
      rel_y = abs_x - x - board_x;
      // invert y-axis => rotated board
      int board_size_y;
      if( bitmap_handler.dimensions.rotate_symmetric )
      {
	board_size_y = bitmap_handler.dimensions.field_height * game.board.get_x_size() 
	  + bitmap_handler.dimensions.field_height / 2;
      }
      else
      {
	board_size_y = bitmap_handler.dimensions.field_width * game.board.get_x_size() 
	  + bitmap_handler.dimensions.field_width / 2;
      }
      rel_x = board_size_y - rel_x - 1;
    }
    else
    {
      rel_x = abs_x - x - board_x;
      rel_y = abs_y - y - board_y;
    }
    
    int half_diff = (bitmap_handler.dimensions.field_height - 
		     bitmap_handler.dimensions.field_packed_height) / 2;
    int row = (rel_y - half_diff) / bitmap_handler.dimensions.field_packed_height;
    int offset = (row & 1) ? 0 : bitmap_handler.dimensions.field_width / 2;
    
    if( (rel_x >= offset) && (rel_y >= 0 ) )
    {
      int col = (rel_x - offset) / bitmap_handler.dimensions.field_width;
      
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
      if( bitmap_handler.dimensions.rotate_symmetric )
      {
	if( row & 1 )		// even row
	  offset = bitmap_handler.dimensions.field_height / 2;
	else
	  offset = 0;
	int anf_x = x + board_x - bitmap_handler.dimensions.field_x;
	int anf_y = y + board_y - bitmap_handler.dimensions.field_y + offset;

	// swap x-y and invert y-axis => rotated board
	int field_y = anf_y
	  + bitmap_handler.dimensions.field_height * (game.board.get_x_size() - 1 - col);
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
	  + bitmap_handler.dimensions.field_width * (game.board.get_x_size() - 1 - col);
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
    if( settings.rotate_board && !bitmap_handler.dimensions.rotate_symmetric )
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
    if( settings.multiple_stones )
    {
      width  = bitmap_handler.dimensions.field_width * settings.max_stones;
      height = 3 * bitmap_handler.dimensions.field_height;
    }
    else
    {
      width  = 3 * bitmap_handler.dimensions.field_width;
      height = bitmap_handler.dimensions.field_height;
    }
  }
  
  void Stone_Panel::draw( wxDC &dc ) const
  {
    int stone_type;
    int y_pos = y, x_pos = x;
    for( stone_type = Stones::white_stone; stone_type <= Stones::black_stone; ++stone_type )
    {
      if( settings.multiple_stones )
      {
	int count = stones.stone_count[ Stones::Stone_Type(stone_type) ];
	if( count > settings.max_stones ) count = settings.max_stones;
	
	x_pos = x;
	int i;
	for( i = 0; i < count; ++i )
	{
	  if( settings.rotate_stones && !bitmap_handler.dimensions.rotate_symmetric )
	  {
	    dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ field_empty ], 
			   x_pos, y_pos, true );
	    dc.DrawBitmap( bitmap_handler.rotated.stone_bitmaps[ Stones::Stone_Type(stone_type) ], 
			   x_pos, y_pos, true );
	  }
	  else
	    dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ Field_State_Type(stone_type) ], 
			   x_pos, y_pos, true );
	  
	  x_pos += bitmap_handler.dimensions.field_width;
	}
	for( ; i < settings.max_stones; ++i )
	{
	  dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ field_removed ], x_pos, y_pos, true );
	  x_pos += bitmap_handler.dimensions.field_width;
	}
      
	y_pos += bitmap_handler.dimensions.field_height;
      }
      else			// display only one stone and write count as text on it
      {
	if( settings.rotate_stones && !bitmap_handler.dimensions.rotate_symmetric )
	{
	  dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ field_empty ], 
			 x_pos, y_pos, true );
	  dc.DrawBitmap( bitmap_handler.rotated.stone_bitmaps[ Stones::Stone_Type(stone_type) ], 
			 x_pos, y_pos, true );
	}
	else
	  dc.DrawBitmap( bitmap_handler.normal.field_bitmaps[ Field_State_Type(stone_type) ], 
			 x_pos, y_pos, true );

	x_pos += bitmap_handler.dimensions.field_width;
      }
    }
  }

  void Stone_Panel::draw_text( wxDC &dc ) const
  {
    if( !settings.multiple_stones ) // display only one stone and write count as text on it
    {
      int stone_type;
      int y_pos = y, x_pos = x;
      for( stone_type = Stones::white_stone; stone_type <= Stones::black_stone; ++stone_type )
      {
	int count = stones.stone_count[ Stones::Stone_Type(stone_type) ];

	wxString str;
	str.Printf("%d", count);

	dc.SetTextForeground(*wxRED/*settings.stone_font_colour*/);
	wxFont font(20,wxDECORATIVE,wxNORMAL,wxNORMAL);
	dc.SetFont(font/*settings.stone_font*/);
	wxCoord w,h;
	dc.GetTextExtent(str,&w,&h);

	dc.DrawText( str, 
		     x_pos + (bitmap_handler.dimensions.field_width  - w) / 2 +
		     bitmap_handler.dimensions.field_x, 
		     y_pos + (bitmap_handler.dimensions.field_height - h) / 2 +
		     bitmap_handler.dimensions.field_y );

	x_pos += bitmap_handler.dimensions.field_width;
      }
    }
  }
  
  void Stone_Panel::on_click( int click_x, int click_y ) const
  {
    wxString msg;
    if( sequence_generator )
    {
      std::pair<Stones::Stone_Type,int> clicked_stone = get_stone( click_x, click_y );
      Stones::Stone_Type &type( clicked_stone.first );
      int &col = clicked_stone.second;

      if( type != Stones::invalid_stone )
      {
	if( col < stones.stone_count[type] )
	{
	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click_common_stone( type );
	  gui_manager.reset();
	  
	  switch( state )
	  {
	    case Sequence_Generator::finished:
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
	      gui_manager.show_user_information( true );
	      break;
	    case Sequence_Generator::fatal_error:
	      msg.Printf( _("Fatal Error!") );
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
	      /*
		default:
		msg.Printf( _("Click impossible") );
	    
		wxMessageBox(msg, _("Click"), wxOK | wxICON_INFORMATION, 0);
	      */
	  }
	}
      }
    }
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

  wxFont Player_Panel::Settings::default_font( 20, wxDECORATIVE, wxNORMAL, wxNORMAL );

  Player_Panel::Settings::Settings( const holtz::Stone_Panel::Settings &stone_settings,
				    wxFont player_font )
    : stone_settings(stone_settings), player_font( player_font )
  {
  }
				   
  
  Player_Panel::Player_Panel( Settings &settings, Player &player, Game_Manager &game_manager,
			      WX_GUI_Manager &gui_manager, Sequence_Generator* &sg )
    : settings(settings), player(player), 
      game_manager(game_manager), gui_manager(gui_manager), 
      bitmap_handler(gui_manager.get_bitmap_handler()), 
      sequence_generator(sg), 
      stone_panel( settings.stone_settings, player.stones, game_manager, gui_manager, sg ),
      header_panel( settings, player )
  {
    add( &header_panel );
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
      std::pair<Stones::Stone_Type,int> clicked_stone = stone_panel.get_stone( cl_x, cl_y );
      if( clicked_stone.first != Stones::invalid_stone )
      {
	Stones::Stone_Type type = clicked_stone.first;

	if( clicked_stone.second < player.stones.stone_count[type] )
	{
	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click_player_stone( player.id, type );
	  gui_manager.reset();

	  switch( state )
	  {
	    case Sequence_Generator::finished:
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
	      gui_manager.show_user_information( true );
	      break;
	    case Sequence_Generator::fatal_error:
	      msg.Printf( _("Fatal Error!") );
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
	      /*
		default:
		msg.Printf( _("Click impossible") );
	    
		wxMessageBox(msg, _("Click"), wxOK | wxICON_INFORMATION, 0);
	      */
	  }
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
    wxFont font(20,wxDECORATIVE,wxNORMAL,wxNORMAL);
    dc.SetFont(font/*settings.player_font*/);
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
    wxFont font(20,wxDECORATIVE,wxNORMAL,wxNORMAL);
    dc.SetFont(font/*settings.player_font*/);
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
    sequence_generator.reset();
    sequence_generator_hook = &sequence_generator;

    stop_watch.Start();
    gui_manager.allow_user_activity();
    if( game.current_player->help_mode == Player::show_hint )
    {
      ai = game_manager.get_hint_ai();
      ai->determine_hints();
    }
  }
  void Mouse_Handler::disable_mouse_input() 
  {
    Game &game = gui_manager.get_display_game();
    sequence_generator_hook = 0;

    used_time = stop_watch.Time();
    if( game.current_player->help_mode == Player::show_hint )
    {
      ai->abort();
      gui_manager.remove_hint();
    }
    gui_manager.stop_user_activity();
  }
  long Mouse_Handler::get_used_time()
  {
    return used_time;
  }

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
      board_panel( settings.board_settings, game_manager, gui_manager, sequence_generator ),
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

  void Game_Panel::rearrange_panels()
  {
    clear();
    switch( settings.arrangement )
    {
      case Settings::arrange_standard:
      {
	Vertical_Sizer *vertical = new Vertical_Sizer();
	vertical->add( &board_panel );
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
	add( &board_panel );
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

  // ----------------------------------------------------------------------------
  // WX_GUI_Manager
  // ----------------------------------------------------------------------------

  WX_GUI_Manager::WX_GUI_Manager( Game_Manager& game_manager, Game_Window &game_window )
    : Game_UI_Manager( game_manager.get_game() ),
      game_manager(game_manager), game_window(game_window),
      default_font(20, wxDECORATIVE, wxNORMAL, wxNORMAL),
      game_settings( Board_Panel::Settings( true, true, default_font ),
		     Player_Panel::Settings( Stone_Panel::Settings( true ), default_font ),
		     Stone_Panel::Settings( true ) ),
      game_panel( game_settings, game_manager, *this, sequence_generator ), 
      sequence_generator(0),
      field_mark_x(-1), 
      move_animation( new Move_Sequence_Animation(*this,game_window) ),
      mouse_handler( game_manager, *this, sequence_generator )
  {
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

  void WX_GUI_Manager::setup_game_display() // setup all windows to display game
  {
    game_panel.remove_players();
    std::list<Player>::iterator i;
    for( i = get_display_game().players.begin(); i != get_display_game().players.end(); ++i )
    {
      game_panel.add_player( *i );
    }
    calc_dimensions();
  }

  void WX_GUI_Manager::set_board( const Game &new_game )
  {
    display_game = new_game;
    refresh();
  }

  void WX_GUI_Manager::update_board( const Game &new_game )
  {
    set_board( new_game );
  }

  void WX_GUI_Manager::report_winner( Player *player )
  {
    if( player )
    {
      wxString msg;
      msg.Printf( _("And the winner is: %s"), player->name.c_str() );
      report_information( msg, _("Winner") );
    }
    else
    {
      report_information( _("It seems that nobody could win the game."), _("Winner") );
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
    field_mark_positions.clear();
    field_mark2_positions.clear();
    if( visible )
    {
      switch( game.current_player->help_mode )
      {
	case Player::no_help: break;
	case Player::show_possible_moves: 
	{
	  if( sequence_generator )
	  {
	    if( sequence_generator->get_required_move_type() != Move::set_move )
	    {
	      std::list<Field_Pos> clicks = sequence_generator->get_possible_clicks();
	      std::list<Field_Pos>::iterator click;
	      for( click = clicks.begin(); click != clicks.end(); ++click )
	      {
		field_mark2_positions.push_back
		  ( game_panel.get_board_panel().get_field_pos( click->x, click->y ) );
	      }
	    }
	  }
	}
	break;
	case Player::show_hint:
	{
	  if( current_hint.valid )
	  {
	    const std::list<holtz::Move*> &moves = current_hint.sequence.get_moves();
	    std::list<holtz::Move*>::const_iterator move;
	    for( move = moves.begin(); move != moves.end(); ++move )
	    {
	      switch( (*move)->get_type() )
	      {
	      case Move::no_move:
	      case Move::finish_move:
		break;
	      case Move::knock_out_move:
	      {
		Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move *>(*move);
		field_mark2_positions.push_back
		  ( game_panel.get_board_panel().get_field_pos( knock_move->from.x, knock_move->from.y ) );
		field_mark2_positions.push_back
		  ( game_panel.get_board_panel().get_field_pos( knock_move->to.x, knock_move->to.y ) );
	      }
	      break;
	      case Move::set_move:
	      {
		Set_Move *set_move = dynamic_cast<Set_Move *>(*move);
		if( !set_move->own_stone )
		{
		  int stone_count = game.common_stones.stone_count[ set_move->stone_type ];
		  if( stone_count > 0 )
		  {
		    field_mark2_positions.push_back
		      ( game_panel.get_stone_panel().get_field_pos( stone_count-1, set_move->stone_type ) );
		  }
		}
		else
		{
		  std::list<Player_Panel*>::const_iterator panel;
		  for( panel = game_panel.get_player_panels().begin(); 
		       panel != game_panel.get_player_panels().end(); ++panel )
		  {
		    if( (*panel)->get_id() == game.current_player->id )
		    {
		      int stone_count = game.current_player->stones.stone_count[ set_move->stone_type ];
		      if( stone_count > 0 )
		      {
			field_mark2_positions.push_back
			  ( (*panel)->get_stone_panel().get_field_pos( stone_count - 1, 
								       set_move->stone_type ) );
		      }
		      break;
		    }
		  }
		}
		field_mark2_positions.push_back
		  ( game_panel.get_board_panel().get_field_pos( set_move->pos.x, set_move->pos.y ) );
	      }
	      break;
	      case Move::remove:
	      {
		Remove *remove = dynamic_cast<Remove *>(*move);
		field_mark_positions.push_back
		  ( game_panel.get_board_panel().get_field_pos(remove->remove_pos.x,remove->remove_pos.y) );
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
	  case Move::knock_out_move:
	    show_status_text(_("Please do a knock out move")); // remove old status text message
	    break;
	  case Move::set_move:
	    show_status_text(_("You should set a stone")); // remove old status text message
	    break;
	  case Move::remove:
	    show_status_text(_("Just remove a field")); // remove old status text message
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
    field_mark_x = x;
    field_mark_y = y;

    wxClientDC dc(&game_window);
    dc.BeginDrawing();
    game_window.PrepareDC(dc);
    draw_mark(dc);
    dc.EndDrawing();
  }

  void WX_GUI_Manager::draw_mark( wxDC &dc )
  {
    std::list< std::pair<int,int> >::iterator position;
    for( position = field_mark2_positions.begin();
	 position != field_mark2_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( bitmap_handler.field_mark2, x, y, true );
    }

    for( position = field_mark_positions.begin();
	 position != field_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( bitmap_handler.field_mark, x, y, true );
    }

    if( field_mark_x >= 0 )
      dc.DrawBitmap( bitmap_handler.field_mark, field_mark_x, field_mark_y, true );
  }

  void WX_GUI_Manager::allow_user_activity()
  {
    show_user_information( true );
    beep();			// tell user that his activity is recommended
  }
  void WX_GUI_Manager::stop_user_activity()
  {
    show_user_information( false, false );
  }

  void WX_GUI_Manager::do_move_slowly( Sequence sequence, wxEvtHandler *done_handler, 
				       int event_id  ) // show user how move is done
  {
    Game &game = get_display_game();
    assert( sequence.check_sequence( game ) );
    bool ret = move_animation->start( sequence, game, done_handler, event_id );
    //!!! should be handled easier in the future !!!
    assert( ret );
  }

  void WX_GUI_Manager::show_status_text( wxString text ) // shows text in status bar
  {
    game_window.show_status_text( text );
  }

  void WX_GUI_Manager::beep()
  {
#if wxUSE_WAVE
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

    if( cfg->Read( wxT("skin_file"), &buf) )
    {
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
      buf = wxT(DEFAULT_SKIN_FILE);
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  cfg->Write( wxT("skin_file"), buf);
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
			    wxOPEN );
      if( wxFileExists(buf) )
      {
	if( load_skin( buf ) )
	{
	  cfg->Write( wxT("skin_file"), buf);
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
      bitmap_handler.normal.stone_bitmaps[Stones::white_stone]	= wxBitmap(field_white_xpm);
      bitmap_handler.normal.stone_bitmaps[Stones::grey_stone]	= wxBitmap(field_grey_xpm);
      bitmap_handler.normal.stone_bitmaps[Stones::black_stone]	= wxBitmap(field_black_xpm);
      bitmap_handler.field_mark					= wxBitmap(field_mark_xpm);
      bitmap_handler.field_mark2				= wxBitmap(field_mark2_xpm);
      bitmap_handler.background					= wxBitmap(background_xpm);
      bitmap_handler.dimensions = Dimensions( field_black_xpm );

      bitmap_handler.setup_field_stone_bitmaps();
      bitmap_handler.setup_rotated_bitmaps();
    }

    ok = false;
    if( cfg->Read( wxT("beep_file"), &buf) )
    {
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
      buf = wxT(DEFAULT_BEEP_FILE);
      if( wxFileExists(buf) )
      {
	if( load_beep(buf) )
	{
	  cfg->Write( wxT("beep_file"), buf);
	  cfg->Flush();
	  game_settings.beep_file = buf;
	  ok = true;
	}
      }
    }
    if( !ok )
    {
      if( load_beep(buf) )
      {
	cfg->Write( wxT("beep_file"), buf );
	cfg->Flush();
	game_settings.beep_file = buf;
	ok = true;
      }
    }
    if( !ok )		// disable sound if there is no valid sound file
    {
      game_settings.play_sound = false;
      cfg->Write( wxT("play_sound"), false );
      cfg->Flush();
    }
    else
    {
      bool play_sound;
      if( cfg->Read( wxT("play_sound"), &play_sound ) )
      {
	game_settings.play_sound = play_sound;
      }
      else
      {
	game_settings.play_sound = true;
	cfg->Write( wxT("play_sound"), true );
	cfg->Flush();
      }
    }
  }

  void WX_GUI_Manager::save_settings()
  {
    wxConfigBase* cfg = wxConfig::Get();
    cfg->Write( wxT("skin_file"),  game_settings.skin_file );
    cfg->Write( wxT("beep_file"),  game_settings.beep_file );
    cfg->Write( wxT("play_sound"), game_settings.play_sound );
    cfg->Flush();
  }

  bool WX_GUI_Manager::load_skin( wxString filename )
  {
    wxZipInputStream *input;
    wxImage image;

    input = new wxZipInputStream( filename, wxT("field_removed.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_removed] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_empty.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.field_bitmaps[field_empty] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_white.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.stone_bitmaps[Stones::white_stone] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_grey.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.stone_bitmaps[Stones::grey_stone] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_black.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.normal.stone_bitmaps[Stones::black_stone] = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("field_mark.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.field_mark = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_mark2.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.field_mark2 = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("background.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmap_handler.background = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("skin.ini") );
    //if( input->Eof() ) { delete input; return false; } // ini file might be empty
    wxFileConfig config( *input );
    bitmap_handler.dimensions = Dimensions( config, bitmap_handler.normal.field_bitmaps[field_empty] );
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
#if wxUSE_WAVE
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

  void WX_GUI_Manager::mouse_click_right( int, int )
  {
    if( sequence_generator )
    {
      delete sequence_generator->undo_click();
    }
    set_mark(-1,-1);
    show_status_text(wxT("")); // remove old status text message
    refresh();
  }
  
  void WX_GUI_Manager::refresh()
  {
    game_window.refresh();
  }

  // ----------------------------------------------------------------------------
  // Game Window
  // ----------------------------------------------------------------------------

  Game_Window::Game_Window( wxFrame *parent_frame )
    : wxScrolledWindow( parent_frame ),
      parent_frame(*parent_frame),
      gui_manager( game_manager, *this ),
      game_dialog( new Game_Dialog(this, game_manager, gui_manager) )
  {
    game_manager.set_ui_manager( &gui_manager );

    SetBackgroundColour(*wxWHITE);
  }

  Game_Window::~Game_Window()
  {
    game_manager.stop_game();
    delete game_dialog;
  }
  
  bool Game_Window::on_close()
  {
    return true;
  }

  void Game_Window::load_settings()
  {
    gui_manager.load_settings();
  }
  
  void Game_Window::new_game()
  {
    game_dialog->game_setup();
  }

  void Game_Window::settings_dialog()
  {
    Settings_Dialog dialog( this, gui_manager );
    dialog.Center();
    dialog.ShowModal();
  }

  void Game_Window::show_status_text( wxString text ) // shows text in status bar
  {
    parent_frame.SetStatusText( text );
  }
  void Game_Window::init_scrollbars()
  {
    SetScrollbars( 10, 10, gui_manager.get_width() / 10 + 1, gui_manager.get_height() / 10 + 1 );
  }
    
  void Game_Window::OnDraw( wxDC &_dc )
  {
    // prepare background:
    int width, width2; 
    int height, height2;
    GetVirtualSize(&width,&height); // get size from scrolled window
    GetSize(&width2,&height2);
    if( width2  > width  ) width  = width2;
    if( height2 > height ) height = height2;

#ifdef DOUBLE_BUFFER
    wxBitmap buffer( width, height );
    wxMemoryDC mem;
    mem.SelectObject(buffer);
    PrepareDC(mem);
    wxDC *dc = &mem;
#else
    wxDC *dc = &_dc;
    dc->BeginDrawing();
#endif

#ifdef DRAW_BACKGROUND
    int bg_width = gui_manager.get_bitmap_handler().background.GetWidth();
    int bg_height = gui_manager.get_bitmap_handler().background.GetHeight();
    for( int y = 0; y < height + bg_height; y += bg_height )
    {
      for( int x = 0; x < width + bg_width; x += bg_width )
      {
	dc->DrawBitmap( gui_manager.get_bitmap_handler().background, x, y );
      }
    }
#endif

    gui_manager.draw( *dc );
    gui_manager.draw_mark( *dc );

#ifdef DOUBLE_BUFFER
#ifdef __WXGTK__		// work around for wxGTK which doesn't draw text on MemoryDC
    // draw text directly on the real device context
    _dc.BeginDrawing();
    _dc.Blit(0,0, width, height, dc, 0, 0 );
    dc = &_dc;			
#endif
#endif

    gui_manager.draw_text( *dc );

#ifdef DOUBLE_BUFFER
#ifndef __WXGTK__
    // draw buffer on the real device context
    _dc.BeginDrawing();
    _dc.Blit(0,0, width, height, dc, 0, 0 );
    _dc.EndDrawing();
#else
    _dc.EndDrawing();
#endif
#else
    dc->EndDrawing();
#endif
  }
  void Game_Window::on_erase_background( wxEraseEvent &event )
  {
    // do nothing, just don't erase background...
  }
  void Game_Window::on_mouse_event( wxMouseEvent &event )
  {
    int cl_x, cl_y;
    GetViewStart( &cl_x, &cl_y );	// window might be scrolled
    cl_x = cl_x*10 + event.GetX();
    cl_y = cl_y*10 + event.GetY();

    if( event.LeftDown() )	// if event is left click
    {
      gui_manager.mouse_click_left( cl_x, cl_y );
    }
    else
    {
      if( event.RightDown() )	// if event is left click
      {
	gui_manager.mouse_click_right( cl_x, cl_y );
      }
    }
  }
  void Game_Window::refresh()
  {
    Refresh();
    Update();
  }
  wxDC *Game_Window::get_client_dc()	// must be destroyed
  {
    wxDC *dc = new wxClientDC(this);
    PrepareDC(*dc);
    return dc;
  }
  
  // ----------------------------------------------------------------------------
  // main frame
  // ----------------------------------------------------------------------------

  // frame constructor
  Main_Frame::Main_Frame( const wxString& title )
    : wxFrame( /*paRent*/0, /*id*/-1, title, restore_position(), restore_size(), 
	       wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL ),
      game_window(this),
      setting_menu(0)
  {
    // set the frame icon
    SetIcon(wxICON(icon));

    // create the menu
    SetMenuBar(create_menu());

#if wxUSE_STATUSBAR
    // create a status bar just for fun (by default with 1 pane only)
    CreateStatusBar(1);
    SetStatusText(_("Welcome to Holtz!"));
#endif // wxUSE_STATUSBAR

    game_window.Show(true);
  }

  Main_Frame::~Main_Frame()
  {
  }

  wxMenuBar* Main_Frame::create_menu()
  {
    // create a menu bar
    wxMenu *menu_file = new wxMenu;
    menu_file->Append(HOLTZ_NEW_GAME,		_("&New Game...\tCtrl-N"), _("Start a new game"));
    menu_file->AppendSeparator();
    menu_file->Append(HOLTZ_QUIT,		_("E&xit\tAlt-X"), _("Quit Holtz"));

    // the "Setting" item should be in the help menu
    setting_menu = new wxMenu;
    setting_menu->Append(HOLTZ_SETTINGS, _("Display s&ettings...\tCtrl-E"),  _("Change display settings"));

    // the "About" item should be in the help menu
    wxMenu *help_menu = new wxMenu;
	help_menu->Append(HOLTZ_HELP_CONTENTS, _("Contents\tF1"), _("Show help file"));
	help_menu->Append(HOLTZ_HELP_LICENSE, _("License"), _("Information about the Holtz license"));
    help_menu->Append(HOLTZ_ABOUT, _("About"), _("Show about dialog"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menu_bar = new wxMenuBar();
    menu_bar->Append(menu_file,    _("&File"));
    menu_bar->Append(setting_menu, _("&Settings"));
    menu_bar->Append(help_menu,    _("&Help"));

    return menu_bar;
  }

  void Main_Frame::save_size_and_position()
  {
    wxPoint pos = GetPosition();
    wxSize size = GetSize();
    wxConfigBase* cfg = wxConfig::Get();
    cfg->Write(wxT("MainXPos"), (long)pos.x);
    cfg->Write(wxT("MainYPos"), (long)pos.y);
    cfg->Write(wxT("MainXSize"), (long)size.GetWidth());
    cfg->Write(wxT("MainYSize"), (long)size.GetHeight());
    cfg->Flush();
  }

  wxSize Main_Frame::restore_size()
  {
    wxConfigBase* cfg = wxConfig::Get();
    wxSize size;
    size.SetWidth(cfg->Read(wxT("MainXSize"), 640));
    size.SetHeight(cfg->Read(wxT("MainYSize"), 480));
    return size;
  }

  wxPoint Main_Frame::restore_position()
  {
    wxConfigBase* cfg = wxConfig::Get();
    wxPoint pos;
    pos.x = cfg->Read(wxT("MainXPos"), -1); // -1 is the default position
    pos.y = cfg->Read(wxT("MainYPos"), -1);
    return pos;
  }

  void Main_Frame::load_settings()
  {
    game_window.load_settings();
  }
  
  // event handlers

  void Main_Frame::on_new_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.new_game();
  }

  void Main_Frame::on_settings(wxCommandEvent& WXUNUSED(event))
  {
    game_window.settings_dialog();
  }

  void Main_Frame::on_quit(wxCommandEvent& WXUNUSED(event))
  {
    // TRUE is to force the frame to close
    Close(TRUE);
  }

  void Main_Frame::on_help_contents(wxCommandEvent&)
  {
    ::wxGetApp().get_help_controller().DisplayContents();
  }

  void Main_Frame::on_help_license(wxCommandEvent&)
  {
    ::wxGetApp().get_help_controller().DisplaySection(wxT("helplic.htm"));
  }

  void Main_Frame::on_about(wxCommandEvent& WXUNUSED(event))
  {
    wxString msg;
    msg =  _("Holtz is game about making sacrifices.\n");
    msg += _("GPLed by Martin Trautmann (2003)\n");
	msg += _("Based on Zèrtz (r) and (c) by Don & Co NV, 2001.");

    wxMessageBox(msg, _("About Holtz"), wxOK | wxICON_INFORMATION, this);
  }

  void Main_Frame::on_close(wxCloseEvent& WXUNUSED(event))
  {
    save_size_and_position();
    game_window.on_close();
    Destroy();
  }
}

// Create a new application object: this macro will allow wxWindows to create
// the application object during program execution (it's better than using a
// static object for many reasons) and also declares the accessor function
// wxGetApp() which will return the reference of the right type (i.e. RoboTopApp and
// not wxApp)
IMPLEMENT_APP(holtz::wxHoltz)					//**/

