/*
 * dialogs.hpp
 * 
 * GUI declaration
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
#include <wx/spinctrl.h>

#ifndef __WXHOLTZ_DIALOG__
#define __WXHOLTZ_DIALOG__

#include "network.hpp"
#include "wxholtz.hpp"
#include "holtz.hpp"

namespace holtz
{
  // ============================================================================
  // Network_Clients_Dialog
  // ============================================================================

  class Network_Clients_Dialog : public wxDialog, Network_Connection_Handler
  {
  public:
    Network_Clients_Dialog( wxWindow *parent, Network_Manager &network_manager );
    ~Network_Clients_Dialog();

    virtual void new_connection( wxIPV4address host, wxSocketBase *socket );
    virtual void closed_connection( wxSocketBase *socket );

    void on_disconnect( wxCommandEvent& event );
    void on_dclick( wxCommandEvent& event );
  private:
    wxListBox *client_list;
    std::map<void*,wxSocketBase*> client_data;
    std::map<wxSocketBase*,int> client_item;
    Network_Manager &network_manager;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
  };

  // ============================================================================
  // Player_Setup_Dialog
  // ============================================================================

  class Player_Setup_Page : public wxPanel, public Player_Handler
  {
  public:
    Player_Setup_Page( wxWindow *parent, Player_Setup_Dialog *, Game_Window &game_window, 
		       Player_Setup_Manager &player_setup_manager );
    ~Player_Setup_Page();

    void on_ready( wxCommandEvent& event );
    void on_cancel( wxCommandEvent& event );
    void on_player_name( wxCommandEvent& event );
    void on_add_player( wxCommandEvent& event );
    void on_remove_player( wxCommandEvent& event );
    void on_player_up( wxCommandEvent& event );
    void on_player_down( wxCommandEvent& event );
    void on_change_ruleset( wxCommandEvent& event );
    void on_close( wxCloseEvent& event );
    void set_custom_ruleset( Ruleset& );

    // as player handler this dialog has to show results of player manipulations
    virtual void player_added( const Player & );
    virtual void player_removed( const Player & );
    virtual void player_up( const Player & );
    virtual void player_down( const Player & );
    virtual void player_change_denied();
    virtual void ruleset_changed( Ruleset::Ruleset_Type );
    virtual void ruleset_changed( Ruleset::Ruleset_Type, Ruleset& );
    virtual void ruleset_change_denied();
    virtual void aborted();

  private:
    Player_Setup_Dialog *dialog;
    Game_Window &game_window;
    Player_Setup_Manager *player_setup_manager;

    wxTextCtrl *player_name;
    wxCheckBox *ai;
    wxListBox *player_list;
    std::map<int,int> player_id;   // item_number->id
    std::map<int,int> player_item; // id->item_number

    wxRadioBox *help_choice, *ruleset_choice;
    int current_ruleset, last_ruleset;

    DECLARE_EVENT_TABLE()
  };

  class Ruleset_Setup_Page : public wxPanel
  {
  public:
    Ruleset_Setup_Page( wxWindow *parent, Player_Setup_Dialog * );
    ~Ruleset_Setup_Page();

    Ruleset *get_ruleset();	// get copy of ruleset
    void set_ruleset( Ruleset * );
    void restore_ruleset();

    void on_apply  ( wxCommandEvent& event );
    void on_restore( wxCommandEvent& event );
    void on_cancel ( wxCommandEvent& event );
    void on_change_win  ( wxCommandEvent& event );
    void on_spin_win ( wxCommandEvent& event );
    void on_change_stones  ( wxCommandEvent& event );
    void on_spin_stones ( wxCommandEvent& event );
  private:
    Player_Setup_Dialog *dialog;
    Ruleset *ruleset;

    wxRadioBox *board_choice;
    wxRadioBox *win_choice;
    wxSpinCtrl *win_white, *win_grey, *win_black, *win_all;
    wxRadioBox *stones_choice;
    wxSpinCtrl *stones_white, *stones_grey, *stones_black;

    DECLARE_EVENT_TABLE()
  };

  class Player_Setup_Dialog : public wxDialog
  {
  public:
    Player_Setup_Dialog( wxWindow *parent, Game_Window &game_window, 
			 Player_Setup_Manager &player_setup_manager );

    void aborted();

    wxNotebook *notebook;

    Player_Setup_Page  *player_page;
    Ruleset_Setup_Page *ruleset_page;
  };

  // ============================================================================
  // Settings Dialog
  // ============================================================================

  class Display_Setup_Page : public wxPanel
  {
  public:
    Display_Setup_Page( wxWindow *parent, Settings_Dialog *, Game_Window & );
    ~Display_Setup_Page();

    void restore_settings();

    void on_ok     ( wxCommandEvent& event );
    void on_apply  ( wxCommandEvent& event );
    void on_restore( wxCommandEvent& event );
    void on_cancel ( wxCommandEvent& event );
  private:
    Settings_Dialog *dialog;
    Game_Window &game_window;

    Game_Panel::Settings game_settings;

    wxRadioBox *orientation_choice;
    wxCheckBox *show_coordinates;
    wxRadioBox *arrangement_choice;
    wxCheckBox *multiple_player_stones;
    wxCheckBox *multiple_common_stones;

    DECLARE_EVENT_TABLE()
  };

  class Settings_Dialog : public wxDialog
  {
  public:
    Settings_Dialog( wxWindow *parent, Game_Window &game_window );

    wxNotebook *notebook;

    Display_Setup_Page *display_page;
  };

  // ============================================================================
  // Network_Connection_Dialog
  // ============================================================================

  class Network_Connection_Dialog : public wxDialog // modal dialog
  {
  public:
    Network_Connection_Dialog( wxWindow *parent );

    wxFlexGridSizer *sizer;
    wxRadioButton *server, *client;
    wxTextCtrl *hostname;
    wxSpinCtrl *port;
  private:
    DECLARE_EVENT_TABLE()
  };
}

#endif
