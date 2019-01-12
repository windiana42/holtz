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

#include "dialogs.hpp"		// this should be loaded from common/ directory

#if defined(VERSION_ZERTZ)
#  undef VERSION_ZERTZ
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_ZERTZ
#  define DEFAULT_PORT DEFAULT_PORT_ZERTZ
#elif defined(VERSION_DVONN)
#  undef VERSION_DVONN
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_DVONN
#  define DEFAULT_PORT DEFAULT_PORT_DVONN
#elif defined(VERSION_BLOKS)
#  undef VERSION_BLOKS
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_BLOKS
#  define DEFAULT_PORT DEFAULT_PORT_BLOKS
#elif defined(VERSION_RELAX)
#  undef VERSION_RELAX
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_RELAX
#  define DEFAULT_PORT DEFAULT_PORT_RELAX
#endif

#include <wx/cshelp.h>
#include <wx/dir.h>
#include <wx/fontdlg.h>
#include <wx/sizer.h>
#include <wx/treectrl.h>
#include <fstream>

#if defined(VERSION_ZERTZ)
namespace zertz
#elif defined(VERSION_DVONN)
namespace dvonn
#elif defined(VERSION_BLOKS)
namespace bloks
#elif defined(VERSION_RELAX)
namespace relax
#endif
{

  // ----------------------------------------------------------------------------
  // Setup_Manager_Page
  // ----------------------------------------------------------------------------

  Setup_Manager_Page::Setup_Manager_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true), 
      connecting(false), connected(false), auto_next(false)
  {
    alone          = new wxRadioButton( this, -1, _("Play alone") );
    network_server = new wxRadioButton( this, -1, _("Setup network server") );
    network_client = new wxRadioButton( this, -1, _("Connect to server") );
    don_t_change   = new wxRadioButton( this, -1, _("Keep last game connection") );

    server_port = new wxSpinCtrl( this, DIALOG_SERVER_PORT );
    hostname = new wxTextCtrl( this, DIALOG_HOST_NAME, game_dialog.hostname, wxDefaultPosition, 
			       wxDefaultSize, wxTE_PROCESS_ENTER );
    client_port = new wxSpinCtrl( this, DIALOG_CLIENT_PORT );
    server_port->SetRange( 1, 65535 );
    server_port->SetValue( game_dialog.server_port );
    client_port->SetRange( 1, 65535 );
    client_port->SetValue( game_dialog.client_port );
    client_status = new wxStaticText(this,-1,wxT(""));
    

    wxBoxSizer *server_sizer = new wxBoxSizer( wxHORIZONTAL );
    server_sizer->Add( server_port, 0, 0, 0 );
    wxBoxSizer *client_sizer1 = new wxBoxSizer( wxHORIZONTAL );
    client_sizer1->Add( hostname, 1, wxEXPAND, 0 );
    wxBoxSizer *client_sizer2 = new wxBoxSizer( wxHORIZONTAL );
    client_sizer2->Add( client_port, 0, wxRIGHT | wxALIGN_CENTER_VERTICAL, 10 );
    client_sizer2->Add( new wxPanel(this,-1,wxDefaultPosition,wxSize(30,1)) );  // spacer
    client_sizer2->Add( new wxButton( this, DIALOG_CONNECT, _("Connect") ), 0, 
		       wxALIGN_CENTER_VERTICAL, 0 );
    
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer( 3 );
    grid_sizer->Add( alone, 0, wxALL, 10 );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );

    grid_sizer->Add( network_server, 0, wxALL, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Port")), 0, 
		     wxRIGHT | wxALIGN_CENTER_VERTICAL, 10 );
    grid_sizer->Add( server_sizer, 0, wxTOP | wxBOTTOM | wxEXPAND, 10 );

    grid_sizer->Add( network_client, 0, wxALL, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Hostname")), 0, 
		     wxRIGHT | wxALIGN_CENTER_VERTICAL, 10 );
    grid_sizer->Add( client_sizer1, 0, wxTOP | wxBOTTOM | wxEXPAND, 10 );

    grid_sizer->Add( client_status, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Port")), 0, 
		     wxRIGHT | wxBOTTOM | wxALIGN_CENTER_VERTICAL, 10 );
    grid_sizer->Add( client_sizer2, 0, wxBOTTOM | wxEXPAND, 10 );

    grid_sizer->Add( don_t_change, 0, wxALL, 10 );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );

    grid_sizer->Add( new wxPanel( this, -1 ), 0, wxALL, 0 );
    grid_sizer->Add( new wxPanel( this, -1 ), 0, wxALL, 0 );
    grid_sizer->Add( new wxPanel( this, -1 ), 1, wxEXPAND, 0 );

    top_sizer->Add( grid_sizer, 0, wxCENTER | wxALL, 10 );
    // set sizer
    SetAutoLayout( true );	// tell dialog to use sizer
    SetSizer( top_sizer );	// actually set the sizer
  }

  wxWizardPage *Setup_Manager_Page::GetPrev() const
  {
    return 0;
  }

  wxWizardPage *Setup_Manager_Page::GetNext() const
  {
    bool can_choose = true;
    bool changes = this->changes;  // create writable copy of member <changes>
    // detect changes
    if(!changes)
    {
      if( alone->GetValue() != alone_val ||
	  network_server->GetValue() != network_server_val ||
	  network_client->GetValue() != network_client_val ||
	  hostname->IsModified() )
	changes = true;
    }
    // in case of changes the current game_setup_manager might change
    if( changes && !don_t_change->GetValue() )
    {
      Game_Setup_Manager *dummy_manager = 0;
      if( alone->GetValue() )
      {
	dummy_manager = new Standalone_Game_Setup_Manager( game_dialog.game_manager );
      } 
      else if( network_server->GetValue() )
      {
	dummy_manager 
	  = new Network_Manager_BGP100a_Server( game_dialog.game_manager, 
						game_dialog.gui_manager );
      }
      else if( network_client->GetValue() )
      {
	dummy_manager 
	  = new Network_Manager_BGP100a_Client( game_dialog.game_manager, 
						game_dialog.gui_manager );
      }
      can_choose = dummy_manager->can_choose_board();
      delete dummy_manager;
    }
    else
    {
      if( game_dialog.game_setup_manager )
	can_choose = game_dialog.game_setup_manager->can_choose_board();
    }

    if( can_choose )
      return game_dialog.get_board_page();
    else
      return game_dialog.get_player_page();
  }

  bool Setup_Manager_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back
    bool ret = true;

    if( connecting )
    {
      wxMessageDialog( this,
		       _("Please wait until connection attempt succeeds or fails and try again"), 
		       _("Wait"), wxOK | wxICON_INFORMATION ).ShowModal();
      return false;
    }

    // detect changes
    if(!changes)
    {
      if( alone->GetValue() != alone_val ||
	  network_server->GetValue() != network_server_val ||
	  network_client->GetValue() != network_client_val ||
	  hostname->IsModified() )
	changes = true;
    }
    // apply changes  
    if( changes )
    {
      if( !don_t_change->GetValue() )
      {
	// cleanup common to all choices
	if( game_dialog.clients_dialog )
	{
	  game_dialog.clients_dialog->destroy();
	  game_dialog.clients_dialog = 0;
	}
	if( game_dialog.game_setup_manager )
	{
	  game_dialog.game_setup_manager->set_display_handler(0);
	  delete game_dialog.game_setup_manager;
	  game_dialog.game_setup_manager = 0;
	}
	connected = false;
	connecting = false;

	if( alone->GetValue() )
	{
	  game_dialog.game_setup_manager 
	    = new Standalone_Game_Setup_Manager( game_dialog.game_manager );
	}
	else if( network_server->GetValue() )
	{
	  // save choice of connection parameters
	  game_dialog.server_port = server_port->GetValue();

	  Network_Manager_BGP100a_Server *network_manager 
	    = new Network_Manager_BGP100a_Server( game_dialog.game_manager, 
						  game_dialog.gui_manager );
	    
	  if( network_manager->setup_server(server_port->GetValue()) )
	  {
	    game_dialog.game_setup_manager = network_manager;

	    game_dialog.clients_dialog 
	      = new Network_Clients_Dialog(&game_dialog.gui_manager.get_game_window(),
					   *network_manager);
	    game_dialog.clients_dialog->Show(true);
	  }
	  else
	  {
	    delete network_manager;
	    wxMessageDialog(this, _("Can't listen port"), _("Network Message"), 
			    wxOK | wxICON_ERROR).ShowModal();
	    return false;
	  }
	}
	else if( network_client->GetValue() )
	{
	  // save choice of connection parameters
	  game_dialog.client_port = client_port->GetValue();
	  game_dialog.hostname = hostname->GetValue();

	  Network_Manager_BGP100a_Client *network_manager 
	    = new Network_Manager_BGP100a_Client( game_dialog.game_manager, 
						  game_dialog.gui_manager );

	  if( network_manager->connect_to_server(wxstr_to_str(hostname->GetValue()),
						 client_port->GetValue()) )
	  {
	    game_dialog.game_setup_manager = network_manager;
	    if( !connected )
	    {
	      connecting = true;
	      client_status->SetLabel(_("Connecting..."));
	      ret = false;	// prevent user from continuing until connection is established
	    }
	  }
	  else
	  {
	    delete network_manager;
	    wxMessageDialog(this, _("Connection to Server failed"), _("Network Message"), 
			    wxOK | wxICON_ERROR).ShowModal();
	    return false;
	  }
	}
	else
	  return false;		// nothing checked ?!?
      }
      game_dialog.game_setup_manager->set_display_handler( &game_dialog );
      game_dialog.get_data_from_setup_manager();
      changes = false;
#ifdef BACKCOMPATIBLE_WX_2_6
      hostname->SetValue(hostname->GetValue());
#else
      hostname->SetModified(false);
#endif
      // store setting:
      alone_val = alone->GetValue();
      network_server_val = network_server->GetValue();
      network_client_val = network_client->GetValue();
    }
    if( ret && game_dialog.game_setup_manager )
      if( !game_dialog.game_setup_manager->can_enter_player_setup() &&
	  GetNext() == game_dialog.get_player_page() )
      {
	wxMessageDialog( this, _("Please wait for connection to allow player setup"), 
			 _("Connecting"), wxOK | wxICON_INFORMATION ).ShowModal();
	return false;
      }

    return ret;
  }

  void Setup_Manager_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
    auto_next = true;
  }

  void Setup_Manager_Page::on_server_port( wxSpinEvent& WXUNUSED(event) )
  {
    changes = true;
    network_server->SetValue(true);
  }

  void Setup_Manager_Page::on_client_port( wxSpinEvent& WXUNUSED(event) )
  {
    changes = true;
    network_client->SetValue(true);
  }

  void Setup_Manager_Page::on_host_name( wxCommandEvent& event )
  {
    //the hostname change can be detected later with hostname->IsModified()
    network_client->SetValue(true);
    on_connect(event);
  }

  void Setup_Manager_Page::on_connect( wxCommandEvent& WXUNUSED(event) )
  {
    network_client->SetValue(true);
    // early setup of client connection
    transfer_data_from_window(true/*forward direction*/);
    auto_next = false;
  }


  void Setup_Manager_Page::restore()
  {
    changes = true;		// take first setting as change
    if( game_dialog.game_setup_manager )	// if there is a setup manager
    {
      don_t_change->SetValue( true );		// default: don't change it
      don_t_change->Enable( true );		// enable don't change
    }
    else
    {
      alone->SetValue( true );			// default: play alone
      don_t_change->Enable( false );		// disable don't change
    }
  }

  void Setup_Manager_Page::client_connected()
  {
    client_status->SetLabel(_("Connected"));
    connected = true;
    connecting = false;
    if( auto_next )
    {
      bool changes = this->changes;  // create local copy of member <changes>
      // detect changes
      if(!changes)
      {
	if( alone->GetValue() != alone_val ||
	    network_server->GetValue() != network_server_val ||
	    network_client->GetValue() != network_client_val ||
	    hostname->IsModified() )
	  changes = true;
      }
      if( !changes )
      {
	// switch to next page automatically
	game_dialog.wizard->ShowPage( GetNext() );
      }
    }
  }

  bool Setup_Manager_Page::connection_closed()
  {
    if( connecting )
    {
      client_status->SetLabel(_("Connection Lost"));
      connected = false;
      connecting = false;
      changes = true;		// cause a new connection attempt on <next>
      return true;
    }
    return false;
  }

  BEGIN_EVENT_TABLE(Setup_Manager_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD, Setup_Manager_Page::on_page_changing) //**/
    EVT_TEXT_ENTER(DIALOG_HOST_NAME,	Setup_Manager_Page::on_host_name)	//**/
    EVT_BUTTON(DIALOG_CONNECT,		Setup_Manager_Page::on_connect)		//**/
    EVT_SPINCTRL(DIALOG_SERVER_PORT,	Setup_Manager_Page::on_server_port)	//**/
    EVT_SPINCTRL(DIALOG_CLIENT_PORT,	Setup_Manager_Page::on_client_port)	//**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Board_Page
  // ----------------------------------------------------------------------------

  Board_Page::Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    new_game      = new wxRadioButton( this, -1, _("Start new game") );
    continue_game = new wxRadioButton( this, -1, _("Continue game") );
    don_t_change  = new wxRadioButton( this, -1, _("Don't care which game to play") );
    
#if defined(VERSION_ZERTZ)
    wxString new_game_choices[5];
    new_game_choices[0] = wxString(_("Basic Rules"));
    new_game_choices[1] = wxString(_("Standard Rules"));
    new_game_choices[2] = wxString(_("Tournament Rules"));
    new_game_choices[3] = wxString(_("Custom"));
    new_game_choices[4] = wxString(_("Rules of last game"));
    new_game_choice = new wxRadioBox( this, DIALOG_NEW_GAME_CHOICE, 
				      _("Rules for new game"), wxDefaultPosition,
				      wxDefaultSize, 5, new_game_choices, 2, wxRA_SPECIFY_COLS );
#elif defined(VERSION_DVONN)
    wxString new_game_choices[3];
    new_game_choices[0] = wxString(_("Standard Rules"));
    new_game_choices[1] = wxString(_("Random Start Board"));
    new_game_choices[2] = wxString(_("Rules of last game"));
    new_game_choice = new wxRadioBox( this, DIALOG_NEW_GAME_CHOICE, 
				      _("Rules for new game"), wxDefaultPosition,
				      wxDefaultSize, 3, new_game_choices, 2, wxRA_SPECIFY_COLS );
#elif defined(VERSION_BLOKS)
    wxString new_game_choices[3];
    new_game_choices[0] = wxString(_("Standard Rules (4 colors)"));
    new_game_choices[1] = wxString(_("Small Board Rules (2 colors)"));
    new_game_choices[2] = wxString(_("Rules of last game"));
    new_game_choice = new wxRadioBox( this, DIALOG_NEW_GAME_CHOICE, 
				      _("Rules for new game"), wxDefaultPosition,
				      wxDefaultSize, 3, new_game_choices, 2, wxRA_SPECIFY_COLS );
#elif defined(VERSION_RELAX)
    wxString new_game_choices[3];
    new_game_choices[0] = wxString(_("Standard Rules"));
    new_game_choices[1] = wxString(_("Extended Rules"));
    new_game_choices[2] = wxString(_("Rules of last game"));
    new_game_choice = new wxRadioBox( this, DIALOG_NEW_GAME_CHOICE, 
				      _("Rules for new game"), wxDefaultPosition,
				      wxDefaultSize, 3, new_game_choices, 2, wxRA_SPECIFY_COLS );
#endif

    wxString continue_game_choices[3];
    continue_game_choices[0] = wxString(_("Continue last game"));
    int n = 1;
#if defined(VERSION_ZERTZ) || defined(VERSION_DVONN)
    continue_game_choices[1] = wxString(_("Load PBM game"));
    n = 2;
#endif
#if defined(VERSION_DVONN)
    continue_game_choices[2] = wxString(_("Load Little Golem game"));
    n = 3;
#endif
    continue_game_choice = new wxRadioBox( this, DIALOG_CONTINUE_GAME_CHOICE, 
					   _("Which game to continue"), wxDefaultPosition,
					   wxDefaultSize, n, continue_game_choices, 1, 
					   wxRA_SPECIFY_COLS );

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    top_sizer->Add( new_game, 0, wxALL, 10 );
    top_sizer->Add( new_game_choice, 0, wxCENTER | wxALL, 10 );
    top_sizer->Add( continue_game, 0, wxALL, 10 );
    top_sizer->Add( continue_game_choice, 0, wxCENTER | wxALL, 10 );
    top_sizer->Add( don_t_change, 0, wxALL, 10 );

    // set sizer
    SetAutoLayout( true );	// tell dialog to use sizer
    SetSizer( top_sizer );	// actually set the sizer
  }

  wxWizardPage *Board_Page::GetPrev() const
  {
    return game_dialog.get_setup_manager_page();
  }

  wxWizardPage *Board_Page::GetNext() const
  {
#if defined(VERSION_ZERTZ)
    if( new_game->GetValue() && (new_game_choice->GetSelection() == 3) )
      return game_dialog.get_custom_board_page();
    else 
#endif
    {
      if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 1) )
	return game_dialog.get_load_pbm_board_page();
#if defined(VERSION_DVONN)
      else if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 2) )
	return game_dialog.get_load_lg_board_page();
#endif
      else
	return game_dialog.get_player_page();
    }
  }

  bool Board_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back

    // detect changes
    if(!changes)
    {
      if( new_game->GetValue() != new_game_val ||
	  continue_game->GetValue() != continue_game_val )
      {
	changes = true;
      }
    }
    // apply changes  
    if( changes )
    {
      if( new_game->GetValue() )
      {
#if defined(VERSION_ZERTZ)
	switch( new_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = Game( Basic_Ruleset() ); break;
	  case 1: game_dialog.game = Game( Standard_Ruleset() ); break;
	  case 2: game_dialog.game = Game( Tournament_Ruleset() ); break;
	  case 3: /*game will be set up in next page*/ break;
	  case 4: game_dialog.game = Game( *game_dialog.game_manager.get_game().ruleset ); 
	}
	if( new_game_choice->GetSelection() != 3 )
#elif defined(VERSION_DVONN)
	switch( new_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = Game( Standard_Ruleset() ); break;
// #warning TODO: change back from micro ruleset to random ruleset
// 	  case 1: game_dialog.game = Game( Micro_Ruleset() ); break;
	  case 1: game_dialog.game = Game( Random_Ruleset() ); break;
	  case 2: game_dialog.game = Game( *game_dialog.game_manager.get_game().ruleset ); 
	}
#elif defined(VERSION_BLOKS)
	switch( new_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = Game( Standard_Ruleset() ); break;
// #warning TODO: change back from micro ruleset to small board
// 	  case 1: game_dialog.game = Game( Micro_Ruleset() ); break;
	  case 1: game_dialog.game = Game( Small_Board_Ruleset() ); break;
	  case 2: game_dialog.game = Game( *game_dialog.game_manager.get_game().ruleset ); 
	}
#elif defined(VERSION_RELAX)
	switch( new_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = Game( Standard_Ruleset() ); break;
	  case 1: game_dialog.game = Game( Extended_Ruleset() ); break;
	  case 2: game_dialog.game = Game( *game_dialog.game_manager.get_game().ruleset ); 
	}
#endif
	{
	  if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	      Game_Setup_Manager::deny )
	  {
	    game_dialog.board_change_denied();
	  }
	}
      }
      else if( continue_game->GetValue() )
      {
	switch( continue_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = game_dialog.game_manager.get_game(); break;
	  case 1: /*game will be set up in next page*/ break;
	}
	if( continue_game_choice->GetSelection() != 1 )
	{
	  if( (game_dialog.game.players.size() >= game_dialog.game.get_min_players()) &&
	      (game_dialog.game.players.size() <= game_dialog.game.get_max_players()) )
	  {
	    // limit number of players to the amount present in the game to be continued
	    game_dialog.game.ruleset->min_players = game_dialog.game.players.size();
	    game_dialog.game.ruleset->max_players = game_dialog.game.players.size();
	  }
	  if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	      Game_Setup_Manager::deny )
	  {
	    game_dialog.board_change_denied();
	  }
	}
      }
      game_dialog.get_custom_board_page()->restore();
      game_dialog.get_load_pbm_board_page()->restore();
#if defined(VERSION_DVONN)
      game_dialog.get_load_lg_board_page()->restore();
#endif
      game_dialog.get_player_page()->players_changed();

      changes = false;
      // store setting:
      new_game_val = new_game->GetValue();
      continue_game_val = continue_game->GetValue();
    }
    if( game_dialog.game_setup_manager )
      if( !game_dialog.game_setup_manager->can_enter_player_setup() &&
	  GetNext() == game_dialog.get_player_page() )
      {
	wxMessageDialog( this, _("Please wait for connection to allow player setup"), 
			 _("Connecting"), wxOK | wxICON_INFORMATION ).ShowModal();
	return false;
      }

    return true;
  }

  void Board_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
  }

  void Board_Page::on_new_game_choice	   ( wxCommandEvent& /*event*/ )
  {
    changes = true;
    new_game->SetValue(true);
  }

  void Board_Page::on_continue_game_choice ( wxCommandEvent& /*event*/ )
  {
    changes = true;
    continue_game->SetValue(true);
  }

  void Board_Page::restore()	// display stored game state
  {
    changes = true;		// take first setting as change
    //!!! ask game_setup_manager which option is default !!! Client: don_t_change
    bool can_choose = true;
    if( game_dialog.game_setup_manager )
      can_choose = game_dialog.game_setup_manager->can_choose_board();
    if(can_choose)
    {
      new_game->SetValue(true);
#if defined(VERSION_ZERTZ)
      switch( game_dialog.game.ruleset->get_type() )
      {
	case Ruleset::basic:      new_game_choice->SetSelection(0); break;
	case Ruleset::standard:   new_game_choice->SetSelection(1); break;
	case Ruleset::tournament: new_game_choice->SetSelection(2); break;
	case Ruleset::custom:     new_game_choice->SetSelection(3); break;
      }
      new_game_choice->Enable(true);
#elif defined(VERSION_DVONN)
      switch( game_dialog.game.ruleset->get_type() )
      {
	case Ruleset::standard:   new_game_choice->SetSelection(0); break;
	case Ruleset::custom:     new_game_choice->SetSelection(1); break;
      }
      new_game_choice->Enable(true);
#elif defined(VERSION_BLOKS)
      switch( game_dialog.game.ruleset->get_type() )
      {
	case Ruleset::standard:    new_game_choice->SetSelection(0); break;
	case Ruleset::small_board: new_game_choice->SetSelection(1); break;
	case Ruleset::custom:      new_game_choice->SetSelection(2); break;
      }
      new_game_choice->Enable(true);
#elif defined(VERSION_RELAX)
      switch( game_dialog.game.ruleset->get_type() )
      {
	case Ruleset::standard:   new_game_choice->SetSelection(0); break;
	case Ruleset::extended:   new_game_choice->SetSelection(1); break;
	case Ruleset::custom:     new_game_choice->SetSelection(2); break;
      }
      new_game_choice->Enable(true);
#endif
      don_t_change->Enable(false);
      new_game->Enable(true);
      continue_game->Enable(true);
      continue_game_choice->Enable(true);
    } 
    else
    {
      new_game_choice->Enable(false);
      don_t_change->SetValue(true);
      don_t_change->Enable(true);
      new_game->Enable(false);
      continue_game->Enable(false);
      continue_game_choice->Enable(false);
    }
  }

  wxWizardPage *Board_Page::get_last_board_page() const
  {
#if defined(VERSION_ZERTZ)
    if( new_game->GetValue() && (new_game_choice->GetSelection() == 3) )
      return game_dialog.get_custom_board_page();
    else 
#endif
    {
      if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 1) )
	return game_dialog.get_load_pbm_board_page();
#if defined(VERSION_DVONN)
      else if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 2) )
	return game_dialog.get_load_lg_board_page();
#endif
      else
	return game_dialog.get_board_page();
    }
  }

  BEGIN_EVENT_TABLE(Board_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD, Board_Page::on_page_changing)	   //**/
    EVT_RADIOBOX(DIALOG_NEW_GAME_CHOICE, Board_Page::on_new_game_choice)	   //**/
    EVT_RADIOBOX(DIALOG_CONTINUE_GAME_CHOICE, Board_Page::on_continue_game_choice) //**/
  END_EVENT_TABLE()								   //**/

  // ----------------------------------------------------------------------------
  // Custom_Board_Setup_Panel
  // ----------------------------------------------------------------------------

  Custom_Board_Setup_Panel::Custom_Board_Setup_Panel( wxWindow *parent, Game_Dialog &game_dialog )
    : wxPanel( parent, -1 ), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

#if defined(VERSION_ZERTZ)
    wxString board_choices[5];
    board_choices[0] = wxString(_("37 Rings (default)"));
    board_choices[1] = wxString(_("40 Rings"));
    board_choices[2] = wxString(_("44 Rings"));
    board_choices[3] = wxString(_("48 Rings"));
    board_choices[4] = wxString(_("61 Rings"));
    //board_choices[5] = wxString(_("custom"));
    board_choice = new wxRadioBox( this, DIALOG_BOARD_CHOICE, _("Choose Board"), wxDefaultPosition,
				   wxDefaultSize, 
				   5, board_choices, 2, wxRA_SPECIFY_ROWS );
    top_sizer->Add( board_choice, 0, wxALIGN_CENTER | wxALL, 10  );

    wxBoxSizer *win_condition_sizer = new wxBoxSizer( wxHORIZONTAL );
    wxString win_choices[3];
    win_choices[0] = wxString(_("Basic"));
    win_choices[1] = wxString(_("Standard"));
    win_choices[2] = wxString(_("Custom"));
    win_choice = new wxRadioBox( this, DIALOG_WIN_CHOICE, _("Win condition"), wxDefaultPosition,
				 wxDefaultSize, 
				 3, win_choices, 1, wxRA_SPECIFY_COLS );
    win_condition_sizer->Add( win_choice, 0, wxALIGN_CENTER | wxALL, 10 );

    wxGridSizer *win_custom_sizer = new wxGridSizer(2);
    win_custom_sizer->Add( new wxStaticText(this, -1, _("White marbles")), 0,  wxALL, 5 );
    win_white = new wxSpinCtrl(this, DIALOG_WIN_SPIN, wxT("3")); win_white->SetRange(1,99);
    win_custom_sizer->Add( win_white, 0,  wxALL, 5 );
    win_custom_sizer->Add( new wxStaticText(this, -1, _("Grey marbles")),  0,  wxALL, 5 );
    win_grey  = new wxSpinCtrl(this, DIALOG_WIN_SPIN, wxT("4")); win_grey ->SetRange(1,99);
    win_custom_sizer->Add( win_grey,  0,  wxALL, 5 );
    win_custom_sizer->Add( new wxStaticText(this, -1, _("Black marbles")), 0,  wxALL, 5 );
    win_black = new wxSpinCtrl(this, DIALOG_WIN_SPIN, wxT("5")); win_black->SetRange(1,99);
    win_custom_sizer->Add( win_black, 0,  wxALL, 5 );
    win_custom_sizer->Add( new wxStaticText(this, -1, _("Each colour")),   0,  wxALL, 5 );
    win_all   = new wxSpinCtrl(this, DIALOG_WIN_SPIN, wxT("2")); win_all  ->SetRange(1,99);
    win_custom_sizer->Add( win_all,   0,  wxALL, 5 );
    win_condition_sizer->Add( win_custom_sizer, 0, wxALIGN_CENTER | wxALL, 10 );
    top_sizer->Add( win_condition_sizer, 0, wxALIGN_CENTER );

    wxBoxSizer *common_stones_sizer = new wxBoxSizer( wxHORIZONTAL );
    wxString stones_choices[3];
    stones_choices[0] = wxString(_("Basic"));
    stones_choices[1] = wxString(_("Standard"));
    stones_choices[2] = wxString(_("Custom"));
    stones_choice = new wxRadioBox( this, DIALOG_STONES_CHOICE, _("Common marbles"), 
				    wxDefaultPosition, wxDefaultSize, 
				    3, stones_choices, 1, wxRA_SPECIFY_COLS );
    common_stones_sizer->Add( stones_choice, 0, wxALIGN_CENTER | wxALL, 10 );

    wxGridSizer *stones_custom_sizer = new wxGridSizer(2);
    stones_custom_sizer->Add( new wxStaticText(this, -1, _("White marbles")), 0,  wxALL, 5 );
    stones_white = new wxSpinCtrl(this, DIALOG_STONES_SPIN, wxT("5")); stones_white->SetRange(0,99);
    stones_custom_sizer->Add( stones_white, 0,  wxALL, 5 );
    stones_custom_sizer ->Add( new wxStaticText(this, -1, _("Grey marbles")),  0,  wxALL, 5 );
    stones_grey  = new wxSpinCtrl(this, DIALOG_STONES_SPIN, wxT("7")); stones_grey ->SetRange(0,99);
    stones_custom_sizer->Add( stones_grey,  0,  wxALL, 5 );
    stones_custom_sizer->Add( new wxStaticText(this, -1, _("Black marbles")), 0,  wxALL, 5 );
    stones_black = new wxSpinCtrl(this, DIALOG_STONES_SPIN, wxT("9")); stones_black->SetRange(0,99);
    stones_custom_sizer->Add( stones_black, 0,  wxALL, 5 );
    common_stones_sizer->Add( stones_custom_sizer, 0, wxALIGN_CENTER | wxALL, 10 );
    top_sizer->Add( common_stones_sizer, 0, wxALIGN_CENTER );
#endif

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  void Custom_Board_Setup_Panel::restore()
  {
#if defined(VERSION_ZERTZ)
    Ruleset *ruleset = game_dialog.game.ruleset;
    wxCommandEvent evt;
    switch( ruleset->board.board_type )
    {
      case Board::s37_rings: board_choice->SetSelection(0); break;
      case Board::s40_rings: board_choice->SetSelection(1); break;
      case Board::s44_rings: board_choice->SetSelection(2); break;
      case Board::s48_rings: board_choice->SetSelection(3); break;
      case Board::s61_rings: board_choice->SetSelection(4); break;
      case Board::custom:    /* board_choice->SetSelection(5); // shouldn't happen */ break;
    }
    switch( ruleset->win_condition->get_type() )
    {
      case Win_Condition::basic:     win_choice->SetSelection(0); on_change_win(evt); break;
      case Win_Condition::standard:  win_choice->SetSelection(1); on_change_win(evt); break;
      case Win_Condition::generic:     
      {
	win_choice->SetSelection(2);
	Generic_Win_Condition *wc = dynamic_cast<Generic_Win_Condition*>( ruleset->win_condition );
	win_white->SetValue( wc->num_white );
	win_grey ->SetValue( wc->num_grey );
	win_black->SetValue( wc->num_black );
	win_all  ->SetValue( wc->num_all );
      }
      break;
      case Win_Condition::full_custom: /*win_choice->SetSelection(3); // shouldn't happen */ break;
    }
    switch( ruleset->common_stones.get_type() )
    {
      case Common_Stones::basic:    stones_choice->SetSelection(0); on_change_stones(evt); break;
      case Common_Stones::standard: stones_choice->SetSelection(1); on_change_stones(evt); break;
      case Common_Stones::custom:
      {
	stones_choice->SetSelection(2);
	stones_white->SetValue( ruleset->common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( ruleset->common_stones.stone_count[ Stones::grey_stone  ] );
	stones_black->SetValue( ruleset->common_stones.stone_count[ Stones::black_stone ] );
      }
      break;
    }
#endif
    changes = true;
  }

  Game Custom_Board_Setup_Panel::get_board()
  {
#if defined(VERSION_ZERTZ)
    Board *board;
    Common_Stones *common_stones;
    Win_Condition *win_condition;

    switch( board_choice->GetSelection() )
    {
      case -1:
      case 0:
	board = new Board( (const int*) board_37, 
			   sizeof(board_37[0]) / sizeof(board_37[0][0]),
			   sizeof(board_37)    / sizeof(board_37[0]),
			   Board::s37_rings );
	break;
      case 1:
	board = new Board( (const int*) board_40, 
			   sizeof(board_40[0]) / sizeof(board_40[0][0]),
			   sizeof(board_40)    / sizeof(board_40[0]),
			   Board::s40_rings );
	break;
      case 2:
	board = new Board( (const int*) board_44, 
			   sizeof(board_44[0]) / sizeof(board_44[0][0]),
			   sizeof(board_44)    / sizeof(board_44[0]),
			   Board::s44_rings );
	break;
      case 3:
	board = new Board( (const int*) board_48, 
			   sizeof(board_48[0]) / sizeof(board_48[0][0]),
			   sizeof(board_48)    / sizeof(board_48[0]),
			   Board::s48_rings );
	break;
      case 4:
	board = new Board( (const int*) board_61, 
			   sizeof(board_61[0]) / sizeof(board_61[0][0]),
			   sizeof(board_61)    / sizeof(board_61[0]),
			   Board::s61_rings );
	break;
      default:
	assert(false);
	break;
    }
    switch( win_choice->GetSelection() )
    {
      case 0:
	win_condition = new Basic_Win_Condition();
	break;
      case 1:
	win_condition = new Standard_Win_Condition();
	break;
      case 2:
	win_condition = new Generic_Win_Condition( win_white->GetValue(), 
						   win_grey ->GetValue(), 
						   win_black->GetValue(), 
						   win_all  ->GetValue() );
	break;
      default:
	assert(false);
	break;
    }
    switch( stones_choice->GetSelection() )
    {
      case 0:
	common_stones = new Basic_Common_Stones();
	break;
      case 1:
	common_stones = new Standard_Common_Stones();
	break;
      case 2:
	common_stones = new Custom_Common_Stones( stones_white->GetValue(), 
						  stones_grey ->GetValue(),
						  stones_black->GetValue() );
	break;
      default:
	assert(false);
	break;
    }
    Game game( *(new Custom_Ruleset( *board, *common_stones, win_condition, 
				    new Standard_Coordinate_Translator(*board) )) );
    delete board;
    delete common_stones;

    changes = false;
    return game;
#else
    return Game( Standard_Ruleset() );
#endif
  }

  void Custom_Board_Setup_Panel::on_change_board  ( wxCommandEvent& )
  {
    changes = true;
  }

  void Custom_Board_Setup_Panel::on_change_win  ( wxCommandEvent& )
  {
    changes = true;
#if defined(VERSION_ZERTZ)
    switch( win_choice->GetSelection() )
    {
      case 0:
      {
	Basic_Win_Condition basic;
	win_white->SetValue( basic.num_white );
	win_grey ->SetValue( basic.num_grey );
	win_black->SetValue( basic.num_black );
	win_all  ->SetValue( basic.num_all );
      }
      break;
      case 1:
      {
	Standard_Win_Condition standard;
	win_white->SetValue( standard.num_white );
	win_grey ->SetValue( standard.num_grey );
	win_black->SetValue( standard.num_black );
	win_all  ->SetValue( standard.num_all );
      }
      break;
      case 2:
	break;
    }
#endif
  }

  void Custom_Board_Setup_Panel::on_spin_win ( wxSpinEvent& )
  {
    changes = true;
#if defined(VERSION_ZERTZ)
    win_choice->SetSelection(2);
#endif
  }

  void Custom_Board_Setup_Panel::on_change_stones  ( wxCommandEvent& )
  {
    changes = true;
#if defined(VERSION_ZERTZ)
    switch( stones_choice->GetSelection() )
    {
      case 0:
      {
	Basic_Common_Stones common_stones;
	stones_white->SetValue( common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( common_stones.stone_count[ Stones::grey_stone  ] );
	stones_black->SetValue( common_stones.stone_count[ Stones::black_stone ] );
      }
      break;
      case 1:
      {
	Standard_Common_Stones common_stones;
	stones_white->SetValue( common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( common_stones.stone_count[ Stones::grey_stone  ] );
	stones_black->SetValue( common_stones.stone_count[ Stones::black_stone ] );
      }
      break;
      case 2:
	break;
    }
#endif
  }
  void Custom_Board_Setup_Panel::on_spin_stones ( wxSpinEvent& WXUNUSED(event) )
  {
    changes = true;
#if defined(VERSION_ZERTZ)
    stones_choice->SetSelection(2);
#endif
  }

  Custom_Board_Setup_Panel::~Custom_Board_Setup_Panel()
  {
  }
  
  BEGIN_EVENT_TABLE(Custom_Board_Setup_Panel, wxPanel)			
    EVT_RADIOBOX(DIALOG_BOARD_CHOICE,  	Custom_Board_Setup_Panel::on_change_board)
    EVT_RADIOBOX(DIALOG_WIN_CHOICE,  	Custom_Board_Setup_Panel::on_change_win)
    EVT_SPINCTRL(DIALOG_WIN_SPIN,	Custom_Board_Setup_Panel::on_spin_win)	
    EVT_RADIOBOX(DIALOG_STONES_CHOICE,	Custom_Board_Setup_Panel::on_change_stones)	
    EVT_SPINCTRL(DIALOG_STONES_SPIN,	Custom_Board_Setup_Panel::on_spin_stones)	//**/
  END_EVENT_TABLE()									//**/

  // ----------------------------------------------------------------------------
  // Custom_Board_Page
  // ----------------------------------------------------------------------------

  Custom_Board_Page::Custom_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog)
  {
    wxNotebook *notebook = new wxNotebook( this, -1 );
    wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add(notebook, 1, wxEXPAND );

    custom_board_panel = new Custom_Board_Setup_Panel( notebook, game_dialog );
    notebook->AddPage( custom_board_panel, _("Ruleset") );

    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer(sizer);
  }

  wxWizardPage *Custom_Board_Page::GetPrev() const
  {
    return game_dialog.get_board_page();
  }

  wxWizardPage *Custom_Board_Page::GetNext() const
  {
    return game_dialog.get_player_page();
  }

  bool Custom_Board_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back

    if( custom_board_panel->is_changes() )
    {
      game_dialog.game = custom_board_panel->get_board();
      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }

      custom_board_panel->set_changes(false);
    }
    if( game_dialog.game_setup_manager )
      if( !game_dialog.game_setup_manager->can_enter_player_setup() &&
	  GetNext() == game_dialog.get_player_page() )
      {
	wxMessageDialog( this, _("Please wait for connection to allow player setup"), 
			 _("Connecting"), wxOK | wxICON_INFORMATION ).ShowModal();
	return false;
      }

    return true;
  }

  void Custom_Board_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
  }

  void Custom_Board_Page::restore()		// display stored game state
  {
    custom_board_panel->restore();
  }

  BEGIN_EVENT_TABLE(Custom_Board_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD, Custom_Board_Page::on_page_changing) //**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Load_PBM_Board_Page
  // ----------------------------------------------------------------------------

  Load_PBM_Board_Page::Load_PBM_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *pbm_sizer = new wxBoxSizer( wxHORIZONTAL );
    pbm_sizer->Add( new wxStaticText(this,-1,_("PBM Directory") ), 0, 
		    wxRIGHT | wxALIGN_CENTER_VERTICAL, 10 );
    pbm_directory = new wxTextCtrl( this, DIALOG_CHANGE_DIRECTORY, wxT(""), wxDefaultPosition, 
				    wxDefaultSize, wxTE_PROCESS_ENTER );
    pbm_sizer->Add( pbm_directory, 1, wxEXPAND | wxRIGHT | wxLEFT, 10 );
    pbm_sizer->Add( new wxButton( this, DIALOG_CHOOSE_DIRECTORY, _("Choose...") ), 0, wxLEFT, 10 );
    top_sizer->Add( pbm_sizer, 0, wxEXPAND | wxBOTTOM, 10 );

    pbm_game_list = new wxListBox( this,-1,wxDefaultPosition, wxSize(100,100), 0, 0, wxLB_SINGLE );
    top_sizer->Add( pbm_game_list, 1, wxEXPAND, 0 );

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  wxWizardPage *Load_PBM_Board_Page::GetPrev() const
  {
    return game_dialog.get_board_page();
  }

  wxWizardPage *Load_PBM_Board_Page::GetNext() const
  {
    return game_dialog.get_player_page();
  }

  bool Load_PBM_Board_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back

    // check for changes
    if( !changes )
    {
      if( pbm_game_list_val != pbm_game_list->GetSelection() )
	changes = true;
    }
    // make changes
    if( changes )
    {
      int index = pbm_game_list->GetSelection();
      int board_num = int(intptr_t( pbm_game_list->GetClientData(index) ));
				// attention: retrieving integer from pointer variable (see
				// pbm_game_list->Append(,xxx))

      // load board
      int current_move = 0, max_move, num_moves;
      std::list< std::pair<PBM_Content,std::string> > &board_files = pbm_files[board_num];
      do
      {
	max_move = -1; 

	std::list< std::pair<PBM_Content,std::string> >::iterator i, max;
	for( i = board_files.begin(); i != board_files.end(); ++i )
	{
	  if( (i->first.from <= current_move) && (i->first.to > max_move) &&
	      (i->first.to >= current_move) )
	  {
	    max_move = i->first.to;
	    max = i;
	  }
	}
	if( max_move >= 0 )
	{
	  std::ifstream is( max->second.c_str() );
	  num_moves = load_pbm_file( is, game_dialog.game, current_move );
	  if( num_moves <= 0 ) return false;
	  current_move += num_moves;
	}
#ifndef __WXMSW__
	std::cout << "Number of Players: " << game_dialog.game.players.size() <<  std::endl;
	if( game_dialog.game.players.size() )
	  std::cout << "First Player: " << game_dialog.game.players.front().name << std::endl;
#endif
      }while( (max_move >= 0) && (num_moves > 0) );

      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }
      else
      {
	game_dialog.get_player_page()->players_changed();
      }

      changes = false;
      // store setting:
      pbm_game_list_val = pbm_game_list->GetSelection();
    }
    if( game_dialog.game_setup_manager )
      if( !game_dialog.game_setup_manager->can_enter_player_setup() &&
	  GetNext() == game_dialog.get_player_page() )
      {
	wxMessageDialog( this, _("Please wait for connection to allow player setup"), 
			 _("Connecting"), wxOK | wxICON_INFORMATION ).ShowModal();
	return false;
      }

    return true;
  }

  void Load_PBM_Board_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
  }

  void Load_PBM_Board_Page::restore()		// display stored game state
  {
    changes = true;		// take first setting as change

    if( game_dialog.valid_pbm_directory != wxT("") )
      {
	pbm_directory->SetValue( game_dialog.valid_pbm_directory );
	scan_directory( game_dialog.valid_pbm_directory );
      }
    else
      pbm_files.clear();
  }

  bool Load_PBM_Board_Page::scan_directory( wxString directory )
  {
    wxDir dir(directory);

    if( !dir.IsOpened() ) return false;

    // scan files
    pbm_files.clear();
    wxString wx_filename;
    bool ok = dir.GetFirst( &wx_filename, wxT("*"), wxDIR_FILES );
    while( ok )
    {
      std::string filename = wxstr_to_str(directory + wxT('/') + wx_filename);
      std::ifstream is( filename.c_str() );
      if( is )
      {
	PBM_Content content = scan_pbm_file( is );
	
	if( content.id > 0 )
	{
	  pbm_files[content.id].push_back( std::pair<PBM_Content,std::string>( content, filename ) );
	}
      }

      ok = dir.GetNext( &wx_filename );
    }

    // setup list box
    pbm_game_list->Clear();
    pbm_game_list_val = -1;
    std::map< int, std::list< std::pair<PBM_Content,std::string> > >::iterator i;
    for( i = pbm_files.begin(); i != pbm_files.end(); ++i )
    {
      std::list< std::pair<PBM_Content,std::string> > &board_files = i->second;
      PBM_Content master_content; master_content.id = -1;

      std::list< std::pair<PBM_Content,std::string> >::iterator j;
      for( j = board_files.begin(); j != board_files.end(); ++j )
      {
	PBM_Content &content = j->first;

	if( master_content.id == -1 ) // if master_content not yet filled
	{
	  master_content = content; // fill complete master content
	}
	else
	{
	  // adapt move range
	  if( content.from < master_content.from ) master_content.from = content.from;
	  if( content.to   > master_content.to   ) master_content.to   = content.to;
	}
      }

      assert( master_content.id >= 0 );
      
      if( master_content.from == 0 ) // display only boards that are specified from the first move on
      {
	wxString board_str;
#if defined(VERSION_DVONN)
	board_str << _("Board");
#elif defined(VERSION_RELAX)
	board_str << _("Board");
#elif defined(VERSION_ZERTZ)
	switch( master_content.ruleset_type )
	{
	  case PBM_Content::standard:   board_str << _("Zertz"); break;
	  case PBM_Content::tournament: board_str << _("Zertz+11"); break;
	}
#endif
  	board_str << wxT(" ") << master_content.id << wxT(" ") 
		  << str_to_wxstr(master_content.player1) << wxT(":")
		  << str_to_wxstr(master_content.player2) << wxT(" (") 
		  << master_content.to << wxT(" ") << _("moves") << wxT(")");
	pbm_game_list->Append( board_str, ((char*) 0) + (master_content.id) ); 
				// attention: storing integer in pointer variable instead of
				// dynamically allocating memory
      }
    }

    return true;
  }

  void Load_PBM_Board_Page::on_choose_directory( wxCommandEvent& WXUNUSED(event) )
  {
    wxString directory = game_dialog.valid_pbm_directory;
    if( directory == wxT("") ) 
      directory = wxGetCwd();

#ifdef BACKCOMPATIBLE_WX_2_6
    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory );
#else
    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory, 
					   wxDD_DIR_MUST_EXIST | wxDD_CHANGE_DIR );
#endif
    if( dialog->ShowModal() == wxID_OK )
    {
      changes = true;
      if( scan_directory( dialog->GetPath() ) )
      {
	game_dialog.valid_pbm_directory = dialog->GetPath();
	pbm_directory->SetValue( game_dialog.valid_pbm_directory );
      }
    }
  }

  void Load_PBM_Board_Page::on_change_directory( wxCommandEvent& WXUNUSED(event) )
  {
    changes = true;
    if( scan_directory( pbm_directory->GetValue() ) )
    {
      game_dialog.valid_pbm_directory = pbm_directory->GetValue();
    }
  }

  BEGIN_EVENT_TABLE(Load_PBM_Board_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD,	Load_PBM_Board_Page::on_page_changing) 
    EVT_BUTTON(DIALOG_CHOOSE_DIRECTORY,		Load_PBM_Board_Page::on_choose_directory)	
    EVT_TEXT_ENTER(DIALOG_CHANGE_DIRECTORY,	Load_PBM_Board_Page::on_change_directory) //**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Load_LG_Board_Page
  // ----------------------------------------------------------------------------
#if defined(VERSION_DVONN)

  Load_LG_Board_Page::Load_LG_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *lg_sizer = new wxBoxSizer( wxHORIZONTAL );
    lg_sizer->Add( new wxStaticText(this,-1,_("Little Golem Directory") ), 0, 
		    wxRIGHT | wxALIGN_CENTER_VERTICAL, 10 );
    lg_directory = new wxTextCtrl( this, DIALOG_CHANGE_DIRECTORY, wxT(""), wxDefaultPosition, 
				    wxDefaultSize, wxTE_PROCESS_ENTER );
    lg_sizer->Add( lg_directory, 1, wxEXPAND | wxRIGHT | wxLEFT, 10 );
    lg_sizer->Add( new wxButton( this, DIALOG_CHOOSE_DIRECTORY, _("Choose...") ), 0, wxLEFT, 10 );
    top_sizer->Add( lg_sizer, 0, wxEXPAND | wxBOTTOM, 10 );

    lg_game_list = new wxListBox( this,-1,wxDefaultPosition, wxSize(100,100), 0, 0, wxLB_SINGLE );
    top_sizer->Add( lg_game_list, 1, wxEXPAND, 0 );

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  wxWizardPage *Load_LG_Board_Page::GetPrev() const
  {
    return game_dialog.get_board_page();
  }

  wxWizardPage *Load_LG_Board_Page::GetNext() const
  {
    return game_dialog.get_player_page();
  }

  bool Load_LG_Board_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back

    // check for changes
    if( !changes )
    {
      if( lg_game_list_val != lg_game_list->GetSelection() )
	changes = true;
    }
    // make changes
    if( changes )
    {
      int index = lg_game_list->GetSelection();

      // load board
      int cnt=0;
      std::list< std::pair<LG_Content,std::string> >::iterator i;
      for( i = lg_files.begin(); i != lg_files.end(); ++i )
      {
	if( cnt == index )
	{
	  std::ifstream is( i->second.c_str() );
	  int num_moves = load_littlegolem_file( is, game_dialog.game );
	  if( num_moves < 0 ) return false;

#ifndef __WXMSW__
	  std::cout << "Loaded Littlegolem file. Moves: " << num_moves <<  std::endl;
	  std::cout << "Number of Players: " << game_dialog.game.players.size() <<  std::endl;
#endif
	  break;
	}
	cnt++;
      }

      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }
      else
      {
	game_dialog.get_player_page()->players_changed();
      }

      changes = false;
      // store setting:
      lg_game_list_val = lg_game_list->GetSelection();
    }
    if( game_dialog.game_setup_manager )
      if( !game_dialog.game_setup_manager->can_enter_player_setup() &&
	  GetNext() == game_dialog.get_player_page() )
      {
	wxMessageDialog( this, _("Please wait for connection to allow player setup"), 
			 _("Connecting"), wxOK | wxICON_INFORMATION ).ShowModal();
	return false;
      }

    return true;
  }

  void Load_LG_Board_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
  }

  void Load_LG_Board_Page::restore()		// display stored game state
  {
    changes = true;		// take first setting as change

    if( game_dialog.valid_lg_directory != wxT("") )
      scan_directory( game_dialog.valid_lg_directory );
    else
      lg_files.clear();
  }

  bool Load_LG_Board_Page::scan_directory( wxString directory )
  {
    wxDir dir(directory);

    if( !dir.IsOpened() ) return false;

    // scan files
    lg_files.clear();
    wxString wx_filename;
    bool ok = dir.GetFirst( &wx_filename, wxT("*"), wxDIR_FILES );
    while( ok )
    {
      std::string filename = wxstr_to_str(directory + wxT('/') + wx_filename);
      std::ifstream is( filename.c_str() );
      if( is )
      {
	LG_Content content = scan_littlegolem_file( is );
	if( content.moves >= 0 )
	{
	  lg_files.push_back( std::pair<LG_Content,std::string>( content, filename ) );
	}
      }

      ok = dir.GetNext( &wx_filename );
    }

    // setup list box
    lg_game_list->Clear();
    std::list< std::pair<LG_Content,std::string> >::iterator i;
    for( i = lg_files.begin(); i != lg_files.end(); ++i )
    {
      wxString board_str;
      board_str << /*_("Event ") << */str_to_wxstr(i->first.event) << wxT(" ") 
		<< str_to_wxstr(i->first.white_name) << wxT(":")
		<< str_to_wxstr(i->first.black_name) << wxT(" (") 
		<< i->first.moves << wxT(" ") << _("moves") << wxT(")");
	/*
	board_str.Printf( _("Board %d %s : %s (%d moves)"), master_content.id, 
p			  str_to_wxstr(master_content.player1).c_str(), str_to_wxstr(master_content.player2).c_str(), 
			  master_content.to );
	*/
      lg_game_list->Append( board_str );
    }

    return true;
  }

  void Load_LG_Board_Page::on_choose_directory( wxCommandEvent& WXUNUSED(event) )
  {
    wxString directory = game_dialog.valid_lg_directory;
    if( directory == wxT("") ) 
      directory = wxGetCwd();

#ifdef BACKCOMPATIBLE_WX_2_6
    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory );
#else
    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory, 
					   wxDD_DIR_MUST_EXIST | wxDD_CHANGE_DIR );
#endif
    if( dialog->ShowModal() == wxID_OK )
    {
      changes = true;
      if( scan_directory( dialog->GetPath() ) )
      {
	game_dialog.valid_lg_directory = dialog->GetPath();
	lg_directory->SetValue( game_dialog.valid_lg_directory );
      }
    }
  }

  void Load_LG_Board_Page::on_change_directory( wxCommandEvent& WXUNUSED(event) )
  {
    changes = true;
    if( scan_directory( lg_directory->GetValue() ) )
    {
      game_dialog.valid_lg_directory = lg_directory->GetValue();
    }
  }

  BEGIN_EVENT_TABLE(Load_LG_Board_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD,	Load_LG_Board_Page::on_page_changing) 
    EVT_BUTTON(DIALOG_CHOOSE_DIRECTORY,		Load_LG_Board_Page::on_choose_directory)	
    EVT_TEXT_ENTER(DIALOG_CHANGE_DIRECTORY,	Load_LG_Board_Page::on_change_directory) //**/
  END_EVENT_TABLE()								//**/
#endif	// #if defined(VERSION_DVONN)

  // ----------------------------------------------------------------------------
  // Player_Setup_Panel
  // ----------------------------------------------------------------------------
  
  Player_Setup_Panel::Player_Setup_Panel( wxWindow *parent, Game_Dialog &game_dialog )
    : wxPanel(parent, -1), game_dialog(game_dialog)
  {
    // create the child controls
    player_name = new wxTextCtrl( this, DIALOG_PLAYER_NAME, _("Player 1"), wxDefaultPosition, 
				  wxDefaultSize, wxTE_PROCESS_ENTER );
    ai = new wxCheckBox( this, -1, _("AI") );
    player_list = new wxListBox( this, LISTBOX_DCLICK, wxDefaultPosition, 
				  wxSize(200,150), 0, 0, wxLB_SINGLE );
    // update choice names with correct translation
    wxString help_choices[3];
    help_choices[0] = wxString(_("No help"));
    help_choices[1] = wxString(_("Show possible moves"));
    help_choices[2] = wxString(_("Show Hint"));
    help_choice = new wxRadioBox( this, -1, _("Help mode"), wxDefaultPosition,
				  wxDefaultSize, 
				  3, help_choices, 3, wxRA_SPECIFY_COLS );

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *add_player_sizer  = new wxBoxSizer( wxHORIZONTAL );
    add_player_sizer->Add( player_name, 0, wxALL, 10 );
    add_player_sizer->Add( ai, 0, wxALIGN_CENTER );
    wxButton *add_button = new wxButton(this,DIALOG_ADD_PLAYER,_("Add player"));
    add_button->SetDefault();
    add_button->SetFocus();
    add_player_sizer->Add( add_button, 0, wxALL, 10 );
    top_sizer->Add( add_player_sizer, 0, wxALIGN_CENTER );

    top_sizer->Add( help_choice, 0, wxALIGN_CENTER | wxALL, 10 );

    wxBoxSizer *player_list_sizer  = new wxBoxSizer( wxHORIZONTAL );
    player_list_sizer->Add( player_list, 1, wxEXPAND | wxALL, 10 );
    wxBoxSizer *player_list_buttons_sizer  = new wxBoxSizer( wxVERTICAL );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_PLAYER_UP,_("Up")), 0, wxALL, 10 );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_PLAYER_DOWN,_("Down")), 0, wxALL, 10 );
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_REMOVE_PLAYER,_("Remove")), 
				    0, wxALL, 10 );
    player_list_sizer->Add( player_list_buttons_sizer, 0, wxALIGN_CENTER );
    top_sizer->Add( player_list_sizer, 0, wxALIGN_CENTER );

    wxBoxSizer *button_sizer  = new wxBoxSizer( wxHORIZONTAL );
    ready_button = new wxButton(this,DIALOG_READY,_("Ready"));
    button_sizer->Add(ready_button, 0, wxALL, 10);
    top_sizer->Add( button_sizer, 0, wxALIGN_CENTER );

    status_display = new wxStaticText(this,-1,wxT(""));
    top_sizer->Add(status_display, 0, wxALIGN_CENTER | wxALL, 10);

    // set help texts
#ifndef __WXMSW__
    button_sizer->Add(new wxContextHelpButton(this), 0, wxALIGN_CENTER | wxALL, 10);
#endif

    player_name->SetHelpText(_("Type the new player's name here."));
    player_list->SetHelpText(_("Shows all players which will be part of the new game."));
    help_choice->SetHelpText(_("Select what kind of help you want for the new player."));
    add_button->SetHelpText(_("Add the new player to the game."));
    ai->SetHelpText(_("Check this box if the new player should be computer-controlled, uncheck if it is a human player."));
    FindWindow(DIALOG_PLAYER_UP)->SetHelpText(_("Move the currently selected player upwards. (The upmost player begins the game.)"));
    FindWindow(DIALOG_PLAYER_DOWN)->SetHelpText(_("Move the currently selected player downwards. (The upmost player begins the game.)"));
    FindWindow(DIALOG_REMOVE_PLAYER)->SetHelpText(_("Remove the currently selected player from the new game."));
    FindWindow(DIALOG_READY)->SetHelpText(_("Click this button as soon as you are happy with the settings. \nNetwork games start when all players have clicked 'Ready'."));

    // set sizer
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
  }

  Player_Setup_Panel::~Player_Setup_Panel()
  {
  }

  void Player_Setup_Panel::on_ready( wxCommandEvent& WXUNUSED(event) )
  {
    game_dialog.get_player_page()->set_ready(true);
    game_dialog.game_setup_manager->ready();
    update_status_display();
  }

  void Player_Setup_Panel::on_player_name( wxCommandEvent& event )
  {
    on_add_player(event);
  }

  void Player_Setup_Panel::on_add_player( wxCommandEvent& WXUNUSED(event) )
  {
#ifdef BACKCOMPATIBLE_WX_2_6
    player_name->SetValue(player_name->GetValue());
#else
    player_name->SetModified(false);
#endif
    if( game_dialog.game_setup_manager )
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
      Player_Input *input;
      if( ai->GetValue() )
      {
	type  = Player::ai;
	input = game_dialog.game_manager.get_player_ai();
      }
      else
      {
	type = Player::user;
	input = game_dialog.gui_manager.get_user_input();
      }

      Player player( wxstr_to_str(player_name->GetValue()), -1, input, Player::no_output, "", type, 
		     help_mode );

      int num_players = player_list->GetCount();
      if( !game_dialog.game_setup_manager->add_player( player ) )
      {
	wxMessageDialog(this, _("Could not add player"), _("Add player"), 
			wxOK | wxICON_INFORMATION).ShowModal();
      }
      else
      {
	set_player_name( get_default_name(num_players + 2) );
      }
    }
    update_status_display();
  }

  void Player_Setup_Panel::on_remove_player( wxCommandEvent& WXUNUSED(event) )
  {
    if( game_dialog.game_setup_manager )
    {
      int item = player_list->GetSelection();
      if( item >= 0 )
      {
	int num_players = player_list->GetCount();
	if( !game_dialog.game_setup_manager->remove_player( item_player[item] ) )
	{
	  wxMessageDialog( this, _("Could not remove player!"), 
			   _("Remove player"), wxOK | wxICON_INFORMATION ).ShowModal();
	}
	else
	{
	  set_player_name( get_default_name(num_players) );
	}
      }
    }
    update_status_display();
  }
    
  void Player_Setup_Panel::on_player_up( wxCommandEvent& WXUNUSED(event) )
  {
    if( game_dialog.game_setup_manager )
    {
      int item = player_list->GetSelection();
      if( item >= 1 )
      {
	if( !game_dialog.game_setup_manager->player_up( item_player[item] ) )
	{
	  wxMessageDialog( this, _("Could not move player!"), _("Move player"), 
			   wxOK | wxICON_INFORMATION ).ShowModal();
	}
      }
    }
    update_status_display();
  }

  void Player_Setup_Panel::on_player_down( wxCommandEvent& WXUNUSED(event) )
  {
    if( game_dialog.game_setup_manager )
    {
      int item = player_list->GetSelection();
      if( (item >= 0) && (item < (int)player_list->GetCount()-1) )
      {
	if( !game_dialog.game_setup_manager->player_down( item_player[item] ) )
	{
	  wxMessageDialog( this, _("Could not move player!"), _("Move player"), 
			   wxOK | wxICON_INFORMATION ).ShowModal();
	}
      }
    }
    update_status_display();
  }

  void Player_Setup_Panel::player_added( const Player &player )
  {
    // setup lookup tables
    int item = player_list->GetCount();
    player_item[player.id] = item;
    item_player[item] = player; 

    // add hostname to player name
    std::string name = player.name;
    if( player.origin != Player::local )
      name += '(' + player.host + ')';

    player_list->Append( str_to_wxstr(name) );

    if( !player_name->IsModified() )
    {
      set_player_name( get_default_name(player_list->GetCount() + 1) );
    }
    update_status_display();
  }

  void Player_Setup_Panel::player_removed( const Player &player )
  {
    int item = player_item[player.id];
    player_list->Delete( item );
    player_item.erase( player.id );
    item_player.erase( item );

    // adapt player item numbers of higher players
    std::map<int,int>::iterator i;
    for( i = player_item.begin(); i != player_item.end(); ++i )
    {
      if( i->second >= item ) 
	--i->second;
    }
    std::map<int,Player> item_player_old = item_player;
    item_player.clear();
    std::map<int,Player>::iterator j;
    for( j = item_player_old.begin(); j != item_player_old.end(); ++j )
    {
      int new_item = j->first;
      if( new_item >= item )
	--new_item;
      item_player[new_item] = j->second;
    }

    if( !player_name->IsModified() )
    {
      set_player_name( get_default_name(player_list->GetCount() + 1) );
    }
    update_status_display();
  }

  void Player_Setup_Panel::player_up( const Player &player )
  {
    int item = player_item[player.id];

    if( item > 0 )
    {
      int item0 = item;
      int item1 = item - 1;
      // copy everything
      wxString str0 = player_list->GetString(item0);
      wxString str1 = player_list->GetString(item1);
      Player player0 = item_player[item0];
      Player player1 = item_player[item1];
      // swap everything
      player_list->SetString(item0,str1);
      player_list->SetString(item1,str0);
      player_item[player0.id] = item1;
      player_item[player1.id] = item0;
      item_player[item0] = player1;
      item_player[item1] = player0;

      player_list->SetSelection(item1);
      player_list->Refresh();
    }
    update_status_display();
  }

  void Player_Setup_Panel::player_down( const Player &player )
  {
    int item = player_item[player.id];
    if( item < (int)player_list->GetCount() - 1 )
    {
      int item0 = item;
      int item1 = item + 1;
      // copy everything
      wxString str0 = player_list->GetString(item0);
      wxString str1 = player_list->GetString(item1);
      Player player0 = item_player[item0];
      Player player1 = item_player[item1];
      // swap everything
      player_list->SetString(item0,str1);
      player_list->SetString(item1,str0);
      player_item[player0.id] = item1;
      player_item[player1.id] = item0;
      item_player[item0] = player1;
      item_player[item1] = player0;

      player_list->SetSelection(item1);
      player_list->Refresh();
    }
    update_status_display();
  }

  void Player_Setup_Panel::player_change_denied()
  {
    wxMessageDialog( this, _("Change for this player denied!"), _("Denied"), 
		     wxOK | wxICON_INFORMATION ).ShowModal();
  }

  void Player_Setup_Panel::player_ready( const Player & )
  {
    // show that player is ready
  }

  /*
  void Player_Setup_Panel::ruleset_changed( Ruleset::Ruleset_Type type )
  {
    int new_type = 0;
    switch( type )
    {
      case Ruleset::standard: !!! obsolete
	new_type = 0;
	break;
      case Ruleset::tournament:
	new_type = 1;
	break;
      case Ruleset::custom:
	new_type = 2;
	break;
    }
    if( new_type != ruleset_choice->GetSelection() )
    {
      ruleset_choice->SetSelection(new_type);
    }
  }

  void Player_Setup_Panel::ruleset_changed( Ruleset::Ruleset_Type type, Ruleset &ruleset )
  {
    int new_type = 0;
    switch( type )
    {
      case Ruleset::standard: !!! obsolete
	new_type = 0;
	break;
      case Ruleset::tournament:
	new_type = 1;
	break;
      case Ruleset::custom:
	new_type = 2;
	dialog->ruleset_page->set_ruleset(&ruleset); // shouldn't be done at once
	break;
    }
    if( new_type != ruleset_choice->GetSelection() )
    {
      ruleset_choice->SetSelection(new_type);
    }
  }

  void Player_Setup_Panel::ruleset_change_denied()
  {
    current_ruleset = last_ruleset;
    ruleset_choice->SetSelection( last_ruleset );
    wxMessageDialog( this, _("Change of ruleset denied!"), _("Ruleset denied"), 
                     wxOK | wxICON_INFORMATION ).ShowModal();
  }
  */

  // display stored game state
  void Player_Setup_Panel::restore()		
  {
    player_list->Clear();
    item_player.clear();
    player_item.clear();
    std::list<Player>::iterator i;
    for( i = game_dialog.players.begin(); i != game_dialog.players.end(); ++i )
      player_added( *i );

    set_player_name( get_default_name(player_list->GetCount() + 1) );
    update_status_display();
  }

  // default players changed
  void Player_Setup_Panel::players_changed()	
  {
    if( !player_name->IsModified() )
    {
      set_player_name( get_default_name(player_list->GetCount() + 1) );
    }
  }

  // all players are ready
  void Player_Setup_Panel::everything_ready()
  {
    update_status_display();
  }
  
  void Player_Setup_Panel::update_status_display()
  {
    switch( game_dialog.game_setup_manager->can_start() )
    {
      case Game_Setup_Manager::too_few_players: 
	status_display->SetLabel(_("Please add more players"));
	ready_button->Enable(false);
	break;
      case Game_Setup_Manager::too_many_players: 
	status_display->SetLabel(_("Please remove some players"));
	ready_button->Enable(false);
	break;
      case Game_Setup_Manager::not_ready: 
	if( game_dialog.get_player_page()->is_ready() )
	{
	  status_display->SetLabel(_("Wait for others to become ready"));
	  ready_button->Enable(false);
	}
	else
	{
	  status_display->SetLabel(_("Click on ready when ready"));
	  ready_button->Enable(true);
	}
	break;
      case Game_Setup_Manager::everyone_ready: 
	status_display->SetLabel(_("Please continue"));
	ready_button->Enable(false);
	break;
    }
  }

  wxString Player_Setup_Panel::get_default_name( int player_num )
  {
    if( game_dialog.game.players.size() >= unsigned(player_num) &&
	game_dialog.game_setup_manager->can_choose_board() )
    {
      std::vector<Player>::iterator i = game_dialog.game.players.begin();
      for( int n = 1; n < player_num; ++n )
	++i;
      return str_to_wxstr( i->name ); 
    }
    else
    {
      wxString name;
      name.Printf( _("Player %d"), player_num );
      return name;
    }
  }

  void Player_Setup_Panel::set_player_name( wxString name )
  {
    player_name->SetValue(name);
    player_name->SetSelection( 0, player_name->GetLastPosition() );
    player_name->SetFocus();
  }

  BEGIN_EVENT_TABLE(Player_Setup_Panel, wxPanel)				
    EVT_BUTTON(DIALOG_READY,		Player_Setup_Panel::on_ready)		
    EVT_BUTTON(DIALOG_ADD_PLAYER,    	Player_Setup_Panel::on_add_player)	
    EVT_BUTTON(DIALOG_REMOVE_PLAYER, 	Player_Setup_Panel::on_remove_player)	
    EVT_BUTTON(DIALOG_PLAYER_UP,	Player_Setup_Panel::on_player_up)	
    EVT_BUTTON(DIALOG_PLAYER_DOWN,   	Player_Setup_Panel::on_player_down)	
    EVT_TEXT_ENTER(DIALOG_PLAYER_NAME,	Player_Setup_Panel::on_player_name)	
  END_EVENT_TABLE()								//**/
  
  // ----------------------------------------------------------------------------
  // Player_Page
  // ----------------------------------------------------------------------------

  Player_Page::Player_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), ready(false)
  {
    notebook = new wxNotebook( this, -1 );
    wxBoxSizer *sizer = new wxBoxSizer( wxVERTICAL );
    sizer->Add(notebook, 1, wxEXPAND);

    player_setup_panel = new Player_Setup_Panel( notebook, game_dialog );
    notebook->AddPage( player_setup_panel, _("Player setup") );

    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer(sizer);
  }

  wxWizardPage *Player_Page::GetPrev() const
  {
    bool can_choose = true;
    if( game_dialog.game_setup_manager )
      can_choose = game_dialog.game_setup_manager->can_choose_board();

    if( can_choose )
      return game_dialog.get_board_page()->get_last_board_page();
    else
      return game_dialog.get_setup_manager_page();
  }

  wxWizardPage *Player_Page::GetNext() const
  {
    return 0;
  }

  bool Player_Page::transfer_data_from_window(bool direction)
  {
    if( !direction ) return true; // always allow to go back

    switch( game_dialog.game_setup_manager->can_start() )
    {
    case Game_Setup_Manager::too_few_players: 
      wxMessageDialog( this, _("Some more players are needed!"), _("Can't start"), 
		       wxOK | wxICON_INFORMATION ).ShowModal();
      break;
    case Game_Setup_Manager::too_many_players: 
      wxMessageDialog( this, _("Too many players!"), _("Can't start"), 
		       wxOK | wxICON_INFORMATION ).ShowModal();
      break;
    case Game_Setup_Manager::not_ready: 
      wxMessageDialog( this, _("Please wait until everyone is ready"), _("Can't start"), 
		       wxOK | wxICON_INFORMATION ).ShowModal();
      break;
    case Game_Setup_Manager::everyone_ready: 
      return true;
    }
    return false;
  }

  void Player_Page::on_page_changing( wxWizardEvent& event )
  {
    if( !transfer_data_from_window(event.GetDirection()) )
      event.Veto();
  }

  // display stored game state
  void Player_Page::restore()		
  {
    ready = false;
    player_setup_panel->restore();
  }

  // default players changed
  void Player_Page::players_changed()
  {
    player_setup_panel->players_changed();
  }

  // all players are ready
  void Player_Page::everything_ready()
  {
    player_setup_panel->everything_ready();
  }

  BEGIN_EVENT_TABLE(Player_Page, wxWizardPage)				
    EVT_WIZARD_PAGE_CHANGING(DIALOG_WIZARD, Player_Page::on_page_changing) //**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Game_Setup_Wizard
  // ----------------------------------------------------------------------------

  wxSize bounding_size( wxSize size1, wxSize size2 )
  {
    wxSize ret;
    ret.SetWidth ( size1.GetWidth()  > size2.GetWidth()  ? size1.GetWidth()  : size2.GetWidth()  );
    ret.SetHeight( size1.GetHeight() > size2.GetHeight() ? size1.GetHeight() : size2.GetHeight() );
    return ret;
  }

  Game_Setup_Wizard::Game_Setup_Wizard( wxWindow *parent, int id, Game_Dialog& game_dialog )
    : wxWizard(parent, id, _("Setup game")),
      setup_manager_page( new Setup_Manager_Page(this, game_dialog) ),
      board_page( new Board_Page(this, game_dialog) ),
      custom_board_page( new Custom_Board_Page(this, game_dialog) ),
      load_pbm_board_page( new Load_PBM_Board_Page(this, game_dialog) ),
#if defined(VERSION_DVONN)
      load_lg_board_page( new Load_LG_Board_Page(this, game_dialog) ),
#endif
      player_page( new Player_Page(this, game_dialog) )
  {
    best_size = bounding_size( setup_manager_page->GetBestSize(), board_page->GetBestSize() );
    best_size = bounding_size( best_size, custom_board_page->GetBestSize() );
    best_size = bounding_size( best_size, custom_board_page->GetBestSize() );
    best_size = bounding_size( best_size, load_pbm_board_page->GetBestSize() );
#if defined(VERSION_DVONN)
    best_size = bounding_size( best_size, load_lg_board_page->GetBestSize() );
#endif
    best_size = bounding_size( best_size, player_page->GetBestSize() );
    SetPageSize(best_size);
  }

  // ----------------------------------------------------------------------------
  // Game_Dialog
  // ----------------------------------------------------------------------------

  Game_Dialog::Game_Dialog( wxWindow* /*parent*/, Game_Manager &game_manager, 
			    WX_GUI_Manager &gui_manager )
    : game_manager(game_manager), gui_manager(gui_manager),
      game( Standard_Ruleset() ), game_setup_manager(0),
      hostname(wxT("")), server_port(DEFAULT_PORT), client_port(DEFAULT_PORT),
      wizard(0), clients_dialog(0)
  {
    // self register
    game_manager.set_game_setup_display_handler( this );
  }

  Game_Dialog::~Game_Dialog()
  {
    if( wizard )
    {
      wizard->Destroy();
      wizard = 0;
    }
    if( clients_dialog )
    {
      clients_dialog->destroy();
      clients_dialog = 0;
    }
    if( game_setup_manager )
    {
      game_setup_manager->set_display_handler(0);
      delete game_setup_manager;
      game_setup_manager = 0;
    }
  }

  void Game_Dialog::game_setup()
  {
    if( wizard ) return; // already wizard running
    // create wizard
    wizard = new Game_Setup_Wizard( &gui_manager.get_game_window(), DIALOG_WIZARD, *this );
    // init variables
    game_setup_manager = game_manager.get_game_setup_manager();
    // init pages
    get_setup_manager_page()->restore();
    // other pages are initialized after setup_manager change

    if( game_setup_manager )
      game_setup_manager->game_setup_entered();

    wizard->ShowPage(get_setup_manager_page());
    wizard->Show(true);
  }

  void Game_Dialog::set_board( const Game &new_game )
  {
    game = new_game;
    get_custom_board_page()->restore();
  }

  bool Game_Dialog::ask_change_board( const Game &new_game, wxString who )
  {
    wxString msg = who + _(" asks to use another board. Accept?");
    if( wxMessageDialog( 0, msg, _("Accept board?"), 
			 wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
    {
      game = new_game;
      get_custom_board_page()->restore();
      return true;
    } 
    return false;
  }

  void Game_Dialog::change_board( const Game &new_game )
  {
    game = new_game;
    wxString msg = _("Board changed. Do you want to view the new settings?");
    if( wxMessageDialog( 0, msg, _("View board?"), 
			 wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
    {
      get_custom_board_page()->restore();
    }
  }

  void Game_Dialog::board_change_accepted()
  {
  }

  void Game_Dialog::board_change_denied()
  {
    wxString msg = _("Board change denied!");
    wxMessageDialog( 0, msg, _("Change denied!"), wxOK | wxICON_INFORMATION ).ShowModal();
    game = game_setup_manager->get_board();
  }

  void Game_Dialog::player_added( const Player &player )
  {
    get_player_page()->player_setup_panel->player_added( player );
  }
  void Game_Dialog::player_removed( const Player &player )
  {
    get_player_page()->player_setup_panel->player_removed( player );
  }
  void Game_Dialog::player_up( const Player &player )
  {
    get_player_page()->player_setup_panel->player_up( player );
  }
  void Game_Dialog::player_down( const Player &player )
  {
    get_player_page()->player_setup_panel->player_down( player );
  }
  void Game_Dialog::player_change_denied()
  {
    get_player_page()->player_setup_panel->player_change_denied();
  }
  void Game_Dialog::player_ready( const Player &player )
  {
    get_player_page()->player_setup_panel->player_ready( player );
  }

  void Game_Dialog::enter_player_setup()
  {
    get_setup_manager_page()->client_connected();
  }
  void Game_Dialog::everything_ready()
  {
    get_player_page()->everything_ready();
  }
  void Game_Dialog::aborted()
  {
    bool tell_user = true;
    if( wizard )
      if( get_setup_manager_page()->connection_closed() )
	tell_user = false;
    if( tell_user )
    {
      wxMessageDialog( 0, _("Connection was closed! This may be caused intentinally by your fellow players or be a result of technical problems of the connection."), _("disconnect"), 
		       wxOK | wxICON_INFORMATION ).ShowModal();
    }
  }
  void Game_Dialog::game_started()
  {
    if( wizard )
    {
      wizard->Destroy();
      wizard = 0;
    }
  }

  bool Game_Dialog::ask_new_game( wxString who ) // other player asks for a new game (true: accept)
  {
    wxString msg = who + _(" asks to play new game. Accept?");
    if( wxMessageDialog( 0, msg, _("New game?"), 
			 wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
    {
      return true;
    } 
    return false;
  }
  bool Game_Dialog::ask_undo_moves( wxString who, int n ) 
  {
    wxString msg;
    if( n == 1 )
      msg.Printf( _("%s asks undo one half move. Accept?"), who.c_str(), n);
    else
      msg.Printf( _("%s asks undo %d half moves. Accept?"), who.c_str(), n);

    if( wxMessageDialog( 0, msg, _("Allow undo?"), 
			 wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
    {
      return true;
    } 
    return false;
  }

  void Game_Dialog::on_wizard_finished( wxWizardEvent& )
  {
    wizard = 0;			// wizard will destroy itself
    if( game_setup_manager )
    {
      game_setup_manager->disable_player_feedback();
      assert( game_setup_manager->can_start() == Game_Setup_Manager::everyone_ready );
      game_setup_manager->start_game();
    }
  }

  void Game_Dialog::on_wizard_page_changing( wxWizardEvent& )
  {
    // will be handled by page
  }

  void Game_Dialog::on_wizard_cancel( wxWizardEvent& event )
  {
    if( game_setup_manager )
    {
      if(!game_setup_manager->is_allow_game_setup_abort())
      {
	wxMessageDialog( 0, _("Closing the player setup now would leave the game in a bad state. Please go back to make changes to the game setup or finish the current setup. You can also start another game from the File menu."), 
			 _("Please don't close"), wxOK | wxICON_INFORMATION ).ShowModal();
	
	event.Veto();
	return;
      }
    }
    wizard = 0;			// wizard will destroy itself
  }

  void Game_Dialog::get_data_from_setup_manager()
  {
    // init variables
    if( game_setup_manager )
    {
      game    = game_setup_manager->get_board();
      players = game_setup_manager->enable_player_feedback();
    }

    // init all pages
    get_player_page()->restore();	// initializes player list from players
    get_board_page()->restore();
    get_custom_board_page()->restore();
    get_load_pbm_board_page()->restore();
#if defined(VERSION_DVONN)
    get_load_lg_board_page()->restore();
#endif
  }

  // ============================================================================
  // Display_Setup_Page
  // ============================================================================

  Display_Setup_Page::Display_Setup_Page( wxWindow *parent, Settings_Dialog *dialog, 
					  WX_GUI_Manager &gui_manager )
    : wxPanel( parent, -1 ), dialog(dialog), gui_manager(gui_manager),
      game_settings( gui_manager.get_game_settings() )
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

#if defined(VERSION_ZERTZ)
    wxString orientation_choices[2];
    orientation_choices[0] = wxString(_("Horizontal"));
    orientation_choices[1] = wxString(_("Vertical"));
    orientation_choice = new wxRadioBox( this, -1, _("Board orientation"), wxDefaultPosition,
					 wxDefaultSize, 2, orientation_choices, 1, 
					 wxRA_SPECIFY_ROWS );
    top_sizer->Add( orientation_choice, 0, wxALL, 10  );

    wxString arrangement_choices[2];
    arrangement_choices[0] = wxString(_("Board and common stones left"));
    arrangement_choices[1] = wxString(_("Board left all stones right"));
    arrangement_choice = new wxRadioBox( this, -1, _("Panels arrangement"), wxDefaultPosition,
					 wxDefaultSize, 2, arrangement_choices, 1, 
					 wxRA_SPECIFY_COLS );
    top_sizer->Add( arrangement_choice, 0, wxALL, 10  );
    orientation_choice->
      SetHelpText(_("Select the orientation in which the board is displayed on screen."));
#elif defined(VERSION_BLOKS)
    wxString orientation_choices[2];
    orientation_choices[0] = wxString(_("Horizontal"));
    orientation_choices[1] = wxString(_("Vertical"));
    orientation_choice = new wxRadioBox( this, -1, _("Board orientation"), wxDefaultPosition,
					 wxDefaultSize, 2, orientation_choices, 1, 
					 wxRA_SPECIFY_ROWS );
    top_sizer->Add( orientation_choice, 0, wxALL, 10  );

    wxString arrangement_choices[2];
    arrangement_choices[0] = wxString(_("Board top stones below"));
    arrangement_choices[1] = wxString(_("Board left stones right"));
    arrangement_choice = new wxRadioBox( this, -1, _("Panels arrangement"), wxDefaultPosition,
					 wxDefaultSize, 2, arrangement_choices, 1, 
					 wxRA_SPECIFY_COLS );
    top_sizer->Add( arrangement_choice, 0, wxALL, 10  );
    orientation_choice->
      SetHelpText(_("Select the orientation in which the board is displayed on screen."));
#endif

    show_coordinates = new wxCheckBox( this, -1, _("Show field coordinates") );
    top_sizer->Add( show_coordinates, 0, wxALL, 10 );

#if defined(VERSION_ZERTZ)
    multiple_common_stones = new wxCheckBox( this, -1, _("Display all common stones individually") );
    top_sizer->Add( multiple_common_stones, 0, wxALL, 10 );

    multiple_player_stones = new wxCheckBox( this, -1, _("Display all player stones individually") );
    top_sizer->Add( multiple_player_stones, 0, wxALL, 10 );

#elif defined(VERSION_BLOKS)
    multiple_player_stones = new wxCheckBox( this, -1, _("Display all player stones individually") );
    top_sizer->Add( multiple_player_stones, 0, wxALL, 10 );
#endif

    // set help texts
    show_coordinates->SetHelpText(_("If checked, the field coordinates (a1...g4) will be displayed next to the board. This can be useful for discussing the game."));

    SetAutoLayout( true );
    SetSizer( top_sizer );

    restore_settings();
  }

  Display_Setup_Page::~Display_Setup_Page()
  {
  }

  wxString get_font_name( wxFont &font )
  {
    wxString name;
    if( font.GetFaceName() != wxT("") )
      name = font.GetFaceName();
    else
    {
      switch( wxFontFamily(font.GetFamily()) )
      {
	case wxDEFAULT:    name = _("default family"); break;
	case wxDECORATIVE: name = _("decorative family"); break;
	case wxROMAN:      name = _("roman family"); break;
	case wxSCRIPT:     name = _("script family"); break;
	case wxSWISS:      name = _("swiss family"); break;
	case wxMODERN:     name = _("modern family"); break;
	case wxTELETYPE:   name = _("teletype family"); break;
	default:	   name = _("<unknown font>"); break;
      }
    }

    bool brackets = false;
    switch( font.GetStyle() )
    {
      case wxSLANT:  name += wxT("("); name += _("slant"); brackets=true; break;
      case wxITALIC: name += wxT("("); name += _("italic"); brackets=true; break;
    }

    switch( font.GetWeight() )
    {
      case wxLIGHT: 
	if( !brackets )
	  name += wxT("(");
	else
	  name += wxT(",");
	name += _("light"); 
	name += wxT(")"); 
	break;
      case wxBOLD:  
	if( !brackets )
	  name += wxT("(");
	else
	  name += wxT(",");
	name += _("bold");
	name += wxT(")"); 
	break;
      default: 
	if( brackets ) 
	  name += wxT(")");
	break;
    }

    return name + wxString::Format(wxT(" : %d pt"),font.GetPointSize());
  }

  void Display_Setup_Page::restore_settings()
  {
    show_coordinates->SetValue( dialog->game_settings.board_settings.show_coordinates );
#if defined(VERSION_ZERTZ)
    orientation_choice->SetSelection( dialog->game_settings.board_settings.rotate_board ? 1 : 0 );
    switch( dialog->game_settings.arrangement )
    {
      case Game_Panel::Settings::arrange_standard:     arrangement_choice->SetSelection(0); break;
      case Game_Panel::Settings::arrange_stones_right: arrangement_choice->SetSelection(1); break;
    }
    multiple_common_stones->SetValue( dialog->game_settings.common_stone_settings.multiple_stones );
    multiple_player_stones->SetValue( dialog->game_settings
				      .player_settings.stone_settings.multiple_stones );
#elif defined(VERSION_BLOKS)
    orientation_choice->SetSelection( dialog->game_settings.board_settings.rotate_board ? 1 : 0 );
    switch( dialog->game_settings.arrangement )
    {
      case Game_Panel::Settings::arrange_standard:     arrangement_choice->SetSelection(0); break;
      case Game_Panel::Settings::arrange_player_stones_right: arrangement_choice->SetSelection(1); break;
    }
    multiple_player_stones->SetValue( dialog->game_settings
				      .player_settings.stone_settings.multiple_stones );
#endif
  }

  void Display_Setup_Page::apply()
  {
    dialog->game_settings.board_settings.show_coordinates              
      = show_coordinates->GetValue();
#if defined(VERSION_ZERTZ)
    dialog->game_settings.board_settings.rotate_board 
      = orientation_choice->GetSelection() == 0 ? false : true;
    dialog->game_settings.common_stone_settings.rotate_stones          
      = dialog->game_settings.board_settings.rotate_board;
    dialog->game_settings.player_settings.stone_settings.rotate_stones 
      = dialog->game_settings.board_settings.rotate_board;
    switch( arrangement_choice->GetSelection() )
    {
      case 0: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_standard; break;
      case 1: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_stones_right; break;
    }
    dialog->game_settings.common_stone_settings.multiple_stones		 
      = multiple_common_stones->GetValue();
    dialog->game_settings.player_settings.stone_settings.multiple_stones 
      = multiple_player_stones->GetValue();
#elif defined(VERSION_BLOKS)
    dialog->game_settings.board_settings.rotate_board 
      = orientation_choice->GetSelection() == 0 ? false : true;
    dialog->game_settings.common_stone_settings.rotate_stones          
      = dialog->game_settings.board_settings.rotate_board;
    dialog->game_settings.player_settings.stone_settings.rotate_stones 
      = dialog->game_settings.board_settings.rotate_board;
    switch( arrangement_choice->GetSelection() )
    {
      case 0: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_standard; break;
      case 1: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_player_stones_right; break;
    }
    dialog->game_settings.player_settings.stone_settings.multiple_stones 
      = multiple_player_stones->GetValue();
#endif
  }

  BEGIN_EVENT_TABLE(Display_Setup_Page, wxPanel)			
    //EVT_BUTTON(DIALOG_CANCEL,	   Settings_Dialog::on_cancel)	//**/
  END_EVENT_TABLE()						//**/

  // ============================================================================
  // Look_Feel_Page
  // ============================================================================

  Look_Feel_Page::Look_Feel_Page( wxWindow *parent, Settings_Dialog *dialog, 
					  WX_GUI_Manager &gui_manager )
    : wxPanel( parent, -1 ), dialog(dialog), gui_manager(gui_manager)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer( 3 );

    grid_sizer->Add( new wxStaticText(this,-1,_("Skin file") ), 0, 
		     wxALL | wxALIGN_CENTER_VERTICAL, 10 );
    skin_file = new wxTextCtrl( this, DIALOG_CHANGE_SKIN_FILE, wxT("") );
    grid_sizer->Add( skin_file, 1, wxEXPAND | wxALL, 10 );
    grid_sizer->Add( new wxButton( this, DIALOG_CHOOSE_SKIN_FILE, _("Choose...") ), 0, wxALL, 10 );

    grid_sizer->Add( new wxStaticText(this,-1,_("Beep file") ), 0, 
		     wxALL | wxALIGN_CENTER_VERTICAL, 10 );
    beep_file = new wxTextCtrl( this, DIALOG_CHANGE_BEEP_FILE, wxT("") );
    grid_sizer->Add( beep_file, 1, wxEXPAND | wxALL, 10 );
    grid_sizer->Add( new wxButton( this, DIALOG_CHOOSE_BEEP_FILE, _("Choose...") ), 0, wxALL, 10 );
    grid_sizer->AddGrowableCol(1, 1);

    top_sizer->Add( grid_sizer, 0, wxALL | wxEXPAND, 10 );

    play_sound = new wxCheckBox( this, -1, _("Play sounds") );
    top_sizer->Add( play_sound, 0, wxALL, 10 );

    wxFlexGridSizer *font_sizer = new wxFlexGridSizer( 3 );

    //wxBoxSizer *player_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for player names") ), 0, wxALL, 10 );
    player_font_name = new wxStaticText(this,-1,wxT(""));
    player_font_name->SetSize(wxSize(200,-1));
    font_sizer->Add( player_font_name, 0, wxALL, 10 );
    font_sizer->Add( new wxButton( this, DIALOG_CHOOSE_PLAYER_FONT, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( player_font_sizer, 0, wxALL | wxEXPAND, 10 );

    //wxBoxSizer *coord_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for coordinates") ), 0, wxALL, 10 );
    coord_font_name = new wxStaticText(this,-1,wxT(""));
    coord_font_name->SetSize(wxSize(200,-1));
    font_sizer->Add( coord_font_name, 0, wxALL, 10 );
    font_sizer->Add( new wxButton( this, DIALOG_CHOOSE_COORD_FONT, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( coord_font_sizer, 0, wxALL | wxEXPAND, 10 );

    //wxBoxSizer *stone_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for text on stones") ), 0, wxALL, 10 );
    stone_font_name = new wxStaticText(this,-1,wxT(""));
    stone_font_name->SetSize(wxSize(200,-1));
    font_sizer->Add( stone_font_name, 0, wxALL, 10 );
    font_sizer->Add( new wxButton( this, DIALOG_CHOOSE_STONE_FONT, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( stone_font_sizer, 0, wxALL | wxEXPAND, 10 );

    top_sizer->Add( font_sizer, 0, wxALL | wxEXPAND, 10 );

    SetAutoLayout( true );
    SetSizer( top_sizer );

    restore_settings();
  }

  Look_Feel_Page::~Look_Feel_Page()
  {
  }

  void Look_Feel_Page::restore_settings()
  {
    valid_skin_file = dialog->game_settings.skin_file;
    valid_beep_file = dialog->game_settings.beep_file;
    player_font = dialog->game_settings.player_settings.player_font;
    player_font_name->SetLabel( get_font_name(player_font) );
    coord_font = dialog->game_settings.board_settings.coord_font;
    coord_font_name->SetLabel( get_font_name(coord_font) );
    stone_font = dialog->game_settings.common_stone_settings.stone_font;
    stone_font_name->SetLabel( get_font_name(stone_font) );
    skin_file->SetValue( valid_skin_file );
    beep_file->SetValue( valid_beep_file );
    play_sound->SetValue( dialog->game_settings.play_sound );
  }

  void Look_Feel_Page::apply()
  {
    wxString filename;
    // ********************
    // get TextCtrl content
    filename = skin_file->GetValue();
    if( wxFileExists(filename) )
      valid_skin_file = filename;
    else
    {
      /*!!! output error !!!*/;
      skin_file->SetValue(valid_skin_file);
    }
    if( play_sound->GetValue() )
    {
      filename = beep_file->GetValue();
      if( wxFileExists(filename) )
	valid_beep_file = filename;
      else
      {
	/*!!! output error !!!*/
	beep_file->SetValue(valid_beep_file);
      }
    }

    // ********************
    // load skin
    if( dialog->game_settings.skin_file != valid_skin_file )
    {
      if( gui_manager.load_skin( valid_skin_file ) )
	dialog->game_settings.skin_file = valid_skin_file;
      else
      {
	/*!!! output error !!!*/
	gui_manager.load_skin( dialog->game_settings.skin_file );
      }
    }

    // ********************
    // load sound
    if( play_sound->GetValue() )
    {
      if( (dialog->game_settings.beep_file  != valid_beep_file) || 
	  (!dialog->game_settings.play_sound) )
      {
	if( gui_manager.load_beep( valid_beep_file ) )
	  dialog->game_settings.beep_file = valid_beep_file;
	else
	{
	  /*!!! output error !!!*/
	  if( !gui_manager.load_beep( dialog->game_settings.beep_file ) )
	  {
	    play_sound->SetValue( false );
	  }
	}
      }
    }
    dialog->game_settings.play_sound = play_sound->GetValue();

    // ********************
    // set fonts
    dialog->game_settings.player_settings.player_font		    = player_font;
    dialog->game_settings.board_settings.coord_font		    = coord_font;
    dialog->game_settings.common_stone_settings.stone_font	    = stone_font;
#if defined(VERSION_ZERTZ) || defined(VERSION_BLOKS)
    dialog->game_settings.player_settings.stone_settings.stone_font = stone_font;
#elif defined(VERSION_DVONN)
    dialog->game_settings.board_settings.stack_font		    = stone_font;
#endif
  }

  void Look_Feel_Page::on_choose_skin( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = wxFileSelector( _("Choose a skin file"), wxPathOnly(valid_skin_file), 
					wxT(""), wxT(""), _("Skin files (*.zip)|*.zip"),
					wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    if( wxFileExists(filename) )
    {
      valid_skin_file = filename;
      skin_file->SetValue( filename );
    }
    else 
    {
      /*!!! output error !!!*/
    }
  }
  void Look_Feel_Page::on_change_skin( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = skin_file->GetValue();
    if( wxFileExists(filename) )
      valid_skin_file = filename;
    else
    {
      /*!!! output error !!!*/
    }
  }
  void Look_Feel_Page::on_choose_beep( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = wxFileSelector( _("Choose a beep file"), wxPathOnly(valid_beep_file), 
					wxT(""), wxT(""), _("Beep sounds (*.wav)|*.wav"),
					wxFD_OPEN | wxFD_FILE_MUST_EXIST );
    if( wxFileExists(filename) )
    {
      valid_beep_file = filename;
      beep_file->SetValue( filename );
    }
    else
    {
      /*!!! output error !!!*/
    }
  }
  void Look_Feel_Page::on_change_beep( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = beep_file->GetValue();
    if( wxFileExists(filename) )
      valid_beep_file = filename;
    else
    {
      /*!!! output error !!!*/
    }
  }
  void Look_Feel_Page::on_choose_player_font( wxCommandEvent& WXUNUSED(event) )
  {
    wxFontData initial_font;
    initial_font.SetInitialFont( player_font );
    initial_font.SetChosenFont( player_font );
    wxFontDialog *dialog = new wxFontDialog(this,initial_font);
    int ret = dialog->ShowModal();

    wxFontData &data = dialog->GetFontData();
    if( (ret == wxID_OK) && (data.GetChosenFont() != wxNullFont) )
    {
      player_font = data.GetChosenFont();
      player_font_name->SetLabel( get_font_name(player_font) );
    }
    dialog->Destroy();
  }
  void Look_Feel_Page::on_choose_coord_font( wxCommandEvent& WXUNUSED(event) )
  {
    wxFontData initial_font;
    initial_font.SetInitialFont( coord_font );
    initial_font.SetChosenFont( coord_font );
    wxFontDialog *dialog = new wxFontDialog(this,initial_font);
    int ret = dialog->ShowModal();

    wxFontData &data = dialog->GetFontData();
    if( (ret == wxID_OK) && (data.GetChosenFont() != wxNullFont) )
    {
      coord_font = data.GetChosenFont();
      coord_font_name->SetLabel( get_font_name(coord_font) );
    }
    dialog->Destroy();
  }
  void Look_Feel_Page::on_choose_stone_font( wxCommandEvent& WXUNUSED(event) )
  {
    wxFontData initial_font;
    initial_font.SetInitialFont( stone_font );
    initial_font.SetChosenFont( stone_font );
    wxFontDialog *dialog = new wxFontDialog(this,initial_font);
    int ret = dialog->ShowModal();

    wxFontData &data = dialog->GetFontData();
    if( (ret == wxID_OK) && (data.GetChosenFont() != wxNullFont) )
    {
      stone_font = data.GetChosenFont();
      stone_font_name->SetLabel( get_font_name(stone_font) );
    }
    dialog->Destroy();
  }

  BEGIN_EVENT_TABLE(Look_Feel_Page, wxPanel)			
    EVT_BUTTON(DIALOG_CHOOSE_SKIN_FILE,	  Look_Feel_Page::on_choose_skin)	
    EVT_BUTTON(DIALOG_CHANGE_SKIN_FILE,	  Look_Feel_Page::on_change_skin)	
    EVT_BUTTON(DIALOG_CHOOSE_BEEP_FILE,	  Look_Feel_Page::on_choose_beep)	
    EVT_BUTTON(DIALOG_CHANGE_BEEP_FILE,	  Look_Feel_Page::on_change_beep)	
    EVT_BUTTON(DIALOG_CHOOSE_PLAYER_FONT, Look_Feel_Page::on_choose_player_font)
    EVT_BUTTON(DIALOG_CHOOSE_COORD_FONT, Look_Feel_Page::on_choose_coord_font)  
    EVT_BUTTON(DIALOG_CHOOSE_STONE_FONT, Look_Feel_Page::on_choose_stone_font)   //**/
  END_EVENT_TABLE()								 //**/

  // ============================================================================
  // Settings Dialog
  // ============================================================================

  Settings_Dialog::Settings_Dialog( wxWindow *parent, WX_GUI_Manager &gui_manager )
    : gui_manager(gui_manager),
      game_settings( gui_manager.get_game_settings() )
  {
    // create the dialog
#ifdef __WXMSW__ // this has to be done before the dialog is created, ...
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
#endif // ... hence the two-step construction (Create() below) is necessary.
    wxDialog::Create(parent,-1,wxString(_("Settings")));

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    //wxPanel *notebook_panel = new wxPanel( this, -1 );
    notebook = new wxNotebook( this, -1 );

    display_page = new Display_Setup_Page( notebook, this, gui_manager );
    look_feel_page = new Look_Feel_Page( notebook, this, gui_manager );

    notebook->AddPage( display_page, _("Arrangement") );
    notebook->AddPage( look_feel_page, _("Look && Feel") );

    top_sizer->Add( notebook, 0, wxEXPAND | wxALL, 10 );

    wxBoxSizer *button_sizer = new wxBoxSizer( wxHORIZONTAL );
    button_sizer->Add( new wxButton(this, DIALOG_OK,      _("OK"),      wxDefaultPosition), 0, wxALL, 10 );
    SetDefaultItem(FindWindow(DIALOG_OK));
    button_sizer->Add( new wxButton(this, DIALOG_APPLY,   _("Apply"),   wxDefaultPosition), 0, wxALL, 10 );
    button_sizer->Add( new wxButton(this, DIALOG_RESTORE, _("Restore"), wxDefaultPosition), 0, wxALL, 10 );
    button_sizer->Add( new wxButton(this, DIALOG_CANCEL,  _("Cancel"),  wxDefaultPosition), 0, wxALL, 10 );
#ifndef __WXMSW__
    button_sizer->Add(new wxContextHelpButton(this), 0, wxALIGN_CENTER | wxALL, 10);
#endif
    top_sizer->Add( button_sizer );

    FindWindow(DIALOG_OK)->SetHelpText(_("Accepts the changes made in this dialog and returns to the game."));
    FindWindow(DIALOG_APPLY)->SetHelpText(_("Accepts the changes made in this dialog and leaves the dialog opened."));
    FindWindow(DIALOG_RESTORE)->SetHelpText(_("Reverts the changes made in this dialog (since the last 'Apply') and leaves it opened."));
    FindWindow(DIALOG_CANCEL)->SetHelpText(_("Closes the dialog without accepting the changes."));

    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer(top_sizer);
    top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  void Settings_Dialog::on_ok     ( wxCommandEvent& event )
  {
    on_apply( event );

    Show(false);
  }

  void Settings_Dialog::on_apply  ( wxCommandEvent& )
  {
    display_page->apply();
    look_feel_page->apply();

    gui_manager.game_settings = game_settings;
    gui_manager.save_settings();
    gui_manager.calc_dimensions();
    gui_manager.show_user_information(true,true /*do_refresh*/);
  }

  void Settings_Dialog::on_restore( wxCommandEvent& )
  {
    display_page->restore_settings();
    look_feel_page->restore_settings();
  }

  void Settings_Dialog::on_cancel ( wxCommandEvent& )
  {
    Show(false);
  }

  BEGIN_EVENT_TABLE(Settings_Dialog, wxDialog)			
    EVT_BUTTON(DIALOG_OK,   	   Settings_Dialog::on_ok)   	
    EVT_BUTTON(DIALOG_APPLY,	   Settings_Dialog::on_apply)	
    EVT_BUTTON(DIALOG_RESTORE,	   Settings_Dialog::on_restore)	
    EVT_BUTTON(DIALOG_CANCEL,	   Settings_Dialog::on_cancel)	//**/
  END_EVENT_TABLE()						//**/

  // ============================================================================
  // Network_Clients_Dialog
  // ============================================================================

  Network_Clients_Dialog::Network_Clients_Dialog( wxWindow *parent, 
						  Basic_Network_Server &network_server )
    : wxDialog(),
      network_server(network_server), destroyed(false)
  {
    // create the dialog
    long style = wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER;
#ifdef __WXMSW__ // this has to be done before the dialog is created, ...
    style |= wxDIALOG_EX_CONTEXTHELP;
#endif // ... hence the two-step construction (Create() below) is necessary.
    wxDialog::Create(parent,-1,wxString(_("Network clients")),wxPoint(0,0), wxDefaultSize, style );

    // create the child controls
    client_list = new wxListBox( this, LISTBOX_DCLICK, wxDefaultPosition, 
				 wxDefaultSize, 0, 0, wxLB_SINGLE );
    allow_connect = new wxCheckBox( this, DIALOG_ALLOW_CONNECT, _("allow new connections") );
    allow_connect->SetValue(true);  // default value is true

    // self register
    network_server.set_connection_handler(this);

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    top_sizer->Add(client_list, 1, wxALL | wxEXPAND, 10);
    top_sizer->Add(allow_connect, 0, wxLEFT | wxRIGHT | wxALIGN_CENTER_HORIZONTAL, 10);
    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(button_sizer, 0, wxALL | wxALIGN_CENTER_HORIZONTAL, 0);

    button_sizer->Add( new wxButton( this, DIALOG_DISCONNECT, _("Disconnect All") ), 0, wxALL, 10 );

    // set help texts
#ifndef __WXMSW__
    button_sizer->Add(new wxContextHelpButton(this), 0, wxALIGN_CENTER | wxALL, 10);
#endif
    client_list->SetHelpText(_("Lists all connected client computers. Double-click one of the clients to throw him out of the game."));
    FindWindow(DIALOG_DISCONNECT)->SetHelpText(_("Disconnect all client computers from the game."));
    
    // set sizer
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
    SetSize(wxSize(200,160));
    //top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    //top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  Network_Clients_Dialog::~Network_Clients_Dialog()
  {
    // should already be unregistered
  }

  void Network_Clients_Dialog::new_connection( std::string name, 
					       Basic_Network_Server::Connection_Id conn_id )
  {
    client_data[reinterpret_cast<void*>(conn_id)] = conn_id;
    client_item[conn_id] = client_list->GetCount();
    client_list->Append( str_to_wxstr(name), reinterpret_cast<void*>(conn_id) );
  }

  void Network_Clients_Dialog::closed_connection( Basic_Network_Server::Connection_Id conn_id )
  {
    int item = client_item[conn_id];
    client_list->Delete( item );
    // decrement higher item numbers
    std::map<Basic_Network_Server::Connection_Id,int>::iterator it;
    for( it = client_item.begin(); it != client_item.end(); ++it )
    {
      int &check_item = it->second;
      if( check_item > item ) 
	--check_item;
    }
  }

  void Network_Clients_Dialog::destroy()
  {
    if( destroyed ) return;
    destroyed=true;
    // self unregister
    network_server.set_connection_handler(0);

    this->Destroy();
  }

  void Network_Clients_Dialog::on_disconnect( wxCommandEvent& WXUNUSED(event) )
  {
    if( destroyed ) return;
    network_server.close_connections();
  }

  void Network_Clients_Dialog::on_dclick( wxCommandEvent& event )
  {
    if( destroyed ) return;
    Basic_Network_Server::Connection_Id conn_id = client_data[event.GetClientData()];
    wxString hostname = event.GetString();
    if( network_server.may_disconnect_id( conn_id ) )
    {
      wxString msg;
      msg.Printf( _("Do you really want to disconnect %s?"), hostname.c_str() );
      if( wxMessageDialog( this, msg, _("Disconnect?"), 
			   wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
      {
	network_server.disconnect_id( conn_id );
      }
    }
    else
    {
      wxString msg;
      msg.Printf( _("Can't disconnect host %s yet!"), hostname.c_str() );
      wxMessageDialog( this, msg, _("Can't disconnect!"), wxOK | wxICON_ERROR ).ShowModal();
    }
  }

  void Network_Clients_Dialog::on_allow_connect( wxCommandEvent& event )
  {
    if( event.IsChecked() )
    {
      network_server.allow_connect(true);
    }
    else
    {
      network_server.allow_connect(false);
    }
  }

  BEGIN_EVENT_TABLE(Network_Clients_Dialog, wxDialog)				//**/
    EVT_BUTTON(DIALOG_DISCONNECT, Network_Clients_Dialog::on_disconnect)	//**/
    EVT_LISTBOX_DCLICK(LISTBOX_DCLICK, Network_Clients_Dialog::on_dclick)	//**/
    EVT_CHECKBOX(DIALOG_ALLOW_CONNECT, Network_Clients_Dialog::on_allow_connect)//**/
  END_EVENT_TABLE()								//**/

  // ============================================================================
  // Network_Connection_Dialog
  // ============================================================================

  Network_Connection_Dialog::Network_Connection_Dialog( wxWindow *parent )
    : wxDialog()
  {      
    // create the dialog
#ifdef __WXMSW__ // this has to be done before the dialog is created, ...
    SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
#endif // ... hence the two-step construction (Create() below) is necessary.
    wxDialog::Create(parent,-1,wxString(_("Network Game")));

    // create the child controls
    server = new wxRadioButton(this, -1, _("Setup server") );
    client = new wxRadioButton(this, -1, _("Connect to server") );
    hostname = new wxTextCtrl(this, -1, wxT("localhost"));
    port = new wxSpinCtrl(this, -1, wxT("6211"));

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    server->SetValue(true);
    top_sizer->Add( server, 0, wxALL, 10 );
    top_sizer->Add( client, 0, wxALL, 10 );
    top_sizer->Add( hostname, 0, wxALL, 10 );
    top_sizer->Add( port, 0, wxALL, 10 );
    port->SetRange(0,65535);

    wxBoxSizer *button_sizer  = new wxBoxSizer( wxHORIZONTAL );
    wxButton *ok_button = new wxButton(this, DIALOG_OK, _("OK"), wxDefaultPosition);
	SetDefaultItem(FindWindow(DIALOG_OK));
    ok_button->SetFocus();
    button_sizer->Add( ok_button , 0, wxALL, 10 );
    button_sizer->Add( new wxButton(this, DIALOG_CANCEL, _("Cancel"), wxDefaultPosition), 
		       0, wxALL, 10 );
    top_sizer->Add( button_sizer, 0, wxALIGN_CENTER );

	// set help texts
#ifndef __WXMSW__
    button_sizer->Add(new wxContextHelpButton(this), 0, wxALIGN_CENTER | wxALL, 10);
#endif
	server->SetHelpText(_("Select this option to setup a new server, so that other players on the network can join your game."));
	client->SetHelpText(_("Select this option to connect to someone else's server and join his game."));
	hostname->SetHelpText(_("Enter the IP address or hostname of the computer you want to connect to. (If you are setting up a server, this field has no meaning.)"));
	port->SetHelpText(_("Enter the port number of your (or the other person's) server. You can usually leave this unchanged."));
	FindWindow(DIALOG_OK)->SetHelpText(_("Start up the network connection using the given options."));
    
    // set sizer
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
    top_sizer->Fit( this );            // set size to minimum size as calculated by the sizer
    top_sizer->SetSizeHints( this );   // set size hints to honour mininum size     
  }

  BEGIN_EVENT_TABLE(Network_Connection_Dialog, wxDialog)		//**/
  //  EVT_BUTTON(DIALOG_OK,     Network_Connection_Dialog::OnOK)		//**/
  //  EVT_BUTTON(DIALOG_CANCEL, Network_Connection_Dialog::OnCancel)	//**/
  END_EVENT_TABLE()							//**/

  // ============================================================================
  // Game_Variants_Panel
  // ============================================================================

  Game_Variants_Panel::Game_Variants_Panel( wxWindow *parent )
    : wxPanel(parent,-1), current_tree_version(1)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    tree = new wxTreeCtrl( this,-1,wxDefaultPosition,wxDefaultSize,
			   wxTR_HAS_BUTTONS | wxTR_SINGLE | wxTR_LINES_AT_ROOT );
    selected_variant_id = tree->GetRootItem();
    top_sizer->Add(tree, 1, wxEXPAND, 0);

    // set sizer
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer
  }

  void recursive_add_variant( wxTreeCtrl *tree, 
			      wxTreeItemId cur_item, Variant_Tree_Item_Data *cur_data,
			      const Variant_Tree *var_tree, const Variant *add_variant, 
			      const Variant *sel_variant, wxTreeItemId *sel_variant_id,
			      Coordinate_Translator *coordinate_translator,
			      unsigned tree_version )
  {
    wxTreeItemId new_item;
    Variant_Tree_Item_Data *new_data;
    // check whether variant already exists
    if( does_include(cur_data->varid_treeid_map,add_variant->id) )
    {
      new_item = cur_data->varid_treeid_map[add_variant->id];
      new_data = static_cast<Variant_Tree_Item_Data*>(tree->GetItemData(new_item));
      new_data->tree_version = tree_version;
    }
    else
    {
      // create new tree item for variant
      Standard_Move_Translator move_translator(coordinate_translator);
      std::string name = move_translator.encode(add_variant->move_sequence);
      new_data = new Variant_Tree_Item_Data(tree_version,var_tree->get_variant_id_path(add_variant));
      new_item = tree->AppendItem(cur_item,str_to_wxstr(name),-1,-1,new_data);
      cur_data->varid_treeid_map[add_variant->id] = new_item;
    }
    // check whether variant is selected
    if( sel_variant == add_variant )
      *sel_variant_id = new_item;
    // add child variants
    std::list<Variant*>::const_iterator it;
    for( it = add_variant->variants.begin(); it != add_variant->variants.end(); ++it )
      {
	Variant *new_variant = *it;
	recursive_add_variant( tree, new_item, new_data, var_tree, new_variant, sel_variant, 
			       sel_variant_id, coordinate_translator, tree_version );
      }
  }

  void recursive_find_old_variants( wxTreeCtrl *tree, wxTreeItemId cur_item, unsigned tree_version,
				    std::list<wxTreeItemId> *delete_list )
  {
    wxTreeItemIdValue cookie;
    // depth first search for all items in tree
    wxTreeItemId id = tree->GetFirstChild(cur_item,cookie);
    while( id.IsOk() )
    {
      recursive_find_old_variants( tree, id, tree_version, delete_list );
      id = tree->GetNextChild(cur_item,cookie);
    }
    // work on items in depth first order
    Variant_Tree_Item_Data *cur_data 
      = static_cast<Variant_Tree_Item_Data*>(tree->GetItemData(cur_item));
    if( cur_data->tree_version != tree_version )
    {
      delete_list->push_back(cur_item);
    }
  }

  void Game_Variants_Panel::reset()
  {
    current_tree_version++;
    tree->DeleteAllItems();
    selected_variant_id = tree->GetRootItem(); // selected_variant_id.IsOk() should be false now
  }

  void Game_Variants_Panel::show_variant_tree(const Variant_Tree &new_variant_tree,
					      Coordinate_Translator *coordinate_translator)
  {
    if(!IsShown())
      return;

    current_tree_version++;
    variant_tree = new_variant_tree;

    // unselect current selected variant
    if( selected_variant_id.IsOk() )
      tree->SetItemBackgroundColour(selected_variant_id,wxNullColour);

    // get or create root
    Variant_Tree_Item_Data *root_data = 0;
    wxTreeItemId root = tree->GetRootItem();
    selected_variant_id = root; // default value if current_variant wasn't found
    if( root.IsOk() )
    {
      root_data = static_cast<Variant_Tree_Item_Data*>(tree->GetItemData(root));
      root_data->tree_version = current_tree_version;
    }
    else
    {
      root_data = new Variant_Tree_Item_Data(current_tree_version);
      root = tree->AddRoot(_("start"),-1,-1,root_data);
    }
    // add leaves of root (each recursively)
    const Variant *add_variant = variant_tree.get_root_variant();
    std::list<Variant*>::const_iterator it;
    for( it = add_variant->variants.begin(); it != add_variant->variants.end(); ++it )
      {
	Variant *new_variant = *it;
	recursive_add_variant( tree, root, root_data, &variant_tree, new_variant, 
			       variant_tree.get_current_variant(), &selected_variant_id,
			       coordinate_translator, current_tree_version );
      }

    if( selected_variant_id.IsOk() )
    {
      // expand current variant excluding current variant
      wxTreeItemId cur_item = root;
      Variant_Tree_Item_Data *cur_data = root_data;
      Variant_Tree_Item_Data *sel_data 
	= static_cast<Variant_Tree_Item_Data*>(tree->GetItemData(selected_variant_id));
      std::list<unsigned>::const_iterator it2;
      for( it2 = sel_data->variant_id_path.begin(); it2 != sel_data->variant_id_path.end(); ++it2 )
      {
	unsigned id = *it2;
	tree->Expand(cur_item); 
	cur_item = cur_data->varid_treeid_map[id];
	cur_data = static_cast<Variant_Tree_Item_Data*>(tree->GetItemData(cur_item));
      }
      // mark current variant
      tree->SetItemBackgroundColour(selected_variant_id,*wxRED);
      //tree->SelectItem(selected_variant_id); //currently selection is always vetoed
      tree->ScrollTo(selected_variant_id);
    }
    // delete outdated tree items
    std::list<wxTreeItemId> delete_list;
    recursive_find_old_variants( tree, root, current_tree_version, &delete_list );
    for( std::list<wxTreeItemId>::iterator it=delete_list.begin(); it!=delete_list.end(); ++it )
    {
      if( selected_variant_id == *it )
	selected_variant_id = tree->GetRootItem();
      tree->Delete(*it);
    }
  }

  void Game_Variants_Panel::on_activated( wxTreeEvent& event )
  {
    Variant_Tree_Item_Data *item_data 
      = static_cast<Variant_Tree_Item_Data*>(tree->GetItemData( event.GetItem() ));
    if( item_data != 0 )
      {
	if( allow_selection && notifiee )
	  {
	    notifiee->selected_variant( item_data->variant_id_path );
	  }
      }
    else
      {
	std::cerr << "Internal Error: clicked on TreeCtrlItem with invalid data associated" 
		  << std::endl;
      }
  }
  void Game_Variants_Panel::on_changing( wxTreeEvent& event )
  {
    event.Veto();
  }

  BEGIN_EVENT_TABLE(Game_Variants_Panel, wxPanel)			//**/
    EVT_TREE_ITEM_ACTIVATED(-1, Game_Variants_Panel::on_activated)	//**/
    EVT_TREE_SEL_CHANGING(-1, Game_Variants_Panel::on_changing)		//**/
  END_EVENT_TABLE()							//**/

  // ============================================================================
  // Game_Variants_Frame
  // ============================================================================

  Game_Variants_Frame::Game_Variants_Frame( wxWindow *parent )
    : wxFrame(parent,-1,_("Variants"),wxDefaultPosition,wxDefaultSize,
	      wxFRAME_FLOAT_ON_PARENT | wxDEFAULT_FRAME_STYLE)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    
    game_variants = new Game_Variants_Panel(this);
    top_sizer->Add(game_variants, 1, wxEXPAND, 0);

    // set sizer
    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer( top_sizer );      // actually set the sizer

    // initialize panels
    game_variants->Show(false);
  }

  void Game_Variants_Frame::show_frame()
  {
    game_variants->Show(true);
    Show(true);
  }

  void Game_Variants_Frame::on_close( wxCloseEvent &event )
  {
    Show(false);
    game_variants->Show(false);
    event.Veto();		// don't destroy frame
  }

  BEGIN_EVENT_TABLE(Game_Variants_Frame, wxFrame)			//**/
  EVT_CLOSE(Game_Variants_Frame::on_close)		//**/
  END_EVENT_TABLE()							//**/
}
