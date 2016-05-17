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
#include "zertz/network.hpp" //workaround for cygwin
#include "dvonn/network.hpp" //workaround for cygwin
#include "bloks/network.hpp" //workaround for cygwin
#include "relax/network.hpp" //workaround for cygwin

#include "wxmain.hpp"

#include "util.hpp"

#include <wx/wx.h>
#include <wx/image.h>
#include <wx/html/helpctrl.h> // use HTML help
#include <wx/cshelp.h>
#include <wx/zipstrm.h>
#include <wx/filesys.h>
#include <wx/fs_zip.h>
#include <wx/filefn.h>
#include <wx/stdpaths.h>

#ifndef DEFAULT_DATA_DIR
#define DEFAULT_DATA_DIR "./"	// will be overridden by Makefile.am
#endif
#ifndef DEFAULT_DATA_DIR2
#define DEFAULT_DATA_DIR2 "./"
#endif

// enables to reproduce a specific sequence of random numbers
#define RANDOMIZE
//#define FIXED_RANDOM_SEED 123456789

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
  EVT_MENU(HOLTZ_NEW_ZERTZ_GAME,  Main_Frame::on_new_zertz_game)	//**/
  EVT_MENU(HOLTZ_NEW_DVONN_GAME,  Main_Frame::on_new_dvonn_game)	//**/
  EVT_MENU(HOLTZ_NEW_BLOKS_GAME,  Main_Frame::on_new_bloks_game)	//**/
  EVT_MENU(HOLTZ_NEW_RELAX_GAME,  Main_Frame::on_new_relax_game)	//**/
  EVT_MENU(HOLTZ_UNDO,		  Main_Frame::on_undo_move)		//**/
  EVT_MENU(HOLTZ_VARIANTS,	  Main_Frame::on_variants)		//**/
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
  EVT_WIZARD_CANCEL(DIALOG_WIZARD, Game_Window::on_wizard_cancel)	//**/
  EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD, Game_Window::on_wizard_page_changing) //**/
  EVT_WIZARD_FINISHED(DIALOG_WIZARD, Game_Window::on_wizard_finished)	//**/

#ifdef DRAW_BACKGROUND
  EVT_ERASE_BACKGROUND(Game_Window::on_erase_background)		//**/
#endif
  END_EVENT_TABLE()							//**/

  // ============================================================================
  // implementation
  // ============================================================================

  /* for translation selection dialog
  // language data
  static const wxLanguage langIds[] =
  {
    wxLANGUAGE_USER_DEFINED,
    wxLANGUAGE_GERMAN
  };

  // note that it makes no sense to translate these strings, they are
  // shown before we set the locale anyhow
  const wxString langNames[] =
    {
      wxT("English"),
      wxT("Deutsch")
    };
  */

  // ----------------------------------------------------------------------------
  // the application class
  // ----------------------------------------------------------------------------

  // 'Main program' equivalent: the program execution "starts" here
  bool wxHoltz::OnInit()
  {
#ifdef RANDOMIZE
    randomize();
#else
#  ifdef FIXED_RANDOM_SEED
#    warning "!!!undefine FIXED_RANDOM_SEED before compiling production code!!!"
    srand(FIXED_RANDOM_SEED);
#  endif
#endif  

    wxLocale *loc = new wxLocale();

    int language = wxLANGUAGE_DEFAULT;
    if( argc > 1 )
      {
	if( wxString(argv[1]) == wxT("de") || wxString(argv[1]) == wxT("de_DE") )
	{
	  language = wxLANGUAGE_GERMAN;
	}	
	if( wxString(argv[1]) == wxT("it") || wxString(argv[1]) == wxT("it_IT") )
	{
	  language = wxLANGUAGE_ITALIAN;
	}
      }
    loc->Init(language); // get default language from OS
    wxLocale::AddCatalogLookupPathPrefix(wxT("locale")); // enable translation lookup from ./locale/
    loc->AddCatalog(wxT("holtz")); // load translation file holtz.mo if available
    loc->AddCatalog(wxT("holtz-hotkey")); // load translation file holtz-hotkey.mo if available

    SetAppName(wxT("Holtz"));
    SetVendorName(wxT("Martin Trautmann"));
    global_config = new wxConfig(GetAppName());
    wxConfig::Set(global_config);

    // this will link all image libraries, also the unused ones
    // wxInitAllImageHandlers(); // make it possible to load PNG images
    // that's better, we don't use other image formats:
    wxImage::AddHandler(new wxXPMHandler);
    wxImage::AddHandler(new wxPNGHandler);
    wxImage::AddHandler(new wxJPEGHandler);
    // for html help
    wxHelpControllerHelpProvider* provider = new wxHelpControllerHelpProvider;
    provider->SetHelpController(&get_help_controller());
    wxHelpProvider::Set(provider);
    // for help: zip files
    wxFileSystem::AddHandler(new wxZipFSHandler);
    if(!init_help(*loc))
      wxLogWarning(wxT("%s"),_("No help file found."));

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
    // - data_dir1/help/help_la_co.*
    // - data_dir1/help/help_la.*
    // - data_dir2/help/help_la_co.*
    // - data_dir2/help/help_la.*
    // - data_dir1/help/help_en.*
    // - data_dir2/help/help_en.*
    // where 'la' is the language, and 'co' the country of the currently used locale
    // if all fails, return false
    wxString language = loc.GetCanonicalName();
    if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
					 + wxT("help/help_") + language))
      if(language.Len() <= 2 
	 || !get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
					      + wxT("help/help_") + language.Left(2)))
	if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR2)) 
					     + wxT("help/help_") + language))
	  if(language.Len() <= 2 
	     || !get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR2)) 
						  + wxT("help/help_") 
						  + language.Left(2)))
	    if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR)) 
						 + wxT("help/help_en")))
	      if(!get_help_controller().Initialize(wxString(wxT(DEFAULT_DATA_DIR2)) 
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
      active_game(NO_GAME),
      zertz_game_manager(0), zertz_variants_frame(0), zertz_gui_manager(0), zertz_game_dialog(0),
      dvonn_game_manager(0), dvonn_variants_frame(0), dvonn_gui_manager(0), dvonn_game_dialog(0),
      bloks_game_manager(0), bloks_variants_frame(0), bloks_gui_manager(0), bloks_game_dialog(0),
      relax_game_manager(0), relax_variants_frame(0), relax_gui_manager(0), relax_game_dialog(0)
  {
    SetBackgroundColour(*wxWHITE);
  }

  Game_Window::~Game_Window()
  {
    close_game();
  }
  void Game_Window::close_game()
  {
    switch( active_game ) {
    case ZERTZ:
      if( zertz_game_manager )
	zertz_game_manager->stop_game();
      active_game = NO_GAME;
      delete zertz_game_dialog; // still needs game manager
      delete zertz_gui_manager;
      delete zertz_game_manager;
      zertz_variants_frame->Destroy();
      zertz_variants_frame = 0;
      break;
    case DVONN:
      if( dvonn_game_manager )
	dvonn_game_manager->stop_game();
      active_game = NO_GAME;
      delete dvonn_game_dialog; // still needs game manager
      delete dvonn_gui_manager;
      delete dvonn_game_manager;
      dvonn_variants_frame->Destroy();
      dvonn_variants_frame = 0;
      break;
    case BLOKS:
      if( bloks_game_manager )
	bloks_game_manager->stop_game();
      active_game = NO_GAME;
      delete bloks_game_dialog; // still needs game manager
      delete bloks_gui_manager;
      delete bloks_game_manager;
      bloks_variants_frame->Destroy();
      bloks_variants_frame = 0;
      break;
    case RELAX:
      if( relax_game_manager )
	relax_game_manager->stop_game();
      active_game = NO_GAME;
      delete relax_game_dialog; // still needs game manager
      delete relax_gui_manager;
      delete relax_game_manager;
      relax_variants_frame->Destroy();
      relax_variants_frame = 0;
      break;
    case NO_GAME:
      break;
    }
  }
  
  bool Game_Window::on_close()
  {
    return true;
  }

  void Game_Window::load_settings()
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_gui_manager->load_settings();
      break;
    case DVONN:
      dvonn_gui_manager->load_settings();
      break;
    case BLOKS:
      bloks_gui_manager->load_settings();
      break;
    case RELAX:
      relax_gui_manager->load_settings();
      break;
    case NO_GAME:
      break;
    }
  }

  void Game_Window::init_zertz() 
  {
    close_game();
    zertz_game_manager   = new zertz::Game_Manager(wxEVT_ZERTZ_NOTIFY);
    zertz_variants_frame = new zertz::Game_Variants_Frame( this );
    zertz_gui_manager    = new zertz::WX_GUI_Manager( *zertz_game_manager, *this, 
						      *zertz_variants_frame->get_game_variants() );
    zertz_game_dialog    = new zertz::Game_Dialog( this, *zertz_game_manager, *zertz_gui_manager );

    active_game = ZERTZ;
    refresh();
  }

  void Game_Window::init_dvonn() 
  {
    close_game();
    dvonn_game_manager   = new dvonn::Game_Manager(wxEVT_DVONN_NOTIFY);
    dvonn_variants_frame = new dvonn::Game_Variants_Frame( this );
    dvonn_gui_manager    = new dvonn::WX_GUI_Manager( *dvonn_game_manager, *this, 
						      *dvonn_variants_frame->get_game_variants() );
    dvonn_game_dialog    = new dvonn::Game_Dialog( this, *dvonn_game_manager, *dvonn_gui_manager );

    active_game = DVONN;
    refresh();
  }
  
  void Game_Window::init_bloks() 
  {
    close_game();
    bloks_game_manager   = new bloks::Game_Manager(wxEVT_BLOKS_NOTIFY);
    bloks_variants_frame = new bloks::Game_Variants_Frame( this );
    bloks_gui_manager    = new bloks::WX_GUI_Manager( *bloks_game_manager, *this, 
						      *bloks_variants_frame->get_game_variants() );
    bloks_game_dialog    = new bloks::Game_Dialog( this, *bloks_game_manager, *bloks_gui_manager );

    active_game = BLOKS;
    refresh();
  }
  
  void Game_Window::init_relax() 
  {
    close_game();
    relax_game_manager   = new relax::Game_Manager(wxEVT_RELAX_NOTIFY);
    relax_variants_frame = new relax::Game_Variants_Frame( this );
    relax_gui_manager    = new relax::WX_GUI_Manager( *relax_game_manager, *this, 
						      *relax_variants_frame->get_game_variants() );
    relax_game_dialog    = new relax::Game_Dialog( this, *relax_game_manager, *relax_gui_manager );

    active_game = RELAX;
    refresh();
  }
   
  void Game_Window::new_zertz_game()
  {
    if( active_game != ZERTZ )
    {
      if( active_game != NO_GAME )
      {
	if( wxMessageDialog
	    ( this, _("You are about to switch to Zertz and close your current game. Proceed?"),
	      _("Closing Game"), wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() 
	    != wxID_YES )
	{
	  return;
	}      
      }
      init_zertz();
    }
    
    switch( active_game ) {
    case ZERTZ:
      zertz_game_manager->new_game();
      break;
    case DVONN:
      dvonn_game_manager->new_game();
      break;
    case BLOKS:
      bloks_game_manager->new_game();
      break;
    case RELAX:
      relax_game_manager->new_game();
      break;
    case NO_GAME:
      break;
    }
  }
  
  void Game_Window::new_dvonn_game()
  {
    if( active_game != DVONN )
    {
      if( active_game != NO_GAME )
      {
	if( wxMessageDialog
	    ( this, _("You are about to switch to Dvonn and close your current game. Proceed?"),
	      _("Closing Game"), wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal()
	    != wxID_YES )
	{
	  return;
	}      
      }
      init_dvonn();
    }

    switch( active_game ) {
    case ZERTZ:
      zertz_game_manager->new_game();
      break;
    case DVONN:
      dvonn_game_manager->new_game();
      break;
    case BLOKS:
      bloks_game_manager->new_game();
      break;
    case RELAX:
      relax_game_manager->new_game();
      break;
    case NO_GAME:
      break;
    }
  }
  
  void Game_Window::new_bloks_game()
  {
    if( active_game != BLOKS )
    {
      if( active_game != NO_GAME )
      {
	if( wxMessageDialog
	    ( this, _("You are about to switch to Bloks and close your current game. Proceed?"),
	      _("Closing Game"), wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal()
	    != wxID_YES )
	{
	  return;
	}      
      }
      init_bloks();
    }

    switch( active_game ) {
    case ZERTZ:
      zertz_game_manager->new_game();
      break;
    case DVONN:
      dvonn_game_manager->new_game();
      break;
    case BLOKS:
      bloks_game_manager->new_game();
      break;
    case RELAX:
      relax_game_manager->new_game();
      break;
    case NO_GAME:
      break;
    }
  }
  
  void Game_Window::new_relax_game()
  {
    if( active_game != RELAX )
    {
      if( active_game != NO_GAME )
      {
	if( wxMessageDialog
	    ( this, _("You are about to switch to Relax and close your current game. Proceed?"),
	      _("Closing Game"), wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal()
	    != wxID_YES )
	{
	  return;
	}      
      }
      init_relax();
    }

    switch( active_game ) {
    case ZERTZ:
      zertz_game_manager->new_game();
      break;
    case DVONN:
      dvonn_game_manager->new_game();
      break;
    case BLOKS:
      bloks_game_manager->new_game();
      break;
    case RELAX:
      relax_game_manager->new_game();
      break;
    case NO_GAME:
      break;
    }
  }

  void Game_Window::undo_move()
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_gui_manager->clear_target_variant();
      zertz_game_manager->undo_moves();
      break;
    case DVONN:
      dvonn_gui_manager->clear_target_variant();
      dvonn_game_manager->undo_moves();
      break;
    case BLOKS:
      bloks_gui_manager->clear_target_variant();
      bloks_game_manager->undo_moves();
      break;
    case RELAX:
      relax_gui_manager->clear_target_variant();
      relax_game_manager->undo_moves();
      break;
    case NO_GAME:
      break;
    }
  }

  void Game_Window::variants()
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_variants_frame->show_frame();
      zertz_gui_manager->refresh();
      break;
    case DVONN:
      dvonn_variants_frame->show_frame();
      dvonn_gui_manager->refresh();
      break;
    case BLOKS:
      bloks_variants_frame->show_frame();
      bloks_gui_manager->refresh();
      break;
    case RELAX:
      relax_variants_frame->show_frame();
      relax_gui_manager->refresh();
      break;
    case NO_GAME:
      break;
    }
  }

  void Game_Window::settings_dialog()
  {
    switch( active_game ) {
    case ZERTZ:
      {
	zertz::Settings_Dialog dialog( this, *zertz_gui_manager );
	dialog.Center();
	dialog.ShowModal();
      }
      break;
    case DVONN:
      {
	dvonn::Settings_Dialog dialog( this, *dvonn_gui_manager );
	dialog.Center();
	dialog.ShowModal();
      }
      break;
    case BLOKS:
      {
	bloks::Settings_Dialog dialog( this, *bloks_gui_manager );
	dialog.Center();
	dialog.ShowModal();
      }
      break;
    case RELAX:
      {
	relax::Settings_Dialog dialog( this, *relax_gui_manager );
	dialog.Center();
	dialog.ShowModal();
      }
      break;
    case NO_GAME:
      break;
    }
  }

  void Game_Window::show_status_text( wxString text ) // shows text in status bar
  {
    parent_frame.SetStatusText( text );
  }

  void Game_Window::init_scrollbars()
  {
    switch( active_game ) {
    case ZERTZ:
      SetScrollbars( 10, 10, zertz_gui_manager->get_width() / 10 + 1, 
		     zertz_gui_manager->get_height() / 10 + 1 );
      break;
    case DVONN:
      SetScrollbars( 10, 10, dvonn_gui_manager->get_width() / 10 + 1, 
		     dvonn_gui_manager->get_height() / 10 + 1 );
      break;
    case BLOKS:
      SetScrollbars( 10, 10, bloks_gui_manager->get_width() / 10 + 1, 
		     bloks_gui_manager->get_height() / 10 + 1 );
      break;
    case RELAX:
      SetScrollbars( 10, 10, relax_gui_manager->get_width() / 10 + 1, 
		     relax_gui_manager->get_height() / 10 + 1 );
      break;
    case NO_GAME:
      break;
    }
  }
  
  const wxBitmap &Game_Window::get_background_bitmap() 
  {
    switch( active_game ) {
    case ZERTZ:
      return zertz_gui_manager->get_game_panel().get_background();
    case DVONN:
      return dvonn_gui_manager->get_game_panel().get_background();
    case BLOKS:
      return bloks_gui_manager->get_game_panel().get_background();
    case RELAX:
      return relax_gui_manager->get_game_panel().get_background();
    case NO_GAME:
      break;
    }
    assert(false);
    return *new wxBitmap();
  }
  
  void Game_Window::OnDraw( wxDC &_dc )
  {
    if( active_game == NO_GAME ) 
      return;

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
    //dc->BeginDrawing();
#endif

#ifdef DRAW_BACKGROUND
    const wxBitmap &background = get_background_bitmap();
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

    switch( active_game ) {
    case ZERTZ:
      zertz_gui_manager->draw( *dc );
      zertz_gui_manager->draw_mark( *dc );
      break;
    case DVONN:
      dvonn_gui_manager->draw( *dc );
      dvonn_gui_manager->draw_mark( *dc );
      break;
    case BLOKS:
      bloks_gui_manager->draw( *dc );
      bloks_gui_manager->draw_mark( *dc );
      break;
    case RELAX:
      relax_gui_manager->draw( *dc );
      relax_gui_manager->draw_mark( *dc );
      break;
    case NO_GAME:
      break;
    }

#ifdef DOUBLE_BUFFER
#ifdef __WXGTK__		// work around for wxGTK which doesn't draw text on MemoryDC
    // draw text directly on the real device context
    //_dc.BeginDrawing();
    _dc.Blit(0,0, width, height, dc, 0, 0 );
    dc = &_dc;	
#endif
#endif

    switch( active_game ) {
    case ZERTZ:
      zertz_gui_manager->draw_text( *dc );
      break;
    case DVONN:
      dvonn_gui_manager->draw_text( *dc );
      break;
    case BLOKS:
      bloks_gui_manager->draw_text( *dc );
      break;
    case RELAX:
      relax_gui_manager->draw_text( *dc );
      break;
    case NO_GAME:
      break;
    }

#ifdef DOUBLE_BUFFER
#ifndef __WXGTK__
    // draw buffer on the real device context
    //_dc.BeginDrawing();
    _dc.Blit(0,0, width, height, dc, 0, 0 );
    //_dc.EndDrawing();
#else
    //_dc.EndDrawing();
#endif
#else
    // dc->EndDrawing();
#endif
  }
  void Game_Window::on_erase_background( wxEraseEvent &event )
  {
    if( active_game == NO_GAME ) 
      event.Skip(); // execute parent handler for this event
    else
    {
      // do nothing, just don't erase background...
    }
  }
  void Game_Window::on_mouse_event( wxMouseEvent &event )
  {
    int cl_x, cl_y;
    GetViewStart( &cl_x, &cl_y );	// window might be scrolled
    cl_x = cl_x*10 + event.GetX();
    cl_y = cl_y*10 + event.GetY();

    if( event.LeftDown() )	// if event is left click
    {
      switch( active_game ) {
      case ZERTZ:
	zertz_gui_manager->mouse_click_left( cl_x, cl_y );
	break;
      case DVONN:
	dvonn_gui_manager->mouse_click_left( cl_x, cl_y );
	break;
      case BLOKS:
	bloks_gui_manager->mouse_click_left( cl_x, cl_y );
	break;
      case RELAX:
	relax_gui_manager->mouse_click_left( cl_x, cl_y );
	break;
      case NO_GAME:
	break;
      }
    }
    else
    {
      if( event.RightDown() )	// if event is left click
      {
	switch( active_game ) {
	case ZERTZ:
	  zertz_gui_manager->mouse_click_right( cl_x, cl_y );
	  break;
	case DVONN:
	  dvonn_gui_manager->mouse_click_right( cl_x, cl_y );
	  break;
	case BLOKS:
	  bloks_gui_manager->mouse_click_right( cl_x, cl_y );
	  break;
	case RELAX:
	  relax_gui_manager->mouse_click_right( cl_x, cl_y );
	  break;
	case NO_GAME:
	  break;
	}
      }
    }
  }

  void Game_Window::on_wizard_page_changing( wxWizardEvent& event )
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_game_dialog->on_wizard_page_changing( event );
      break;
    case DVONN:
      dvonn_game_dialog->on_wizard_page_changing( event );
      break;
    case BLOKS:
      bloks_game_dialog->on_wizard_page_changing( event );
      break;
    case RELAX:
      relax_game_dialog->on_wizard_page_changing( event );
      break;
    case NO_GAME:
      break;
    }
  }
  void Game_Window::on_wizard_finished( wxWizardEvent& event )
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_game_dialog->on_wizard_finished( event );
      break;
    case DVONN:
      dvonn_game_dialog->on_wizard_finished( event );
      break;
    case BLOKS:
      bloks_game_dialog->on_wizard_finished( event );
      break;
    case RELAX:
      relax_game_dialog->on_wizard_finished( event );
      break;
    case NO_GAME:
      break;
    }
  }
  void Game_Window::on_wizard_cancel( wxWizardEvent& event )
  {
    switch( active_game ) {
    case ZERTZ:
      zertz_game_dialog->on_wizard_cancel( event );
      break;
    case DVONN:
      dvonn_game_dialog->on_wizard_cancel( event );
      break;
    case BLOKS:
      bloks_game_dialog->on_wizard_cancel( event );
      break;
    case RELAX:
      relax_game_dialog->on_wizard_cancel( event );
      break;
    case NO_GAME:
      break;
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
	       wxDEFAULT_FRAME_STYLE ),
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
    file_menu->Append(HOLTZ_NEW_ZERTZ_GAME, _("New &Zertz Game...\tCtrl-Z"), 
		      _("Start a new Zertz game"));
    file_menu->Append(HOLTZ_NEW_DVONN_GAME, _("New &Dvonn Game...\tCtrl-D"), 
		      _("Start a new Dvonn game"));
    file_menu->Append(HOLTZ_NEW_BLOKS_GAME, _("New &Bloks Game...\tCtrl-B"), 
		      _("Start a new Bloks game"));
    file_menu->Append(HOLTZ_NEW_RELAX_GAME, _("New &Relax Game...\tCtrl-R"), 
		      _("Start a new Relax game"));
    file_menu->AppendSeparator();
    file_menu->Append(HOLTZ_QUIT,	    _("E&xit\tAlt-X"), _("Quit Holtz"));

    // the "Setting" item should be in the help menu
    setting_menu = new wxMenu;
    setting_menu->Append(HOLTZ_SETTINGS, _("Display s&ettings...\tCtrl-E"),  
			 _("Change display settings"));

    // the "Setting" item should be in the help menu
    game_menu = new wxMenu;
    game_menu->Append(HOLTZ_UNDO, _("&Undo move\tCtrl-U"),  _("Try to undo move"));
    game_menu->Append(HOLTZ_VARIANTS, _("Show &Variants\tCtrl-V"),  
		      _("Show window with variant tree"));

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

  void Main_Frame::on_new_zertz_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.new_zertz_game();
  }

  void Main_Frame::on_new_dvonn_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.new_dvonn_game();
  }

  void Main_Frame::on_new_bloks_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.new_bloks_game();
  }

  void Main_Frame::on_new_relax_game(wxCommandEvent& WXUNUSED(event))
  {
    game_window.new_relax_game();
  }

  void Main_Frame::on_settings(wxCommandEvent& WXUNUSED(event))
  {
    game_window.settings_dialog();
  }

  void Main_Frame::on_undo_move(wxCommandEvent& WXUNUSED(event))
  {
    game_window.undo_move();
  }

  void Main_Frame::on_variants(wxCommandEvent& WXUNUSED(event))
  {
    game_window.variants();
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
    msg =  _("Holtz is an implementation of the Gipf Project Games\n");
    msg += _("Zertz and Dvonn (www.gipf.com). Additionally it supports Bloks and Relax.\n");
    msg += _("GPLed by Martin Trautmann (2007)\n");

    wxMessageDialog(this, msg, _("About Holtz"), wxOK | wxICON_INFORMATION).ShowModal();
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

