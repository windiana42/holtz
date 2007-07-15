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

#include "wxmain.hpp"
#include "util.hpp"

#include <wx/cshelp.h>
#include <wx/dir.h>
#include <wx/fontdlg.h>
#include <fstream>

namespace dvonn
{

  // ----------------------------------------------------------------------------
  // Setup_Manager_Page
  // ----------------------------------------------------------------------------

  Setup_Manager_Page::Setup_Manager_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true), changed_setup_manager(false)
  {
    alone          = new wxRadioButton( this, -1, _("Play alone") );
    network_server = new wxRadioButton( this, -1, _("Setup network server") );
    network_client = new wxRadioButton( this, -1, _("Connect to server") );
    don_t_change   = new wxRadioButton( this, -1, _("Keep last game connection") );

    server_port = new wxSpinCtrl( this, -1 );
    client_port = new wxSpinCtrl( this, -1 );
    server_port->SetRange( 1, 65535 );
    server_port->SetValue( DEFAULT_PORT );
    client_port->SetRange( 1, 65535 );
    client_port->SetValue( DEFAULT_PORT );
    hostname = new wxTextCtrl( this, -1 );
    
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    wxFlexGridSizer *grid_sizer = new wxFlexGridSizer( 5 );
    grid_sizer->Add( alone, 0, wxALL, 10 );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( network_server, 0, wxALL, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Port")), 0, wxALL, 10 );
    grid_sizer->Add( server_port, 0, wxALL, 10 );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( new wxPanel( this, -1 ) );
    grid_sizer->Add( network_client, 0, wxALL, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Port")), 0, wxALL, 10 );
    grid_sizer->Add( client_port, 0, wxALL, 10 );
    grid_sizer->Add( new wxStaticText(this,-1,_("Hostname")), 0, wxALL, 10 );
    grid_sizer->Add( hostname, 0, wxALL, 10 );
    grid_sizer->Add( don_t_change, 0, wxALL, 10 );

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
    return &game_dialog.board_page;
  }

  bool Setup_Manager_Page::TransferDataFromWindow()
  {
    if( changes )
    {
      if( !don_t_change->GetValue() )
      {
	if( alone->GetValue() )
	{
	  if( changed_setup_manager ) // is setup_manager created in this dialog run
	    delete game_dialog.game_setup_manager;
	  
	  game_dialog.game_setup_manager = new Standalone_Game_Setup_Manager( game_dialog.game_manager );
	  changed_setup_manager = true;
	}
	else if( network_server->GetValue() )
	{
	  wxIPV4address port;
	  if( port.Service( server_port->GetValue() ) )
	  {
	    Network_Manager *network_manager = new Network_Manager( game_dialog.game_manager, 
								    game_dialog.gui_manager );
	    
	    if( network_manager->setup_server(port) )
	    {
	      if( changed_setup_manager ) // is setup_manager created in this dialog run
		delete game_dialog.game_setup_manager;
	      
	      game_dialog.game_setup_manager = network_manager;
	      changed_setup_manager = true;
	    }
	    else
	    {
	      delete network_manager;
	      wxMessageBox(_("Can't listen port"), _("Network Message"), wxOK | wxICON_ERROR, this);
	      return false;
	    }
	  }
	  else
	  {
	    wxMessageBox(_("Illegal port"), _("Network Message"), wxOK | wxICON_ERROR, this);
	    return false;
	  }
	}
	else if( network_client->GetValue() )
	{
	  wxIPV4address host;
	  if( host.Hostname(hostname->GetValue()) && 
	      host.Service(client_port->GetValue()) )
	  {	  
	    Network_Manager *network_manager = new Network_Manager( game_dialog.game_manager, 
								    game_dialog.gui_manager );
	    if( network_manager->setup_client(host) )
	    {
	      if( changed_setup_manager ) // is setup_manager created in this dialog run
		delete game_dialog.game_setup_manager;
	      
	      game_dialog.game_setup_manager = network_manager;
	      changed_setup_manager = true;
	    }
	    else
	    {
	      wxMessageBox(_("Connection to Server failed"), _("Network Message"), wxOK | wxICON_ERROR,this);
	      return false;
	    }
	  }
	  else
	  {
	    wxMessageBox(_("Illegal hostname"), _("Network Message"), wxOK | wxICON_ERROR, this);
	    return false;
	  }
	}
	else
	  return false;		// nothing checked ?!?
	
	don_t_change->SetValue( true ); // when user goes back: default = don't change

	// restore data from game_setup_manager
      }
      game_dialog.game_setup_manager->set_display_handler( &game_dialog );
      game_dialog.get_data_from_setup_manager();
      changes = false;
    }
    return true;
  }

  void Setup_Manager_Page::restore()
  {
    changes = true;		// take first setting as change
    if( game_dialog.game_setup_manager )	// if there is a setup manager
    {
      don_t_change->SetValue( true );		// default: don't change it
      don_t_change->Enable( true );		// disable don't change
    }
    else
    {
      alone->SetValue( true );			// default: play alone
      don_t_change->Enable( false );		// disable don't change
    }
  }

  BEGIN_EVENT_TABLE(Setup_Manager_Page, wxWizardPage)				
    // EVT_TEXT_ENTER(DIALOG_SETUP_MANAGER,	Setup_Manager_Page::on_player_name)	//**/
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

    /*
    wxString new_game_choices[4];
    new_game_choices[0] = wxString(_("Standard Rules"));
    new_game_choices[1] = wxString(_("Tournament Rules"));
    new_game_choices[2] = wxString(_("Custom"));
    new_game_choices[3] = wxString(_("Rules of last game"));
    new_game_choice = 0;
    new_game_choice = new wxRadioBox( this, -1, _("Rules for new game"), wxDefaultPosition,
    				      wxDefaultSize, 4, new_game_choices, 2, wxRA_SPECIFY_COLS );
    */

    wxString continue_game_choices[3];
    continue_game_choices[0] = wxString(_("Continue last game"));
    continue_game_choices[1] = wxString(_("Load PBM game"));
    continue_game_choices[2] = wxString(_("Load Littlegolem game"));
    continue_game_choice = new wxRadioBox( this, -1, _("Which game to continue"), wxDefaultPosition,
					   wxDefaultSize, 3, continue_game_choices, 1, wxRA_SPECIFY_COLS );

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    top_sizer->Add( new_game, 0, wxALL, 10 );
    //top_sizer->Add( new_game_choice, 0, wxCENTER | wxALL, 10 );
    top_sizer->Add( continue_game, 0, wxALL, 10 );
    top_sizer->Add( continue_game_choice, 0, wxCENTER | wxALL, 10 );
    top_sizer->Add( don_t_change, 0, wxALL, 10 );

    // set sizer
    SetAutoLayout( true );	// tell dialog to use sizer
    SetSizer( top_sizer );	// actually set the sizer
  }

  wxWizardPage *Board_Page::GetPrev() const
  {
    return &game_dialog.setup_manager_page;
  }

  wxWizardPage *Board_Page::GetNext() const
  {
    //if( new_game->GetValue() && (new_game_choice->GetSelection() == 2) )
    //  return &game_dialog.custom_board_page;
    //else 
    if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 1) )
      return &game_dialog.load_pbm_board_page;
    else if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 2) )
      return &game_dialog.load_lg_board_page;
    else
      return &game_dialog.player_page;
  }

  bool Board_Page::TransferDataFromWindow()
  {
    if( changes )
    {
      if( new_game->GetValue() )
      {
	game_dialog.game = Game( Standard_Ruleset() );
	/*
	switch( new_game_choice->GetSelection() )
	{
	  case 0: game_dialog.game = Game( Standard_Ruleset() ); break;
	    //case 1: game_dialog.game = Game( Tournament_Ruleset() ); break;
	  case 2: / *game will be set up in next page* / break;
	  case 3: game_dialog.game = Game( *game_dialog.game_manager.get_game().ruleset ); 
	}
	if( new_game_choice->GetSelection() != 2 )
	{
	*/
	  if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	      Game_Setup_Manager::deny )
	  {
	    game_dialog.board_change_denied();
	  }
	//}
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
	  game_dialog.game.ruleset->min_players = game_dialog.players.size();
	  game_dialog.game.ruleset->max_players = game_dialog.players.size();
	  if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	      Game_Setup_Manager::deny )
	  {
	    game_dialog.board_change_denied();
	  }
	}
      }
      game_dialog.custom_board_page.restore();
      game_dialog.load_pbm_board_page.restore();
      game_dialog.load_lg_board_page.restore();
      game_dialog.player_page.restore();

      changes = false;
    }
    return true;
  }

  void Board_Page::restore()	// display stored game state
  {
    changes = true;		// take first setting as change
    //!!! ask game_setup_manager which option is default !!! Client: don_t_change
    new_game->SetValue(true);
    /*
    switch( game_dialog.game.ruleset->get_type() )
    {
      case Ruleset::standard:   new_game_choice->SetSelection(0); break;
	//      case Ruleset::tournament: new_game_choice->SetSelection(1); break;
      case Ruleset::custom:     new_game_choice->SetSelection(2); break;
    }
    */
  }

  wxWizardPage *Board_Page::get_last_board_page() const
  {
    /*
    if( new_game->GetValue() && (new_game_choice->GetSelection() == 2) )
      return &game_dialog.custom_board_page;
    else 
    */
    if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 1) )
      return &game_dialog.load_pbm_board_page;
    else if( continue_game->GetValue() && (continue_game_choice->GetSelection() == 2) )
      return &game_dialog.load_lg_board_page;
    else
      return &game_dialog.board_page;
  }

  BEGIN_EVENT_TABLE(Board_Page, wxWizardPage)				
    // EVT_TEXT_ENTER(DIALOG_BOARD,	Board_Page::on_player_name)	//**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Custom_Board_Setup_Panel
  // ----------------------------------------------------------------------------

  Custom_Board_Setup_Panel::Custom_Board_Setup_Panel( wxWindow *parent, Game_Dialog &game_dialog )
    : wxPanel( parent, -1 ), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxString board_choices[5];
    board_choices[0] = wxString(_("37 Rings (default)"));
    board_choices[1] = wxString(_("40 Rings"));
    board_choices[2] = wxString(_("44 Rings"));
    board_choices[3] = wxString(_("48 Rings"));
    board_choices[4] = wxString(_("61 Rings"));
    //board_choices[5] = wxString(_("custom"));
    board_choice = new wxRadioBox( this, -1, _("Choose Board"), wxDefaultPosition,
				   wxDefaultSize, 
				   5, board_choices, 2, wxRA_SPECIFY_ROWS );
    top_sizer->Add( board_choice, 0, wxALIGN_CENTER | wxALL, 10  );

    wxBoxSizer *win_condition_sizer = new wxBoxSizer( wxHORIZONTAL );
    wxString win_choices[3];
    win_choices[0] = wxString(_("Standard"));
    win_choices[1] = wxString(_("Tournament"));
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
    stones_choices[0] = wxString(_("Standard"));
    stones_choices[1] = wxString(_("Tournament"));
    stones_choices[2] = wxString(_("Custom"));
    stones_choice = new wxRadioBox( this, DIALOG_STONES_CHOICE, _("Common marbles"), wxDefaultPosition,
				    wxDefaultSize, 
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

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  void Custom_Board_Setup_Panel::restore()
  {
    Ruleset *ruleset = game_dialog.game.ruleset;
    wxCommandEvent dummy;
    switch( ruleset->board.board_type )
    {
      case Board::standard: board_choice->SetSelection(0); break;
	/*
      case Board::s40_rings: board_choice->SetSelection(1); break;
      case Board::s44_rings: board_choice->SetSelection(2); break;
      case Board::s48_rings: board_choice->SetSelection(3); break;
      case Board::s61_rings: board_choice->SetSelection(4); break;
	*/
      case Board::custom:    /* board_choice->SetSelection(5); // shouldn't happen */ break;
    }
    switch( ruleset->win_condition->get_type() )
    {
      case Win_Condition::standard:    win_choice->SetSelection(0); on_change_win(dummy); break;
	//      case Win_Condition::tournament:  win_choice->SetSelection(1); on_change_win(dummy); break;
	/*
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
	*/
      case Win_Condition::full_custom: /*win_choice->SetSelection(3); // shouldn't happen */ break;
    }
    switch( ruleset->common_stones.get_type() )
    {
      case Common_Stones::standard:   stones_choice->SetSelection(0); on_change_stones(dummy); break;
	//      case Common_Stones::tournament: stones_choice->SetSelection(1); on_change_stones(dummy); break;
      case Common_Stones::custom:
      {
	stones_choice->SetSelection(2);
	stones_white->SetValue( ruleset->common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( ruleset->common_stones.stone_count[ Stones::red_stone  ] );
	stones_black->SetValue( ruleset->common_stones.stone_count[ Stones::black_stone ] );
      }
      break;
    }
    changes = true;
  }

  Game Custom_Board_Setup_Panel::get_board()
  {
    Board *board;
    Common_Stones *common_stones;
    Win_Condition *win_condition;

    switch( board_choice->GetSelection() )
    {
      case -1:
      case 0:
	board = new Board( (const int*) standard_board, 
			   sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
			   sizeof(standard_board)    / sizeof(standard_board[0]),
			   Board::standard );
	break;
	/*
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
	*/
      default:
	assert(false);
	break;
    }
    switch( win_choice->GetSelection() )
    {
      case 0:
	win_condition = new Standard_Win_Condition();
	break;
      case 1:
	//	win_condition = new Tournament_Win_Condition();
	break;
      case 2:
	/*
	win_condition = new Generic_Win_Condition( win_white->GetValue(), 
						   win_grey ->GetValue(), 
						   win_black->GetValue(), 
						   win_all  ->GetValue() );
	*/
	break;
      default:
	assert(false);
	break;
    }
    switch( stones_choice->GetSelection() )
    {
      case 0:
	common_stones = new Standard_Common_Stones();
	break;
      case 1:
	//	common_stones = new Tournament_Common_Stones();
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
  }

  void Custom_Board_Setup_Panel::on_change_win  ( wxCommandEvent& event )
  {
    changes = true;
    switch( win_choice->GetSelection() )
    {
      case 0:
      {
	Standard_Win_Condition standard;
	/*
	win_white->SetValue( standard.num_white );
	win_grey ->SetValue( standard.num_grey );
	win_black->SetValue( standard.num_black );
	win_all  ->SetValue( standard.num_all );
	*/
      }
      break;
      case 1:
      {
	/*
	Tournament_Win_Condition tournament;
	win_white->SetValue( tournament.num_white );
	win_grey ->SetValue( tournament.num_grey );
	win_black->SetValue( tournament.num_black );
	win_all  ->SetValue( tournament.num_all );
	*/
      }
      break;
      case 2:
	break;
    }
  }

  void Custom_Board_Setup_Panel::on_spin_win ( wxSpinEvent& event )
  {
    changes = true;
    win_choice->SetSelection(2);
  }

  void Custom_Board_Setup_Panel::on_change_stones  ( wxCommandEvent& event )
  {
    changes = true;
    switch( stones_choice->GetSelection() )
    {
      case 0:
      {
	Standard_Common_Stones common_stones;
	stones_white->SetValue( common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( common_stones.stone_count[ Stones::red_stone  ] );
	stones_black->SetValue( common_stones.stone_count[ Stones::black_stone ] );
      }
      break;
      case 1:
      {
	/*
	Tournament_Common_Stones common_stones;
	stones_white->SetValue( common_stones.stone_count[ Stones::white_stone ] );
	stones_grey ->SetValue( common_stones.stone_count[ Stones::red_stone  ] );
	stones_black->SetValue( common_stones.stone_count[ Stones::black_stone ] );
	*/
      }
      break;
      case 2:
	break;
    }
  }
  void Custom_Board_Setup_Panel::on_spin_stones ( wxSpinEvent& WXUNUSED(event) )
  {
    changes = true;
    stones_choice->SetSelection(2);
  }

  Custom_Board_Setup_Panel::~Custom_Board_Setup_Panel()
  {
  }
  
  BEGIN_EVENT_TABLE(Custom_Board_Setup_Panel, wxPanel)			
    EVT_RADIOBOX(DIALOG_WIN_CHOICE,  	Custom_Board_Setup_Panel::on_change_win)
    EVT_SPINCTRL(DIALOG_WIN_SPIN,	Custom_Board_Setup_Panel::on_spin_win)	
    EVT_RADIOBOX(DIALOG_STONES_CHOICE,	Custom_Board_Setup_Panel::on_change_stones)	
    EVT_SPINCTRL(DIALOG_STONES_SPIN,	Custom_Board_Setup_Panel::on_spin_stones)	//**/
  END_EVENT_TABLE()									//**/

  // ----------------------------------------------------------------------------
  // Custom_Board_Page
  // ----------------------------------------------------------------------------

  Custom_Board_Page::Custom_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    /*
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    custom_board_panel = new Custom_Board_Setup_Panel( this, game_dialog );
    top_sizer->Add( custom_board_panel );

    SetAutoLayout( true );
    SetSizer( top_sizer );
    */
    wxNotebook *notebook = new wxNotebook( this, -1 );
    wxNotebookSizer *sizer = new wxNotebookSizer( notebook );

    custom_board_panel = new Custom_Board_Setup_Panel( notebook, game_dialog );
    notebook->AddPage( custom_board_panel, _("Ruleset") );

    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer(sizer);
  }

  wxWizardPage *Custom_Board_Page::GetPrev() const
  {
    return &game_dialog.board_page;
  }

  wxWizardPage *Custom_Board_Page::GetNext() const
  {
    return &game_dialog.player_page;
  }

  bool Custom_Board_Page::TransferDataFromWindow()
  {
    if( changes )
    {
      game_dialog.game = custom_board_panel->get_board();
      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }
      game_dialog.player_page.restore();

      changes = false;
    }
    return true;
  }

  void Custom_Board_Page::restore()		// display stored game state
  {
    changes = true;		// take first setting as change
    custom_board_panel->restore();
  }

  BEGIN_EVENT_TABLE(Custom_Board_Page, wxWizardPage)				
    // EVT_TEXT_ENTER(DIALOG_CUSTOM_BOARD,	Custom_Board_Page::on_player_name)	//**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Load_PBM_Board_Page
  // ----------------------------------------------------------------------------

  Load_PBM_Board_Page::Load_PBM_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *pbm_sizer = new wxBoxSizer( wxHORIZONTAL );
    pbm_sizer->Add( new wxStaticText(this,-1,_("PBM Directory") ), 0, wxALL, 10 );
    pbm_directory = new wxTextCtrl( this, DIALOG_CHANGE_PBM_DIRECTORY, wxT("") );
    pbm_sizer->Add( pbm_directory, 1, wxALL, 10 );
    pbm_sizer->Add( new wxButton( this, DIALOG_CHOOSE_PBM_DIRECTORY,_("Choose...") ), 0, wxALL, 10 );
    top_sizer->Add( pbm_sizer, 0, wxEXPAND | wxALL, 10 );

    pbm_game_list = new wxListBox( this,-1,wxDefaultPosition, wxSize(400,100), 0, 0, wxLB_SINGLE );
    top_sizer->Add( pbm_game_list, 1, wxEXPAND | wxALL, 20 );

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  wxWizardPage *Load_PBM_Board_Page::GetPrev() const
  {
    return &game_dialog.board_page;
  }

  wxWizardPage *Load_PBM_Board_Page::GetNext() const
  {
    return &game_dialog.player_page;
  }

  bool Load_PBM_Board_Page::TransferDataFromWindow()
  {
    if( changes )
    {
      int index = pbm_game_list->GetSelection();
      int board_num = int( pbm_game_list->GetClientData(index) );

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
#endif
      }while( (max_move >= 0) && (num_moves > 0) );

      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }
      game_dialog.player_page.restore();

      changes = false;
    }
    return true;
  }

  void Load_PBM_Board_Page::restore()		// display stored game state
  {
    changes = true;		// take first setting as change

    if( valid_pbm_directory != wxT("") )
      scan_pbm_directory( valid_pbm_directory );
  }

  bool Load_PBM_Board_Page::scan_pbm_directory( wxString directory )
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
      
      if( master_content.from == 0 )	// display only boards that are specified from the first move on
      {
	wxString board_str;
  	board_str << _("Board ") << master_content.id << wxT(" ") << str_to_wxstr(master_content.player1) << wxT(":")
		  << str_to_wxstr(master_content.player2) << wxT(" (") << master_content.to << wxT(" ") << _("moves") << wxT(")");
	/*
	board_str.Printf( _("Board %d %s : %s (%d moves)"), master_content.id, 
			  str_to_wxstr(master_content.player1).c_str(), str_to_wxstr(master_content.player2).c_str(), 
			  master_content.to );
	*/
	pbm_game_list->Append( board_str, (void*) (master_content.id) );
      }
    }

    return true;
  }

  void Load_PBM_Board_Page::on_choose_pbm_directory( wxCommandEvent& WXUNUSED(event) )
  {
    wxString directory = valid_pbm_directory;
    if( directory == wxT("") ) 
      directory = wxGetCwd();

    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory );
    if( dialog->ShowModal() == wxID_OK )
    {
      changes = true;
      if( scan_pbm_directory( dialog->GetPath() ) )
      {
	valid_pbm_directory = dialog->GetPath();
	pbm_directory->SetValue( valid_pbm_directory );
      }
    }
  }

  void Load_PBM_Board_Page::on_change_pbm_directory( wxCommandEvent& WXUNUSED(event) )
  {
    changes = true;
    if( scan_pbm_directory( pbm_directory->GetValue() ) )
    {
      valid_pbm_directory = pbm_directory->GetValue();
    }
  }

  BEGIN_EVENT_TABLE(Load_PBM_Board_Page, wxWizardPage)				
    EVT_BUTTON(DIALOG_CHOOSE_PBM_DIRECTORY,	Load_PBM_Board_Page::on_choose_pbm_directory)		
    EVT_TEXT_ENTER(DIALOG_CHANGE_PBM_DIRECTORY,	Load_PBM_Board_Page::on_change_pbm_directory)//**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Load_LG_Board_Page
  // ----------------------------------------------------------------------------

  Load_LG_Board_Page::Load_LG_Board_Page( wxWizard *parent, Game_Dialog &game_dialog )
    : wxWizardPage(parent), game_dialog(game_dialog), changes(true)
  {
    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );

    wxBoxSizer *lgolem_sizer = new wxBoxSizer( wxHORIZONTAL );
    lgolem_sizer->Add( new wxStaticText(this,-1,_("Littlegolem Directory") ), 0, wxALL, 10 );
    lgolem_directory = new wxTextCtrl( this, DIALOG_CHANGE_LG_DIRECTORY, wxT("") );
    lgolem_sizer->Add( lgolem_directory, 1, wxALL, 10 );
    lgolem_sizer->Add( new wxButton( this, DIALOG_CHOOSE_LG_DIRECTORY, _("Choose...") ), 0, wxALL, 10 );
    top_sizer->Add( lgolem_sizer, 0, wxEXPAND | wxALL, 10 );

    lgolem_game_list = new wxListBox( this,-1,wxDefaultPosition, wxSize(400,100), 0, 0, wxLB_SINGLE );
    top_sizer->Add( lgolem_game_list, 1, wxEXPAND | wxALL, 20 );

    SetAutoLayout( true );
    SetSizer( top_sizer );
  }

  wxWizardPage *Load_LG_Board_Page::GetPrev() const
  {
    return &game_dialog.board_page;
  }

  wxWizardPage *Load_LG_Board_Page::GetNext() const
  {
    return &game_dialog.player_page;
  }

  bool Load_LG_Board_Page::TransferDataFromWindow()
  {
    if( changes )
    {
      int index = lgolem_game_list->GetSelection();

      // load board
      int cnt=0;
      std::list< std::pair<LG_Content,std::string> >::iterator i;
      for( i = lgolem_files.begin(); i != lgolem_files.end(); ++i )
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
	}
	cnt++;
      }

      if( game_dialog.game_setup_manager->ask_change_board( game_dialog.game ) == 
	  Game_Setup_Manager::deny )
      {
	game_dialog.board_change_denied();
      }
      game_dialog.player_page.restore();

      changes = false;
    }
    return true;
  }

  void Load_LG_Board_Page::restore()		// display stored game state
  {
    changes = true;		// take first setting as change

    if( valid_lgolem_directory != wxT("") )
      scan_lgolem_directory( valid_lgolem_directory );
  }

  bool Load_LG_Board_Page::scan_lgolem_directory( wxString directory )
  {
    wxDir dir(directory);

    if( !dir.IsOpened() ) return false;

    // scan files
    lgolem_files.clear();
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
	  lgolem_files.push_back( std::pair<LG_Content,std::string>( content, filename ) );
	}
      }

      ok = dir.GetNext( &wx_filename );
    }

    // setup list box
    lgolem_game_list->Clear();
    std::list< std::pair<LG_Content,std::string> >::iterator i;
    for( i = lgolem_files.begin(); i != lgolem_files.end(); ++i )
    {
      wxString board_str;
      board_str << /*_("Event ") << */str_to_wxstr(i->first.event) << wxT(" ") 
		<< str_to_wxstr(i->first.white_name) << wxT(":")
		<< str_to_wxstr(i->first.black_name) << wxT(" (") 
		<< i->first.moves << wxT(" ") << _("moves") << wxT(")");
	/*
	board_str.Printf( _("Board %d %s : %s (%d moves)"), master_content.id, 
			  str_to_wxstr(master_content.player1).c_str(), str_to_wxstr(master_content.player2).c_str(), 
			  master_content.to );
	*/
      lgolem_game_list->Append( board_str );
    }

    return true;
  }

  void Load_LG_Board_Page::on_choose_lgolem_directory( wxCommandEvent& WXUNUSED(event) )
  {
    wxString directory = valid_lgolem_directory;
    if( directory == wxT("") ) 
      directory = wxGetCwd();

    wxDirDialog *dialog = new wxDirDialog( this, _("Choose a directory"), directory );
    if( dialog->ShowModal() == wxID_OK )
    {
      changes = true;
      if( scan_lgolem_directory( dialog->GetPath() ) )
      {
	valid_lgolem_directory = dialog->GetPath();
	lgolem_directory->SetValue( valid_lgolem_directory );
      }
    }
  }

  void Load_LG_Board_Page::on_change_lgolem_directory( wxCommandEvent& WXUNUSED(event) )
  {
    changes = true;
    if( scan_lgolem_directory( lgolem_directory->GetValue() ) )
    {
      valid_lgolem_directory = lgolem_directory->GetValue();
    }
  }

  BEGIN_EVENT_TABLE(Load_LG_Board_Page, wxWizardPage)				
    EVT_BUTTON(DIALOG_CHOOSE_LG_DIRECTORY,	Load_LG_Board_Page::on_choose_lgolem_directory)
    EVT_TEXT_ENTER(DIALOG_CHANGE_LG_DIRECTORY,	Load_LG_Board_Page::on_change_lgolem_directory)//**/
  END_EVENT_TABLE()								//**/

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
    wxString help_choices[3], ruleset_choices[3];
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
    player_list_buttons_sizer->Add( new wxButton(this,DIALOG_REMOVE_PLAYER,_("Remove")), 0, wxALL, 10 );
    player_list_sizer->Add( player_list_buttons_sizer, 0, wxALIGN_CENTER );
    top_sizer->Add( player_list_sizer, 0, wxALIGN_CENTER );

    wxBoxSizer *button_sizer  = new wxBoxSizer( wxHORIZONTAL );
    button_sizer->Add(new wxButton(this,DIALOG_READY,_("Ready")), 0, wxALL, 10);
    top_sizer->Add( button_sizer, 0, wxALIGN_CENTER );
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
    game_dialog.game_setup_manager->ready();
  }

  void Player_Setup_Panel::on_player_name( wxCommandEvent& event )
  {
    on_add_player(event);
  }

  void Player_Setup_Panel::on_add_player( wxCommandEvent& WXUNUSED(event) )
  {
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
      /*
      if( ai->GetValue() )
      {
	type  = Player::ai;
	input = game_dialog.game_manager.get_player_ai();
      }
      else
      */
      {
	type = Player::user;
	input = game_dialog.gui_manager.get_user_input();
      }

      Player player( wxstr_to_str(player_name->GetValue()), -1, input, 
		     Player::no_output, "", type, 
		     help_mode );

      int num_players = player_list->GetCount();
      if( !game_dialog.game_setup_manager->add_player( player ) )
      {
	wxMessageBox(_("Could not add player"), _("Add player"), wxOK | wxICON_INFORMATION, this);
      }
      else
      {
	player_name->SetValue( get_default_name(num_players + 2) );
      }
    }
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
	  wxMessageBox( _("Could not remove player!"), 
		        _("Remove player"), wxOK | wxICON_INFORMATION, this );
	}
	else
	{
	  player_name->SetValue( get_default_name(num_players) );
	}
      }
    }
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
	  wxMessageBox( _("Could not move player!"), _("Move player"), 
		        wxOK | wxICON_INFORMATION, this );
	}
      }
    }
  }

  void Player_Setup_Panel::on_player_down( wxCommandEvent& WXUNUSED(event) )
  {
    if( game_dialog.game_setup_manager )
    {
      int item = player_list->GetSelection();
      if( (item >= 0) && (item < player_list->GetCount()-1) )
      {
	if( !game_dialog.game_setup_manager->player_down( item_player[item] ) )
	{
	  wxMessageBox( _("Could not move player!"), _("Move player"), 
		        wxOK | wxICON_INFORMATION, this );
	}
      }
    }
  }

  void Player_Setup_Panel::player_added( const Player &player )
  {
    // setup lookup tables
    int item = player_list->GetCount();
    player_item[player.id] = item;
    item_player[item] = player; 

    // add hostname to player name
    std::string name = player.name;
    if( player.host != "" )
      name += '(' + player.host + ')';

    player_list->Append( str_to_wxstr(name) );

    /* this could overwrite the name at any time
    int num_players = player_list->GetCount();
    wxString default_name;
    default_name.Printf( _("Player %d"), num_players + 1 );get_default_name(num_players + 1)
    player_name->SetValue( default_name );
    player_name->SetSelection( 0, player_name->GetLastPosition() );
    */
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
    }
  }

  void Player_Setup_Panel::player_down( const Player &player )
  {
    int item = player_item[player.id];
    if( item < player_list->GetCount() - 1 )
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
    }
  }

  void Player_Setup_Panel::player_change_denied()
  {
    wxMessageBox( _("Change for this player denied!"), _("Denied"), wxOK | wxICON_INFORMATION, this);
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
      case Ruleset::standard:
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
      case Ruleset::standard:
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
    wxMessageBox( _("Change of ruleset denied!"), _("Ruleset denied"), wxOK | wxICON_INFORMATION, this);
  }
  */

  void Player_Setup_Panel::restore()		// display stored game state
  {
    player_list->Clear();
    item_player.clear();
    player_item.clear();
    std::vector<Player>::iterator i;
    for( i = game_dialog.players.begin(); i != game_dialog.players.end(); ++i )
      player_added( *i );

    player_name->SetValue( get_default_name(player_list->GetCount() + 1) );
  }

  wxString Player_Setup_Panel::get_default_name( int player_num )
  {
    if( game_dialog.game.players.size() >= unsigned(player_num) )
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
    : wxWizardPage(parent), game_dialog(game_dialog)
  {
    notebook = new wxNotebook( this, -1 );
    wxNotebookSizer *sizer = new wxNotebookSizer( notebook );

    player_setup_panel = new Player_Setup_Panel( notebook, game_dialog );
    notebook->AddPage( player_setup_panel, _("Player setup") );

    SetAutoLayout( true );     // tell dialog to use sizer
    SetSizer(sizer);
  }

  wxWizardPage *Player_Page::GetPrev() const
  {
    return game_dialog.board_page.get_last_board_page();
  }

  wxWizardPage *Player_Page::GetNext() const
  {
    return 0;
  }

  bool Player_Page::TransferDataFromWindow()
  {
    switch( game_dialog.game_setup_manager->can_start() )
    {
      case Game_Setup_Manager::too_few_players: 
	wxMessageBox( _("Some more players are needed!"), _("Can't start"), wxOK | wxICON_INFORMATION, this);
	break;
      case Game_Setup_Manager::too_many_players: 
	wxMessageBox( _("Too many players!"), _("Can't start"), wxOK | wxICON_INFORMATION, this);
	break;
      case Game_Setup_Manager::not_ready: 
	wxMessageBox( _("Please wait until everyone is ready"), _("Can't start"), wxOK | wxICON_INFORMATION,
		      this);
	break;
      case Game_Setup_Manager::everyone_ready: 
	return true;
    }
    return false;
  }

  void Player_Page::restore()		// display stored game state
  {
    player_setup_panel->restore();
  }

  BEGIN_EVENT_TABLE(Player_Page, wxWizardPage)				
    // EVT_TEXT_ENTER(DIALOG_PLAYER_NAME,	Player_Page::on_player_name)	//**/
  END_EVENT_TABLE()								//**/

  // ----------------------------------------------------------------------------
  // Game_Dialog
  // ----------------------------------------------------------------------------

  wxSize bounding_size( wxSize size1, wxSize size2 )
  {
    wxSize ret;
    ret.SetWidth ( size1.GetWidth()  > size2.GetWidth()  ? size1.GetWidth()  : size2.GetWidth()  );
    ret.SetHeight( size1.GetHeight() > size2.GetHeight() ? size1.GetHeight() : size2.GetHeight() );
    return ret;
  }

  Game_Dialog::Game_Dialog( wxWindow *parent, Game_Manager &game_manager, WX_GUI_Manager &gui_manager )
    : game_manager(game_manager), gui_manager(gui_manager),
      game( Standard_Ruleset() ), game_setup_manager(0),
      wizard( new wxWizard( &gui_manager.get_game_window(), -1, _("Setup game") ) ),
      setup_manager_page( wizard, *this ),
      board_page( wizard, *this ),
      custom_board_page( wizard, *this ),
      load_pbm_board_page( wizard, *this ),
      load_lg_board_page( wizard, *this ),
      player_page( wizard, *this ),
      dummy( wizard )
  {
    wxSize size = bounding_size( setup_manager_page.GetBestSize(), board_page.GetBestSize() );
    size = bounding_size( size, custom_board_page.GetBestSize() );
    size = bounding_size( size, custom_board_page.GetBestSize() );
    size = bounding_size( size, load_pbm_board_page.GetBestSize() );
    size = bounding_size( size, load_lg_board_page.GetBestSize() );
    size = bounding_size( size, player_page.GetBestSize() );
    wizard->SetPageSize(size);
  }

  Game_Dialog::~Game_Dialog()
  {
    wizard->Destroy();
    if( game_setup_manager && setup_manager_page.changed_setup_manager )
    {
      game_setup_manager->set_display_handler(0);
      delete game_setup_manager;
      game_setup_manager = 0;
      setup_manager_page.changed_setup_manager = false;
    }
  }

  void Game_Dialog::game_setup()
  {
    // init variables
    game_setup_manager = game_manager.get_game_setup_manager();
    // init pages
    setup_manager_page.restore();
    // other pages are initialized after setup_manager change

    if( wizard->RunWizard(&setup_manager_page) )
    {
      if( setup_manager_page.changed_setup_manager )
	game_manager.set_game_setup_manager( game_setup_manager );
      setup_manager_page.changed_setup_manager = false;
      assert( game_setup_manager->can_start() == Game_Setup_Manager::everyone_ready );
      game_setup_manager->start_game();
    }
    else
    {
      if( setup_manager_page.changed_setup_manager )
	delete game_setup_manager;
      game_setup_manager = 0;
      setup_manager_page.changed_setup_manager = false;
    }

    // workaround for wxWizard to make it posible to rerun the wizard
    wizard->ShowPage(&dummy);	
    dummy.Show(false);
  }

  void Game_Dialog::set_board( const Game &new_game )
  {
    game = new_game;
    custom_board_page.restore();
  }

  bool Game_Dialog::ask_change_board( const Game &new_game, wxString who )
  {
    wxString msg = who + _(" asks to use another board. Accept?");
    if( wxMessageBox( msg, _("Accept board?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
    {
      game = new_game;
      custom_board_page.restore();
      return true;
    } 
    return false;
  }

  void Game_Dialog::change_board( const Game &new_game )
  {
    game = new_game;
    wxString msg = _("Board changed. Do you want to view the new settings?");
    if( wxMessageBox( msg, _("View board?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
    {
      custom_board_page.restore();
    }
  }

  void Game_Dialog::board_change_accepted()
  {
  }

  void Game_Dialog::board_change_denied()
  {
    wxString msg = _("Board change denied!");
    wxMessageBox( msg, _("Change denied!"), wxOK | wxICON_INFORMATION );
    game = game_setup_manager->get_board();
  }

  void Game_Dialog::player_added( const Player &player )
  {
    if( game_setup_manager )
      players = game_setup_manager->get_players();
    player_page.player_setup_panel->player_added( player );
  }
  void Game_Dialog::player_removed( const Player &player )
  {
    if( game_setup_manager )
      players = game_setup_manager->get_players();
    player_page.player_setup_panel->player_removed( player );
  }
  void Game_Dialog::player_up( const Player &player )
  {
    if( game_setup_manager )
      players = game_setup_manager->get_players();
    player_page.player_setup_panel->player_up( player );
  }
  void Game_Dialog::player_down( const Player &player )
  {
    if( game_setup_manager )
      players = game_setup_manager->get_players();
    player_page.player_setup_panel->player_down( player );
  }
  void Game_Dialog::player_change_denied()
  {
    player_page.player_setup_panel->player_change_denied();
  }
  void Game_Dialog::player_ready( const Player &player )
  {
    player_page.player_setup_panel->player_ready( player );
  }

  void Game_Dialog::everything_ready()
  {
  }
  void Game_Dialog::aborted()
  {
  }

  bool Game_Dialog::ask_new_game( wxString who ) // other player asks for a new game (true: accept)
  {
    wxString msg = who + _(" asks to play new game. Accept?");
    if( wxMessageBox( msg, _("New game?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
    {
      game_manager.stop_game();
      game_setup();
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

    if( wxMessageBox( msg, _("Allow undo?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
    {
      return true;
    } 
    return false;
  }

  void Game_Dialog::get_data_from_setup_manager()
  {
    // init variables
    if( game_setup_manager )
    {
      game    = game_setup_manager->get_board();
      players = game_setup_manager->get_players();
    }

    // init all pages
    board_page.restore();
    custom_board_page.restore();
    load_pbm_board_page.restore();
    load_lg_board_page.restore();
    player_page.restore();
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

    wxString orientation_choices[2];
    orientation_choices[0] = wxString(_("Horizontal"));
    orientation_choices[1] = wxString(_("Vertical"));
    orientation_choice = new wxRadioBox( this, -1, _("Board orientation"), wxDefaultPosition,
					 wxDefaultSize, 2, orientation_choices, 1, wxRA_SPECIFY_ROWS );
    top_sizer->Add( orientation_choice, 0, wxEXPAND | wxALL, 10  );

    show_coordinates = new wxCheckBox( this, -1, _("Show field coordinates") );
    top_sizer->Add( show_coordinates, 0, wxALL, 10 );

    wxString arrangement_choices[2];
    arrangement_choices[0] = wxString(_("Board and common stones left"));
    arrangement_choices[1] = wxString(_("Board left all stones right"));
    arrangement_choice = new wxRadioBox( this, -1, _("Panels arrangement"), wxDefaultPosition,
					 wxDefaultSize, 2, arrangement_choices, 1, wxRA_SPECIFY_COLS );
    top_sizer->Add( arrangement_choice, 0, wxEXPAND | wxALL, 10  );

    multiple_common_stones = new wxCheckBox( this, -1, _("Display all common stones individually") );
    top_sizer->Add( multiple_common_stones, 0, wxALL, 10 );

    multiple_player_stones = new wxCheckBox( this, -1, _("Display all player stones individually") );
    top_sizer->Add( multiple_player_stones, 0, wxALL, 10 );

    // set help texts
    orientation_choice->SetHelpText(_("Select the orientation in which the board is displayed on-screen."));
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
    orientation_choice->SetSelection( dialog->game_settings.board_settings.rotate_board ? 1 : 0 );
    show_coordinates->SetValue( dialog->game_settings.board_settings.show_coordinates );
    switch( dialog->game_settings.arrangement )
    {
      case Game_Panel::Settings::arrange_standard:     arrangement_choice->SetSelection(0); break;
      case Game_Panel::Settings::arrange_stones_right: arrangement_choice->SetSelection(1); break;
    }
    multiple_common_stones->SetValue( dialog->game_settings.common_stone_settings.multiple_stones );
    //multiple_player_stones->SetValue( dialog->game_settings.player_settings.stone_settings.multiple_stones );
  }

  void Display_Setup_Page::apply()
  {
    dialog->game_settings.board_settings.rotate_board 
      = orientation_choice->GetSelection() == 0 ? false : true;
    dialog->game_settings.common_stone_settings.rotate_stones          
      = dialog->game_settings.board_settings.rotate_board;
    //dialog->game_settings.player_settings.stone_settings.rotate_stones 
    //  = dialog->game_settings.board_settings.rotate_board;
    dialog->game_settings.board_settings.show_coordinates              
      = show_coordinates->GetValue();
    switch( arrangement_choice->GetSelection() )
    {
      case 0: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_standard; break;
      case 1: dialog->game_settings.arrangement = Game_Panel::Settings::arrange_stones_right; break;
    }
    dialog->game_settings.common_stone_settings.multiple_stones		 
      = multiple_common_stones->GetValue();
    //dialog->game_settings.player_settings.stone_settings.multiple_stones 
    //  = multiple_player_stones->GetValue();
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

    //wxBoxSizer *skin_sizer = new wxBoxSizer( wxHORIZONTAL );
    grid_sizer->Add( new wxStaticText(this,-1,_("Skin file") ), 0, wxALL, 10 );
    skin_file = new wxTextCtrl( this, DIALOG_CHANGE_SKIN_FILE, wxT("") );
    skin_file->SetSize(wxSize(300,-1));
    grid_sizer->Add( skin_file, 1, wxALL | wxEXPAND, 10 );
    grid_sizer->Add( new wxButton( this, DIALOG_CHOOSE_SKIN_FILE, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( skin_sizer, 0, wxALL | wxEXPAND, 10 );

    //wxBoxSizer *beep_sizer = new wxBoxSizer( wxHORIZONTAL );
    grid_sizer->Add( new wxStaticText(this,-1,_("Beep file") ), 0, wxALL, 10 );
    beep_file = new wxTextCtrl( this, DIALOG_CHANGE_BEEP_FILE, wxT("") );
    beep_file->SetSize(wxSize(300,-1));
    grid_sizer->Add( beep_file, 1, wxALL | wxEXPAND, 10 );
    grid_sizer->Add( new wxButton( this, DIALOG_CHOOSE_BEEP_FILE, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( beep_sizer, 0, wxALL | wxEXPAND, 10 );

    top_sizer->Add( grid_sizer, 0, wxALL | wxEXPAND, 10 );

    play_sound = new wxCheckBox( this, -1, _("Play sounds") );
    top_sizer->Add( play_sound, 0, wxALL, 10 );

    wxFlexGridSizer *font_sizer = new wxFlexGridSizer( 3 );

    //wxBoxSizer *player_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for player names") ), 0, wxALL, 10 );
    player_font_name = new wxStaticText(this,-1,_(""));
    player_font_name->SetSize(wxSize(200,-1));
    font_sizer->Add( player_font_name, 0, wxALL, 10 );
    font_sizer->Add( new wxButton( this, DIALOG_CHOOSE_PLAYER_FONT, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( player_font_sizer, 0, wxALL | wxEXPAND, 10 );

    //wxBoxSizer *coord_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for coordinates") ), 0, wxALL, 10 );
    coord_font_name = new wxStaticText(this,-1,_(""));
    coord_font_name->SetSize(wxSize(200,-1));
    font_sizer->Add( coord_font_name, 0, wxALL, 10 );
    font_sizer->Add( new wxButton( this, DIALOG_CHOOSE_COORD_FONT, _("Choose...") ), 0, wxALL, 10 );
    //top_sizer->Add( coord_font_sizer, 0, wxALL | wxEXPAND, 10 );

    //wxBoxSizer *stone_font_sizer = new wxBoxSizer( wxHORIZONTAL );
    font_sizer->Add( new wxStaticText(this,-1,_("Font for text on stones") ), 0, wxALL, 10 );
    stone_font_name = new wxStaticText(this,-1,_(""));
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
    dialog->game_settings.board_settings.stack_font		    = stone_font; //!!! own font setting?
    dialog->game_settings.common_stone_settings.stone_font	    = stone_font;
    //dialog->game_settings.player_settings.stone_settings.stone_font = stone_font;
  }

  void Look_Feel_Page::on_choose_skin( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = wxFileSelector( _("Choose a skin file"), wxPathOnly(valid_skin_file), 
					wxT(""), wxT(""), _("Skin files (*.zip)|*.zip"),
					wxOPEN | wxFILE_MUST_EXIST );
    if( wxFileExists(filename) )
    {
      valid_skin_file = filename;
      skin_file->SetValue( filename );
    }
    else
      /*!!! output error !!!*/;
  }
  void Look_Feel_Page::on_change_skin( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = skin_file->GetValue();
    if( wxFileExists(filename) )
      valid_skin_file = filename;
    else
      /*!!! output error !!!*/;
  }
  void Look_Feel_Page::on_choose_beep( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = wxFileSelector( _("Choose a beep file"), wxPathOnly(valid_beep_file), 
					wxT(""), wxT(""), _("Beep sounds (*.wav)|*.wav"),
					wxOPEN | wxFILE_MUST_EXIST );
    if( wxFileExists(filename) )
    {
      valid_beep_file = filename;
      beep_file->SetValue( filename );
    }
    else
      /*!!! output error !!!*/;
  }
  void Look_Feel_Page::on_change_beep( wxCommandEvent& WXUNUSED(event) )
  {
    wxString filename = beep_file->GetValue();
    if( wxFileExists(filename) )
      valid_beep_file = filename;
    else
      /*!!! output error !!!*/;
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
    wxNotebookSizer *notebook_sizer = new wxNotebookSizer( notebook );

    display_page = new Display_Setup_Page( notebook, this, gui_manager );
    look_feel_page = new Look_Feel_Page( notebook, this, gui_manager );

    notebook->AddPage( display_page, _("Arrangement") );
    notebook->AddPage( look_feel_page, _("Look & Feel") );

    top_sizer->Add( notebook_sizer, 0, wxEXPAND );

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

  Network_Clients_Dialog::Network_Clients_Dialog( wxWindow *parent, Network_Manager &network_manager )
    : wxDialog(),
      network_manager(network_manager)
  {
	// create the dialog
#ifdef __WXMSW__ // this has to be done before the dialog is created, ...
	SetExtraStyle(wxDIALOG_EX_CONTEXTHELP);
#endif // ... hence the two-step construction (Create() below) is necessary.
	wxDialog::Create(parent,-1,wxString(_("Network client setup")));

	// create the child controls
    client_list = new wxListBox( this, LISTBOX_DCLICK, wxDefaultPosition, 
				  wxSize(120,120), 0, 0, wxLB_SINGLE );

    network_manager.set_connection_handler(this);

    wxBoxSizer *top_sizer = new wxBoxSizer( wxVERTICAL );
    
    top_sizer->Add(client_list, 0, wxALL, 10);

    wxBoxSizer* button_sizer = new wxBoxSizer(wxHORIZONTAL);
    top_sizer->Add(button_sizer, 0, wxALL, 0);

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
    client_item[socket] = client_list->GetCount();
    wxString port;
    port.Printf(wxT("%d"), host.Service());
    /*
    std::ostringstream port;
    port << host.Service();
    */
    client_list->Append( host.Hostname() + wxT('(') + port + wxT(')'), socket );
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
      wxString msg;
      msg.Printf( _("Do you really want to disconnect %s?"), hostname.c_str() );
      if( wxMessageBox( msg, _("Disconnect?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
      {
	network_manager.disconnect( socket );
	client_list->Delete(event.m_commandInt);
      }
    }
    else
    {
      wxString msg;
      msg.Printf( _("Can't disconnect host %s yet!"), hostname.c_str() );
      wxMessageBox( msg, _("Can't disconnect!"), wxOK | wxICON_ERROR );
    }

  }

  BEGIN_EVENT_TABLE(Network_Clients_Dialog, wxDialog)			//**/
  EVT_BUTTON(DIALOG_DISCONNECT, Network_Clients_Dialog::on_disconnect)	//**/
  EVT_LISTBOX_DCLICK(LISTBOX_DCLICK, Network_Clients_Dialog::on_dclick)	//**/
  END_EVENT_TABLE()							//**/

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
    button_sizer->Add( new wxButton(this, DIALOG_CANCEL, _("Cancel"), wxDefaultPosition) , 0, wxALL, 10 );
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
  EVT_BUTTON(DIALOG_OK,     Network_Connection_Dialog::OnOK)		//**/
  EVT_BUTTON(DIALOG_CANCEL, Network_Connection_Dialog::OnCancel)	//**/
  END_EVENT_TABLE()							//**/
}
