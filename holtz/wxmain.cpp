/*
 * wxmain.cpp
 * 
 * wxWindows GUI framework
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

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------

#include "wxmain.hpp"

#include "util.hpp"

#include <wx/wx.h>
#include <wx/html/helpctrl.h> // use HTML help
#include <wx/cshelp.h>
#include <wx/zipstrm.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>

namespace holtz
{
  // ----------------------------------------------------------------------------
  // resources
  // ----------------------------------------------------------------------------

  // the application icon
#ifndef __WXMSW__
#include "icon.xpm"
#endif

  // ----------------------------------------------------------------------------
  // event tables and other macros for wxWindows
  // ----------------------------------------------------------------------------

  // the event tables connect the wxWindows events with the functions (event
  // handlers) which process them. It can be also done at run-time, but for the
  // simple menu events like this the static method is much simpler.
  BEGIN_EVENT_TABLE(Main_Frame, wxFrame)				//**/
  EVT_MENU(HOLTZ_NEW_GAME,	  Main_Frame::on_new_game)		//**/
  EVT_MENU(HOLTZ_UNDO,		  Main_Frame::on_undo_move)		//**/
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
  // Game Window
  // ----------------------------------------------------------------------------

  Game_Window::Game_Window( wxFrame *parent_frame )
    : wxScrolledWindow( parent_frame ),
      parent_frame(*parent_frame),
      gui_manager( game_manager, *this ),
      game_dialog( this, game_manager, gui_manager )
  {
    game_manager.set_ui_manager( &gui_manager );
    Standalone_Game_Setup_Manager *game_setup_manager = new Standalone_Game_Setup_Manager( game_manager );
    game_manager.set_game_setup_manager( game_setup_manager );
    game_setup_manager->set_display_handler( &game_dialog );

    SetBackgroundColour(*wxWHITE);
  }

  Game_Window::~Game_Window()
  {
    game_manager.stop_game();
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
    game_manager.new_game();
  }

  void Game_Window::undo_move()
  {
    game_manager.undo_move();
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
    wxMenu *file_menu = new wxMenu;
    file_menu->Append(HOLTZ_NEW_GAME,		_("&New Game...\tCtrl-N"), _("Start a new game"));
    file_menu->AppendSeparator();
    file_menu->Append(HOLTZ_QUIT,		_("E&xit\tAlt-X"), _("Quit Holtz"));

    // the "Setting" item should be in the help menu
    setting_menu = new wxMenu;
    setting_menu->Append(HOLTZ_SETTINGS, _("Display s&ettings...\tCtrl-E"),  _("Change display settings"));

    // the "Setting" item should be in the help menu
    game_menu = new wxMenu;
    game_menu->Append(HOLTZ_UNDO, _("&Undo move\tCtrl-U"),  _("Try to undo move"));

    // the "About" item should be in the help menu
    wxMenu *help_menu = new wxMenu;
    help_menu->Append(HOLTZ_HELP_CONTENTS, _("Contents\tF1"), _("Show help file"));
    help_menu->Append(HOLTZ_HELP_LICENSE, _("License"), _("Information about the Holtz license"));
    help_menu->Append(HOLTZ_ABOUT, _("About"), _("Show about dialog"));

    // now append the freshly created menu to the menu bar...
    wxMenuBar *menu_bar = new wxMenuBar();
    menu_bar->Append(file_menu,    _("&File"));
    menu_bar->Append(setting_menu, _("&Settings"));
    menu_bar->Append(game_menu,    _("&Game"));
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

  void Main_Frame::on_undo_move(wxCommandEvent& WXUNUSED(event))
  {
    game_window.undo_move();
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

