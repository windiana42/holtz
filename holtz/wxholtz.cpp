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
#define DEFAULT_SKIN_FILE DEFAULT_DATA_DIR "skins/hex70.zip"
#endif
#ifndef DEFAULT_BEEP_FILE
#define DEFAULT_BEEP_FILE DEFAULT_DATA_DIR "sounds/beep.wav"
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
#include "ai.hpp"
#include "util.hpp"

#include <wx/zipstrm.h>
#include <wx/image.h>
#include <wx/fileconf.h>
#include <wx/dcmemory.h>
#include <assert.h>
#ifndef __OLD_GCC__
  #include <sstream>
#endif
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
#include "purist/field_removed.xpm"
#include "purist/field_empty.xpm"
#include "purist/field_white.xpm"
#include "purist/field_gray.xpm"
#include "purist/field_black.xpm"
#include "purist/field_mark.xpm"
#include "purist/field_mark2.xpm"
#include "purist/background.xpm"
  // field dimensions
#include "purist/skin.hpp"
#endif

#ifdef HEX_50
  // -----------------------
  // hexagonal pictures 50px

  // field pictures
#include "hex50/field_removed.xpm"
#include "hex50/field_empty.xpm"
#include "hex50/field_white.xpm"
#include "hex50/field_gray.xpm"
#include "hex50/field_black.xpm"
#include "hex50/field_mark.xpm"
#include "hex50/field_mark2.xpm"
#include "hex50/background.xpm"
  // field dimensions
#include "hex50/skin.hpp"
#endif

#ifdef HEX_70
  // -----------------------
  // hexagonal pictures 70px

  // field pictures
#include "hex70/field_removed.xpm"
#include "hex70/field_empty.xpm"
#include "hex70/field_white.xpm"
#include "hex70/field_gray.xpm"
#include "hex70/field_black.xpm"
#include "hex70/field_mark.xpm"
#include "hex70/field_mark2.xpm"
#include "hex70/background.xpm"
  // field dimensions
#include "hex70/skin.hpp"
#endif

#ifdef HEX_100
  // -----------------------
  // hexagonal pictures 100px

  // field pictures
#include "hex100/field_removed.xpm"
#include "hex100/field_empty.xpm"
#include "hex100/field_white.xpm"
#include "hex100/field_gray.xpm"
#include "hex100/field_black.xpm"
#include "hex100/field_mark.xpm"
#include "hex100/field_mark2.xpm"
#include "hex100/background.xpm"
  // field dimensions
#include "hex100/skin.hpp"
#endif

#ifdef EINS
  // -----------------------
  // hexagonal pictures 100px

  // field pictures
#include "eins/field_removed.xpm"
#include "eins/field_empty.xpm"
#include "eins/field_white.xpm"
#include "eins/field_gray.xpm"
#include "eins/field_black.xpm"
#include "eins/field_mark.xpm"
#include "eins/field_mark2.xpm"
#include "eins/background.xpm"
  // field dimensions
#include "eins/skin.hpp"
#endif

  // ----------------------------------------------------------------------------
  // event tables and other macros for wxWindows
  // ----------------------------------------------------------------------------

  // the event tables connect the wxWindows events with the functions (event
  // handlers) which process them. It can be also done at run-time, but for the
  // simple menu events like this the static method is much simpler.
  BEGIN_EVENT_TABLE(Main_Frame, wxFrame)				//**/
  EVT_MENU(HOLTZ_NEW_GAME,	  Main_Frame::on_new_game)		//**/
  EVT_MENU(HOLTZ_STANDALONE_GAME, Main_Frame::on_standalone_game)	//**/
  EVT_MENU(HOLTZ_NETWORK_GAME,	  Main_Frame::on_network_game)		//**/
  EVT_MENU(HOLTZ_SKIN,		  Main_Frame::on_choose_skin)		//**/
  EVT_MENU(HOLTZ_BEEP,		  Main_Frame::on_choose_beep)		//**/
  EVT_MENU(HOLTZ_SOUND,		  Main_Frame::on_toggle_sound)		//**/
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

  Player_Handler::~Player_Handler()
  {
  }

  Player_Setup_Manager::~Player_Setup_Manager()
  {
  }

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
	    if(!get_help_controller().Initialize(wxString(DEFAULT_DATA_DIR) + wxT("help/help_") + language))
	      if(language.Len() <= 2 || !get_help_controller().Initialize(wxString(DEFAULT_DATA_DIR) + wxT("help/help_") + language.Left(2)))
		if(!get_help_controller().Initialize(wxT("help/help_en")))
		  if(!get_help_controller().Initialize(wxString(DEFAULT_DATA_DIR) + wxT("help/help_en")))
	  	  return false;
	return true;
  }

  bool wxHoltz::check_config()
  {
    bool ok = false;
    wxConfigBase* cfg = wxConfig::Get();
    wxString buf;
    if( cfg->Read( wxT("skin_file"), &buf) )
    {
      if( wxFileExists(buf) )
	ok = true;
    }
    if( !ok )
    {
      buf = wxT(DEFAULT_SKIN_FILE);
      if( wxFileExists(buf) )
      {
	cfg->Write( wxT("skin_file"), buf);
	cfg->Flush();
	ok = true;
      }
    }
    if( !ok )
    {
      buf = wxFileSelector( _("Choose a skin File"), wxT(""), 
			    wxT(""), wxT(""), _("Skin Files (*.zip)|*.zip"),
			    wxOPEN );
      if( wxFileExists(buf) )
      {
	cfg->Write( wxT("skin_file"), buf);
	cfg->Flush();
      }
    }     

    ok = false;
    if( cfg->Read( wxT("beep_file"), &buf) )
    {
      if( wxFileExists(buf) )
	ok = true;
    }
    if( !ok )
    {
      buf = wxT(DEFAULT_BEEP_FILE);
      if( wxFileExists(buf) )
      {
	cfg->Write( wxT("beep_file"), buf);
	cfg->Flush();
	ok = true;
      }
    }
    if( !ok )
    {
      cfg->Write( wxT("play_sound"), false );
      cfg->Flush();
    }

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
    : field_width(51), field_height(51), field_packed_height(44),
      caption_height(30), stone_offset(20), 
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10)
  {
  }

  Dimensions::Dimensions( char **field_xpm )
    : caption_height(30), stone_offset(20), 
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10)
  {
    wxBitmap bitmap(field_xpm);
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)
  }

  Dimensions::Dimensions( wxConfigBase &config, wxBitmap &bitmap )	// load from configuration
    : caption_height(30), stone_offset(20), 
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10)
  {
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)

    int buf;

    if( config.Read( wxT("field_width"), &buf ) ) field_width = buf;
    if( config.Read( wxT("field_height"), &buf ) ) field_height = buf;
    if( config.Read( wxT("field_packed_height"), &buf ) ) field_packed_height = buf;
    if( config.Read( wxT("caption_height"), &buf ) ) caption_height = buf;
    if( config.Read( wxT("stone_offset"), &buf ) ) stone_offset = buf;
    if( config.Read( wxT("board_stones_spacing"), &buf ) ) board_stones_spacing = buf;
    if( config.Read( wxT("player_player_spacing"), &buf ) ) player_player_spacing = buf;
    if( config.Read( wxT("stones_player_spacing"), &buf ) ) stones_player_spacing = buf;
  }

  // ----------------------------------------------------------------------------
  // Board Panel
  // ----------------------------------------------------------------------------

  Board_Panel::Board_Panel( Game &game, Game_Window &game_window,
			    std::map<Field_State_Type,wxBitmap> &bitmaps, 
			    int x, int y, Sequence_Generator* &sg, 
			    Dimensions &dimensions )
    : game(game), game_window(game_window), bitmaps(bitmaps), x(x), y(y),
      sequence_generator( sg ), dimensions(dimensions)
  {
  }

  int Board_Panel::get_width() const
  {
    return dimensions.field_width * game.board.get_x_size();
  }
  int Board_Panel::get_height() const
  {
    return dimensions.field_packed_height * (game.board.get_y_size()-1) + 
      dimensions.field_height;
  }
  void Board_Panel::set_x( int _x )
  {
    x = _x;
  }
  void Board_Panel::set_y( int _y )
  {
    y = _y;
  }
  void Board_Panel::place_child_windows()
  {
  }

  void Board_Panel::draw( wxDC &dc )
  {
    int offset;
    for( int fy = 0; fy < game.board.get_y_size(); ++fy )
    {
      if( fy & 1 )		// even row
	offset = 0;
      else
	offset = dimensions.field_width / 2;

      for( int fx = 0; fx < game.board.get_x_size(); ++fx )
      {
	dc.DrawBitmap( bitmaps[ game.board.field[fx][fy] ], 
		       x + fx*dimensions.field_width + offset, 
		       y + fy*dimensions.field_packed_height, true );
      }
    }
  }
  void Board_Panel::draw_text( wxDC &dc )
  {
  }

  void Board_Panel::on_click( long click_x, long click_y )
  {
    if( sequence_generator )
    {
      wxString msg;
      int half_diff = (dimensions.field_height - dimensions.field_packed_height) / 2;
      int row = (click_y - y - half_diff) / dimensions.field_packed_height;
      int offset = (row & 1) ? 0 : dimensions.field_width / 2;
      
      if( (click_x >= x + offset) && (click_y >= y ) )
      {
	int col = (click_x - x - offset) / dimensions.field_width;

	if( (col >= 0) && (row >= 0) && 
	    (col < game.board.get_x_size()) && 
	    (row < game.board.get_y_size()) )
	{
	  Field_Pos pos;
	  pos.x = col;
	  pos.y = row;

	  Sequence_Generator::Sequence_State state;
	  state = sequence_generator->add_click( pos );
	  game_window.set_mark( -1, -1 );
	  game_window.get_frame().SetStatusText(wxT(""));

	  switch( state )
	  {
	    case Sequence_Generator::finished:
	      sequence_generator = 0;
	      game_window.continue_game();
	      break;
	    case Sequence_Generator::hold_white:
	    case Sequence_Generator::hold_gray:
	    case Sequence_Generator::hold_black:
	      game_window.set_mark( x + offset + col * dimensions.field_width, 
				    y + row * dimensions.field_packed_height );
	      game_window.show_user_information( true );
	      break;
	    case Sequence_Generator::another_click:
	      game_window.show_user_information( true );
	      break;
	    case Sequence_Generator::fatal_error:
	      msg.Printf( _("Fatal Error!") );
	      
	      wxMessageBox( msg, _("Click Error"), wxOK | wxICON_ERROR, &game_window );
	      break;
	    case Sequence_Generator::error_require_knock_out:
	      game_window.get_frame().SetStatusText(_("You must do the knock out move!"));
	      break;
	    case Sequence_Generator::error_require_set:
	      game_window.get_frame().SetStatusText(_("You have to set a stone!"));
	      break;
	    case Sequence_Generator::error_require_remove:
	      game_window.get_frame().SetStatusText(_("You have to remove an empty border field!"));
	      break;
	    case Sequence_Generator::error_can_t_remove:
	      game_window.get_frame().SetStatusText(_("Field can't be removed without moving another stone!"));
	      break;
	    case Sequence_Generator::error_can_t_move_here:
	      game_window.get_frame().SetStatusText(_("You can't move to that field!"));
	      break;
	    case Sequence_Generator::error_can_t_set_here:
	      game_window.get_frame().SetStatusText(_("Please set the stone on an empty field!"));
	      break;
	    case Sequence_Generator::error_must_pick_common_stone:
	      game_window.get_frame().SetStatusText(_("You must pick a common stone!"));
	      break;
	    case Sequence_Generator::error_wrong_player:
	      game_window.get_frame().SetStatusText(_("Its not the turn of that player!"));
	      break;
	    case Sequence_Generator::error_impossible_yet:
	      game_window.get_frame().SetStatusText(_("You can't do that at the moment!"));
	      break;
	    case Sequence_Generator::error_must_knock_out_with_same_stone:
	      game_window.get_frame().SetStatusText(_("You must knock out once more with the same stone!"));
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
  }

  std::pair<int,int> Board_Panel::get_field_pos( int col, int row ) const
  {
    int offset;

    if( row & 1 )		// even row
      offset = 0;
    else
      offset = dimensions.field_width / 2;

    return std::pair<int,int>( x + col*dimensions.field_width + offset, 
			       y + row*dimensions.field_packed_height );
  }

  // ----------------------------------------------------------------------------
  // Stone Panel
  // ----------------------------------------------------------------------------

  Stone_Panel::Stone_Panel( Stones &stones, Game_Window &game_window,
			    std::map<Field_State_Type,wxBitmap> &bitmaps, 
			    int x, int y, Sequence_Generator* &sg, 
			    Dimensions &dimensions, int max_stones )
    : stones(stones), game_window(game_window), bitmaps(bitmaps), x(x), y(y),
      sequence_generator(sg), dimensions(dimensions), max_stones(max_stones)
  {
  }

  int Stone_Panel::get_width() const
  {
    return dimensions.field_width * max_stones;
  }
  
  int Stone_Panel::get_height() const
  {
    return 3 * dimensions.field_height;
  }
  void Stone_Panel::set_x( int _x )
  {
    x = _x;
  }
  void Stone_Panel::set_y( int _y )
  {
    y = _y;
  }
  void Stone_Panel::place_child_windows()
  {
  }
  
  void Stone_Panel::draw( wxDC &dc )
  {
    int stone_type;
    int y_pos = y, x_pos;
    for( stone_type = Stones::white_stone; stone_type <= Stones::black_stone; ++stone_type )
    {
      int count = stones.stone_count[ Stones::Stone_Type(stone_type) ];
      if( count > max_stones ) count = max_stones;
      
      x_pos = x;
      int i;
      for( i = 0; i < count; ++i )
      {
	dc.DrawBitmap( bitmaps[ Field_State_Type(stone_type) ], x_pos, y_pos, true );

	x_pos += dimensions.field_width;
      }
      for( ; i < max_stones; ++i )
      {
	dc.DrawBitmap( bitmaps[ field_removed ], x_pos, y_pos, true );
	x_pos += dimensions.field_width;
      }
      
      y_pos += dimensions.field_height;
    }
  }
  void Stone_Panel::draw_text( wxDC &dc )
  {
  }
  
  void Stone_Panel::on_click( long click_x, long click_y )
  {
    wxString msg;
    if( sequence_generator )
    {
      if( (click_x >= x) && (click_y >= y ) )
      {
	int row = (click_y - y) / dimensions.field_height;
	int col = (click_x - x) / dimensions.field_width;

	if( (row >= 0) && (row < 3) && (col >= 0) && (col < max_stones) )
	{
	  Stones::Stone_Type type( Stones::Stone_Type(row + 1) );
	  assert( Board::is_stone(Field_State_Type(type)) );
	
	  if( col < stones.stone_count[type] )
	  {
	    Sequence_Generator::Sequence_State state;
	    state = sequence_generator->add_click_common_stone( type );
	    game_window.set_mark( -1, -1 );
	    game_window.get_frame().SetStatusText(wxT(""));
	  
	    switch( state )
	    {
	      case Sequence_Generator::finished:
		sequence_generator = 0;
		game_window.continue_game();
		break;
	      case Sequence_Generator::hold_white:
	      case Sequence_Generator::hold_gray:
	      case Sequence_Generator::hold_black:
		game_window.set_mark( x + col * dimensions.field_width, 
				      y + row * dimensions.field_height );
		game_window.show_user_information( true );
		break;
	      case Sequence_Generator::another_click:
		game_window.show_user_information( true );
		break;
	      case Sequence_Generator::fatal_error:
		msg.Printf( _("Fatal Error!") );
	    
		wxMessageBox(msg, _("Click Error"), wxOK | wxICON_ERROR, &game_window);
		break;
	      case Sequence_Generator::error_require_knock_out:
		game_window.get_frame().SetStatusText(_("You must do the knock out move!"));
		break;
	      case Sequence_Generator::error_require_set:
		game_window.get_frame().SetStatusText(_("You have to set a stone!"));
		break;
	      case Sequence_Generator::error_require_remove:
		game_window.get_frame().SetStatusText(_("You have to remove an empty border field!"));
		break;
	      case Sequence_Generator::error_can_t_remove:
		game_window.get_frame().SetStatusText(_("Field can't be removed without moving another stone!"));
		break;
	      case Sequence_Generator::error_can_t_move_here:
		game_window.get_frame().SetStatusText(_("You can't move to that field!"));
		break;
	      case Sequence_Generator::error_can_t_set_here:
		game_window.get_frame().SetStatusText(_("Please set the stone on an empty field!"));
		break;
	      case Sequence_Generator::error_must_pick_common_stone:
		game_window.get_frame().SetStatusText(_("You must pick a common stone!"));
		break;
	      case Sequence_Generator::error_wrong_player:
		game_window.get_frame().SetStatusText(_("Its not the turn of that player!"));
		break;
	      case Sequence_Generator::error_impossible_yet:
		game_window.get_frame().SetStatusText(_("You can't do that at the moment!"));
		break;
	      case Sequence_Generator::error_must_knock_out_with_same_stone:
		game_window.get_frame().SetStatusText(_("You must knock out once more with the same stone!"));
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
  }

  std::pair<int,int> Stone_Panel::get_field_pos( int col, Stones::Stone_Type type ) const
  {
    return std::pair<int,int>( x + col * dimensions.field_width, 
			       y + (int(type) - 1) * dimensions.field_height );
  }

  // ----------------------------------------------------------------------------
  // Player Panel
  // ----------------------------------------------------------------------------
  
  Player_Panel::Player_Panel( Player &player, Game_Window &game_window,
			      std::map<Field_State_Type,wxBitmap> &bitmaps, 
			      int x, int y, Sequence_Generator* &sg, 
			      Dimensions &dimensions )
    : player(player), game_window(game_window), 
      player_font(20, wxDECORATIVE, wxNORMAL, wxNORMAL), 
      x(x), y(y), sequence_generator(sg),
      stone_panel(player.stones, game_window, bitmaps, 
		  x + dimensions.stone_offset, 
		  y + dimensions.caption_height, sg, 
		  dimensions, 6 /*max_stones*/),
      dimensions(dimensions)
  {
    /*
    caption_text = new wxStaticText( &game_window, -1, str_to_wxstr(player.name), wxPoint(x,y) );
    caption_text->SetFont(player_font);
    caption_text->SetBackgroundColour(game_window.GetBackgroundColour());
    */
  }

  int Player_Panel::get_width() const
  {
    return dimensions.stone_offset + stone_panel.get_width();
  }

  int Player_Panel::get_height() const
  {
    return dimensions.caption_height + stone_panel.get_height();
  }
  void Player_Panel::set_x( int _x )
  {
    x = _x;
  }
  void Player_Panel::set_y( int _y )
  {
    y = _y;
  }
  void Player_Panel::place_child_windows()
  {
    stone_panel.set_x( x + dimensions.stone_offset );
    stone_panel.set_y( y + dimensions.caption_height );
  }

  void Player_Panel::draw( wxDC &dc )
  {
    stone_panel.draw( dc );
  }
  void Player_Panel::draw_text( wxDC &dc )
  {
    std::string str = player.name;
    if( player.host != "" )
      str = str + " (" + player.host + ")";
    wxString wxstr = str_to_wxstr(str);
    if( player.type == Player::ai )
      wxstr = _("[AI] ") + wxstr;

    dc.SetFont(player_font);
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
      //dc.SetTextBackground(game_window.GetBackgroundColour());
      //caption_text->SetBackgroundColour(game_window.GetBackgroundColour());
#ifndef DRAW_BACKGROUND
      dc.SetBrush( wxBrush(game_window.GetBackgroundColour(), wxSOLID) );
      dc.DrawRectangle( x, y, w, h );
#endif
    }

    dc.DrawText(wxstr,x,y);
    //caption_text->SetLabel( str_to_wxstr(str) );

    //dc.SetBackgroundMode(wxTRANSPARENT);

    stone_panel.draw_text( dc );
  }

  void Player_Panel::on_click( long click_x, long click_y )
  {
    wxString msg;
    if( sequence_generator )
    {
      click_x -= dimensions.stone_offset;
      click_y -= dimensions.caption_height;

      if( (click_x >= x) && (click_y >= y ) )
      {
	int row = (click_y - y) / dimensions.field_height;
	int col = (click_x - x) / dimensions.field_width;

	if( (row >= 0) && (row < 3) && (col >= 0) && (col < 6/*max_stones*/) )
	{
	  Stones::Stone_Type type( Stones::Stone_Type(row + 1) );
	  assert( Board::is_stone(Field_State_Type(type)) );

	  if( col < player.stones.stone_count[type] )
	  {
	    Sequence_Generator::Sequence_State state;
	    state = sequence_generator->add_click_player_stone( player.id, type );
	    game_window.set_mark( -1, -1 );
	    game_window.get_frame().SetStatusText(wxT(""));

	    switch( state )
	    {
	      case Sequence_Generator::finished:
		sequence_generator = 0;
		game_window.continue_game();
		break;
	      case Sequence_Generator::hold_white:
	      case Sequence_Generator::hold_gray:
	      case Sequence_Generator::hold_black:
		game_window.set_mark( x + col * dimensions.field_width + dimensions.stone_offset, 
				      y + row * dimensions.field_height + dimensions.caption_height );
		game_window.show_user_information( true );
		break;
	      case Sequence_Generator::another_click:
		game_window.show_user_information( true );
		break;
	      case Sequence_Generator::fatal_error:
		msg.Printf( _("Fatal Error!") );
	  
		wxMessageBox(msg, _("Click Error"), wxOK | wxICON_ERROR, &game_window);
		break;
	      case Sequence_Generator::error_require_knock_out:
		game_window.get_frame().SetStatusText(_("You must do the knock out move!"));
		break;
	      case Sequence_Generator::error_require_set:
		game_window.get_frame().SetStatusText(_("You have to set a stone!"));
		break;
	      case Sequence_Generator::error_require_remove:
		game_window.get_frame().SetStatusText(_("You have to remove an empty border field!"));
		break;
	      case Sequence_Generator::error_can_t_remove:
		game_window.get_frame().SetStatusText(_("Field can't be removed without moving another stone!"));
		break;
	      case Sequence_Generator::error_can_t_move_here:
		game_window.get_frame().SetStatusText(_("You can't move to that field!"));
		break;
	      case Sequence_Generator::error_can_t_set_here:
		game_window.get_frame().SetStatusText(_("Please set the stone on an empty field!"));
		break;
	      case Sequence_Generator::error_must_pick_common_stone:
		game_window.get_frame().SetStatusText(_("You must pick a common stone!"));
		break;
	      case Sequence_Generator::error_wrong_player:
		game_window.get_frame().SetStatusText(_("Its not the turn of that player!"));
		break;
	      case Sequence_Generator::error_impossible_yet:
		game_window.get_frame().SetStatusText(_("You can't do that at the moment!"));
		break;
	      case Sequence_Generator::error_must_knock_out_with_same_stone:
		game_window.get_frame().SetStatusText(_("You must knock out once more with the same stone!"));
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
  }

  // ----------------------------------------------------------------------------
  // Mouse Handler
  // ----------------------------------------------------------------------------
  
  Mouse_Handler::Mouse_Handler( Game &game, Game_Window &game_window, Sequence_Generator * &sg )
    : Generic_Mouse_Input(game), game_window(game_window), sequence_generator_hook(sg), ai(0)
  {
  }
  
  void Mouse_Handler::init_mouse_input() 
  {
    sequence_generator.reset();
    sequence_generator_hook = &sequence_generator;

    game_window.allow_user_activity();
    if( game.current_player->help_mode == Player::show_hint )
    {
      ai = &game_window.get_ai_handler();
      ai->determine_hints();
    }
  }
  void Mouse_Handler::disable_mouse_input() 
  {
    sequence_generator_hook = 0;

    if( game.current_player->help_mode == Player::show_hint )
    {
      ai->abort();
      game_window.remove_hint();
    }
    game_window.stop_user_activity();
  }

  // ----------------------------------------------------------------------------
  // Game Window
  // ----------------------------------------------------------------------------

  Game_Window::Game_Window( wxFrame *parent_frame, Game &game )
    : wxScrolledWindow( parent_frame ),
      parent_frame(*parent_frame), 
      dimensions( get_skin_dimensions() ),
      board_panel( game, *this, bitmaps, 0, 0, sequence_generator, dimensions ), 
      stone_panel( game.common_stones, *this, bitmaps, 0, 
		   board_panel.get_height() + dimensions.board_stones_spacing, 
		   sequence_generator, dimensions ),
      mouse_handler( game, *this, sequence_generator ), 
      ai( game, *this ), game(game), sequence_generator(0), 
      move_animation( new Move_Sequence_Animation(*this) ),
      player_setup_manager(0), clients_dialog(0), player_dialog(0),
      field_mark(field_mark_xpm), field_mark2(field_mark2_xpm), 
      background(background_xpm), field_mark_x(-1), 
      play_sound(false)
  {
    SetBackgroundColour(*wxWHITE);

    wxConfigBase* cfg = wxConfig::Get();
    wxString buf; bool bool_buf;
    if( cfg->Read( wxT("skin_file"), &buf) )
    {
      skin_file = buf;
      load_skin( skin_file );
    }
    else
    {
      bitmaps[field_removed]	= wxBitmap(field_removed_xpm);
      bitmaps[field_empty]	= wxBitmap(field_empty_xpm);
      bitmaps[field_white]	= wxBitmap(field_white_xpm);
      bitmaps[field_gray]	= wxBitmap(field_gray_xpm);
      bitmaps[field_black]	= wxBitmap(field_black_xpm);
    }
    if( cfg->Read( wxT("play_sound"), &bool_buf) ) play_sound = bool_buf;
#if wxUSE_WAVE
    if( cfg->Read( wxT("beep_file"), &buf) ) beep_file = buf;
    if( beep_file != wxT("") )
    {
      if( !sound_beep.Create( beep_file ) )
	play_sound = false;
    }
    else
      play_sound = false;
#endif

    SetScrollbars( 10, 10, get_width() / 10 + 1, get_height() / 10 + 1 );
  }

  Game_Window::~Game_Window()
  {
    stop_game();
    if( player_dialog )
      player_dialog->Destroy();
    if( clients_dialog )
      clients_dialog->Destroy();
    if( player_setup_manager )
      delete player_setup_manager;

    if( player_panels.size() )
    {
      std::list<Player_Panel*>::iterator panel;
      for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
      {
	delete *panel;
      }
      player_panels.clear();
    }
    delete move_animation;
  }
  
  void Game_Window::on_close()
  {
    if( player_dialog )
    {
      player_dialog->Destroy();
      player_dialog = 0;
    }
    if( clients_dialog )
    {
      clients_dialog->Destroy();
      clients_dialog = 0;
    }
  }

  int Game_Window::get_width() const
  {
    int width = board_panel.get_width();
    int width2 = stone_panel.get_width();
    if( width2 > width ) 
      width = width2;

    if( player_panels.size() )
    {
      width += dimensions.stones_player_spacing;
      width += player_panels.front()->get_width();
    }
    return width;
  }

  int Game_Window::get_height() const
  {
    int height = board_panel.get_height() + 
      dimensions.board_stones_spacing + stone_panel.get_height();
    if( player_panels.size() )
    {
      int height2 = 0;
      std::list<Player_Panel*>::const_iterator i;
      for( i = player_panels.begin(); i != player_panels.end(); ++i )
      {
	height2 += (*i)->get_height() + dimensions.player_player_spacing;
      }
      if( height2 > height ) 
	height = height2;
    }
    
    return height;
  }

  void Game_Window::place_child_windows()
  {
    board_panel.set_x( 0 );
    board_panel.set_y( 0 ); 
    stone_panel.set_x( 0 ); 
    stone_panel.set_y( board_panel.get_height() + dimensions.board_stones_spacing );
    stone_panel.place_child_windows();

    int cur_x = stone_panel.get_width(), cur_y = 0;
    if( board_panel.get_width() > cur_x ) 
      cur_x = board_panel.get_width();
    cur_x += dimensions.stones_player_spacing;

    std::list<Player_Panel*>::iterator panel;
    for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
    {
      (*panel)->set_x( cur_x );
      (*panel)->set_y( cur_y );
      (*panel)->place_child_windows();

      cur_y += (*panel)->get_height() + dimensions.player_player_spacing;
    }

    SetScrollbars( 10, 10, get_width() / 10 + 1, get_height() / 10 + 1 );
  }

  void Game_Window::set_mark( int x, int y )
  {
    field_mark_x = x;
    field_mark_y = y;

    wxClientDC dc(this);
    dc.BeginDrawing();
    PrepareDC(dc);
    draw_mark(dc);
    dc.EndDrawing();
  }

  void Game_Window::draw_mark( wxDC &dc )
  {
    std::list< std::pair<int,int> >::iterator position;
    for( position = field_mark2_positions.begin();
	 position != field_mark2_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( field_mark2, x, y, true );
    }

    for( position = field_mark_positions.begin();
	 position != field_mark_positions.end(); ++position )
    {
      int &x = position->first;
      int &y = position->second;
      dc.DrawBitmap( field_mark, x, y, true );
    }

    if( field_mark_x >= 0 )
      dc.DrawBitmap( field_mark, field_mark_x, field_mark_y, true );
  }

  void Game_Window::show_user_information( bool visible, bool do_refresh )
  {
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
		field_mark2_positions.push_back( board_panel.get_field_pos( click->x, click->y ) );
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
		field_mark2_positions.push_back( board_panel.get_field_pos( knock_move->from.x, 
									    knock_move->from.y ) );
		field_mark2_positions.push_back( board_panel.get_field_pos( knock_move->to.x, 
									    knock_move->to.y ) );
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
		    field_mark2_positions.push_back( stone_panel.get_field_pos( stone_count - 1, 
										set_move->stone_type ) );
		  }
		}
		else
		{
		  std::list<Player_Panel*>::iterator panel;
		  for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
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
		field_mark2_positions.push_back( board_panel.get_field_pos( set_move->pos.x, 
									    set_move->pos.y ) );
	      }
	      break;
	      case Move::remove:
	      {
		Remove *remove = dynamic_cast<Remove *>(*move);
		field_mark_positions.push_back( board_panel.get_field_pos( remove->remove_pos.x, 
									    remove->remove_pos.y ) );
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
	    get_frame().SetStatusText(_("Please do a knock out move")); // remove old status text message
	    break;
	  case Move::set_move:
	    get_frame().SetStatusText(_("You should set a stone")); // remove old status text message
	    break;
	  case Move::remove:
	    get_frame().SetStatusText(_("Just remove a field")); // remove old status text message
	    break;
	  case Move::no_move:
	  case Move::finish_move:
	    break;
	}
      }
    }
    else
    {
      get_frame().SetStatusText(wxT("")); // remove old status text message
    }
    if( do_refresh )
      refresh();
  }

  void Game_Window::give_hint( AI_Result ai_result )
  {
    current_hint = ai_result;
    show_user_information( true, true );
  }

  void Game_Window::remove_hint()
  {
    current_hint = AI_Result();
    show_user_information( true, false );
  }

  void Game_Window::setup_new_game()
  {
    stop_game();
    
    if( !player_setup_manager )
      player_setup_manager = new Standalone_Player_Setup_Manager( *this );
    else
      player_setup_manager->new_game();

    if( player_dialog )
      player_dialog->Destroy();

    player_dialog = new Player_Setup_Dialog( this, *this, *player_setup_manager );
    player_dialog->Center();
    player_dialog->Show(true);
  }

  void Game_Window::setup_standalone_game()
  {
    stop_game();

    if( player_setup_manager )
    {
      delete player_setup_manager;
    }

    player_setup_manager = new Standalone_Player_Setup_Manager( *this );

    if( player_dialog )
      player_dialog->Destroy();

    player_dialog = new Player_Setup_Dialog( this, *this, *player_setup_manager );
    player_dialog->Center();
    player_dialog->Show(true);
  }

  void Game_Window::ask_new_game( wxString host )
  {
    wxString msg;
    msg.Printf( _("New game requested from %s.\nDo you really want to start a new game?"), host.c_str() );
    if( wxMessageBox( msg, _("Network Message"), wxYES | wxNO | wxICON_QUESTION ) == wxYES, this )
    {
      setup_new_game();
    }
  }

  void Game_Window::setup_network_game()
  {
    stop_game();

    if( player_setup_manager )
    {
      delete player_setup_manager;
      player_setup_manager = 0;
    }

    bool ok = false;
    Network_Connection_Dialog dialog(this);
    dialog.Centre();
    if( dialog.ShowModal() == wxID_CANCEL )
      ok = false;
    else
    {
      if( dialog.server->GetValue() ) // setup server?
      {
	wxIPV4address port;
	if( port.Service( dialog.port->GetValue() ) )
	{
	  if( clients_dialog )
	    clients_dialog->Destroy();

	  Network_Manager *network_manager = new Network_Manager( game, *this );
	  clients_dialog = new Network_Clients_Dialog( this, *network_manager );
	  clients_dialog->Show(true);

	  if( network_manager->setup_server(port) )
	  {
	    if( player_setup_manager )
	      delete player_setup_manager;
	    player_setup_manager = network_manager;
	    ok = true;
	  }
	  else
	  {
	    if( clients_dialog )
	    {
	      clients_dialog->Destroy();
	      clients_dialog = 0;
	    }

	    delete network_manager;
	    wxMessageBox(_("Can't listen port"), _("Network Message"), wxOK | wxICON_ERROR, this);
	  }
	}
	else
	  wxMessageBox(_("Illegal port"), _("Network Message"), wxOK | wxICON_ERROR, this);
      }
      else
      {
	assert( dialog.client->GetValue() );

	wxIPV4address host;
	if( host.Hostname(dialog.hostname->GetValue()) && 
	    host.Service(dialog.port->GetValue()) )
	{	  
	  Network_Manager *network_manager = new Network_Manager( game, *this );
	  if( network_manager->setup_client(host) )
	  {
	    if( player_setup_manager )
	      delete player_setup_manager;
	    player_setup_manager = network_manager;
	    ok = true;
	  }
	  else
	  {
	    wxMessageBox(_("Connection to Server failed"), _("Network Message"), wxOK | wxICON_ERROR, this);
	  }
	}
	else
	  wxMessageBox(_("Illegal hostname"), _("Network Message"), wxOK | wxICON_ERROR, this);
      }
    }
    if(ok)
    {
      assert( player_setup_manager );

      if( player_dialog )
	player_dialog->Destroy();

      player_dialog = new Player_Setup_Dialog( this, *this, *player_setup_manager );
      player_dialog->Center();
      player_dialog->Show(true);
    }
  }

  void Game_Window::reset( const Ruleset &ruleset )
  {
    sequence_generator = 0;
    game.reset_game( ruleset );
    game.remove_players();
    if( player_panels.size() )
    {
      std::list<Player_Panel*>::iterator panel;
      for( panel = player_panels.begin(); panel != player_panels.end(); ++panel )
      {
	delete *panel;
      }
      player_panels.clear();
    }
    // remove marks
    field_mark_x = -1;
    field_mark2_positions.clear();
  }

  void Game_Window::new_game( std::list<Player> new_players, const Ruleset &ruleset )
  {
    reset(ruleset);

    int y = 0;
    std::list<Player>::iterator player;
    for( player = new_players.begin(); player != new_players.end(); ++player )
    {
      game.add_player( *player );
      player_panels.push_back( new Player_Panel( game.players.back(), *this, 
						 bitmaps, 
						 stone_panel.get_width() + 
						 dimensions.stones_player_spacing, y, 
						 sequence_generator,
						 dimensions ) );
      y += player_panels.back()->get_height() + dimensions.player_player_spacing;
    }
    SetScrollbars( 10, 10, get_width() / 10 + 1, get_height() / 10 + 1 );
  }

  void Game_Window::continue_game()
  {
    bool go_on = true;
    while(go_on)
    {
      set_mark( -1, -1 ); // remove any mark
      Game::Game_State state;
      try
      {
	state = game.continue_game();
      }
      catch(Exception e)
      {
	wxString msg;
	msg.Printf( _("Exception caught!") );
	
	wxMessageBox(msg, _("Exception!"), wxOK | wxICON_ERROR, this);
      }
      refresh();
      Player *winner;
      switch( state )
      {
	case Game::finished:
	{
	  winner = game.get_winner();

	  wxString msg;
	  if( winner != 0 )
	  {
	    msg.Printf( _("And the Winner is: %s"), str_to_wxstr(winner->name).c_str() );
	    wxMessageBox( msg, _("Winner"), wxOK | wxICON_INFORMATION, this);
	  }
	  else
	  {
	    wxMessageBox( _("Nobody won the game!"), _("Players tied"), wxOK | wxICON_INFORMATION, this);
	  }

	  go_on = false;
	}
	break;
	case Game::wait_for_event:
	  go_on = false;
	  break;
	case Game::next_players_turn:
	  break;
	case Game::interruption_possible:
	  break;
	case Game::wrong_number_of_players:
	  go_on = false;
	  break;
      }
    }
  }
  
  void Game_Window::stop_game()
  {
    sequence_generator = 0;	// disable mouse handler from processing mouse clicks
  }

  void Game_Window::allow_user_activity()
  {
    show_user_information( true );
    beep();			// tell user that his activity is recommended
  }
  void Game_Window::stop_user_activity()
  {
    show_user_information( false, false );
  }

  void Game_Window::do_move_slowly( Sequence sequence, wxEvtHandler *done_handler, 
				    int event_id  ) // show user how move is done
  {
    assert( sequence.check_sequence( game ) );
    bool ret = move_animation->start( sequence, game, done_handler, event_id );
    //!!! should be handled easier in the future !!!
    assert( ret );
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
    int bg_width = background.GetWidth();
    int bg_height = background.GetHeight();
    for( int y = 0; y < height + bg_height; y += bg_height )
    {
      for( int x = 0; x < width + bg_width; x += bg_width )
      {
	dc->DrawBitmap( background, x, y );
      }
    }
#endif

    board_panel.draw( *dc );
    stone_panel.draw( *dc );
    std::list<Player_Panel*>::iterator i;
    for( i = player_panels.begin(); i != player_panels.end(); ++i )
    {
      (*i)->draw( *dc );
    }
    draw_mark( *dc );

#ifdef DOUBLE_BUFFER
#ifndef __WXGTK__		// work around for wxGTK which doesn't draw text on MemoryDC
#else
    // draw text directly on the real device context
    _dc.BeginDrawing();
    _dc.Blit(0,0, width, height, dc, 0, 0 );
    dc = &_dc;			
#endif
#endif

    board_panel.draw_text( *dc );
    stone_panel.draw_text( *dc );
    for( i = player_panels.begin(); i != player_panels.end(); ++i )
    {
      (*i)->draw_text( *dc );
    }

#ifdef DOUBLE_BUFFER
#ifndef __WXGTK__
    // draw text directly on the real device context
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

  void Game_Window::on_erase_background( wxEraseEvent &WXUNUSED(event) )
  {
    // do nothing, just don't erase background...
  }
  void Game_Window::on_mouse_event( wxMouseEvent &event )
  {
    if( event.LeftDown() )	// if event is left click
    {
      get_frame().SetStatusText(wxT("")); // remove old status text message

      int x, y;
      GetViewStart( &x, &y );	// window might be scrolled
      x = x*10 + event.GetX();
      y = y*10 + event.GetY();
      board_panel.on_click( x, y );
      stone_panel.on_click( x, y );
      std::list<Player_Panel*>::iterator i;
      for( i = player_panels.begin(); i != player_panels.end(); ++i )
      {
	(*i)->on_click( x, y );
      }
    }
    else
    {
      if( event.RightDown() )	// if event is left click
      {
	if( sequence_generator )
	{
	  delete sequence_generator->undo_click();
	}
	set_mark(-1,-1);
	get_frame().SetStatusText(wxT(""));
	refresh();
      }
      else
	assert(false);
    }
  }
  
  void Game_Window::refresh()
  {
    Refresh();
    Update();
    /*
    wxWindowDC dc(this);
    dc.BeginDrawing();
    PrepareDC(dc);
    OnDraw(dc);
    dc.EndDrawing();
    */
  }
  
  bool Game_Window::load_skin( wxString filename )
  {
    wxZipInputStream *input;
    wxImage image;

    input = new wxZipInputStream( filename, wxT("field_removed.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmaps[field_removed] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_empty.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmaps[field_empty] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_white.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmaps[field_white] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_gray.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmaps[field_gray] = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_black.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    bitmaps[field_black] = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("field_mark.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    field_mark = image.ConvertToBitmap();
    delete input;
    input = new wxZipInputStream( filename, wxT("field_mark2.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    field_mark2 = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("background.png") );
    if( input->Eof() ) { delete input; return false; }
    image.LoadFile( *input, wxBITMAP_TYPE_PNG );
    background = image.ConvertToBitmap();
    delete input;

    input = new wxZipInputStream( filename, wxT("skin.ini") );
    //if( input->Eof() ) { delete input; return false; } // ini file might be empty
    wxFileConfig config( *input );
    dimensions = Dimensions( config, bitmaps[field_white] );
    delete input;

    place_child_windows();

    return true;
  }
  void Game_Window::set_skin_file( wxString filename )
  {
    skin_file = filename;
    if( load_skin(filename) )
    {
      wxConfigBase* cfg = wxConfig::Get();
      cfg->Write( wxT("skin_file"), filename );
      cfg->Flush();
    }
    //Clear();
    Refresh();
    Update();
    refresh();
  }
  void Game_Window::set_beep_file( wxString filename )
  {
    if( wxFile::Exists( filename ) )
    {
      beep_file = filename;
      set_play_sound( true );
#if wxUSE_WAVE
      if( sound_beep.Create(filename) )
	sound_beep.Play();
      else
	play_sound = false;
#endif
      
      if( play_sound )
      {
	wxConfigBase* cfg = wxConfig::Get();
	cfg->Write( wxT("beep_file"), filename );
	cfg->Flush();
      }
    }
  }
  void Game_Window::set_play_sound( bool sound )
  {
    play_sound = sound;

    wxConfigBase* cfg = wxConfig::Get();
    cfg->Write( wxT("play_sound"), sound );
    cfg->Flush();
  }
  void Game_Window::beep()
  {
#if wxUSE_WAVE
    if( play_sound )
      sound_beep.Play();
#endif
  }

  const Player_Panel *Game_Window::get_player_panel( int id ) const
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
  // Standalone_Player_Setup_Manager
  // ----------------------------------------------------------------------------

  Standalone_Player_Setup_Manager::Standalone_Player_Setup_Manager( Game_Window &game_window )
    : game_window(game_window), player_handler(0), current_id(42),
      ruleset(Standard_Ruleset()), ruleset_type(Ruleset::standard_ruleset)
  {
  }
  Standalone_Player_Setup_Manager::~Standalone_Player_Setup_Manager()
  {
    if( player_handler )
      player_handler->aborted();
  }

  // player commands
  void Standalone_Player_Setup_Manager::set_player_handler( Player_Handler *handler )
  {
    player_handler = handler;
    if( handler )
    {
      // tell handler all players
      std::list<Player>::iterator player;
      for( player = players.begin(); player != players.end(); ++player )
      {
	player_handler->player_added(*player);
      }
      // tell handler which ruleset is currently active
      player_handler->ruleset_changed(ruleset_type);
    }
  }
  void Standalone_Player_Setup_Manager::new_game()
  {
  }
  void Standalone_Player_Setup_Manager::stop_game()
  {
  }
  bool Standalone_Player_Setup_Manager::add_player( std::string name, Player::Player_Type type, 
						    Player::Help_Mode help_mode )
  {
    if( players.size() < ruleset.get_max_players() )
    {
      int id = current_id; ++current_id;
      
      Player_Input *input;
      if( type == Player::ai )
	input = &game_window.get_ai_handler();
      else
	input = &game_window.get_mouse_handler();

      Player player( name, id, input, Player::no_output,
		   "", type, help_mode );
      
      std::list<Player>::iterator p = players.insert(players.end(), player);
      id_player[id] = p;
    
      if( player_handler )
	player_handler->player_added(*p);

      return true;
    }
    return false;
  }
  bool Standalone_Player_Setup_Manager::remove_player( int id )
  {
    std::list<Player>::iterator player = id_player[id];
    if( player_handler )
      player_handler->player_removed(*player);

    players.erase(player);
    id_player.erase( id ); 
    return true;
  }
  bool Standalone_Player_Setup_Manager::player_up( int id )
  {
    std::list<Player>::iterator player = id_player[id];
    if( player == players.begin() ) // is first player
      return false;
    std::list<Player>::iterator player2 = player; --player2;

    Player p1 = *player;
    players.erase(player);
    player = players.insert(player2,p1);	// insert player before player 2
    
    id_player[p1.id] = player;

    if( player_handler )
      player_handler->player_up(*player);

    return true;
  }
  bool Standalone_Player_Setup_Manager::player_down( int id )
  {
    std::list<Player>::iterator player = id_player[id];
    std::list<Player>::iterator player2 = player; ++player2;

    if( player2 == players.end() ) // is last player
      return false;

    Player p2 = *player2;
    players.erase(player2);
    player2 = players.insert(player,p2);	// insert player 2 before player
    
    id_player[p2.id] = player2;

    if( player_handler )
      player_handler->player_down(*player);

    return true;
  }

  bool Standalone_Player_Setup_Manager::ready()
  {
    if( players.size() >= ruleset.get_min_players() )
    {
      game_window.new_game( players, ruleset );
      game_window.continue_game();
      return true;
    }
    return false;
  }

  bool Standalone_Player_Setup_Manager::change_ruleset( Ruleset::type type, Ruleset new_ruleset )
  {
    ruleset_type = type;
    ruleset = new_ruleset;	// standalone may trust that ruleset corresponds to type
    return true;
  }

  // ----------------------------------------------------------------------------
  // main frame
  // ----------------------------------------------------------------------------

  // frame constructor
  Main_Frame::Main_Frame( const wxString& title )
    : wxFrame( /*parent*/0, /*id*/-1, title, restore_position(), restore_size(), 
	       wxDEFAULT_FRAME_STYLE | wxHSCROLL | wxVSCROLL ),
      ruleset( Standard_Ruleset() ),
      game( ruleset ),
      game_window(this, game),
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
    menu_file->Append(HOLTZ_NEW_GAME,		_("&New Game\tCtrl-N"), _("Start a new game"));
    menu_file->Append(HOLTZ_STANDALONE_GAME,	_("Stand&alone Game\tCtrl-A"), 
						_("Start a game without network access"));
    menu_file->Append(HOLTZ_NETWORK_GAME,	_("Ne&twork Game\tCtrl-T"), _("Start a network game"));
    menu_file->AppendSeparator();
    menu_file->Append(HOLTZ_QUIT,		_("E&xit\tAlt-X"), _("Quit Holtz"));

    // the "Setting" item should be in the help menu
    setting_menu = new wxMenu;
    setting_menu->Append(HOLTZ_SKIN, _("Choose &Skin\tCtrl-S"),       _("Choose skin package file"));
    setting_menu->Append(HOLTZ_BEEP, _("Choose &Beep Sound\tCtrl-B"), _("Choose beep wav file"));
    setting_menu->Append(HOLTZ_SOUND, _("&Play Sound\tCtrl-P"), _("Switch Sounds on/off"), true);
    setting_menu->Check( HOLTZ_SOUND, game_window.is_play_sound() );

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
  
  // event handlers

  void Main_Frame::on_new_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.setup_new_game();
  }

  void Main_Frame::on_standalone_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.setup_standalone_game();
  }
  
  void Main_Frame::on_network_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.setup_network_game();
  }

  void Main_Frame::on_choose_skin(wxCommandEvent& event)
  {
    wxString filename = wxFileSelector( _("Choose a skin File"), wxT(""), 
					wxT(""), wxT(""), _("Skin Files (*.zip)|*.zip"),
					wxOPEN );
    if( filename )
      game_window.set_skin_file( filename );
  }

  void Main_Frame::on_choose_beep(wxCommandEvent& event)
  {
    wxString filename = wxFileSelector( _("Choose a beep File"), wxT(""), 
					wxT(""), wxT(""), _("Audio (*.wav)|*.wav"),
					wxOPEN );
    if( filename )
      game_window.set_beep_file( filename );

    // change Menu
    bool sound = game_window.is_play_sound();
    setting_menu->Check( HOLTZ_SOUND, sound );
  }

  void Main_Frame::on_toggle_sound(wxCommandEvent& event)
  {
    bool sound = game_window.is_play_sound();
    sound = !sound;		// toggle
    game_window.set_play_sound( sound );
    sound = game_window.is_play_sound(); // get real state

    // change Menu
    setting_menu->Check( HOLTZ_SOUND, sound );
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
    msg =  _("Holtz is game about making sacrifices\n");
    msg += _("GPLed by Martin Trautmann (2003)");

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

