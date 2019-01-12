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

#include "manager.hpp"		// this should be loaded from common/ directory

#include <wx/wx.h>

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

  Game_Manager::Game_Manager( int game_event_id, Game_UI_Manager *ui_manager )
    : game_setup_manager(0), game_setup_display_handler(0), ui_manager(ui_manager), 
      game(Standard_Ruleset()), valid_game(false), game_stopped(true), 
      ai(new AI_Input(*this, ui_manager)), undo_requested(false), new_game_requested(false),
      game_event_id(game_event_id)
  {
  }
  Game_Manager::~Game_Manager()
  {
    ai->destroy_ai(); // use pending delete because AI thread has to terminate first
  }

  void Game_Manager::set_ui_manager( Game_UI_Manager *new_ui_manager )
  {
    ui_manager = new_ui_manager;
    ai->set_ui_manager(ui_manager);
  }
  
  void Game_Manager::start_game()
  {
    stop_game();
    if( ui_manager )
    {
      ui_manager->set_board( game );
    }
    valid_game = true;
    continue_game();
  }

  void Game_Manager::continue_game()
  {
    if( !valid_game ) return;
    game_stopped = false;
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
	case Game::finished_scores:
	{
	  game.stop_game();
	  if( ui_manager )
	    ui_manager->update_board( game ); // update board display

	  if( ui_manager )
	  {
	    ui_manager->report_scores( &game, game.get_scores() );
	  }

	  go_on = false;
	}
	break;
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

  void Game_Manager::new_game()
  {
    if( new_game_requested )
    {
      if( ui_manager )
      {
	ui_manager->report_error(_("Already new game requested"),_("New game"));
      }
    }
    else
    {
      stop_game();
      if( game_setup_manager )
      {
	new_game_requested = true;
	switch( game_setup_manager->ask_new_game() )
	{
	  case Game_Setup_Manager::accept: new_game_accepted(); break;
	  case Game_Setup_Manager::deny:   new_game_denied(); break;
	  case Game_Setup_Manager::wait_for_answer: 
	    if( ui_manager )
	    {
	      ui_manager->show_status_text(_("Asking other Users..."));
	    }
	    break;
	}
      }
      else if( game_setup_display_handler )
      {
	game_setup_display_handler->game_setup();
      } 
      else
      {
	continue_game();
      }
    }
  }

  void Game_Manager::force_new_game(bool on_own)
  {
    stop_game();
    assert( game_setup_manager );
    game_setup_manager->force_new_game(on_own);
  }

  void Game_Manager::new_game_accepted()
  {
    if( ui_manager )
    {
      ui_manager->show_status_text(wxT(""));
    }
    if( !new_game_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested new_game accepted?!?"),_("Why new_game"));
    }
    else
    {
      new_game_requested = false;
      // setup new game
      force_new_game();
    }
  }

  void Game_Manager::new_game_denied()
  {
    if( ui_manager )
    {
      ui_manager->show_status_text(wxT(""));
    }
    if( !new_game_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested new game denied?!?"),_("Why new_game"));
    }
    else
    {
      new_game_requested = false;
      wxString msg = _("The idea of starting a new game was rejected. Do you want to start a new game on your own?");
      if( wxMessageDialog( 0, msg, _("New game?"), 
			   wxYES_NO | wxCANCEL | wxICON_QUESTION ).ShowModal() == wxID_YES )
      {
	force_new_game(true/*on own*/);
      }
      else
      {
	continue_game();
      }
    }
  }

  void Game_Manager::undo_moves(int num_undo)
  {
    if( undo_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Already undo requested"),_("Undo failed"));
      return;
    }

    if( game_stopped )
      if( ui_manager )
	ui_manager->stop_animations(); // stop animations to enable quick multiple undos
				       // (this can modify game_stopped )

    if( game_stopped )
    {
      if( ui_manager )
	ui_manager->report_error(_("Can't undo move since game is stopped."),_("Undo failed"));
    }
    else
    {
      // determine number of moves to undo
      const Game &game = get_game();
      if( !game.variant_tree.is_first() )  // is there any move to undo?
      {
	stop_game();
	if( num_undo < 0 ) // for num_undo < 0, num_undo is automatically determined
	{
	  const Variant *variant = game.variant_tree.get_current_variant();

	  num_undo = 1;
	  while( (variant->prev != game.variant_tree.get_root_variant()) &&
		 ( (game.get_player(variant->current_player_index)->origin==Player::remote) ||
		   (game.get_player(variant->current_player_index)->type==Player::ai) ||
		   (variant->prev->possible_variants==1) ) )
	  {
	    variant = variant->prev;
	    num_undo++;
	  }
	}

	// ask for undo
	undo_requested = true;
	num_undo_moves = num_undo;
	assert( game_setup_manager );
	switch( game_setup_manager->ask_undo_moves(num_undo) )
	{
	  case Game_Setup_Manager::accept: undo_accepted(); break;
	  case Game_Setup_Manager::deny:   undo_denied(); break;
	  case Game_Setup_Manager::wait_for_answer: 
	    if( ui_manager )
	    {
	      ui_manager->show_status_text(_("Asking other Users..."));
	    }
	    break;
	}
      }
    }
  }

  // performs undo animation, calls continues_game and deletes itself
  class Undo_Animation : wxEvtHandler
  {
  public:
    Undo_Animation( Game_Manager &game_manager, int n, int game_event_id )
      : game_manager(game_manager), n(n)
    {
      // connect event functions
      Connect( ANIMATION_DONE, game_event_id, 
	       (wxObjectEventFunction) (wxEventFunction) (wxTimerEventFunction) 
	       &Undo_Animation::on_done );
      Connect( ANIMATION_ABORTED, game_event_id, 
	       (wxObjectEventFunction) (wxEventFunction) (wxTimerEventFunction) 
	       &Undo_Animation::on_aborted );
    }
    
    void finish()
    {
      game_manager.get_ui_manager()->update_board( game_manager.get_game() );
      game_manager.continue_game();
      delete this;
    }

    void step()
    {
      if( n <= 0 )
      {
	finish();
      }
      else
      {
	--n;
	game_manager.get_ui_manager()->undo_move_slowly(this, ANIMATION_DONE, 
							ANIMATION_ABORTED);
      }
    }

    void on_done( wxTimerEvent & ) // current animation done
    {
      step();
    }    
    void on_aborted( wxTimerEvent & ) // current animation aborted
    {
      finish();
    }    
  private:
    Game_Manager &game_manager;
    int n;
    enum{ ANIMATION_DONE=200, ANIMATION_ABORTED };  // to avoid including wxmain.hpp
  };

  void Game_Manager::undo_accepted()
  {
    if( ui_manager )
    {
      ui_manager->show_status_text(wxT(""));
    }
    if( !undo_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested undo accepted?!?"),_("Why undo"));
    }
    else
    {
      undo_requested = false;
      do_undo_moves( num_undo_moves );
    }
  }

  void Game_Manager::undo_denied()
  {
    if( ui_manager )
    {
      ui_manager->show_status_text(wxT(""));
    }
    if( !undo_requested )
    {
      if( ui_manager )
	ui_manager->report_error(_("Unrequested undo denied?!?"),_("Why undo"));
    }
    else
    {
      undo_requested = false;
      if( ui_manager )
	ui_manager->report_information(_("Requested undo was denied"),_("Undo denied"));
      continue_game();
    }
  }

  void Game_Manager::do_undo_moves( int n )
  {
    if( ui_manager )
      ui_manager->abort_all_activity(); // disable user input and animations
    ai->abort();

    bool error = false;
    for( int i=0; i<n; ++i )
    {
      if( !game.undo_move() )
      {
	error = true;
	n = i;		// reduce number of undo moves
	break;
      }
    }
    /*
      if( ui_manager )
      ui_manager->update_board( game ); // update board display
    */
    if( ui_manager )
    {
      if( error )
	ui_manager->report_error(_("Undo move failed"),_("Undo failed"));
      (new Undo_Animation(*this, n, game_event_id))->step();  // will call continue_game
    }
    else
      continue_game();
  }

  void Game_Manager::stop_game()
  {
    ai->abort();
    if( ui_manager )
      ui_manager->abort_all_activity(); // disable user input and animations

    game_stopped = true;
    game.stop_game();
  }
  
  void Game_Manager::set_board  ( const Game& new_game )
  {
    //stop_game();
    assert(game_stopped);
    game = new_game;
  }

//   void Game_Manager::set_players( const std::list<Player>& players )
//   {
//     //stop_game();
//     assert(game_stopped);
//     game.set_players( list_to_vector(players) );
//   }

  AI_Input *Game_Manager::get_hint_ai()
  {
    return ai;
  }
  AI_Input *Game_Manager::get_player_ai()
  {
    return ai;
  }

  // ----------------------------------------------------------------------------
  // Game_UI_Manager
  // ----------------------------------------------------------------------------

  Game_UI_Manager::Game_UI_Manager( const Game &game )
    : display_game(game), target_variant(0)
  {
  }
  Game_UI_Manager::~Game_UI_Manager()
  {
  }

  // ----------------------------------------------------------------------------
  // Standalone_Game_Setup_Manager
  // ----------------------------------------------------------------------------

  Standalone_Game_Setup_Manager::Standalone_Game_Setup_Manager( Game_Manager &game_manager )
    : game_manager(game_manager), display_handler(0), game( game_manager.get_game() ), 
      current_id(42)
  {
    // self register
    game_manager.set_game_setup_manager( this );
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

    // self unregister
    game_manager.set_game_setup_manager( 0 );
  }

  Game_Setup_Manager::Type Standalone_Game_Setup_Manager::get_type()
  {
    return alone;
  }

  void Standalone_Game_Setup_Manager::set_display_handler( Game_Setup_Display_Handler *handler )
  {
    display_handler = handler;
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
    // reload iterators due to erase
    id_player.clear();
    std::list<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }

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
      display_handler->player_up(*player_iterator2);

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
      display_handler->player_down(*player_iterator2);

    return true;
  }

  void Standalone_Game_Setup_Manager::ready()	
  {
  }

  // returns players before feedback
  std::list<Player> Standalone_Game_Setup_Manager::enable_player_feedback() 
  {
    return players;
  }

  // disables feedback about player changes
  void Standalone_Game_Setup_Manager::disable_player_feedback()  
  {
  }

  // whether this player can choose a board to play
  bool Standalone_Game_Setup_Manager::can_choose_board() 
  {
    return true;
  }

  // whether the player setup can be entered
  bool Standalone_Game_Setup_Manager::can_enter_player_setup() 
  {
    return true;
  }

  Game_Setup_Manager::Game_State Standalone_Game_Setup_Manager::can_start()
  {
    if( players.size() < game.get_min_players() )
      return too_few_players;

    if( players.size() > game.get_max_players() )
      return too_many_players;

    return everyone_ready;
  }

  void Standalone_Game_Setup_Manager::start_game()
  {
    assert( can_start() == everyone_ready );
    game.set_players( list_to_vector(players) );
    game_manager.set_board  ( game );
    game_manager.start_game();
  }
  Standalone_Game_Setup_Manager::Answer_Type Standalone_Game_Setup_Manager::ask_new_game()
  {
    return accept;
  }
  Standalone_Game_Setup_Manager::Answer_Type Standalone_Game_Setup_Manager::ask_undo_moves(int /*n*/)
  {
    if( get_board().undo_possible )
      return accept;
    else
      return deny;
  }
  // force new game (may close connections)
  void Standalone_Game_Setup_Manager::force_new_game(bool /*on_own*/) 
  {
    if( display_handler )	// there should be a display handler
				// that may display a game setup
				// dialog
      display_handler->game_setup();
  }  
  void Standalone_Game_Setup_Manager::stop_game()
  {
    game_manager.stop_game();
  }
  void Standalone_Game_Setup_Manager::game_setup_entered()
  {
  }
  // is it allowed to abort the game setup?
  bool Standalone_Game_Setup_Manager::is_allow_game_setup_abort()
  {
    disable_player_feedback();
    game_manager.continue_game();
    return true;
  }
}

