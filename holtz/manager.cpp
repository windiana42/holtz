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

namespace holtz
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
    : ui_manager(ui_manager), game_setup_manager(0), game(Standard_Ruleset()), ai(*this, ui_manager)
  {
  }

  void Game_Manager::set_ui_manager( Game_UI_Manager *new_ui_manager )
  {
    ui_manager = new_ui_manager;
    ai.set_ui_manager(ui_manager);
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

      Player *winner;
      switch( state )
      {
	case Game::finished:
	{
	  game.stop_game();
	  if( ui_manager )
	    ui_manager->update_board( game ); // update board display

	  winner = game.get_winner();

	  if( ui_manager )
	    ui_manager->report_winner( winner );

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

  void Game_Manager::stop_game()
  {
    game.stop_game();
  }
  
  void Game_Manager::set_board  ( const Game& new_game )
  {
    stop_game();
    game = new_game;
  }

  void Game_Manager::set_players( const std::list<Player>& players )
  {
    stop_game();
    game.set_players( players );
  }

  AI_Input *Game_Manager::get_hint_ai()
  {
    return &ai;
  }
  AI_Input *Game_Manager::get_player_ai()
  {
    return &ai;
  }

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
    : game_manager(game_manager), display_handler(0), game( game_manager.get_game() ), current_id(42),
      players( game.players )
  {
    // init id map
    std::list<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }
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
      std::list<Player>::iterator player;
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

  const std::list<Player> &Standalone_Game_Setup_Manager::get_players()
  {
    return players;
  }

  bool Standalone_Game_Setup_Manager::add_player( const Player &player )
  {
    if( players.size() < game.get_max_players() )
    {
      std::list<Player>::iterator p = players.insert(players.end(), player);
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
    std::list<Player>::iterator player_iterator = id_player[player.id];
    if( display_handler )
      display_handler->player_removed(*player_iterator);

    players.erase(player_iterator);
    id_player.erase( player.id ); 
    return true;
  }
  bool Standalone_Game_Setup_Manager::player_up( const Player &player )
  {
    std::list<Player>::iterator player_iterator = id_player[player.id];
    if( player_iterator == players.begin() ) // is first player
      return false;

    std::list<Player>::iterator player_iterator2 = player_iterator; --player_iterator2;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator->id]  = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler )
      display_handler->player_up(*player_iterator);

    return true;
  }
  bool Standalone_Game_Setup_Manager::player_down( const Player &player )
  {
    std::list<Player>::iterator player_iterator  = id_player[player.id];
    std::list<Player>::iterator player_iterator2 = player_iterator; ++player_iterator2;

    if( player_iterator2 == players.end() ) // is last player
      return false;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator ->id] = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler )
      display_handler->player_up(*player_iterator);

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
    game_manager.set_players( players );
    game_manager.start_game();
  }
  Standalone_Game_Setup_Manager::Answer_Type Standalone_Game_Setup_Manager::ask_new_game()
  {
    return accept;
  }
  void Standalone_Game_Setup_Manager::stop_game()
  {
    game_manager.stop_game();
  }
  
}

