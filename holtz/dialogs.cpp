/*
 * dialogs.cpp
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

#include "dialogs.hpp"

#include <sstream>

namespace holtz
{
  const wxString Player_Setup_Dialog::help_choices[] =
  { wxString(_("No help")), wxString(_("Show possible moves")), wxString(_("Show Hint"))};

  const wxString Player_Setup_Dialog::ruleset_choices[] =
  { wxString(_("Standard")), wxString(_("Tournament")), wxString(_("Custom"))};

  Player_Setup_Dialog::Player_Setup_Dialog( wxWindow *parent, Game_Window &game_window, 
					    Player_Setup_Manager &player_setup_manager )
    : wxDialog(parent,-1,wxString(_("Player setup"))),
      game_window(game_window),
      player_setup_manager(&player_setup_manager),
      player_name( new wxTextCtrl( this, -1, _("player 1") ) ),
      ai( new wxCheckBox( this, -1, _("AI") ) ),
      player_list( new wxListBox( this, LISTBOX_DCLICK, wxDefaultPosition, 
				  wxSize(200,150), 0, 0, wxLB_SINGLE ) ),
      help_choice( new wxRadioBox( this, -1, _("Help mode"), wxDefaultPosition,
				      wxDefaultSize, 
				      3, help_choices, 3, wxRA_SPECIFY_COLS ) ),
      ruleset_choice( new wxRadioBox( this,DIALOG_RULESET, _("Ruleset"), wxDefaultPosition,
				      wxDefaultSize, 
				      3, ruleset_choices, 3, wxRA_SPECIFY_COLS ) ),
      current_ruleset(0), last_ruleset(0)
  {
    player_setup_manager.set_player_handler(this);

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *add_player_sizer  = new wxBoxSizer( wxHORIZONTAL );
    add_player_sizer->Add( player_name, 0, wxALL, 10 );
    add_player_sizer->Add( ai, 0, wxALIGN_CENTER );
    wxButton *add_button = new wxButton(this,DIALOG_ADD_PLAYER,_("Add player"));
    add_button->SetDefault();
    add_button->SetFocus();
    add_player_sizer->Add( add_button, 0, wxALL, 10 );
    top_sizer->Add( add_player_sizer, 0, wxALIGN_CENTER );

    top_sizer->Add( help_choice, 0, wxALIGN_CENTER );

    wxBoxSizer *player_list_sizer  = new wxBoxSizer( wxHORIZONTAL );
    player_list_sizer->Add( player_list, 1, wxEXPAND | wxALL, 10 );
    wxBoxSizer *player_list_buttons_sizer  = new wxBoxSizer( wxVERTICAL );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_PLAYER_UP,_("Up")), 0, wxALL, 10 );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_PLAYER_DOWN,_("Down")), 0, wxALL, 10 );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_REMOVE_PLAYER,_("Remove")), 0, wxALL, 10 );
    player_list_sizer->Add( player_list_buttons_sizer, 0, wxALIGN_CENTER );
    top_sizer->Add( player_list_sizer, 0, wxALIGN_CENTER );

    top_sizer->Add( ruleset_choice, 0, wxALIGN_CENTER );

    wxBoxSizer *button_sizer  = new wxBoxSizer( wxHORIZONTAL );
    button_sizer->Add( new wxButton(this, DIALOG_READY,  _("Ready"), wxDefaultPosition) , 0, wxALL, 30 );
    button_sizer->Add( new wxButton(this, DIALOG_CANCEL, _("Cancel"), wxDefaultPosition) , 0, wxALL, 30 );
    top_sizer->Add( button_sizer, 0, wxALIGN_CENTER );
      
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
    top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  Player_Setup_Dialog::~Player_Setup_Dialog()
  {
    if( player_setup_manager )
    {
      player_setup_manager->set_player_handler(0);
    }
  }

  void Player_Setup_Dialog::on_ready( wxCommandEvent& WXUNUSED(event) )
  {
    if( player_setup_manager )
    {
      if( player_setup_manager->ready() )
      {
	game_window.get_frame().SetStatusText(_("Game will start when all players are ready"));
	player_setup_manager->set_player_handler(0);
	player_setup_manager = 0;
	Show(false);
      }
    }
  }

  void Player_Setup_Dialog::on_cancel( wxCommandEvent& WXUNUSED(event) )
  {
    Show(false);
  }

  void Player_Setup_Dialog::on_add_player( wxCommandEvent& WXUNUSED(event) )
  {
    if( player_setup_manager )
    {
      Player::Help_Mode help_mode = Player::no_help;
      switch( help_choice->GetSelection() )
      {
	case 0: help_mode = Player::no_help; break;
	case 1: help_mode = Player::show_possible_moves; break;
	case 2: help_mode = Player::show_hint; break;
	case -1: break;
	default: assert(false);
      };

      Player::Player_Type type;
      if( ai->GetValue() )
	type = Player::ai;
      else
	type = Player::user;

      int num_players = player_list->Number();
      if( !player_setup_manager->add_player( wxstr_to_str(player_name->GetValue()), 
					     type, help_mode ) )
      {
	wxMessageBox(_("Could not add player"), _("Add player"), wxOK | wxICON_INFORMATION, this);
      }
      else
      {
	std::ostringstream os;
	os << "player " << num_players + 2;
	player_name->Clear();
	player_name->AppendText(str_to_wxstr(os.str()));
      }
    }
  }

  void Player_Setup_Dialog::on_remove_player( wxCommandEvent& WXUNUSED(event) )
  {
    if( player_setup_manager )
    {
      int item = player_list->GetSelection();
      if( item >= 0 )
      {
	if( !player_setup_manager->remove_player( player_id[item] ) )
	{
	  wxMessageBox( _("Could not remove player"), 
		        _("Remove player"), wxOK | wxICON_INFORMATION, this );
	}
      }
    }
  }
    
  void Player_Setup_Dialog::on_player_up( wxCommandEvent& WXUNUSED(event) )
  {
    if( player_setup_manager )
    {
      int item = player_list->GetSelection();
      if( item >= 1 )
      {
	if( !player_setup_manager->player_up( player_id[item] ) )
	{
	  wxMessageBox( _("Could not move player"), _("Move player"), 
		        wxOK | wxICON_INFORMATION, this );
	}
      }
    }
  }

  void Player_Setup_Dialog::on_player_down( wxCommandEvent& WXUNUSED(event) )
  {
    if( player_setup_manager )
    {
      int item = player_list->GetSelection();
      if( (item >= 0) && (item < player_list->Number()-1) )
      {
	if( !player_setup_manager->player_down( player_id[item] ) )
	{
	  wxMessageBox( _("Could not move player"), _("Move player"), 
		        wxOK | wxICON_INFORMATION, this );
	}
      }
    }
  }
  void Player_Setup_Dialog::on_change_ruleset( wxCommandEvent& event )
  {
    if( player_setup_manager )
    {
      switch( ruleset_choice->GetSelection() )
      {
	case 0: 
	  if( !player_setup_manager->change_ruleset( Ruleset::standard_ruleset, Standard_Ruleset() ) )
	  {
	    if( current_ruleset != ruleset_choice->GetSelection() )
	      ruleset_choice->SetSelection(current_ruleset);
	    wxMessageBox(_("Could not change Ruleset"), _("Ruleset"), wxOK | wxICON_INFORMATION, this);
	  }
	  break;
	case 1: 
	  if( !player_setup_manager->change_ruleset( Ruleset::tournament_ruleset, Tournament_Ruleset() ) )
	  {
	    if( current_ruleset != ruleset_choice->GetSelection() )
	      ruleset_choice->SetSelection(current_ruleset);
	    wxMessageBox(_("Could not change Ruleset"), _("Ruleset"), wxOK | wxICON_INFORMATION, this);
	  }
	  break;
	case 2: 
	  if( current_ruleset != ruleset_choice->GetSelection() )
	    ruleset_choice->SetSelection(current_ruleset);
	  wxMessageBox(_("Cannot compose a custom ruleset yet"), 
		       _("Custom Ruleset"), wxOK | wxICON_INFORMATION, this);
	  break;
	case -1:
	  break;
      }
      if( ruleset_choice->GetSelection() != -1 )
      {
	last_ruleset = current_ruleset;
	current_ruleset = ruleset_choice->GetSelection();
      }
    }
  }

  void Player_Setup_Dialog::player_added( const Player &player )
  {
    // setup lookup tables
    int item = player_list->Number();
    player_item[player.id] = item;
    player_id[item] = player.id; 

    // add hostname to player name
    std::string name = player.name;
    if( player.host != "" )
      name += '(' + player.host + ')';

    player_list->Append( str_to_wxstr(name) );

    int num_players = player_list->Number();
    std::ostringstream os;
    os << "player " << num_players + 1;
    player_name->Clear();
    player_name->AppendText( str_to_wxstr(os.str()) );
    player_name->SetSelection( 0, player_name->GetLineLength(0) );
  }

  void Player_Setup_Dialog::player_removed( const Player &player )
  {
    int item = player_item[player.id];
    player_list->Delete( item );
    player_item.erase( player.id );
    player_id.erase( item );

    // adapt player item numbers of higher players
    player_id.clear();		// clear id list
    std::map<int,int>::iterator i;
    for( i = player_item.begin(); i != player_item.end(); ++i )
    {
      assert( i->second != item );
      if( i->second > item )
      {
	--i->second;
      }
      player_id[i->second] = i->first; // refill id table
    }
  }

  void Player_Setup_Dialog::player_up( const Player &player )
  {
    int item = player_item[player.id];

    if( item > 0 )
    {
      int item0 = item;
      int item1 = item - 1;
      wxString str0 = player_list->GetString(item0);
      wxString str1 = player_list->GetString(item1);
      int id0 = player_id[item0];
      int id1 = player_id[item1];
      // swap everything
      player_list->SetString(item1,str0);
      player_list->SetString(item0,str1);
      player_item[id0] = item1;
      player_item[id1] = item0;
      player_id[item0] = id1;
      player_id[item1] = id0;

      player_list->SetSelection(item1);
    }
  }

  void Player_Setup_Dialog::player_down( const Player &player )
  {
    int item = player_item[player.id];
    if( item < player_list->Number() - 1 )
    {
      int item0 = item;
      int item1 = item + 1;
      wxString str0 = player_list->GetString(item0);
      wxString str1 = player_list->GetString(item1);
      int id0 = player_id[item0];
      int id1 = player_id[item1];
      // swap everything
      player_list->SetString(item1,str0);
      player_list->SetString(item0,str1);
      player_item[id0] = item1;
      player_item[id1] = item0;
      player_id[item0] = id1;
      player_id[item1] = id0;

      player_list->SetSelection(item1);
    }
  }

  void Player_Setup_Dialog::player_change_denied()
  {
    wxMessageBox( _("Change for this player denied"), _("Denied"), wxOK | wxICON_INFORMATION, this);
  }

  void Player_Setup_Dialog::ruleset_changed( Ruleset::type type )
  {
    int new_type = 0;
    switch( type )
    {
      case Ruleset::standard_ruleset:
	new_type = 0;
	break;
      case Ruleset::tournament_ruleset:
	new_type = 1;
	break;
      case Ruleset::custom_ruleset:
	new_type = 2;
	break;
    }
    if( new_type != ruleset_choice->GetSelection() )
    {
      ruleset_choice->SetSelection(new_type);
    }
  }

  void Player_Setup_Dialog::ruleset_change_denied()
  {
    current_ruleset = last_ruleset;
    ruleset_choice->SetSelection( last_ruleset );
    wxMessageBox( _("Change of ruleset denied"), _("Ruleset denied"), wxOK | wxICON_INFORMATION, this);
  }

  void Player_Setup_Dialog::aborted()
  {
    if( player_setup_manager )
      player_setup_manager->set_player_handler(0);
    player_setup_manager = 0;
    Show(false);
  }

  BEGIN_EVENT_TABLE(Player_Setup_Dialog, wxDialog)				//**/
  EVT_BUTTON(DIALOG_READY,	   Player_Setup_Dialog::on_ready)		//**/
  EVT_BUTTON(DIALOG_CANCEL, 	   Player_Setup_Dialog::on_cancel)		//**/
  EVT_BUTTON(DIALOG_ADD_PLAYER,    Player_Setup_Dialog::on_add_player)		//**/
  EVT_BUTTON(DIALOG_REMOVE_PLAYER, Player_Setup_Dialog::on_remove_player)	//**/
  EVT_BUTTON(DIALOG_PLAYER_UP,	   Player_Setup_Dialog::on_player_up)		//**/
  EVT_BUTTON(DIALOG_PLAYER_DOWN,   Player_Setup_Dialog::on_player_down)		//**/
  EVT_RADIOBOX(DIALOG_RULESET,	   Player_Setup_Dialog::on_change_ruleset)	//**/
  END_EVENT_TABLE()								//**/

  Network_Clients_Dialog::Network_Clients_Dialog( wxWindow *parent, Network_Manager &network_manager )
    : wxDialog(parent,-1,wxString(_("Network client setup"))),
      client_list( new wxListBox( this, LISTBOX_DCLICK, wxDefaultPosition, 
				  wxSize(120,120), 0, 0, wxLB_SINGLE ) ),
      network_manager(network_manager)
  {
    network_manager.set_connection_handler(this);

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    
    top_sizer->Add(client_list, 0, wxALL, 10);

    top_sizer->Add( new wxButton( this, DIALOG_DISCONNECT, _("Disconnect All") ), 0, wxALL, 10 );
    
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
    top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  Network_Clients_Dialog::~Network_Clients_Dialog()
  {
    network_manager.set_connection_handler(0);
  }

  void Network_Clients_Dialog::new_connection( wxIPV4address host, wxSocketBase *socket )
  {
    client_data[static_cast<void*>(socket)] = socket;
    client_item[socket] = client_list->Number();
    std::ostringstream port;
    port << host.Service();
    client_list->Append( host.Hostname() + wxT('(') + str_to_wxstr(port.str()) + wxT(')'), socket );
  }

  void Network_Clients_Dialog::closed_connection( wxSocketBase *socket )
  {
    client_list->Delete( client_item[socket] );
  }
  
  void Network_Clients_Dialog::on_disconnect( wxCommandEvent& WXUNUSED(event) )
  {
    network_manager.close_connection();
  }

  void Network_Clients_Dialog::on_dclick( wxCommandEvent& event )
  {
    wxSocketBase *socket = client_data[event.m_clientData];
    wxString hostname = event.m_commandString;
    if( network_manager.may_disconnect( socket ) )
    {
      if( wxMessageBox( _("Do you really want to disconnect ") + hostname, 
			_("Disconnect?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
      {
	network_manager.disconnect( socket );
	client_list->Delete(event.m_commandInt);
      }
    }
    else
    {
      wxMessageBox( _("Can't disconnect host ") + hostname + _(" yet"), 
		    _("Can't disconnect!"), wxOK | wxICON_ERROR );
    }

  }

  BEGIN_EVENT_TABLE(Network_Clients_Dialog, wxDialog)			//**/
  EVT_BUTTON(DIALOG_DISCONNECT, Network_Clients_Dialog::on_disconnect)	//**/
  EVT_LISTBOX_DCLICK(LISTBOX_DCLICK, Network_Clients_Dialog::on_dclick)	//**/
  END_EVENT_TABLE()							//**/

  Network_Connection_Dialog::Network_Connection_Dialog( wxWindow *parent )
    : wxDialog(parent,-1, wxString(_("Network Game"))),
      server( new wxRadioButton(this, -1, _("Setup server") ) ),
      client( new wxRadioButton(this, -1, _("Connect to server") ) ),
      hostname( new wxTextCtrl(this, -1, wxT("localhost")) ),
      port( new wxSpinCtrl(this, -1, wxT("6211")) )
  {      
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    server->SetValue(true);
    top_sizer->Add( server, 0, wxALL, 10 );
    top_sizer->Add( client, 0, wxALL, 10 );
    top_sizer->Add( hostname, 0, wxALL, 10 );
    top_sizer->Add( port, 0, wxALL, 10 );
    port->SetRange(0,65535);

    wxBoxSizer *button_sizer  = new wxBoxSizer( wxHORIZONTAL );
    wxButton *ok_button = new wxButton(this, DIALOG_OK, _("Ok"), wxDefaultPosition);
    ok_button->SetDefault();
    ok_button->SetFocus();
    button_sizer->Add( ok_button , 0, wxALL, 10 );
    button_sizer->Add( new wxButton(this, DIALOG_CANCEL, _("Cancel"), wxDefaultPosition) , 0, wxALL, 10 );
    top_sizer->Add( button_sizer, 0, wxALIGN_CENTER );
      
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
    top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  BEGIN_EVENT_TABLE(Network_Connection_Dialog, wxDialog)		//**/
  EVT_BUTTON(DIALOG_OK,     Network_Connection_Dialog::OnOK)		//**/
  EVT_BUTTON(DIALOG_CANCEL, Network_Connection_Dialog::OnCancel)	//**/
  END_EVENT_TABLE()							//**/
}
