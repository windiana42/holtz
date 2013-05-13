/*
 * wxmain.hpp
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

#ifndef __WXHOLTZ_MAIN__
#define __WXHOLTZ_MAIN__

namespace holtz
{
  class Game_Window;
}

#include "zertz/manager.hpp"
#include "dvonn/manager.hpp"
#include "bloks/manager.hpp"
#include "relax/manager.hpp"
#include "zertz/dialogs.hpp"
#include "dvonn/dialogs.hpp"
#include "bloks/dialogs.hpp"
#include "relax/dialogs.hpp"
#include "zertz/wxzertz.hpp"
#include "dvonn/wxdvonn.hpp"
#include "bloks/wxbloks.hpp"
#include "relax/wxrelax.hpp"

#include <wx/wx.h>
#include <wx/wizard.h>

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

namespace holtz
{
  // ==========================================================================
  // wx application class
  // ==========================================================================

  class wxHoltz : public wxApp
  {
  public:
    // override base class virtuals
    // ----------------------------
    
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

    bool check_config();
    bool init_help(wxLocale&);
    wxHelpControllerBase& get_help_controller() { return help_controller; }

    ~wxHoltz();

  private:
    wxHtmlHelpController help_controller; // HTML help
    wxConfigBase* global_config; // so that it can be deleted
  };

  // ============================================================================
  // frame or window classes
  // ============================================================================

  /*! class Game_Window
   *  window component on which the game is placed
   */

  class Game_Window : public wxScrolledWindow
  {
  public:
    enum Game_Type { NO_GAME, ZERTZ, DVONN, RELAX, BLOKS };

    Game_Window( wxFrame *parent_frame );
    ~Game_Window();
    bool on_close();		// return true: really close

    void load_settings();

    void close_game();
    void init_zertz();
    void init_dvonn();
    void init_bloks();
    void init_relax();

    void new_zertz_game();
    void new_dvonn_game();
    void new_bloks_game();
    void new_relax_game();
    void undo_move();
    void variants();
    void settings_dialog();

    void show_status_text( wxString text ); // shows text in status bar
    void init_scrollbars();

    const wxBitmap &get_background_bitmap();
    void OnDraw( wxDC &dc );
    void on_erase_background( wxEraseEvent &event );
    void on_mouse_event( wxMouseEvent &event );
    void on_wizard_page_changing( wxWizardEvent& event );
    void on_wizard_finished( wxWizardEvent& event );
    void on_wizard_cancel( wxWizardEvent& event );
    void refresh();
    wxDC *get_client_dc();	// must be destroyed

    inline wxFrame &get_frame() { return parent_frame; }
    Game_Type get_active_game() { return active_game; }
    inline zertz::WX_GUI_Manager &get_zertz_gui_manager() 
    { assert(active_game==ZERTZ); return *zertz_gui_manager; }
    inline dvonn::WX_GUI_Manager &get_dvonn_gui_manager() 
    { assert(active_game==DVONN); return *dvonn_gui_manager; }
    inline bloks::WX_GUI_Manager &get_bloks_gui_manager() 
    { assert(active_game==BLOKS); return *bloks_gui_manager; }
    inline relax::WX_GUI_Manager &get_relax_gui_manager() 
    { assert(active_game==RELAX); return *relax_gui_manager; }
  private:
    wxFrame &parent_frame;

    Game_Type active_game;

    zertz::Game_Manager		*zertz_game_manager;
    zertz::Game_Variants_Frame	*zertz_variants_frame;
    zertz::WX_GUI_Manager	*zertz_gui_manager;
    zertz::Game_Dialog		*zertz_game_dialog;

    dvonn::Game_Manager		*dvonn_game_manager;
    dvonn::Game_Variants_Frame	*dvonn_variants_frame;
    dvonn::WX_GUI_Manager	*dvonn_gui_manager;
    dvonn::Game_Dialog		*dvonn_game_dialog;

    bloks::Game_Manager		*bloks_game_manager;
    bloks::Game_Variants_Frame	*bloks_variants_frame;
    bloks::WX_GUI_Manager	*bloks_gui_manager;
    bloks::Game_Dialog		*bloks_game_dialog;

    relax::Game_Manager		*relax_game_manager;
    relax::Game_Variants_Frame	*relax_variants_frame;
    relax::WX_GUI_Manager	*relax_gui_manager;
    relax::Game_Dialog		*relax_game_dialog;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
  };

  // Define a new frame type: this is going to be our main frame
  class Main_Frame : public wxFrame
  {
  public:
    // ctor(s)
    Main_Frame( const wxString& title );
    ~Main_Frame();
    wxMenuBar* create_menu();

    void save_size_and_position();
    wxSize restore_size();
    wxPoint restore_position();
    void load_settings();
    
    // event handlers (these functions should _not_ be virtual)
    void on_new_zertz_game(wxCommandEvent& event);
    void on_new_dvonn_game(wxCommandEvent& event);
    void on_new_bloks_game(wxCommandEvent& event);
    void on_new_relax_game(wxCommandEvent& event);
    void on_undo_move(wxCommandEvent& event);
    void on_variants(wxCommandEvent& event);
    void on_quit(wxCommandEvent& event);
    void on_settings(wxCommandEvent& event);
    void on_help_contents(wxCommandEvent& event);
    void on_help_license(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_close(wxCloseEvent& event);

    inline Game_Window &get_game_window() { return game_window; }
  private:
    Game_Window game_window;

    wxMenu *setting_menu;
    wxMenu *game_menu;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
  };


  // ----------------------------------------------------------------------------
  // constants
  // ----------------------------------------------------------------------------

  // IDs for the controls and the menu commands
  enum
  {
    // menu items
    HOLTZ_NEW_GAME = 100,
    HOLTZ_NEW_ZERTZ_GAME,
    HOLTZ_NEW_DVONN_GAME,
    HOLTZ_NEW_BLOKS_GAME,
    HOLTZ_NEW_RELAX_GAME,
    HOLTZ_SETTINGS,
    HOLTZ_UNDO,
    HOLTZ_VARIANTS,
    HOLTZ_QUIT,
    HOLTZ_HELP_CONTENTS,
    HOLTZ_HELP_LICENSE,
    HOLTZ_ABOUT,

    DIALOG_READY,
    DIALOG_PLAYER_NAME,
    DIALOG_ADD_PLAYER,
    DIALOG_REMOVE_PLAYER,
    DIALOG_PLAYER_UP,
    DIALOG_PLAYER_DOWN,
    DIALOG_DISCONNECT,
    DIALOG_ALLOW_CONNECT,
    DIALOG_HELP,
    DIALOG_RULESET,
    DIALOG_NEW_GAME_CHOICE,
    DIALOG_CONTINUE_GAME_CHOICE,
    DIALOG_BOARD_CHOICE,
    DIALOG_WIN_CHOICE,
    DIALOG_WIN_SPIN,
    DIALOG_STONES_CHOICE,
    DIALOG_STONES_SPIN,
    DIALOG_WIZARD,
    DIALOG_APPLY,
    DIALOG_RESTORE,
    DIALOG_SPIN,
    DIALOG_HOST_NAME,
    DIALOG_CONNECT,
    DIALOG_SERVER_PORT,
    DIALOG_CLIENT_PORT,
    DIALOG_CHOOSE_PBM_DIRECTORY,
    DIALOG_CHANGE_PBM_DIRECTORY,
    DIALOG_CHOOSE_LG_DIRECTORY,
    DIALOG_CHANGE_LG_DIRECTORY,
    DIALOG_CHOOSE_DIRECTORY,
    DIALOG_CHANGE_DIRECTORY,
    DIALOG_CHOOSE_FILE,
    DIALOG_CHANGE_FILE,
    DIALOG_CHOOSE_SKIN_FILE,
    DIALOG_CHANGE_SKIN_FILE,
    DIALOG_CHOOSE_BEEP_FILE,
    DIALOG_CHANGE_BEEP_FILE,
    DIALOG_CHOOSE_PLAYER_FONT,
    DIALOG_CHOOSE_COORD_FONT,
    DIALOG_CHOOSE_STONE_FONT,

    NETWORK_TIMER,
    ANIMATION_DONE,
    ANIMATION_ABORTED,
    LISTBOX_DCLICK,

    DIALOG_OK = wxID_OK,	// the names are just for consistence
    DIALOG_CANCEL = wxID_CANCEL // wxID_OK and wxID_CANCEL have special functions in dialogs
  };
}

// declare the wxGetApp() function
DECLARE_APP(holtz::wxHoltz) //**/

#endif
