/*
 * manager.cpp
 * 
 * management classes
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

#include "manager.hpp"

#include <wx/wx.h>

namespace dvonn
{
  // ----------------------------------------------------------------------------
  // Game_Setup_Display_Handler
  // ----------------------------------------------------------------------------

  Game_Setup_Display_Handler::~Game_Setup_Display_Handler()
  {
  }

  // ----------------------------------------------------------------------------
  // Game_Setup_Manager
  // ----------------------------------------------------------------------------

  Game_Setup_Manager::~Game_Setup_Manager()
  {
  }

  // ----------------------------------------------------------------------------
  // Game_Manager
  // ----------------------------------------------------------------------------

  Game_Manager::Game_Manager( Game_UI_Manager *ui_manager )
    : game_setup_manager(0), ui_manager(ui_manager), game(Standard_Ruleset()), 
      /*ai(*this, ui_manager),*/ new_game_requested(false), undo_moves_requested(0)
  {
  }

  void Game_Manager::set_ui_manager( Game_UI_Manager *new_ui_manager )
  {
    ui_manager = new_ui_manager;
    //ai.set_ui_manager(ui_manager);
  }
  
  void Game_Manager::start_game()
  {
    stop_game();
    if( ui_manager )
    {
      ui_manager->set_board( game );
      ui_manager->setup_game_display();
    }
    continue_game();
  }

  void Game_Manager::continue_game()
  {
    bool go_on = true;
    while(go_on)
    {
      if( ui_manager )
	ui_manager->reset(); 

      Game::Game_State state;
      try
      {
	state = game.continue_game();
      }
      catch(Exception e)
      {
	if( ui_manager )
	  ui_manager->report_error( _("Exception caught!"), _("Exception!") );
	state = Game::finished;
      }

      int winner;
      switch( state )
      {
	case Game::finished:
	{
	  game.stop_game();
	  if( ui_manager )
	    ui_manager->update_board( game ); // update board display

	  winner = game.get_winner_index();

	  if( ui_manager )
	  {
	    if( winner >= 0 )
	      ui_manager->report_winner( &*game.get_player(winner) );
	    else
	      ui_manager->report_winner( 0 );
	  }
	  
	  go_on = false;
	}
	break;
	case Game::wait_for_event:
	  go_on = false;
	  break;
	case Game::next_players_turn:
	  if( ui_manager )
	    ui_manager->update_board( game ); // update board display
	  break;
	case Game::interruption_possible:
	  break;
	case Game::wrong_number_of_players:
	  go_on = false;
	  break;
      }
    }
  }

  void Game_Manager::ask_new_game()
  {
    if( new_game_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Already new game requested"),_("New game"));
    }
    else
    {
      new_game_requested = true;
      assert( game_setup_manager );
      switch( game_setup_manager->ask_new_game() )
      {
	case Game_Setup_Manager::accept: new_game_accepted(); break;
	case Game_Setup_Manager::deny:   new_game_denied(); break;
	case Game_Setup_Manager::wait_for_answer: break;
      }
    }
  }

  void Game_Manager::new_game_accepted()
  {
    if( !new_game_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested new_game accepted?!?"),_("Why new_game"));
    }
    else
    {
      new_game_requested = false;

      if( ui_manager )
	ui_manager->stop_user_activity(); // disable user input

      // setup new game
      new_game();
    }
  }

  void Game_Manager::new_game_denied()
  {
    if( !new_game_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested new game denied?!?"),_("Why new_game"));
    }
    else
    {
      new_game_requested = false;
      wxString msg = _("The idea of starting a new game was rejected. Do you want to start a new game on your own?");
      if( wxMessageBox( msg, _("New game?"), wxYES | wxNO | wxCANCEL | wxICON_QUESTION ) == wxYES )
      {
	new_game();
      }
    }
  }

  void Game_Manager::ask_undo_moves( int n )
  {
    assert( n > 0 );
    if( undo_moves_requested > 0 )
    {
      if( ui_manager )
	ui_manager->report_error(_("Already undo requested"),_("Undo failed"));
    }
    else
    {
      undo_moves_requested = n;
      assert( game_setup_manager );
      switch( game_setup_manager->ask_undo_moves() )
      {
	case Game_Setup_Manager::accept: undo_accepted(); break;
	case Game_Setup_Manager::deny:   undo_denied(); break;
	case Game_Setup_Manager::wait_for_answer: break;
      }
    }
  }

  void Game_Manager::undo_accepted()
  {
    if( undo_moves_requested <= 0 )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested undo accepted?!?"),_("Why undo"));
    }
    else
    {
      undo_moves( undo_moves_requested );

      undo_moves_requested = 0;
    }
  }

  void Game_Manager::undo_denied()
  {
    if( undo_moves_requested <= 0 )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested undo denied?!?"),_("Why undo"));
    }
    else
    {
      undo_moves_requested = 0;
      if( ui_manager )
	ui_manager->report_information(_("Requested undo was denied"),_("Undo denied"));
    }
  }

  // performs undo animation, calls continues_game and deletes itself
  class Undo_Animation : wxEvtHandler
  {
  public:
    Undo_Animation( Game_Manager &game_manager, int n )
      : game_manager(game_manager), n(n)
    {
      // connect event functions
      Connect( -1, wxEVT_TIMER, 
	       (wxObjectEventFunction) (wxEventFunction) (wxTimerEventFunction) 
	       &Undo_Animation::on_done );
    }

    void step()
    {
      if( n <= 0 )
      {
	game_manager.get_ui_manager()->update_board( game_manager.get_game() );
	game_manager.continue_game();
	delete this;
      }
      else
      {
	--n;
	game_manager.get_ui_manager()->undo_move_slowly(this);
      }
    }

    void on_done( wxTimerEvent &event ) // current animation done
    {
      step();
    }    
  private:
    Game_Manager &game_manager;
    int n;
  };

  void Game_Manager::undo_moves( int n )
  {
    assert( n > 0 );
    if( ui_manager )
      ui_manager->stop_user_activity(); // disable user input 
    // !!! also for non mouse players!!!
    
    bool error = false;
    // undo <n> half moves
    for( int i=0; i<n; ++i )
    {
      if( !game.undo_move() )
      {
	error = true;
	n = i;			// reduce number of undos
	break;
      }
    }
    
    if( ui_manager )
    {
      (new Undo_Animation(*this, n))->step();
      if( error )
	ui_manager->report_error(_("Undo move failed"),_("Undo failed"));
    }
    else
      continue_game();
  }

  void Game_Manager::undo_move()
  {
    // determine number of moves to undo
    const Game &game = get_game();
    if( !game.variant_tree.is_first() )
    {
      const Variant *variant = game.variant_tree.get_current_variant();

      int num_undo = 1;
      while( (variant->prev != game.variant_tree.get_root_variant()) &&
	     ( (game.get_player(variant->current_player_index)->origin==Player::remote) ||
	       (game.get_player(variant->current_player_index)->type==Player::ai) ||
	       (variant->prev->possible_variants==1) ) )
      {
	variant = variant->prev;
	num_undo++;
      }

      // ask to undo the number of moves
      ask_undo_moves(num_undo);
    }
  }

  void Game_Manager::new_game()
  {
    stop_game();
    assert( game_setup_manager );
    game_setup_manager->new_game();
  }

  void Game_Manager::stop_game()
  {
    game.stop_game();
  }
  
  void Game_Manager::set_board  ( const Game& new_game )
  {
    stop_game();
    game = new_game;
  }

  void Game_Manager::set_players( const std::vector<Player>& players )
  {
    stop_game();
    game.set_players( players );
  }
  /*
  AI_Input *Game_Manager::get_hint_ai()
  {
    return &ai;
  }
  AI_Input *Game_Manager::get_player_ai()
  {
    return &ai;
  }
  */
  // ----------------------------------------------------------------------------
  // Game_UI_Manager
  // ----------------------------------------------------------------------------

  Game_UI_Manager::Game_UI_Manager( const Game &game )
    : display_game(game)
  {
  }
  Game_UI_Manager::~Game_UI_Manager()
  {
  }

  // ----------------------------------------------------------------------------
  // Standalone_Game_Setup_Manager
  // ----------------------------------------------------------------------------

  Standalone_Game_Setup_Manager::Standalone_Game_Setup_Manager( Game_Manager &game_manager )
    : game_manager(game_manager), display_handler(0), game( game_manager.get_game() ), current_id(42)
    //,players( game.players )
  {
    // init id map
    /*
    std::deque<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }
    */
  }

  Standalone_Game_Setup_Manager::~Standalone_Game_Setup_Manager()
  {
    stop_game();
    if( display_handler )
      display_handler->aborted();
  }

  Game_Setup_Manager::Type Standalone_Game_Setup_Manager::get_type()
  {
    return alone;
  }

  void Standalone_Game_Setup_Manager::set_display_handler( Game_Setup_Display_Handler *handler )
  {
    display_handler = handler;
    /* handler will request this information anyway
    if( display_handler )
    {
      // tell handler all players
      std::deque<Player>::iterator player;
      for( player = players.begin(); player != players.end(); ++player )
      {
	display_handler->player_added(*player);
      }

      display_handler->set_board( game );
    }
    */
  }

  Standalone_Game_Setup_Manager::Answer_Type 
  Standalone_Game_Setup_Manager::ask_change_board( const Game &new_game )
  {
    game = new_game;
    return accept;
  }

  const Game &Standalone_Game_Setup_Manager::get_board()
  {
    return game;
  }

  std::vector<Player> Standalone_Game_Setup_Manager::get_players()
  {
    std::vector<Player> player_vector;
    player_vector.resize(players.size());
    for( unsigned i=0; i<players.size(); ++i )
      player_vector[i] = players[i];

    return player_vector;
  }

  bool Standalone_Game_Setup_Manager::add_player( const Player &player )
  {
    if( players.size() < game.get_max_players() )
    {
      std::deque<Player>::iterator p = players.insert(players.end(), player);
      p->id = current_id; ++current_id;
      id_player[p->id] = p;
    
      if( display_handler )
	display_handler->player_added(*p);

      return true;
    }
    return false;
  }
  bool Standalone_Game_Setup_Manager::remove_player( const Player &player )
  {
    std::deque<Player>::iterator player_iterator = id_player[player.id];
    assert( player_iterator->id == player.id );
    if( display_handler )
      display_handler->player_removed(*player_iterator);

    players.erase(player_iterator);
    // reload iterators due to erase
    id_player.clear();
    std::deque<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }
    return true;
  }
  bool Standalone_Game_Setup_Manager::player_up( const Player &player )
  {
    std::deque<Player>::iterator player_iterator = id_player[player.id];
    assert( player_iterator->id == player.id );
    if( player_iterator == players.begin() ) // is first player
      return false;

    std::deque<Player>::iterator player_iterator2 = player_iterator; --player_iterator2;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator->id]  = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler )
      display_handler->player_up(*player_iterator2);

    return true;
  }
  bool Standalone_Game_Setup_Manager::player_down( const Player &player )
  {
    std::deque<Player>::iterator player_iterator  = id_player[player.id];
    assert( player_iterator->id == player.id );
    std::deque<Player>::iterator player_iterator2 = player_iterator; ++player_iterator2;

    if( player_iterator2 == players.end() ) // is last player
      return false;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator ->id] = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler )
      display_handler->player_down(*player_iterator2);

    return true;
  }
  void Standalone_Game_Setup_Manager::ready()	
  {
  }
  Game_Setup_Manager::Game_State Standalone_Game_Setup_Manager::can_start()
  {
    if( players.size() < game.ruleset->get_min_players() )
      return too_few_players;

    if( players.size() > game.ruleset->get_max_players() )
      return too_many_players;

    return everyone_ready;
  }
  void Standalone_Game_Setup_Manager::start_game()
  {
    assert( can_start() == everyone_ready );
    game_manager.set_board  ( game );
    std::vector<Player> player_vector;
    player_vector.resize(players.size());
    for( unsigned i=0; i<players.size(); ++i )
      player_vector[i] = players[i];
    game_manager.set_players( player_vector );
    game_manager.start_game();
  }
  Standalone_Game_Setup_Manager::Answer_Type Standalone_Game_Setup_Manager::ask_new_game()
  {
    return accept;
  }
  Standalone_Game_Setup_Manager::Answer_Type Standalone_Game_Setup_Manager::ask_undo_moves( int n )
  {
    return accept;
  }
  void Standalone_Game_Setup_Manager::new_game() // force new game (may close connections)
  {
    if( display_handler )	// there should be a display handler that may display a game setup dialog
      display_handler->game_setup();
  }  
  void Standalone_Game_Setup_Manager::stop_game()
  {
    game_manager.stop_game();
  }
  
}

