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

#ifndef __HOLTZ_DIALOG__
#define __HOLTZ_DIALOG__

namespace holtz
{
  class Network_Clients_Dialog;
  class Player_Setup_Dialog;
  class Network_Connection_Dialog;
}

#include "network.hpp"
#include "wxholtz.hpp"
#include "holtz.hpp"

namespace holtz
{
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

  class Player_Setup_Dialog : public wxDialog, Player_Handler
  {
  public:
    Player_Setup_Dialog( wxWindow *parent, Game_Window &game_window, 
			 Player_Setup_Manager &player_setup_manager );
    ~Player_Setup_Dialog();

    void on_ready( wxCommandEvent& event );
    void on_cancel( wxCommandEvent& event );
    void on_add_player( wxCommandEvent& event );
    void on_remove_player( wxCommandEvent& event );
    void on_player_up( wxCommandEvent& event );
    void on_player_down( wxCommandEvent& event );
    void on_change_ruleset( wxCommandEvent& event );
    /*
    void on_standard_ruleset( wxCommandEvent& event );
    void on_tournament_ruleset( wxCommandEvent& event );
    void on_custom_ruleset( wxCommandEvent& event );
    */

    // as player handler this dialog has to show results of player manipulations
    virtual void player_added( const Player & );
    virtual void player_removed( const Player & );
    virtual void player_up( const Player & );
    virtual void player_down( const Player & );
    virtual void player_change_denied();
    virtual void ruleset_changed( Ruleset::type );
    virtual void ruleset_change_denied();
    virtual void aborted();

  private:
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
