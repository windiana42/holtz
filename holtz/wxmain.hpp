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

#include <wx/wx.h>

#ifndef __WXHOLTZ_MAIN__
#define __WXHOLTZ_MAIN__

#include "holtz.hpp"

namespace holtz
{
  class Game_Window;
}

#include "manager.hpp"
#include "dialogs.hpp"

namespace holtz
{
  // ============================================================================
  // wx application class
  // ============================================================================

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
    Game_Window( wxFrame *parent_frame );
    ~Game_Window();
    bool on_close();		// return true: really close

    void load_settings();

    void new_game();
    void undo_move();
    void settings_dialog();

    void show_status_text( wxString text ); // shows text in status bar
    void init_scrollbars();

    void OnDraw( wxDC &dc );
    void on_erase_background( wxEraseEvent &event );
    void on_mouse_event( wxMouseEvent &event );
    void refresh();
    wxDC *get_client_dc();	// must be destroyed

    inline wxFrame &get_frame() { return parent_frame; }
    inline WX_GUI_Manager &get_gui_manager() { return gui_manager; }
  private:
    wxFrame &parent_frame;

    Game_Manager   game_manager;
    WX_GUI_Manager gui_manager;
    Game_Dialog    game_dialog;

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
    void on_new_game(wxCommandEvent& event);
    void on_undo_move(wxCommandEvent& event);
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
    HOLTZ_SETTINGS,
    HOLTZ_UNDO,
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
    DIALOG_HELP,
    DIALOG_RULESET,
    DIALOG_WIN_CHOICE,
    DIALOG_WIN_SPIN,
    DIALOG_STONES_CHOICE,
    DIALOG_STONES_SPIN,
    DIALOG_APPLY,
    DIALOG_RESTORE,
    DIALOG_SPIN,
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

    LISTBOX_DCLICK,

    DIALOG_OK = wxID_OK,	// the names are just for consistence
    DIALOG_CANCEL = wxID_CANCEL // wxID_OK and wxID_CANCEL have special functions in dialogs
  };
}

// declare the wxGetApp() function
DECLARE_APP(holtz::wxHoltz) //**/

#endif
