/*
 * network.cpp
 * 
 * Netzwork access
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

#include "network.hpp"		// this should be loaded from common/ directory

#if defined(VERSION_ZERTZ)
#  undef VERSION_ZERTZ
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_ZERTZ
#  define GAME_NAME GAME_NAME_ZERTZ
#elif defined(VERSION_DVONN)
#  undef VERSION_DVONN
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_DVONN
#  define GAME_NAME GAME_NAME_DVONN
#elif defined(VERSION_BLOKS)
#  undef VERSION_BLOKS
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_BLOKS
#  define GAME_NAME GAME_NAME_BLOKS
#elif defined(VERSION_RELAX)
#  undef VERSION_RELAX
#  include "wxmain.hpp"
#  include "util.hpp"
#  define VERSION_RELAX
#  define GAME_NAME GAME_NAME_RELAX
#endif

#define PROTOCOL_NUMBER 1.00

#include <iostream>
#include <algorithm>

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
  using namespace holtz;

//=================================================================================================
//=================================================================================================
//  Server
//=================================================================================================
//=================================================================================================

  Network_Manager_BGP100a_Server::Network_Manager_BGP100a_Server
  ( Game_Manager &game_manager, Game_UI_Manager &ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), 
      display_handler(0), 
      game( game_manager.get_game() ),
      timer(this,NETWORK_TIMER),
      msg_net_server(this),
      max_connections(15), allow_visitors(true), response_timeout(6000/*ms*/),
      current_id(11), current_player_id(101),
      display_phase(DISPLAY_INIT), game_phase(BGP::phase_setup), 
      disable_new_connections(false), is_ready(false), sequence_available(false), 
      awaiting_move(false), asking_new_game(false), asking_undo(false), asking_done(false), 
      answer(false), clients_in_setup(1), clients_ready(0), clients_asked(0), asking_client(0),
      asking_n(0), possibly_interrupted_connection(0), delayed_move_visualization(false)
  {
    // self register
    game_manager.set_game_setup_manager( this );
  }

  Network_Manager_BGP100a_Server::~Network_Manager_BGP100a_Server()
  {
    disable_new_connections = true;
    close_connections();

    // self unregister
    game_manager.set_game_setup_manager( 0 );
    if( connection_handler )
      connection_handler->destroy();
  }

  // connection commands
  bool Network_Manager_BGP100a_Server::setup_server( int port )
  {
    close_connections();
    disable_new_connections = false;
    return msg_net_server.bind(port);
  }

  //---------------------------------------------
  //---------------------------------------------
  // commands inherited from Basic_Network_Server
  //---------------------------------------------
  //---------------------------------------------

  void Network_Manager_BGP100a_Server::close_connections()
  {
    std::list<Message_Network<BGP::Message>*> connections;
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      if( it->first )
	connections.push_back(it->first);
    }
    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
    {
      disconnect(*conn_it);
    }
    /* can't be guaranteed if concurrent disconnect was interrupted
    assert(msg_net_connection_state.empty());
    assert(player_id_connection.empty());
    assert(id_connection.empty());
    */
  }

  bool Network_Manager_BGP100a_Server::may_disconnect_id( Connection_Id id )
  {
    if( does_include(id_connection, id) ) return true;
    else return false;
  }
  void Network_Manager_BGP100a_Server::disconnect_id( Connection_Id id )
  {
    assert( does_include(id_connection, id) );
    disconnect(id_connection[id]);
  }
  void Network_Manager_BGP100a_Server::allow_connect( bool allow )
  {
    disable_new_connections = !allow;
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Game_Setup_Manager
  //-------------------------------------------
  //-------------------------------------------

  Game_Setup_Manager::Type Network_Manager_BGP100a_Server::get_type()
  {
    return Game_Setup_Manager::server;
  }

  void Network_Manager_BGP100a_Server::set_display_handler( Game_Setup_Display_Handler* handler )
  {
    display_handler = handler;
  }

  //-------------------------------------------
  // board commands
  //-------------------------------------------
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_change_board(const Game &g)
  {
    game = g;
    return accept;
  }
  const Game &Network_Manager_BGP100a_Server::get_board()
  {
    return game;
  }
  const std::list<Player> &Network_Manager_BGP100a_Server::get_players()
  {
    return players;
  }
  //-------------------------------------------
  // player commands
  //-------------------------------------------
  bool Network_Manager_BGP100a_Server::add_player( const Player &player )
  {
    if( is_ready ) 
      return false;
    do_add_player(player);
    return true;
  }
  bool Network_Manager_BGP100a_Server::remove_player( const Player &player )
  {
    if( is_ready ) 
      return false;
    do_remove_player(player.id);
    return true;
  }
  bool Network_Manager_BGP100a_Server::player_up( const Player &player )
  {
    if( is_ready ) 
      return false;
    return do_player_up(player.id);
  }
  bool Network_Manager_BGP100a_Server::player_down( const Player &player )
  {
    if( is_ready ) 
      return false;
    return do_player_down(player.id);
  }
  // ready with adding players
  void Network_Manager_BGP100a_Server::ready()
  {
    is_ready = true;
    ++clients_ready;
    check_all_ready();
  }
  //-------------------------------------------
  // game commands
  //-------------------------------------------

  // returns players before feedback
  std::list<Player> Network_Manager_BGP100a_Server::enable_player_feedback() 
  {
    display_phase = DISPLAY_SETUP;
    return players;
  }

  // disables feedback about player changes
  void Network_Manager_BGP100a_Server::disable_player_feedback() 
  {
    if(display_phase == DISPLAY_SETUP)
      display_phase = DISPLAY_INIT;
  }

  // whether this player can choose a board to play
  bool Network_Manager_BGP100a_Server::can_choose_board() 
  {
    if( is_ready ) 
      return false;
    else
      return true;
  }

  // whether the player setup can be entered
  bool Network_Manager_BGP100a_Server::can_enter_player_setup() 
  {
    return true;
  }

  // is everyone ready and number of players ok?
  Game_Setup_Manager::Game_State Network_Manager_BGP100a_Server::can_start()
  {
    if( players.size() < game.get_min_players() )
    {
#ifndef __WXMSW__
      std::cout << "Server - players: " << players.size() << "/" << game.get_min_players() 
		<< std::endl;		
#endif
      return too_few_players;
    }
    else if( players.size() > game.get_max_players() )
      return too_many_players;
    else if( game_phase == BGP::phase_playing )
      return everyone_ready;
    else
      return not_ready;
  }
  // call only when can_start() == true
  void Network_Manager_BGP100a_Server::start_prepared_game()  // game has already players
  {
    assert( can_start() == everyone_ready );
    display_phase = DISPLAY_PLAYING;
    game_manager.set_board  ( game );
    int id = game.get_current_player().id;
    if( does_include(player_id_connection,id) )
      msg_net_connection_state[player_id_connection[id]].state = BGP_HIS_TURN;

    game_manager.start_game();
    process_deferred_messages();
  }
  // call only when can_start() == true
  void Network_Manager_BGP100a_Server::start_game()
  {
    assert( can_start() == everyone_ready );
    display_phase = DISPLAY_PLAYING;
    game.set_players( list_to_vector(players) );
    game_manager.set_board ( game );
    int id = game.get_current_player().id;
    if( does_include(player_id_connection,id) )
      msg_net_connection_state[player_id_connection[id]].state = BGP_HIS_TURN;

    game_manager.start_game();
    process_deferred_messages();
  }
  // request to play new game
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_new_game()
  {
    if( !game.players.size() || game_phase == BGP::phase_setup ) return accept;
    int id = game.get_current_player().id;
    if( !does_include(own_player_ids,id) )
    {
      return deny;		// only allow new game when its the players turn
    }
    // send request
    if( !do_ask_new_game(0/*local*/) )
      return deny;
    else
    {
      if( check_all_answered(false/*don't answer local*/) )
      {
	if( answer )
	  return accept;
	else 
	  return deny;
      }
      else
	return wait_for_answer;
    }
  }
  // request to undo n half moves
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_undo_moves(int n)
  {
    if( !game.undo_possible ) return deny;
    if( !game.players.size() || game_phase == BGP::phase_setup ) return accept;
    int id = game.get_current_player().id;
    if( !does_include(own_player_ids,id) )
    {
      return deny;		// only allow new game when its the players turn
    }
    // send request
    if( !do_ask_undo(n,0/*local*/) )
      return deny;
    else
    {
      if( check_all_answered(false/*don't answer local*/) )
      {
	if( answer )
	  return accept;
	else 
	  return deny;
      }
      else
	return wait_for_answer;
    }
  }
  // force new game (may close connections)
  void Network_Manager_BGP100a_Server::force_new_game(bool /*on_own*/)
  {
    // reset local state
    display_phase = DISPLAY_INIT;
    game_phase = BGP::phase_setup;
    is_ready = false;
    sequence_available = false;
    awaiting_move = false;
    asking_new_game = false;
    asking_undo = false;
    clients_ready = 0;
    clients_in_setup = 1; 
    // reset connection states (no concurrency)
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      //Message_Network<BGP::Message>* connection = it->first;
      Connection_State &conn_state = it->second;
      if( !conn_state.in_init )
      {
	if( conn_state.phase == BGP::phase_setup )
	  ++clients_in_setup; 
	if( conn_state.is_ready )
	  ++clients_ready;
      }
    }
    // seperate run for disconnect since disconnect may be interrupted (concurrency!)
    std::list<Message_Network<BGP::Message>*> connections;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      if( it->first )
	connections.push_back(it->first);
    }
    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
    {
      Message_Network<BGP::Message>* connection = *conn_it;
      Connection_State &conn_state = msg_net_connection_state[connection];
      if( !conn_state.in_init )
      {
	if( conn_state.phase == BGP::phase_playing )
	  disconnect(connection);  // connection didn't accept new game
      }
    }
    cleanup_players();
    // open new game dialog
    if( display_handler )	// there should be a display handler
				// that may display a game setup
				// dialog
      display_handler->game_setup();
  }
  // stop game
  void Network_Manager_BGP100a_Server::stop_game()
  {
    game_manager.stop_game();
  }
  // game setup entered 
  void Network_Manager_BGP100a_Server::game_setup_entered()
  {
  }
  // is it allowed to abort the game setup?
  bool Network_Manager_BGP100a_Server::is_allow_game_setup_abort()
  {
    return false;
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Input
  //-------------------------------------------
  //-------------------------------------------

  Player_Input::Player_State Network_Manager_BGP100a_Server::determine_move() throw(Exception)
  {
    if( sequence_available ) 
    {
      assert( awaiting_move == false );
      return Player_Input::finished;
    }
    else
    {
      if( does_include(own_player_ids,game.get_current_player().id) )
      {
#ifndef __WXMSW__
	std::cerr << "Server Error - Asked for move but current player " 
		  << game.get_current_player().name << "(" << game.get_current_player().id 
		  << ") is owned by the server!" << std::endl;
	for( std::set<int>::iterator it=own_player_ids.begin();it!=own_player_ids.end();++it )
	{
	  std::cerr << *it << std::endl;
	}
#endif
	// Server fooled itself, this should not happen!
	assert(false);
	return Player_Input::wait_for_event;
      }
      else
      {
	assert( !sequence_available );
	if( pending_moves.size() )
	{
	  // todo: allow to check player id: pending_moves.front().first
	  sequence = pending_moves.front().second;
	  sequence_available = true;
	  pending_moves.pop_front();
	  return Player_Input::finished;
	}
	else if( does_include(pending_player_move,game.get_current_player().id) )
	{
	  Message_Network<BGP::Message> *connection 
	    = pending_player_move[game.get_current_player().id].second;
	  sequence = pending_player_move[game.get_current_player().id].first;
	  pending_player_move.erase(game.get_current_player().id);
	  awaiting_move = true;
	  process_move(sequence, connection, game.get_current_player().id,
		       /*delay_continue_game=*/true);
	  return Player_Input::wait_for_event;
	}
	else
	{
	  awaiting_move = true;
	  return Player_Input::wait_for_event;
	}
      }
    }
  }
  Move_Sequence Network_Manager_BGP100a_Server::get_move()
  {
    assert(sequence_available);
    sequence_available = false;
    return sequence;
  }
  long Network_Manager_BGP100a_Server::get_used_time()
  {
    return 1;			// !!! todo: measure time
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Output
  //-------------------------------------------
  //-------------------------------------------

  void Network_Manager_BGP100a_Server::report_move( const Move_Sequence &move_sequence )
  {
    process_move(move_sequence);
  }

  //------------------------------------------------
  //------------------------------------------------
  // commands inherited from Message_Network_Handler
  //------------------------------------------------
  //------------------------------------------------

  // is called when message arrives
  // specifics: message must be deleted, message==0: invalid message arrived
  void Network_Manager_BGP100a_Server::process_message( Message_Network<BGP::Message> *connection, 
							BGP::Message *message )
  {
    if( !does_include(msg_net_connection_state,connection) ) return;  // disconnected?
    bool invalid = false;
    Connection_State &conn_state = msg_net_connection_state[connection];
    if( message )
    {
#ifndef __WXMSW__
      // !!! Debug output
      std::cerr << "Server received: "; message->print(std::cerr);
#endif

      //------------------------------
      // non state dependent messages:

      if( message->get_type() == BGP::msg_disconnect )
      {
	  conn_state.state = BGP_UNCONNECTED;
	  disconnect(connection,false/*don't send disconnect message*/);
	  return;
      }
      if( message->get_type() == BGP::msg_ping )
      {
	// send response
	BGP::Msg_Pong msg;
	connection->send_message(&msg);
	return;
      }
      if( message->get_type() == BGP::msg_chat )
      {
	// todo: relay and display chat message
	return;
      }

      //------------------------------
      // state dependent messages:

      switch(conn_state.state) 
      {
	case BGP_UNCONNECTED:
	{
	  invalid = true;
	  break;
	}
	case BGP_CONNECTED:
	{
	  conn_state.timer.stop();
	  switch( message->get_type() )
	  {
	    case BGP::msg_helo:
	    {
	      BGP::Msg_Helo *det_msg = static_cast<BGP::Msg_Helo*>(message);
	      BGP::Protocol protocol = det_msg->get_protocol();
	      // supported protocols: BGP 1.0Xa
	      if( protocol.name == "BGP" && is_equal(protocol.number, PROTOCOL_NUMBER) &&
		  protocol.letter == 'a' )
	      {
		// accept protocol
		conn_state.state = BGP_HANDSHAKE;
		conn_state.nick_name = det_msg->get_host_nick();
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
		// set timeout
		if( does_include(msg_net_connection_state,connection) ) // already deleted?
		  conn_state.timer.start(response_timeout);
	      } else {
		// protocol mismatch => list supported protocols
		std::list<BGP::Protocol> supported_protocols;
		supported_protocols.push_back(BGP::Protocol("BGP",PROTOCOL_NUMBER,'a'));
		// send response
		BGP::Msg_List_Protocols msg(supported_protocols);
		connection->send_message(&msg);
		// set timeout
		if( does_include(msg_net_connection_state,connection) ) // already deleted?
		  conn_state.timer.start(response_timeout);
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_HANDSHAKE:
	{
	  conn_state.timer.stop(); // stop timeout
	  switch( message->get_type() )
	  {
	    case BGP::msg_get_rooms:
	    {
	      conn_state.state = BGP_CHOOSE_ROOM;
	      std::list<BGP::Room> rooms;
	      rooms.push_back(BGP::Room("standard room",get_client_count(),true,get_game_phase()));
	      // send response
	      BGP::Msg_Tell_Rooms msg(rooms);
	      connection->send_message(&msg);
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_CHOOSE_ROOM:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_choose_room:
	    {
	      BGP::Msg_Choose_Room *det_msg = static_cast<BGP::Msg_Choose_Room*>(message);
	      BGP::Room room = det_msg->get_room();
	      if(room.name == "standard room")
	      {
		conn_state.state = BGP_INFO;
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
	      }
	      else
	      {
#ifndef __WXMSW__
		std::cerr << "Server Error - Invalid room name in:" << std::endl;
#endif
		invalid = true;
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_INFO:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_get_phase:
	    {
	      // send response
	      BGP::Msg_Tell_Phase msg(get_game_phase());
	      connection->send_message(&msg);
	      break;
	    }
	    case BGP::msg_get_game:
	    {
	      // send response
	      BGP::Msg_Tell_Game msg(GAME_NAME);
	      connection->send_message(&msg);
	      break;
	    }
	    case BGP::msg_get_setup:
	    {
	      // not necessary for BGP1.0Xa!
	      // send response
	      BGP::Msg_Deny msg;
	      connection->send_message(&msg);
	      break;
	    }
	    case BGP::msg_get_situation:
	    {
	      // not necessary for BGP1.0Xa!
	      // send response
	      BGP::Msg_Deny msg;
	      connection->send_message(&msg);
	      break;
	    }
	    case BGP::msg_request_final_setup:
	    {
	      conn_state.final_setup = true;
	      break;
	    }
	    case BGP::msg_request_move_reminder:
	    {
	      conn_state.move_reminder = true;
	      break;
	    }
	    case BGP::msg_take_player:
	    {
	      BGP::Msg_Take_Player *det_msg = static_cast<BGP::Msg_Take_Player*>(message);
	      int player_id = det_msg->get_id();
	      // just allow former owner to take players back
	      if(does_include(conn_state.abandoned_player_ids,player_id))
	      {
		player_id_connection[player_id] = connection;
		conn_state.controlled_player_ids.insert(player_id);
		abandoned_player_ids.erase(player_id);
		conn_state.abandoned_player_ids.erase(player_id);
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
	      }
	      else
	      {
		// send response
		BGP::Msg_Deny msg;
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_setup:
	    {
	      if( get_game_phase() == BGP::phase_setup )
	      {
		conn_state.state = BGP_SETUP;
		conn_state.in_init = false;
		conn_state.phase = BGP::phase_setup;
		++clients_in_setup;
	      } 
	      else
	      {
		// report changed phase
		// send response
		BGP::Msg_Tell_Phase msg(get_game_phase());
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_get_moves_and_play:
	    {
	      if( get_game_phase() == BGP::phase_playing )
	      {
		// adapt state
		conn_state.in_init = false;
		conn_state.phase = BGP::phase_playing;
		conn_state.state = BGP_OTHERS_TURN;
		if( display_phase == DISPLAY_PLAYING && game.players.size() )
		  if( does_include(conn_state.controlled_player_ids,game.get_current_player().id) )
		    conn_state.state = BGP_HIS_TURN;
		// send response
		if( conn_state.final_setup )
		{
		  BGP::Setup setup;
		  setup.ruleset = *game.ruleset;
		  setup.initial_players = vector_to_list(game.players);
		  while(setup.initial_players.size() > game.get_max_players())
		    setup.initial_players.pop_back();
		  while(setup.initial_players.size() < game.get_min_players())
		    setup.initial_players.push_back(Player(long_to_string(setup.initial_players.
									  size()+1),
							   setup.initial_players.size()+10000));
		  setup.current_players = players;
		  if( game.is_out_of_order() )
		    {
		      BGP::Msg_Tell_Player_Setup msg1(setup);
		      BGP::Msg_Tell_Player_Moves msg2(game.get_played_moves());
		      connection->send_message(&msg1);
		      connection->send_message(&msg2);
		    }
		  else
		    {
		      BGP::Msg_Tell_Setup msg1(setup);
		      BGP::Msg_Tell_Moves msg2(game.get_played_moves());
		      connection->send_message(&msg1);
		      connection->send_message(&msg2);
		    }
		}
	      } 
	      else
	      {
		// report changed phase
		// send response
		BGP::Msg_Tell_Phase msg(get_game_phase());
		connection->send_message(&msg);
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_SETUP:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_add_player:
	    {
	      BGP::Msg_Add_Player *det_msg = static_cast<BGP::Msg_Add_Player*>(message);
	      if( players.size() < game.get_max_players() && !is_ready )
	      {
		// accept
		Player player = det_msg->get_player();
		int id = do_add_player(player, connection);
		// send response
		BGP::Msg_Accept_Player msg(id);
		connection->send_message(&msg);
	      }
	      else
	      {
		// deny
		// send response
		BGP::Msg_Deny msg;
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_remove_player:
	    {
	      BGP::Msg_Remove_Player *det_msg = static_cast<BGP::Msg_Remove_Player*>(message);
	      int player_id = det_msg->get_id();
	      if( !does_include(player_id_connection,player_id) )
	      {
		// send response
		BGP::Msg_Deny msg;
		connection->send_message(&msg);
	      }
	      else if( player_id_connection[player_id] == connection && !is_ready )
	      {
		do_remove_player(player_id);
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
	      }
	      else
	      {
		// send response
		BGP::Msg_Deny msg;
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_ready:
	    {
	      if(!conn_state.is_ready)
	      {
		conn_state.is_ready = true;
		++clients_ready;
		check_all_ready();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_OTHERS_TURN:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_move:
	    {
	      if( display_phase != DISPLAY_PLAYING &&
		  !conn_state.in_init && conn_state.phase == BGP::phase_playing )
	      {
		deferred_messages.push_back(std::pair<Message_Network<BGP::Message>*,BGP::Message*>
					    (connection,message));
#ifndef __WXMSW__
		std::cerr << "Server: deferred message" << std::endl;
#endif
	      }
	      else
	      {
		BGP::Msg_Move *det_msg = static_cast<BGP::Msg_Move*>(message);
		if(!process_move(det_msg->get_move(), connection, det_msg->get_player_id()))
		  invalid = true;
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_HIS_TURN:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_move:
	    {
	      BGP::Msg_Move *det_msg = static_cast<BGP::Msg_Move*>(message);
	      if(!process_move(det_msg->get_move(), connection, det_msg->get_player_id())) 
		invalid = true;
	      break;
	    }
	    case BGP::msg_ask_new_game:
	    {
	      if(!do_ask_new_game(connection)) 
		invalid=true;
	      break;
	    }
	    case BGP::msg_ask_undo:
	    {
	      BGP::Msg_Ask_Undo *det_msg = static_cast<BGP::Msg_Ask_Undo*>(message);
	      if(!do_ask_undo(det_msg->get_n(),connection)) 
		invalid=true;
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ACCEPT_NEW_GAME:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      conn_state.state = BGP_OTHERS_TURN;
	      if( !asking_new_game || clients_asked == 0 ) invalid = true;
	      else
	      {
		--clients_asked;
		check_all_answered();
	      }
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      conn_state.state = BGP_OTHERS_TURN;
	      if( !asking_new_game || clients_asked == 0 ) invalid = true;
	      else
	      {
		answer = false;
		--clients_asked;
		check_all_answered();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ACCEPT_UNDO:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      conn_state.state = BGP_OTHERS_TURN;
	      if( !asking_undo || clients_asked == 0 ) invalid = true;
	      else
	      {
		--clients_asked;
		check_all_answered();
	      }
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      conn_state.state = BGP_OTHERS_TURN;
	      if( !asking_undo || clients_asked == 0 ) invalid = true;
	      else
	      {
		answer = false;
		--clients_asked;
		check_all_answered();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ERROR:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_get_phase:
	    {
	      // reintegrate connection
	      conn_state.state = BGP_INFO;
	      // send response
	      BGP::Msg_Tell_Phase msg(get_game_phase());
	      connection->send_message(&msg);
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	default: invalid = true; break;
      }
    }
    else
    {
      invalid = true;
    }
    //------------------------------
    // error handling:
    if( invalid )
    {
#ifndef __WXMSW__
      std::cerr << "Server Error - Invalid network message from " << conn_state.name << " in state " 
		<< to_string(conn_state.state) << ": ";
      if( message )
	message->print(std::cerr);
      else
	std::cerr << "<unrecognized message>" << std::endl;
#endif
      on_error(connection);	// handle error
    }
  }
  // is called when connection is established (for Message_Network_Client only)
  void Network_Manager_BGP100a_Server::on_connect( Message_Network<BGP::Message>* /*connection*/ )
  {
    assert(false);		// shouldn't happen for server
  }
  // is called when connection was closed or couldn't be established
  void Network_Manager_BGP100a_Server::on_lost( Message_Network<BGP::Message> *connection )
  {
    if( !does_include(msg_net_connection_state,connection) ) return; // already cleaned up?

#ifndef __WXMSW__
    Connection_State &conn_state = msg_net_connection_state[connection];
    std::cerr << "Server Error - Connection Lost to " << conn_state.name << std::endl;
#endif
    disconnect(connection,false/*don't send disconnect message*/);
  }
  // is called when an error occured
  void Network_Manager_BGP100a_Server::on_error( Message_Network<BGP::Message> *connection )
  {
    if( !does_include(msg_net_connection_state,connection) ) return;  // disconnected?

    Connection_State &conn_state = msg_net_connection_state[connection];
#ifndef __WXMSW__
    std::cerr << "Server Error - Protocol error in connection with " << conn_state.name << std::endl;
#endif
    switch(conn_state.state)
    {
      case BGP_UNCONNECTED:
	break;
      case BGP_CONNECTED:
      case BGP_HANDSHAKE:
      case BGP_CHOOSE_ROOM:
	conn_state.state = BGP_UNCONNECTED;
	disconnect(connection);
	break;
      default:
      {
	// discard all influence of connection
	discard(connection);
	// reset client state
	conn_state.final_setup = false;
	conn_state.move_reminder = false;
	conn_state.in_init = true;
	conn_state.is_ready = false;
	conn_state.phase = BGP::phase_setup;
	// set error state
	conn_state.state = BGP_ERROR;
	// send response
	BGP::Msg_Error msg;
	connection->send_message(&msg);
	// check possibly adapted counters
	if( game_phase == BGP::phase_setup )
	  check_all_ready();
	else
	  check_all_answered();
	break;
      }
    }
  }

  //-------------------------------------------------------
  //-------------------------------------------------------
  // commands inherited from Message_Network_Server_Handler
  //-------------------------------------------------------
  //-------------------------------------------------------

  // specifics: message_network must be deleted!
  // ->set_handler() should be called
  void Network_Manager_BGP100a_Server::new_connection( Message_Network<BGP::Message> *connection )
  {
#ifndef __WXMSW__
    std::cerr << "Server - New Connection!" << std::endl;
#endif
    if( disable_new_connections || get_client_count() >= max_connections )
    {
#ifndef __WXMSW__
      std::cerr << "Server - Connection denied!" << std::endl;
#endif
      delete connection;
      return;
    }
    Connection_State &conn_state = msg_net_connection_state[connection];// this inserts automatically
    conn_state.id = current_id++;
    conn_state.state = BGP_CONNECTED;
    conn_state.name 
      = connection->get_remote_host() + ":" + long_to_string(connection->get_remote_port());
    id_connection[conn_state.id] = connection;
    if( connection_handler )
      connection_handler->new_connection(conn_state.name,conn_state.id);
    if( does_include(msg_net_connection_state,connection) ) // already deleted?
    {
      // set timeout for response
      conn_state.timer.init(this,connection);
      conn_state.timer.start(response_timeout);
      // register as handler (this may cause immediate reception of data!)
      connection->set_handler(this);
      // wait for helo
    }
  }

  //----------------
  //----------------
  // event handlers
  //----------------
  //----------------
    
  void Network_Manager_BGP100a_Server::on_timer(wxTimerEvent& WXUNUSED(event))
  {
    if( delayed_move_visualization )
    {
      ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE, ANIMATION_DONE );
      // triggers continue_game() after done
    }
  }
    
  void Network_Manager_BGP100a_Server::on_done(wxTimerEvent& WXUNUSED(event))
  {
    continue_game();
  }

  void Network_Manager_BGP100a_Server::on_connection_timer
  ( Message_Network<BGP::Message> *connection )
  {
#ifndef __WXMSW__
    std::cerr << "Server Error - Timeout triggered" << std::endl;
#endif
    Connection_State &conn_state = msg_net_connection_state[connection];
    switch(conn_state.state) 
    {
      case BGP_CONNECTED:
      case BGP_HANDSHAKE:
      {
	conn_state.state = BGP_UNCONNECTED;
	disconnect(connection);
	break;
      }
      default:
      {
	break;
      }
    }
  }

  //------------------
  //------------------
  // private functions
  //------------------
  //------------------

  std::string Network_Manager_BGP100a_Server::to_string(Protocol_State state)
  {
    switch(state) {
      case BGP_UNCONNECTED: return "UNCONNECTED"; 
      case BGP_CONNECTED: return "CONNECTED"; 
      case BGP_HANDSHAKE: return "HANDSHAKE"; 
      case BGP_CHOOSE_ROOM: return "CHOOSE_ROOM";
      case BGP_INFO: return "INFO"; 
      case BGP_SETUP: return "SETUP"; 
      case BGP_ADD_PLAYER: return "ADD_PLAYER"; 
      case BGP_REMOVE_PLAYER: return "REMOVE_PLAYER"; 
      case BGP_READY: return "READY"; 
      case BGP_OTHERS_TURN: return "OTHERS_TURN"; 
      case BGP_HIS_TURN: return "HIS_TURN"; 
      case BGP_ASK_UNDO: return "ASK_UNDO"; 
      case BGP_ACCEPT_UNDO: return "ACCEPT_UNDO"; 
      case BGP_ASK_NEW_GAME: return "ASK_NEW_GAME"; 
      case BGP_ACCEPT_NEW_GAME: return "ACCEPT_NEW_GAME";
      case BGP_ERROR: return "ERROR";
    }
    assert(false);
    return "<invalid>";
  }

  //------------------------------------------------
  // constructors and operators of Connection_State

  Network_Manager_BGP100a_Server::Connection_State::Connection_State() 
    : private_timer(new Timer()), disconnecting(false), final_setup(false),
      move_reminder(false), in_init(true), is_ready(false),
      phase(BGP::phase_setup), timer(*private_timer)
  {
  }
  Network_Manager_BGP100a_Server::Connection_State::Connection_State(const Connection_State &s) 
    : private_timer(new Timer()), id(s.id), controlled_player_ids(s.controlled_player_ids),
      state(s.state), name(s.name), nick_name(s.nick_name), disconnecting(s.disconnecting),
      final_setup(s.final_setup), move_reminder(s.move_reminder), in_init(s.in_init), 
      is_ready(s.is_ready), phase(s.phase), timer(*private_timer) 
  {
  }
  Network_Manager_BGP100a_Server::Connection_State::~Connection_State() 
  { 
    delete private_timer; 
  }
  Network_Manager_BGP100a_Server::Connection_State&
  Network_Manager_BGP100a_Server::Connection_State::operator=(const Connection_State &s)
  { 
    id=s.id;controlled_player_ids=s.controlled_player_ids;state=s.state;name=s.name;
    nick_name=s.nick_name;disconnecting=s.disconnecting;final_setup=s.final_setup;
    move_reminder=s.move_reminder;in_init=s.in_init;is_ready=s.is_ready;phase=s.phase;
    return *this; 
  }

  //------------------------------------------------
  // diverse support functions

  void Network_Manager_BGP100a_Server::continue_game()
  {
    game_manager.continue_game();
  }

  void Network_Manager_BGP100a_Server::discard(Message_Network<BGP::Message>* connection)
  {
    Connection_State &conn_state = msg_net_connection_state[connection];

    bool in_setup = false;
    if( game_phase == BGP::phase_setup )
    {
      if( !conn_state.in_init && conn_state.phase == BGP::phase_setup )
      {
	in_setup = true;
	--clients_in_setup;
	if( conn_state.is_ready )
	  --clients_ready;
	check_all_ready();
      }
    } 
    else
    {
      if( conn_state.state == BGP_ACCEPT_NEW_GAME || 
	  conn_state.state == BGP_ACCEPT_UNDO )
      {
	--clients_asked;
	check_all_answered();
      }
    }

    std::set<int> player_ids = conn_state.controlled_player_ids;
    for(std::set<int>::iterator it = player_ids.begin(); it != player_ids.end(); ++it )
    {
      int id = *it;

      if( in_setup && !is_ready )
      {
	do_remove_player(id);
      }
      else
      {
#ifndef __WXMSW__
	std::cerr << "Server: Treat player as abandoned " << id << std::endl;
#endif
	player_id_connection.erase(id);
	conn_state.abandoned_player_ids.insert(id);
	abandoned_player_ids.insert(id);
      }
    }
    conn_state.controlled_player_ids.clear();
  }

  void Network_Manager_BGP100a_Server::disconnect(Message_Network<BGP::Message>* connection,
						  bool send_disconnect)
  {
    Connection_State &conn_state = msg_net_connection_state[connection];
    if( conn_state.disconnecting ) return; // already disconnecting
    conn_state.disconnecting = true;

    if( connection_handler )
      connection_handler->closed_connection(conn_state.id);

    if( send_disconnect )
    {
      BGP::Msg_Disconnect msg;
      connection->send_message(&msg);
      connection->flush();
    }

    discard(connection);
    
    msg_net_connection_state.erase(connection);
    id_connection.erase(conn_state.id);
    connection->set_handler(0);	// disable message reception from connection
    delete connection;
    
    // check possibly adapted counters
    if( game_phase == BGP::phase_setup )
      check_all_ready();
    else
      check_all_answered();
  }

  void Network_Manager_BGP100a_Server::broadcast
  ( BGP::Message* message, BGP::Phase_Type phase, 
    bool check_phase, Message_Network<BGP::Message>* skip_connection )
  {
    // copy connections to ensure a stable state when sending messages
    std::list<Message_Network<BGP::Message>*> connections;
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      if( it->first )
	connections.push_back(it->first);
    }
    // send messages
    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
    {
      Message_Network<BGP::Message> *connection = *conn_it;
      if( connection != skip_connection )
      {
	Connection_State &conn_state = msg_net_connection_state[connection];
	if( conn_state.in_init ) continue;  // initialization not finished
	if( !check_phase || conn_state.phase == phase )	 
	{
	  connection->send_message(message);
	}
      }
    }
  }

  void Network_Manager_BGP100a_Server::start_timer(int milliseconds)
  {
    timer.Start(milliseconds,wxTIMER_ONE_SHOT);
  }

  void Network_Manager_BGP100a_Server::stop_timer()
  {
    timer.Stop();
  }

  unsigned Network_Manager_BGP100a_Server::get_client_count()
  {
    return (unsigned)msg_net_connection_state.size();
  }

  BGP::Phase_Type Network_Manager_BGP100a_Server::get_game_phase()
  {
    return game_phase;
  }

  // actually add player and return ID
  int Network_Manager_BGP100a_Server::do_add_player( const Player& player,
						     Message_Network<BGP::Message>* connection )
  {
    std::list<Player>::iterator p = players.insert(players.end(), player);
    p->id = current_id; ++current_id;
    id_player[p->id] = p;
    if( connection )
    {
      Connection_State &conn_state = msg_net_connection_state[connection];
      p->host = conn_state.name;
      p->origin = Player::remote;
      conn_state.controlled_player_ids.insert(p->id);
      player_id_connection[p->id] = connection;
    }
    else
    {
      p->origin = Player::local;
      own_player_ids.insert(p->id);
    }
		
    if( display_handler && display_phase == DISPLAY_SETUP )
    {
      display_handler->player_added(*p);
    }

#ifndef __WXMSW__
    std::cerr << "Server: player added " << players.size() << std::endl;
#endif

    // other protocol versions have to tell connected clients about added player

    return p->id;
  }

  // actually remove player
  void Network_Manager_BGP100a_Server::do_remove_player( int player_id )
  {
    assert(does_include(id_player,player_id));
    if( display_handler && display_phase == DISPLAY_SETUP )
      display_handler->player_removed(*id_player[player_id]);

    if( does_include(player_id_connection,player_id) )
    {
      Connection_State &conn_state = msg_net_connection_state[player_id_connection[player_id]];
      conn_state.controlled_player_ids.erase(player_id);
      player_id_connection.erase(player_id);
    }
    if( does_include(own_player_ids,player_id) )
    {
      own_player_ids.erase(player_id);
    }
    players.erase(id_player[player_id]);
    // reload iterators due to erase
    id_player.clear();
    std::list<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }
		
#ifndef __WXMSW__
    std::cerr << "Server: player removed " << players.size() << std::endl;
#endif

    // other protocol versions have to tell connected clients about removed player
  }

  bool Network_Manager_BGP100a_Server::do_player_up( int player_id )
  {
    std::list<Player>::iterator player_iterator = id_player[player_id];
    if( player_iterator == players.begin() ) // is first player
      return false;

    std::list<Player>::iterator player_iterator2 = player_iterator; --player_iterator2;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator->id]  = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler && display_phase == DISPLAY_SETUP )
      display_handler->player_up(*player_iterator2);

    // other protocol versions have to tell connected clients about player change

    return true;
  }

  bool Network_Manager_BGP100a_Server::do_player_down( int player_id )
  {
    std::list<Player>::iterator player_iterator  = id_player[player_id];
    std::list<Player>::iterator player_iterator2 = player_iterator; ++player_iterator2;

    if( player_iterator2 == players.end() ) // is last player
      return false;

    Player p1 = *player_iterator;
    *player_iterator  = *player_iterator2;
    *player_iterator2 = p1;
    
    id_player[player_iterator ->id] = player_iterator;
    id_player[player_iterator2->id] = player_iterator2;

    if( display_handler && display_phase == DISPLAY_SETUP )
      display_handler->player_down(*player_iterator2);

    // other protocol versions have to tell connected clients about player change

    return true;
  }

  bool Network_Manager_BGP100a_Server::do_ask_new_game
  ( Message_Network<BGP::Message>* asking_connection )
  {
    if( asking_new_game || asking_undo ) 
    {
      if( asking_connection != asking_client )
      {
	BGP::Msg_Deny msg;
	asking_connection->send_message(&msg);
	return true;
      }
      else
	return false;
    }

    std::string ask_name;
    asking_client = asking_connection;
    asking_done = false;
    clients_asked = 1;		// includes the server itself
    asking_new_game = true;
    answer = true;		// default answer (wired-and)

    if( asking_client )
    {
      Connection_State &conn_state = msg_net_connection_state[asking_client];
      ask_name = conn_state.name;
    }
    BGP::Msg_Ask_New_Game msg(ask_name);

    std::list<Message_Network<BGP::Message>*> connections;
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      if( it->first )
	connections.push_back(it->first);
    }
    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
    {
      Message_Network<BGP::Message>* connection = *conn_it;
      if( connection == asking_client ) continue;
      Connection_State &conn_state = msg_net_connection_state[connection];
      if( !conn_state.in_init && conn_state.phase == BGP::phase_playing &&
	  conn_state.state == BGP_OTHERS_TURN &&
	  !conn_state.controlled_player_ids.empty() )  // don't ask viewing only clients
      {
	++clients_asked;
	conn_state.state = BGP_ACCEPT_NEW_GAME;
	connection->send_message(&msg);
      }
    }
    asking_done = true;
    // ask server user
    if( asking_client )
    {
      if( display_handler )
      {
	if( display_handler->ask_new_game(str_to_wxstr(ask_name)) )
	{
	  --clients_asked;	// received answer of this user
	}
	else
	{
	  --clients_asked;	// received answer of this user
	  answer=false;		// new game denied
	}
      }
      else
      {
	--clients_asked;	// not asked
      }
    } 
    else
    {
      --clients_asked;		// server sent the request
    }
    check_all_answered();
    return true;
  }

  bool Network_Manager_BGP100a_Server::do_ask_undo
  ( int n, Message_Network<BGP::Message>* asking_connection )
  {
    if( asking_new_game || asking_undo ) 
    {
      if( asking_connection != asking_client )
      {
	BGP::Msg_Deny msg;
	asking_connection->send_message(&msg);
	return true;
      }
      else
	return false;
    }

    std::string ask_name;
    asking_client = asking_connection;
    asking_n = n;
    asking_done = false;
    clients_asked = 1;		// includes the server itself
    asking_undo = true;
    answer = true;		// default answer (wired-and)

    if( asking_client )
    {
      Connection_State &conn_state = msg_net_connection_state[asking_client];
      ask_name = conn_state.name;
    }
    BGP::Msg_Ask_Undo msg(ask_name, n);

    std::list<Message_Network<BGP::Message>*> connections;
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      if( it->first )
	connections.push_back(it->first);
    }
    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
    {
      Message_Network<BGP::Message>* connection = *conn_it;
      if( connection == asking_client ) continue;
      Connection_State &conn_state = msg_net_connection_state[connection];
      if( !conn_state.in_init && conn_state.phase == BGP::phase_playing &&
	  conn_state.state == BGP_OTHERS_TURN && 
	  !conn_state.controlled_player_ids.empty() )  // don't ask viewing only clients
      {
	++clients_asked;
	conn_state.state = BGP_ACCEPT_UNDO;
	connection->send_message(&msg);
      }
    }
    asking_done = true;
    // ask server user
    if( asking_client )
    {
      if( display_handler )
      {
	if( display_handler->ask_undo_moves(str_to_wxstr(ask_name),n) )
	{
	  --clients_asked;	// received answer of this user
	}
	else
	{
	  --clients_asked;	// received answer of this user
	  answer=false;		// undo denied
	}
      }
      else
      {
	--clients_asked;	// not asked
      }
    } 
    else
    {
      --clients_asked;		// server sent the request
    }
    check_all_answered();
    return true;
  }

  // check whether all clients are ready 
  void Network_Manager_BGP100a_Server::check_all_ready()
  {
#ifndef __WXMSW__
    std::cerr << "Server - Clients ready: " << clients_ready << "/" << clients_in_setup 
	      << std::endl;
#endif
    assert( clients_ready <= clients_in_setup );
    if( clients_ready == clients_in_setup )
    {
      game_phase = BGP::phase_playing;
      setup_players();
      assert(game.ruleset);
      BGP::Setup setup;
      setup.ruleset = *game.ruleset;
      setup.init_moves = game.get_played_moves();
      setup.initial_players = vector_to_list(game.players);
      while(setup.initial_players.size() > game.get_max_players())
	setup.initial_players.pop_back();
      while(setup.initial_players.size() < game.get_min_players())
	setup.initial_players.push_back(Player(long_to_string(setup.initial_players.size()+1),
					       setup.initial_players.size()+10000));
      setup.current_players = players;
      // send final setup to all clients in setup phase that requested it
      BGP::Message *msg;
      if( game.is_out_of_order() )
	msg = new BGP::Msg_Tell_Player_Setup(setup);
      else
	msg = new BGP::Msg_Tell_Setup(setup);
      bool done;
      do{
	// extract current connections since the connections can change (iterators!)
	std::list<Message_Network<BGP::Message>*> connections;
	std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
	for( it=msg_net_connection_state.begin();
	     it!=msg_net_connection_state.end(); ++it )
	{
#ifndef __WXMSW__
	  // !!! Debug output
	  std::cerr << "setup->playing for connection: " << it->first << std::endl; 
#endif
	  if( it->first )
	    connections.push_back(it->first);
	}
	std::list<Message_Network<BGP::Message>*>::iterator conn_it;
	for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
	{
	  Message_Network<BGP::Message>* connection = *conn_it;
	  Connection_State &conn_state = msg_net_connection_state[connection];
	  if(!conn_state.in_init)
	  {
	    assert(conn_state.phase==BGP::phase_setup);
	    conn_state.phase = BGP::phase_playing;
	    conn_state.state = BGP_OTHERS_TURN;
	    if( conn_state.final_setup )
	      connection->send_message(msg);
	  }
	}
	done = true;
	// maybe new connections just joined setup phase
	for( it=msg_net_connection_state.begin();
	     it!=msg_net_connection_state.end(); ++it )
	{
	  Message_Network<BGP::Message>* connection = it->first;
	  Connection_State &conn_state = msg_net_connection_state[connection];
	  if( !conn_state.in_init && conn_state.phase == BGP::phase_setup )
	  {
#ifndef __WXMSW__
	    // !!! Debug output
	    std::cerr << "reiterating transitioning connections to playing phase" << std::endl; 
#endif
	    done = false;
	    break;
	  }
	}
      }while(!done);
      delete msg;

      if( display_handler && display_phase == DISPLAY_SETUP )
	display_handler->everything_ready();
    }
  }   

  // check whether all clients answered to a blocking request (new game/undo)
  bool Network_Manager_BGP100a_Server::check_all_answered(bool answer_local)
  {
    if( !asking_new_game && !asking_undo ) return false;
#ifndef __WXMSW__
    std::cerr << "Server - Remaining answers: " << clients_asked << " " << asking_new_game 
	      << "/" << asking_undo << std::endl;
#endif
    if( asking_done && clients_asked==0 )
    {
      if( asking_new_game )
      {
	asking_new_game = false;
	// answer remote requester
	if( asking_client )	// remote requester?
	{
	  if( answer )
	  {
	    Connection_State &conn_state = msg_net_connection_state[asking_client];
	    conn_state.phase = BGP::phase_setup;
	    conn_state.state = BGP_SETUP;
	    conn_state.is_ready = false;
	    BGP::Msg_Accept msg;
	    asking_client->send_message(&msg);
	  }
	  else
	  {
	    BGP::Msg_Deny msg;
	    asking_client->send_message(&msg);
	  }
	}
	// broadcast conclusions
	if( answer )
	{
	  // broadcast new game message and update state
	  BGP::Msg_New_Game msg;
	  bool done;
	  do{
	    // extract current connections since the connections can change (iterators!)
	    std::list<Message_Network<BGP::Message>*> connections;
	    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
	    for( it=msg_net_connection_state.begin();
		 it!=msg_net_connection_state.end(); ++it )
	    {
#ifndef __WXMSW__
	      // !!! Debug output
	      std::cerr << "playing->setup for connection: " << it->first << std::endl; 
#endif
	      if( it->first )
		connections.push_back(it->first);
	    }
	    std::list<Message_Network<BGP::Message>*>::iterator conn_it;
	    for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
	    {
	      Message_Network<BGP::Message>* connection = *conn_it;
	      Connection_State &conn_state = msg_net_connection_state[connection];
	      if(!conn_state.in_init && connection != asking_client)
	      {
		assert(conn_state.phase==BGP::phase_playing);
		conn_state.phase = BGP::phase_setup;
		conn_state.state = BGP_SETUP;
		conn_state.is_ready = false;
		connection->send_message(&msg);
	      }
	    }
	    done = true;
	    // maybe new connections joined playing phase
	    for( it=msg_net_connection_state.begin();
		 it!=msg_net_connection_state.end(); ++it )
	    {
	      //Message_Network<BGP::Message>* connection = it->first;
	      Connection_State &conn_state = it->second;
	      if( !conn_state.in_init && conn_state.phase == BGP::phase_playing )
	      {
#ifndef __WXMSW__
		// !!! Debug output
		std::cerr << "reiterating transitioning connections to setup phase" << std::endl; 
#endif
		done = false;
		break;
	      }
	    }
	  }while(!done);
	}
	// answer local requester and start new game
	if( !asking_client )	// local requester?
	{
	  if( answer_local )
	  {
	    if( answer )
	      game_manager.new_game_accepted();  // calls force_new_game
	    else
	      game_manager.new_game_denied();
	  }
	}
	else			// remote requester
	{
	  if( answer )
	  {
	    possibly_interrupted_connection = asking_client;
	    game_manager.force_new_game();
	  }
	}
      }
      if( asking_undo )
      {
	asking_undo = false;
	assert(game.players.size());
	// apply undo
	if( answer )
	{
	  for(int i=0; i<asking_n; ++i)
	    if(!game.undo_move())
	      break;
	}
	int id = game.get_current_player().id;
	// answer remote requester
	if( asking_client )	// remote requester?
	{
	  if( answer )
	  {
	    Connection_State &conn_state = msg_net_connection_state[asking_client];
	    if( does_include(conn_state.controlled_player_ids,id) )
	      conn_state.state = BGP_HIS_TURN;
	    else
	      conn_state.state = BGP_OTHERS_TURN;
	    BGP::Msg_Accept msg;
	    asking_client->send_message(&msg);
	  }
	  else
	  {
	    BGP::Msg_Deny msg;
	    asking_client->send_message(&msg);
	  }
	}
	// broadcast conclusions
	if( answer )
	{
	  // broadcast new game message and update state
	  BGP::Msg_Undo msg(asking_n);
	  // extract current connections since the connections can change (iterators!)
	  std::list<Message_Network<BGP::Message>*> connections;
	  std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
	  for( it=msg_net_connection_state.begin();
	       it!=msg_net_connection_state.end(); ++it )
	  {
	    if( it->first )
	      connections.push_back(it->first);
	  }
	  std::list<Message_Network<BGP::Message>*>::iterator conn_it;
	  for( conn_it=connections.begin(); conn_it!=connections.end(); ++conn_it )
	  {
	    Message_Network<BGP::Message>* connection = *conn_it;
	    Connection_State &conn_state = msg_net_connection_state[connection];
	    if(!conn_state.in_init && connection != asking_client)
	    {
	      // update state
	      assert(conn_state.phase==BGP::phase_playing);
	      assert(conn_state.state==BGP_OTHERS_TURN);
	      if( does_include(conn_state.controlled_player_ids,id) )
		conn_state.state = BGP_HIS_TURN;
	      else
		conn_state.state = BGP_OTHERS_TURN;
	      // send message
	      connection->send_message(&msg);
	    }
	  }
	}
	// answer local requester and undo move on display
	if( !asking_client )	// local requester?
	{
	  if( answer_local )
	  {
	    if( answer )
	      game_manager.undo_accepted(); // updates display (animation)
	    else
	      game_manager.undo_denied();
	  }
	}
	else			// remote requester
	{
	  if( answer )
	  {
	    game_manager.do_undo_moves(asking_n);
	  }
	}
      }
      return true;
    }
    return false;
  }

  void Network_Manager_BGP100a_Server::setup_players()
  {
    std::list<Player>::iterator it;
    for( it=players.begin(); it!=players.end(); ++it )
    {
      Player &player = *it;
      if( does_include(own_player_ids,player.id) )
      {
	Player_Input *input;
	if( player.type == Player::ai )
	  input = game_manager.get_player_ai();
	else
	  input = ui_manager.get_user_input();
		  
	player.input = input;
	player.origin = Player::local;
		  
	std::list<Player_Output*> outlist;
	outlist.push_back(this);
	player.outputs = outlist;
      }
      else
      {
	player.origin = Player::remote;
	player.input = this;
      }
    }
  }

  void Network_Manager_BGP100a_Server::cleanup_players()
  {
    if( abandoned_player_ids.size() )
    {
      // discard all players
      players.clear();
      id_player.clear();
      player_id_connection.clear();
      abandoned_player_ids.clear();
      own_player_ids.clear();
      std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
      for( it=msg_net_connection_state.begin();
	   it!=msg_net_connection_state.end(); ++it )
      {
	//Message_Network<BGP::Message>* connection = it->first;
	Connection_State &conn_state = it->second;
	conn_state.abandoned_player_ids.clear();
	conn_state.controlled_player_ids.clear();
      }
    }
  }

  bool Network_Manager_BGP100a_Server::process_move
  ( const Move_Sequence& move_sequence, Message_Network<BGP::Message>* connection, int player_id, 
    bool delay_continue_game )
  {
    bool error = false;
    if( asking_new_game ) // only player who's turn it is is allowed to ask for new game
    {
#ifndef __WXMSW__
      std::cerr << "Server Error - Received move while asking for new game!" << std::endl;
#endif
      error = true;
    }
    if( asking_undo ) // only player who's turn it is is allowed to ask for undo
    {
#ifndef __WXMSW__
      std::cerr << "Server Error - Received move while asking for undo!" << std::endl;
#endif
      error = true;
    }
    if( connection == 0 )	// local move
    {
      assert( player_id == -1 || player_id == game.get_current_player().id );
      player_id = game.get_current_player().id;
      if( !does_include(own_player_ids,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Server Error - Received local move but it is not my turn!" << std::endl;
#endif
	error = true;
      }
    }
    else if( player_id != game.get_current_player().id && game.is_out_of_order() )
    {
      Connection_State &conn_state = msg_net_connection_state[connection];
      if( does_include(own_player_ids,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remote move for local player!" << std::endl;
#endif
	error = true;
      } 
      else if( !does_include(conn_state.controlled_player_ids,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remote move for player of different connection!" << std::endl;
#endif
	error = true;
      } 
      else if( does_include(pending_player_move,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Player already sent a move: " << player_id << "!" 
		  << std::endl;
#endif
	error = true;
      }
      else
      {
	pending_player_move[player_id] = std::make_pair(move_sequence,connection);
	return true;
      }
    }
    else 
    {
      Connection_State &conn_state = msg_net_connection_state[connection];
      
      if( player_id != game.get_current_player().id )
      {
#ifndef __WXMSW__
	std::cerr << "Server Error - Received remote move for player " << player_id 
		  << "but it is the turn of player " << game.get_current_player().id 
		  << "!" << std::endl;
#endif
	error = true;
      }
      if( does_include(own_player_ids,game.get_current_player().id) )
      {
#ifndef __WXMSW__
	std::cerr << "Server Error - Received remote move but it is my turn!" << std::endl;
#endif
	error = true;
      } 
      if( conn_state.state != BGP_HIS_TURN )
      {
#ifndef __WXMSW__
	std::cerr << "Server Error - Received remote move in wrong state!" << std::endl;
#endif
	error = true;
      }
      conn_state.state = BGP_OTHERS_TURN;  // his turn is received
    }
    if( !error && !move_sequence.check_sequence(game) )
    {
#ifndef __WXMSW__
      std::cerr << "Server Error - Invalid Move " << move_sequence << "!" << std::endl;
#endif
      error = true;
    }
    if( error )
    {
      if( connection )
      {
	return false;
      }
      else
      {
	// the server fooled itself, this should not happen! 
	// consider telling the user about this fatal error
	assert(false);
      }
    }
    else
    {
      // execute valid move
      game.do_move(move_sequence);
      int id = game.get_current_player().id;
      if( does_include(player_id_connection,id) )
	msg_net_connection_state[player_id_connection[id]].state = BGP_HIS_TURN;
      // distribute move
      BGP::Msg_Move msg(player_id, move_sequence);
      broadcast(&msg, BGP::phase_playing, /*check_phase=*/true, connection);
      // communicate move to local game if remote move
      if( connection )
      {
	if( awaiting_move && !sequence_available )
	{
	  sequence = move_sequence;
	  sequence_available = true;
	  awaiting_move = false;
	  if( delay_continue_game )
	  {
	    delayed_move_visualization = true;
	    start_timer(1);
	  }
	  else
	  {
	    ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE, ANIMATION_DONE );
	    // triggers continue_game() after done
	  }
	}
	else
	{
	  pending_moves.push_back(std::pair<int/*player_id*/,Move_Sequence>
				  (player_id,move_sequence));
	}
      }
    }
    return true;
  }

  void Network_Manager_BGP100a_Server::process_deferred_messages()
  {
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> > messages 
      = deferred_messages;
    deferred_messages.clear();
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> >::iterator it;
    for( it=messages.begin(); it!=messages.end(); ++it )
    {
      Message_Network<BGP::Message>* connection = it->first;
      BGP::Message* message = it->second;
      process_message(connection,message);
    }
  }

  BEGIN_EVENT_TABLE(Network_Manager_BGP100a_Server, wxEvtHandler)				
    EVT_TIMER(NETWORK_TIMER, Network_Manager_BGP100a_Server::on_timer)	//**/
    EVT_TIMER(ANIMATION_DONE, Network_Manager_BGP100a_Server::on_done)	//**/
  END_EVENT_TABLE()							//**/

//=================================================================================================
//=================================================================================================
//  Client
//=================================================================================================
//=================================================================================================

  Network_Manager_BGP100a_Client::Network_Manager_BGP100a_Client
  ( Game_Manager &game_manager, Game_UI_Manager &ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), 
      display_handler(0), 
      game( game_manager.get_game() ),
      timer(this,NETWORK_TIMER),
      msg_net_client(0),
      response_timeout(6000/*ms*/), 
      connection_lost(false),
      display_phase(DISPLAY_INIT), game_phase(BGP::phase_setup),
      sequence_available(false), awaiting_move(false), asking_new_game(false), 
      asking_undo(false), asking_n(0), error_recovery_attempts(0),
      delayed_move_visualization(false)
  {
    conn_state.state = BGP_UNCONNECTED;
    // self register
    game_manager.set_game_setup_manager( this );
  }

  Network_Manager_BGP100a_Client::~Network_Manager_BGP100a_Client()
  {
    close_connection();

    // self unregister
    game_manager.set_game_setup_manager( 0 );
  }

  //--------------------
  // connection commands
  //--------------------

  // returns true for success
  bool Network_Manager_BGP100a_Client::connect_to_server( std::string host, int port )
  {
    close_connection();
    conn_state.name = host + ":" + long_to_string(port);
    connection_lost = false;
    msg_net_client = new Message_Network_Client<BGP::Message>(host,port,this);
    if( connection_lost )
      return false;
    else
    {
      // setup timeout
      start_timer(response_timeout);

      return true;		// on_lost may be called later on failure
    }
  }

  void Network_Manager_BGP100a_Client::close_connection()
  {
    conn_state.state = BGP_UNCONNECTED;
    if( msg_net_client != 0 )
    {
      BGP::Msg_Disconnect msg;
      msg_net_client->send_message(&msg);
      msg_net_client->flush();
      msg_net_client->set_handler(0);
      delete msg_net_client;
      msg_net_client = 0;
    }
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Game_Setup_Manager
  //-------------------------------------------
  //-------------------------------------------

  Game_Setup_Manager::Type Network_Manager_BGP100a_Client::get_type()
  {
    return Game_Setup_Manager::client;
  }

  void Network_Manager_BGP100a_Client::set_display_handler( Game_Setup_Display_Handler* handler )
  {
    display_handler = handler;
  }

  //-------------------------------------------
  // board commands
  //-------------------------------------------
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_change_board(const Game& /*game*/)
  {
    return deny;		// this protocoll only allows the server to choose the board setup
  }
  const Game &Network_Manager_BGP100a_Client::get_board()
  {
    return game;
  }
  const std::list<Player> &Network_Manager_BGP100a_Client::get_players()
  {
    return players;
  }

  //-------------------------------------------
  // player commands
  //-------------------------------------------
  bool Network_Manager_BGP100a_Client::add_player( const Player& player )
  {
    if( !msg_net_client ) return false;
    if( !conn_state.in_setup || conn_state.state != BGP_SETUP )
      return false;

    conn_state.state = BGP_ADD_PLAYER;
    requested_add_player = player;
    // send request
    BGP::Msg_Add_Player msg(player);
    msg_net_client->send_message(&msg);
    return true;
  }
  bool Network_Manager_BGP100a_Client::remove_player( const Player& player )
  {
    if( !msg_net_client ) return false;
    if( !conn_state.in_setup || conn_state.state != BGP_SETUP )
      return false;
    
    conn_state.state = BGP_REMOVE_PLAYER;
    requested_remove_player_id = player.id;
    // send request
    BGP::Msg_Remove_Player msg(player.id);
    msg_net_client->send_message(&msg);
    return true;
  }
  bool Network_Manager_BGP100a_Client::player_up( const Player & )
  {
    return false;
  }
  bool Network_Manager_BGP100a_Client::player_down( const Player & )
  {
    return false;
  }

  // ready with adding players
  void Network_Manager_BGP100a_Client::ready()
  {
    if( !msg_net_client ) return;
    if( !conn_state.in_setup )
      return;			// !!! consider other error handling

    conn_state.is_ready = true;
    if( conn_state.state == BGP_SETUP )
    {
      conn_state.state = BGP_READY;
      BGP::Msg_Ready msg;
      msg_net_client->send_message(&msg);
    }
  }

  //-------------------------------------------
  // game commands
  //-------------------------------------------

  // returns players before feedback
  std::list<Player> Network_Manager_BGP100a_Client::enable_player_feedback() 
  {
    display_phase = DISPLAY_SETUP;
    return players;
  }

  // disables feedback about player changes
  void Network_Manager_BGP100a_Client::disable_player_feedback() 
  {
    if(display_phase == DISPLAY_SETUP)
      display_phase = DISPLAY_INIT;
  }

  // whether this player can choose a board to play
  bool Network_Manager_BGP100a_Client::can_choose_board() 
  {
    return false;
  }

  // whether the player setup can be entered
  bool Network_Manager_BGP100a_Client::can_enter_player_setup() 
  {
    return conn_state.in_setup;
  }

  // is everyone ready and number of players ok?
  Game_Setup_Manager::Game_State Network_Manager_BGP100a_Client::can_start()
  {
    if( game_phase == BGP::phase_playing )
      return everyone_ready;
    else
      return not_ready;
  }

  // call only when can_start() == true
  void Network_Manager_BGP100a_Client::start_prepared_game() // game has already players
  {
    if( display_phase != DISPLAY_PLAYING && display_handler )
    {
      // client enters an existing game
      display_handler->game_started();
    }

    assert( can_start() == everyone_ready );
    display_phase = DISPLAY_PLAYING;
    game_manager.set_board  ( game );
    if( does_include(own_player_ids,game.get_current_player().id) )
      conn_state.state = BGP_MY_TURN;

    game_manager.start_game();
    process_deferred_messages();
  }

  // call only when can_start() == true
  void Network_Manager_BGP100a_Client::start_game()
  {
    if( display_phase != DISPLAY_PLAYING && display_handler )
    {
      // client enters an existing game
      display_handler->game_started();
    }

    assert( can_start() == everyone_ready );
    display_phase = DISPLAY_PLAYING;
    game.set_players( list_to_vector(players) );
    game_manager.set_board  ( game );
    if( does_include(own_player_ids,game.get_current_player().id) )
      conn_state.state = BGP_MY_TURN;

    game_manager.start_game();
    process_deferred_messages();
  }

  // request to play new game
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_new_game()
  {
    if( !msg_net_client ) 
      return accept;
    if( conn_state.state != BGP_MY_TURN || asking_new_game || asking_undo )
    {
      return deny;		// only allow new game when its the players turn
    }
    // send request
    conn_state.state = BGP_ASK_NEW_GAME;
    asking_new_game = true;
    BGP::Msg_Ask_New_Game msg("");
    msg_net_client->send_message(&msg);
    return wait_for_answer;
  }
  // request to undo n half moves
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_undo_moves(int n)
  {
    if( !game.undo_possible ) return deny;
    if( !msg_net_client ) 
      return accept;
    if( conn_state.state != BGP_MY_TURN || asking_new_game || asking_undo )
    {
      return deny;		// only allow undo when its the players turn
    }
    // send request
    conn_state.state = BGP_ASK_UNDO;
    asking_undo = true;
    asking_n = n;
    BGP::Msg_Ask_Undo msg("",n);
    msg_net_client->send_message(&msg);
    return wait_for_answer;
  }
  // force new game (may close connections)
  void Network_Manager_BGP100a_Client::force_new_game(bool on_own)
  {
    // reset connection state
    conn_state.in_setup = true;
    conn_state.is_ready = false;
    conn_state.state = BGP_SETUP;
    // reset local state
    error_recovery_attempts = 0;  // new game new luck...
    sequence_available = false;
    awaiting_move = false;
    asking_new_game = false;
    asking_undo = false;
    display_phase = DISPLAY_INIT;
    game_phase = BGP::phase_setup;
    cleanup_players();
    // if on my own: disconnect
    if( on_own )
      close_connection();
    // open new game dialog
    if( display_handler )	// there should be a display handler
				// that may display a game setup
				// dialog
      display_handler->game_setup();
  }
  // stop game
  void Network_Manager_BGP100a_Client::stop_game()
  {
    game_manager.stop_game();
  }
  // game setup entered
  void Network_Manager_BGP100a_Client::game_setup_entered()
  {
  }
  // is it allowed to abort the game setup?
  bool Network_Manager_BGP100a_Client::is_allow_game_setup_abort()
  {
    return false;
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Input
  //-------------------------------------------
  //-------------------------------------------

  Player_Input::Player_State Network_Manager_BGP100a_Client::determine_move() throw(Exception)
  {
    if( !msg_net_client ) return Player_Input::wait_for_event;

    if( sequence_available ) 
    {
      assert( awaiting_move == false );
      return Player_Input::finished;
    }
    else
    {
      if( conn_state.state != BGP_OTHERS_TURN )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Move requested from network but it is a local player's turn" 
		  << std::endl;
#endif
	conn_state.state = BGP_ERROR;
	// send message
	BGP::Msg_Error msg;
	msg_net_client->send_message(&msg);
	// setup timeout
	start_timer(response_timeout);
	return Player_Input::wait_for_event;
      }
      else
      {
	assert( !sequence_available );
	if( pending_moves.size() )
	{
	  // todo: allow to check player id: pending_moves.front().first
	  sequence = pending_moves.front().second;
	  sequence_available = true;
	  pending_moves.pop_front();
	  return Player_Input::finished;
	}
	else if( does_include(pending_player_move,game.get_current_player().id) )
	{
	  sequence = pending_player_move[game.get_current_player().id];
	  pending_player_move.erase(game.get_current_player().id);
	  awaiting_move = true;
	  process_move(sequence,/*local=*/false,game.get_current_player().id,
		       /*delay_continue_game=*/true);
	  return Player_Input::wait_for_event;
	}
	else
	{
	  awaiting_move = true;
	  return Player_Input::wait_for_event;
	}
      }
    }
  }
  Move_Sequence Network_Manager_BGP100a_Client::get_move()
  {
    assert(sequence_available);
    sequence_available = false;
    return sequence;
  }
  long Network_Manager_BGP100a_Client::get_used_time()
  {
    return 1;			// !!! todo: measure time
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Output
  //-------------------------------------------
  //-------------------------------------------

  void Network_Manager_BGP100a_Client::report_move( const Move_Sequence &move_sequence )
  {
    process_move(move_sequence,true/*local*/);
  }

  //------------------------------------------------
  //------------------------------------------------
  // commands inherited from Message_Network_Handler
  //------------------------------------------------
  //------------------------------------------------

  // is called when message arrives
  // specifics: message must be deleted, message==0: invalid message arrived
  void Network_Manager_BGP100a_Client::process_message( Message_Network<BGP::Message> *connection, 
							BGP::Message *message )
  {
    if(connection != msg_net_client) return;  // disconnected?

    bool invalid = false;
    if( message )
    {
#ifndef __WXMSW__
      // !!! Debug output
      std::cerr << "Client received: "; message->print(std::cerr);
#endif

      //------------------------------
      // non state dependent messages:

      if( message->get_type() == BGP::msg_disconnect )
      {
	disconnect(false);
	return;
      }
      if( message->get_type() == BGP::msg_ping )
      {
	// send response
	BGP::Msg_Pong msg;
	connection->send_message(&msg);
	return;
      }
      if( message->get_type() == BGP::msg_chat )
      {
	// todo: relay and display chat message
	return;
      }

      //------------------------------
      // state dependent messages:

      switch(conn_state.state) 
      {
	case BGP_UNCONNECTED:
	{
	  invalid = true;
	  break;
	}
	case BGP_HANDSHAKE:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_list_protocols:
	    {
	      BGP::Msg_List_Protocols *det_msg = static_cast<BGP::Msg_List_Protocols*>(message);
	      std::list<BGP::Protocol> protocols = det_msg->get_protocols();
	      bool match = false;
	      // supported protocols: BGP 1.0Xa
	      for( std::list<BGP::Protocol>::iterator it=protocols.begin(); 
		   it!=protocols.end(); ++it )
	      {
		BGP::Protocol &protocol = *it;
		if( protocol.name == "BGP" && is_equal(protocol.number,PROTOCOL_NUMBER) &&
		    protocol.letter == 'a' )
		{
		  match = true;
		  // send response
		  BGP::Msg_Helo msg(protocol,connection->get_local_host());
		  connection->send_message(&msg);
		  // setup timeout
		  start_timer(response_timeout);
		  break;
		}
	      }
	      if( !match )
	      {
#ifndef __WXMSW__
		std::cerr << "Client Error - No protocol match found!" << std::endl;
#endif
		conn_state.state = BGP_UNCONNECTED;
		disconnect();
	      }
	      break;
	    }
	    case BGP::msg_accept:
	    {
	      conn_state.state = BGP_ROOMS;
	      // send response
	      BGP::Msg_Get_Rooms msg;
	      connection->send_message(&msg);
	      // setup timeout
	      start_timer(response_timeout);
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ROOMS:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_rooms:
	    {
	      BGP::Msg_Tell_Rooms *det_msg = static_cast<BGP::Msg_Tell_Rooms*>(message);
	      std::list<BGP::Room> rooms = det_msg->get_rooms();
	      bool match = false;
	      // search standard room
	      for( std::list<BGP::Room>::iterator it=rooms.begin(); it!=rooms.end(); ++it )
	      {
		BGP::Room &room = *it;
		if( room.name == "standard room" && room.is_open )
		{
		  match = true;
		  conn_state.state = BGP_ASK_ROOM;
		  // send response
		  BGP::Msg_Choose_Room msg(room);
		  connection->send_message(&msg);
		  // setup timeout
		  start_timer(response_timeout);
		  break;
		}
	      }
	      if( !match )
	      {
#ifndef __WXMSW__
		std::cerr << "Client Error - Standard room not available!" << std::endl;
#endif
		conn_state.state = BGP_UNCONNECTED;
		disconnect();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ASK_ROOM:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      conn_state.state = BGP_GET_PHASE;
	      // send request
	      BGP::Msg_Get_Phase msg;
	      connection->send_message(&msg);
	      // setup timeout
	      start_timer(response_timeout);
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_GET_PHASE:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_phase:
	    {
	      BGP::Msg_Tell_Phase *det_msg = static_cast<BGP::Msg_Tell_Phase*>(message);
	      switch( det_msg->get_phase() )
	      {
		case BGP::phase_setup:
		{
		  conn_state.state = BGP_SETUP_PREPARE_1;
		  // send request
		  BGP::Msg_Get_Game msg;
		  connection->send_message(&msg);
		  // setup timeout
		  start_timer(response_timeout);
		  break;
		} 
		case BGP::phase_playing:
		{
		  conn_state.state = BGP_PLAY_PREPARE_1;
		  // send request
		  BGP::Msg_Get_Game msg;
		  connection->send_message(&msg);
		  // setup timeout
		  start_timer(response_timeout);
		  break;
		} 
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_PLAY_PREPARE_1:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_game:
	    {
	      BGP::Msg_Tell_Game *det_msg = static_cast<BGP::Msg_Tell_Game*>(message);
	      if( det_msg->get_game() == GAME_NAME )
	      {
		// this implementation doesn't allow manual
		// reassignment of abandoned players => skip BGP_PLAY_PREPARE_2
		BGP::Msg_Request_Final_Setup msg1;
		connection->send_message(&msg1);

		requested_take_player_id = own_player_ids.begin();
		if( requested_take_player_id != own_player_ids.end() )
		{
		  conn_state.state = BGP_ASK_TAKE_PLAYER;
		  // send request
		  BGP::Msg_Take_Player msg2(*requested_take_player_id);
		  connection->send_message(&msg2);
		  // setup timeout
		  start_timer(response_timeout);
		}
		else
		{
		  conn_state.state = BGP_PLAY_PREPARE_3A;
		  // send request
		  BGP::Msg_Get_Moves_And_Play msg2;
		  connection->send_message(&msg2);
		  // setup timeout
		  start_timer(response_timeout);
		}
	      } 
	      else
	      {
#ifndef __WXMSW__
		std::cerr << "Client Error - Server hosts unsupported game: "
			  << det_msg->get_game() << std::endl;
#endif
		disconnect();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ASK_TAKE_PLAYER:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    case BGP::msg_deny:	// don't care whether accepted or not (final setup will tell)
	    {
	      ++requested_take_player_id;
	      if( requested_take_player_id != own_player_ids.end() )
	      {
		// send request
		BGP::Msg_Take_Player msg(*requested_take_player_id);
		connection->send_message(&msg);
		// setup timeout
		start_timer(response_timeout);
	      }
	      else
	      {
		conn_state.state = BGP_PLAY_PREPARE_3A;
		// send request
		BGP::Msg_Get_Moves_And_Play msg;
		connection->send_message(&msg);
		// setup timeout
		start_timer(response_timeout);
	      }
	      break;
	    } 
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_PLAY_PREPARE_3A:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_setup:
	    {
	      if( game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for out-of-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Setup *det_msg = static_cast<BGP::Msg_Tell_Setup*>(message);
		if( process_setup(det_msg->get_setup()) )
		{
		  conn_state.state = BGP_PLAY_PREPARE_4A;
		  // setup timeout
		  start_timer(response_timeout);
		  // wait for follow-up message
		}
		else
		  invalid = true;
	      }
	      break;
	    }
	    case BGP::msg_tell_player_setup:
	    {
	      if( !game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for in-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Player_Setup *det_msg = static_cast<BGP::Msg_Tell_Player_Setup*>(message);
		if( process_setup(det_msg->get_setup()) )
		{
		  conn_state.state = BGP_PLAY_PREPARE_4A;
		  // setup timeout
		  start_timer(response_timeout);
		  // wait for follow-up message
		}
		else
		  invalid = true;
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_PLAY_PREPARE_4A:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_moves:
	    {
	      if( game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for out-of-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Moves *det_msg = static_cast<BGP::Msg_Tell_Moves*>(message);
		game.set_players(list_to_vector(players));
		if( process_moves(det_msg->get_moves()) )
		  start_prepared_game();
		else
		  invalid = true;
		break;
	      }
	    }
	    case BGP::msg_tell_player_moves:
	    {
	      if( !game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for in-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Player_Moves *det_msg = static_cast<BGP::Msg_Tell_Player_Moves*>(message);
		game.set_players(list_to_vector(players));
		if( process_moves(det_msg->get_moves()) )
		  start_prepared_game();
		else
		  invalid = true;
		break;
	      }
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_SETUP_PREPARE_1:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_game:
	    {
	      BGP::Msg_Tell_Game *det_msg = static_cast<BGP::Msg_Tell_Game*>(message);
	      if( det_msg->get_game() == GAME_NAME )
	      {
		// BGP 1.0Xa : just request final setup and enter solitary player setup phase
		conn_state.state = BGP_SETUP;
		conn_state.in_setup = true;
		// send requests
		BGP::Msg_Request_Final_Setup msg1;
		BGP::Msg_Setup msg2;
		connection->send_message(&msg1);
		connection->send_message(&msg2);
		if( display_handler )
		  display_handler->enter_player_setup();
	      } 
	      else
	      {
#ifndef __WXMSW__
		std::cerr << "Client Error - Server hosts unsupported game: "
			  << det_msg->get_game() << std::endl;
#endif
		disconnect();
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_SETUP:
	{
	  switch( message->get_type() )
	  {
	    // server waits for client to signalize ready
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ADD_PLAYER:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept_player:
	    {
	      BGP::Msg_Accept_Player *det_msg = static_cast<BGP::Msg_Accept_Player*>(message);
	      conn_state.state = BGP_SETUP;
	      requested_add_player.id = det_msg->get_id();
	      do_add_player(requested_add_player, true /*local*/);
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      conn_state.state = BGP_SETUP;
	      if( display_handler && display_phase == DISPLAY_SETUP )
		display_handler->player_change_denied();
	      break;
	    }
	    default: invalid = true; break;
	  }
	  if( !invalid && conn_state.is_ready ) // check for meanwhile ready
	  {
	    conn_state.state = BGP_READY;
	    BGP::Msg_Ready msg;
	    connection->send_message(&msg);
	  }
	  break;
	}
	case BGP_REMOVE_PLAYER:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      conn_state.state = BGP_SETUP;
	      do_remove_player(requested_remove_player_id);
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      conn_state.state = BGP_SETUP;
	      if( display_handler && display_phase == DISPLAY_SETUP )
		display_handler->player_change_denied();
	      break;
	    }
	    default: invalid = true; break;
	  }
	  if( !invalid && conn_state.is_ready ) // check for meanwhile ready
	  {
	    conn_state.state = BGP_READY;
	    BGP::Msg_Ready msg;
	    connection->send_message(&msg);
	  }
	  break;
	}
	case BGP_READY:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_tell_setup:
	    {
	      if( game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for out-of-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Setup *det_msg = static_cast<BGP::Msg_Tell_Setup*>(message);
		if( process_setup( det_msg->get_setup() ) )
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  if( display_handler && display_phase == DISPLAY_SETUP )
		    display_handler->everything_ready();
		}
		else
		  invalid = true;
	      }
	      break;
	    }
	    case BGP::msg_tell_player_setup:
	    {
	      if( !game.is_out_of_order() )
	      {
#ifndef __WXMSW__
		std::cout << "Client Error - Invalid Network Command for in-order player game" << std::endl;		
#endif
		invalid = true;
	      }
	      else
	      {
		BGP::Msg_Tell_Player_Setup *det_msg = static_cast<BGP::Msg_Tell_Player_Setup*>(message);
		if( process_setup( det_msg->get_setup() ) )
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  if( display_handler && display_phase == DISPLAY_SETUP )
		    display_handler->everything_ready();
		}
		else
		  invalid = true;
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_OTHERS_TURN:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_move:
	    {
	      if( display_phase != DISPLAY_PLAYING )
	      {
		deferred_messages.push_back(std::pair<Message_Network<BGP::Message>*,BGP::Message*>
					    (msg_net_client,message));
#ifndef __WXMSW__
		std::cerr << "Client: deferred message" << std::endl;
#endif
	      }
	      else
	      {
		BGP::Msg_Move *det_msg = static_cast<BGP::Msg_Move*>(message);
		process_move(det_msg->get_move(), /*local=*/false, det_msg->get_player_id()); 
	      }
	      break;
	    }
	    case BGP::msg_ask_new_game:
	    {
	      BGP::Msg_Ask_New_Game *det_msg = static_cast<BGP::Msg_Ask_New_Game*>(message);
	      if( display_handler )
	      {
		std::string name = det_msg->get_host_nick();
		if( name=="" ) 
		  name = conn_state.name;
		conn_state.state = BGP_ACCEPT_NEW_GAME;
		if( display_handler->ask_new_game(str_to_wxstr(name)) )
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  // send response
		  BGP::Msg_Accept msg;
		  connection->send_message(&msg);
		}
		else
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  // send response
		  BGP::Msg_Deny msg;
		  connection->send_message(&msg);
		}
	      }
	      else
	      {
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_ask_undo:
	    {
	      BGP::Msg_Ask_Undo *det_msg = static_cast<BGP::Msg_Ask_Undo*>(message);
	      if( display_handler )
	      {
		std::string name = det_msg->get_host_nick();
		if( name=="" ) 
		  name = conn_state.name;
		conn_state.state = BGP_ACCEPT_UNDO;
		if( display_handler->ask_undo_moves(str_to_wxstr(name), det_msg->get_n()) )
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  // send response
		  BGP::Msg_Accept msg;
		  connection->send_message(&msg);
		}
		else
		{
		  conn_state.state = BGP_OTHERS_TURN;
		  // send response
		  BGP::Msg_Deny msg;
		  connection->send_message(&msg);
		}
	      }
	      else
	      {
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
	      }
	      break;
	    }
	    case BGP::msg_new_game:
	    {
	      game_manager.force_new_game();
	      break;
	    }
	    case BGP::msg_undo:
	    {
	      BGP::Msg_Undo *det_msg = static_cast<BGP::Msg_Undo*>(message);
	      // apply undo in local game copy
	      int n = det_msg->get_n();
	      for(int i=0; i<n; ++i)
	      {
		if(!game.undo_move())
		{
		  n = i;
		  break;
		}
	      }
	      if( does_include(own_player_ids,game.get_current_player().id) )
		conn_state.state = BGP_MY_TURN;
	      else
		conn_state.state = BGP_OTHERS_TURN;
	      // report to game manager
	      game_manager.do_undo_moves(n);
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_MY_TURN:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_move:
	    {
	      if( game.is_out_of_order() )
	      {
		if( display_phase != DISPLAY_PLAYING )
		{
		  deferred_messages.push_back(std::pair<Message_Network<BGP::Message>*,BGP::Message*>
					      (msg_net_client,message));
#ifndef __WXMSW__
		  std::cerr << "Client: deferred message" << std::endl;
#endif
		}
		else
		{
		  BGP::Msg_Move *det_msg = static_cast<BGP::Msg_Move*>(message);
		  process_move(det_msg->get_move(), /*local=*/false, det_msg->get_player_id()); 
		}
	      }
	      else
		invalid = true;
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ASK_NEW_GAME:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      assert(does_include(own_player_ids,game.get_current_player().id));
	      conn_state.state = BGP_MY_TURN;
	      asking_new_game = false;
	      game_manager.new_game_accepted();	 // calls force_new_game
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      assert(does_include(own_player_ids,game.get_current_player().id));
	      conn_state.state = BGP_MY_TURN;
	      asking_new_game = false;
	      game_manager.new_game_denied();
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ACCEPT_NEW_GAME:
	{
	  switch( message->get_type() )
	  {
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ASK_UNDO:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_accept:
	    {
	      assert(does_include(own_player_ids,game.get_current_player().id));
	      conn_state.state = BGP_MY_TURN;
	      asking_undo = false;
	      // apply undo in local game copy
	      for(int i=0; i<asking_n; ++i)
		if(!game.undo_move())
		  break;
	      if( does_include(own_player_ids,game.get_current_player().id) )
		conn_state.state = BGP_MY_TURN;
	      else
		conn_state.state = BGP_OTHERS_TURN;
	      // report to game manager
	      game_manager.undo_accepted();
	      break;
	    }
	    case BGP::msg_deny:
	    {
	      assert(does_include(own_player_ids,game.get_current_player().id));
	      conn_state.state = BGP_MY_TURN;
	      asking_undo = false;
	      // report to game manager
	      game_manager.undo_denied();
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ACCEPT_UNDO:
	{
	  switch( message->get_type() )
	  {
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ERROR:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_error:
	    {
	      // restart error recovery
	      if( error_recovery_attempts < 100 )  // limit traffic by
						   // avoiding
						   // infinite error
						   // recovery
	      {
		++error_recovery_attempts;
		// stop game
		game_manager.stop_game();
		// reset local state
		conn_state.in_setup = true;
		conn_state.is_ready = false;
		sequence_available = false;
		awaiting_move = false;
		asking_new_game = false;
		asking_undo = false;
		deferred_messages.clear();
		// initiate pre-check for recovery
		conn_state.state = BGP_ERROR_RECOVERY;
		error_recovery_pongs = 0;
		BGP::Msg_Ping msg;
		connection->send_message(&msg);
		start_timer(response_timeout);
	      }
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	case BGP_ERROR_RECOVERY:
	{
	  stop_timer();
	  switch( message->get_type() )
	  {
	    case BGP::msg_error:
	    {
	      break;
	    }
	    case BGP::msg_pong:
	    {
	      ++error_recovery_pongs;
	      if(error_recovery_pongs < 5)
	      {
		// continue ping-pong
		conn_state.state = BGP_ERROR_RECOVERY;
		BGP::Msg_Ping msg;
		connection->send_message(&msg);
		start_timer(response_timeout);
		break;
	      }
	      else
	      {
		// restart by querying phase
		conn_state.state = BGP_GET_PHASE;
		BGP::Msg_Get_Phase msg;
		connection->send_message(&msg);
		start_timer(response_timeout);
		break;
	      }
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	default: invalid = true; break;
      }
    }
    else
    {
      invalid = true;
    }
    //------------------------------
    // error handling:
    if( invalid )
    {
#ifndef __WXMSW__
      std::cerr << "Client Error - Invalid network message from " << conn_state.name << " in state " 
		<< to_string(conn_state.state) << ": ";
      if( message )
	message->print(std::cerr);
      else
	std::cerr << "<unrecognized message>" << std::endl;
#endif
      switch(conn_state.state)
      {
	case BGP_UNCONNECTED:
	  break;
	case BGP_HANDSHAKE:
	case BGP_ROOMS:
	case BGP_ASK_ROOM:
	  conn_state.state = BGP_UNCONNECTED;
	  disconnect();
	  break;
	default:
	  conn_state.state = BGP_ERROR;
	  // send response
	  BGP::Msg_Error msg;
	  connection->send_message(&msg);
	  // setup timeout
	  start_timer(response_timeout);
	  break;
      }
    }
  }
  // is called when connection is established (for Message_Network_Client only)
  void Network_Manager_BGP100a_Client::on_connect( Message_Network<BGP::Message> *connection )
  {
    if( !connection->is_connected() ) return; // false alarm, !!! todo: treat as error

    assert(connection == msg_net_client);
    if(conn_state.state == BGP_UNCONNECTED)
    {
      conn_state.state = BGP_HANDSHAKE;
      // send request
      BGP::Msg_Helo msg(BGP::Protocol("BGP",PROTOCOL_NUMBER,'a'), connection->get_local_host());
      connection->send_message(&msg);
      // setup timeout
      start_timer(response_timeout);
    }
    else
    {
      // misplaced connect
#ifndef __WXMSW__
      std::cerr << "Client Error - Misplaced connect from " << conn_state.name << " in state " 
		<< to_string(conn_state.state) << std::endl;
#endif
      switch(conn_state.state)
      {
	case BGP_UNCONNECTED:
	  break;
	case BGP_HANDSHAKE:
	case BGP_ROOMS:
	case BGP_ASK_ROOM:
	  conn_state.state = BGP_UNCONNECTED;
	  disconnect();
	  break;
	default:
	  conn_state.state = BGP_ERROR;
	  // send response
	  BGP::Msg_Error msg;
	  connection->send_message(&msg);
	  // setup timeout
	  start_timer(response_timeout);
	  break;
      }
    }
  }
  // is called when connection was closed or couldn't be established
  void Network_Manager_BGP100a_Client::on_lost( Message_Network<BGP::Message> *connection )
  {
    connection_lost = true;
    if(connection != msg_net_client) return;  // already disconnected ?
#ifndef __WXMSW__
    std::cerr << "Client Error - Connection Lost!" << std::endl;
#endif
    disconnect(false);
  }
  // is called when an error occured
  void Network_Manager_BGP100a_Client::on_error( Message_Network<BGP::Message> *connection )
  {
    if(connection != msg_net_client) return;  // already disconnected ?
#ifndef __WXMSW__
    std::cerr << "Client Error - Low level message error!" << std::endl;
#endif
    switch(conn_state.state)
    {
      case BGP_UNCONNECTED:
	break;
      case BGP_HANDSHAKE:
      case BGP_ROOMS:
      case BGP_ASK_ROOM:
	conn_state.state = BGP_UNCONNECTED;
	disconnect();
	break;
      case BGP_ERROR:
      default:
	conn_state.state = BGP_ERROR;
	// send response
	BGP::Msg_Error msg;
	connection->send_message(&msg);
	// setup timeout
	start_timer(response_timeout);
	break;
    }
  }

  //----------------
  //----------------
  // event handlers
  //----------------
  //----------------
    
  void Network_Manager_BGP100a_Client::on_timer(wxTimerEvent&)
  {
    if( delayed_move_visualization )
    {
      ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE, ANIMATION_DONE );
      // triggers continue_game() after done
    }
    else			// assume timeout callback
    {
      if(!msg_net_client) return;  // already disconnected ?

      // timeout events
      Message_Network<BGP::Message> *connection = msg_net_client;
#ifndef __WXMSW__
      std::cerr << "Client Error - Timeout triggered" << std::endl;
#endif
      switch(conn_state.state)
      {
	case BGP_UNCONNECTED:
	  if( connection->get_socket()->Error() )
	  {
	    // this happens in case of the client connecting to an
	    // unreachable host but even no negative response is sent by
	    // routers (it can also happen just if connecting takes more
	    // than <response_timeout>)
	    msg_net_client->set_handler(0);
	    delete msg_net_client;
	    msg_net_client = 0;
	    if( display_handler )
	      display_handler->aborted();
	  }
	  break;
	case BGP_HANDSHAKE:
	case BGP_ROOMS:
	case BGP_ASK_ROOM:
	  conn_state.state = BGP_UNCONNECTED;
	  disconnect();
	  break;
	case BGP_ERROR:		// repeat error until error returns or disconnect
	default:
	  conn_state.state = BGP_ERROR;
	  // send response
	  BGP::Msg_Error msg;
	  connection->send_message(&msg);
	  // setup timeout
	  start_timer(response_timeout);
	  break;
      }
    }
  }

  void Network_Manager_BGP100a_Client::on_done(wxTimerEvent& WXUNUSED(event))
  {
    continue_game();
  }

  //------------------
  //------------------
  // private functions
  //------------------
  //------------------

  std::string Network_Manager_BGP100a_Client::to_string(Protocol_State state)
  {
    switch(state) {
      case BGP_UNCONNECTED: return "UNCONNECTED"; 
      case BGP_HANDSHAKE: return "HANDSHAKE"; 
      case BGP_ROOMS: return "ROOMS";
      case BGP_ASK_ROOM: return "ASK_ROOM"; 
      case BGP_GET_PHASE: return "GET_PHASE"; 
      case BGP_SETUP_PREPARE_1: return "SETUP_PREPARE_1"; 
      case BGP_PLAY_PREPARE_1: return "PLAY_PREPARE_1"; 
      case BGP_PLAY_PREPARE_2: return "PLAY_PREPARE_2"; 
      case BGP_PLAY_PREPARE_3A: return "PLAY_PREPARE_3A";
      case BGP_PLAY_PREPARE_3B: return "PLAY_PREPARE_3B"; 
      case BGP_PLAY_PREPARE_4A: return "PLAY_PREPARE_4A";
      case BGP_TAKE_PLAYER: return "TAKE_PLAYER"; 
      case BGP_ASK_TAKE_PLAYER: return "ASK_TAKE_PLAYER";
      case BGP_SETUP: return "SETUP"; 
      case BGP_ADD_PLAYER: return "ADD_PLAYER"; 
      case BGP_REMOVE_PLAYER: return "REMOVE_PLAYER"; 
      case BGP_READY: return "READY"; 
      case BGP_OTHERS_TURN: return "OTHERS_TURN"; 
      case BGP_MY_TURN: return "MY_TURN";
      case BGP_ASK_UNDO: return "ASK_UNDO"; 
      case BGP_ACCEPT_UNDO: return "ACCEPT_UNDO"; 
      case BGP_ASK_NEW_GAME: return "ASK_NEW_GAME"; 
      case BGP_ACCEPT_NEW_GAME: return "ACCEPT_NEW_GAME";
      case BGP_ERROR: return "ERROR";
      case BGP_ERROR_RECOVERY: return "ERROR_RECOVERY";
    }
    assert(false);
    return "<invalid>";
  }

  void Network_Manager_BGP100a_Client::continue_game()
  {
    game_manager.continue_game();
  }

  void Network_Manager_BGP100a_Client::disconnect(bool send_disconnect)
  {
    if(!msg_net_client) return;  // already disconnected ?

    conn_state.state = BGP_UNCONNECTED;
    if( send_disconnect )
    {
      BGP::Msg_Disconnect msg;
      msg_net_client->send_message(&msg);
      msg_net_client->flush();
    }
    msg_net_client->set_handler(0);
    delete msg_net_client;
    msg_net_client = 0;

    if( display_handler )
      display_handler->aborted();
  }

  void Network_Manager_BGP100a_Client::start_timer(int milliseconds)
  {
    timer.Start(milliseconds,wxTIMER_ONE_SHOT);
  }

  void Network_Manager_BGP100a_Client::stop_timer()
  {
    timer.Stop();
  }

  BGP::Phase_Type Network_Manager_BGP100a_Client::get_game_phase()
  {
    return game_phase;
  }

  // actually add player and return ID
  void Network_Manager_BGP100a_Client::do_add_player( const Player& player,
						      bool local )
  {
    std::list<Player>::iterator p = players.insert(players.end(), player);
    id_player[p->id] = p;
    if( local )
      own_player_ids.insert(p->id);
		
    if( display_handler && display_phase == DISPLAY_SETUP )
    {
      display_handler->player_added(*p);
    }
  }

  // actually remove player
  void Network_Manager_BGP100a_Client::do_remove_player( int player_id )
  {
    assert(does_include(id_player,player_id));
    if( display_handler && display_phase == DISPLAY_SETUP )
      display_handler->player_removed(*id_player[player_id]);

    players.erase(id_player[player_id]);
    // reload iterators due to erase
    id_player.clear();
    std::list<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      id_player[i->id] = i;
    }
  }

  void Network_Manager_BGP100a_Client::setup_players()
  {
    if(!msg_net_client) return;  // already disconnected ?

    id_player.clear();
    // localize Player objects for the viewpoint of this client
    std::list<Player>::iterator it;
    for( it=players.begin(); it!=players.end(); ++it )
    {
      Player &player = *it;
      id_player[player.id] = it;
      if( does_include(own_player_ids,player.id) )
      {
	Player_Input *input;
	if( player.type == Player::ai )
	  input = game_manager.get_player_ai();
	else
	  input = ui_manager.get_user_input();
		  
	player.input = input;
	player.host = "";
	player.origin = Player::local;
		  
	std::list<Player_Output*> outlist;
	outlist.push_back(this);
	player.outputs = outlist;
      }
      else
      {
	player.origin = Player::remote;
	player.input = this;
	if(player.host == "" && msg_net_client->is_connected())
	{
	  player.host = conn_state.name;
	}
      }
    }
    // find player ids that were not granted finally
    std::list<int> denied_player_ids;
    std::set<int>::iterator own_it;
    for( own_it = own_player_ids.begin(); 
	 own_it != own_player_ids.end(); ++own_it )
    {
      int id = *own_it;
      if( !does_include(id_player,id) )
	denied_player_ids.push_back(id);
    }
    std::list<int>::iterator denied_it;
    for( denied_it = denied_player_ids.begin();
	 denied_it != denied_player_ids.end(); ++denied_it )
    {
      int id = *denied_it;
      own_player_ids.erase(id);
    }
  }

  void Network_Manager_BGP100a_Client::cleanup_players()
  {
    bool done;
    // remove all remote players since they are managed by the server
    do{
      done = true;
      std::list<Player>::iterator it;
      for( it=players.begin(); it!=players.end(); ++it )
      {
	Player &player = *it;
	if( player.origin == Player::remote )
	{
	  players.erase(it);	// iterator might be affected by erase
	  done = false;
	  break;	
	}
      }
    }while(!done);
  }

  bool Network_Manager_BGP100a_Client::process_setup( BGP::Setup setup )
  {
    bool invalid = false;
    game.reset_game(setup.ruleset);
    game.set_players( list_to_vector(setup.initial_players) );
    players = setup.current_players;
    if( game.players.size() < game.get_min_players() )
    {
#ifndef __WXMSW__
      std::cout << "Client Error - Not enough players in game" << std::endl;		
#endif
      invalid = true;
    }
    else if( game.players.size() > game.get_max_players() )
    {
#ifndef __WXMSW__
      std::cout << "Client Error - Too many players in game" << std::endl;		
#endif
      invalid = true;
    }
    else
    {
      std::list<std::pair<Move_Sequence,int/*player index*/> >::iterator it; 
      int num_moves_read = 0;
      for( it=setup.init_moves.begin(); it!=setup.init_moves.end(); ++it )
      {
	Move_Sequence &sequence = it->first;
	int player_index = it->second;
	if( game_manager.is_out_of_order_game() && player_index >= 0 && player_index < (int)game.players.size() )
	  game.set_current_player_index(player_index);
	if( !sequence.check_sequence(game) ) 
	{
#ifndef __WXMSW__
	  std::cout << "Client Error - Received setup with illegal move " << num_moves_read << ": " 
		    << sequence << std::endl;		
#endif
	  invalid = true; break;
	}
	game.do_move( sequence );
	++num_moves_read;
      }
    }
    if( !invalid )
    {
      // game successfully setup
      setup_players();
      game_phase = BGP::phase_playing;
    }
    return !invalid;
  }

  bool Network_Manager_BGP100a_Client::process_moves
  ( std::list<std::pair<Move_Sequence,int/*player index*/> > moves )
  {
    std::list<std::pair<Move_Sequence,int/*player index*/> >::iterator move_it;
    for( move_it = moves.begin(); move_it != moves.end(); ++move_it )
    {
      Move_Sequence &move = move_it->first;
      int player_index = move_it->second;
      if( game_manager.is_out_of_order_game() && player_index >= 0 && player_index < (int)game.players.size() )
	game.set_current_player_index(player_index);
      if( !move.check_sequence(game) )
	return false;
      game.do_move(move);
    }
    int new_id = game.get_current_player().id;
    if( does_include(own_player_ids,new_id) )
      conn_state.state = BGP_MY_TURN;
    else
      conn_state.state = BGP_OTHERS_TURN;
#ifndef __WXMSW__
    std::cerr << "Client: Current Player " << new_id << " -> " << to_string(conn_state.state) 
	      << std::endl;
    if( own_player_ids.size() )
    {
      std::cerr << "Client: Own Player: " << *own_player_ids.begin() << " / " 
		<< own_player_ids.size() << std::endl;
    }
#endif

    return true;
  }
      
  bool Network_Manager_BGP100a_Client::process_move
  ( const Move_Sequence& move_sequence, bool local, int player_id, bool delay_continue_game )
  {
    if(!msg_net_client) return false;  // already disconnected ?

    bool error = false;
    if( asking_new_game ) // only player who's turn it is is allowed to ask for new game
    {
#ifndef __WXMSW__
      std::cerr << "Client Error - Received move while asking for new game!" << std::endl;
#endif
      error = true;
    }
    if( asking_undo ) // only player who's turn it is is allowed to ask for undo
    {
#ifndef __WXMSW__
      std::cerr << "Client Error - Received move while asking for undo!" << std::endl;
#endif
      error = true;
    }
    if( local )
    {
      if( conn_state.state != BGP_MY_TURN )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received local move but it is not my turn!" << std::endl;
#endif
	error = true;
      }
      if( player_id == -1 )
	player_id = game.get_current_player().id;
      assert( does_include(own_player_ids,player_id) );
    }
    else if( player_id != game.get_current_player().id && game.is_out_of_order() )
    {
      if( does_include(own_player_ids,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remote move for local player!" << std::endl;
#endif
	error = true;
      } 
      else if( does_include(pending_player_move,player_id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Player already sent a move: " << player_id << "!" 
		  << std::endl;
#endif
	error = true;
      }
      else
      {
	pending_player_move[player_id] = move_sequence;
	return true;
      }
    }
    else 
    {
      if( player_id != game.get_current_player().id )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remote move for player " << player_id 
		  << " but it is the turn of player " << game.get_current_player().id 
		  << "!" << std::endl;
#endif
	error = true;
      }
      if( does_include(own_player_ids,game.get_current_player().id) )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remote move but it is my turn!" << std::endl;
#endif
	error = true;
      } 
      if( conn_state.state != BGP_OTHERS_TURN )
      {
#ifndef __WXMSW__
	std::cerr << "Client Error - Received remove move in wrong state!" << std::endl;
#endif
	error = true;
      }
    }
    if( !error && !move_sequence.check_sequence(game) )
    {
#ifndef __WXMSW__
      std::cerr << "Client Error - Invalid Move " << move_sequence << "!" << std::endl;
#endif
      error = true;
    }
    if( error )
    {
      conn_state.state = BGP_ERROR;
      // send response
      BGP::Msg_Error msg;
      msg_net_client->send_message(&msg);
      // setup timeout
      start_timer(response_timeout);
      return false;
    }
    else
    {
      // execute valid move
      game.do_move(move_sequence);
      int new_id = game.get_current_player().id;
      if( does_include(own_player_ids,new_id) )
	conn_state.state = BGP_MY_TURN;
      else
	conn_state.state = BGP_OTHERS_TURN;
#ifndef __WXMSW__
      std::cerr << "Client: Current Player " << new_id << " -> " << to_string(conn_state.state) 
		<< std::endl;
      if( own_player_ids.size() )
      {
	std::cerr << "Client: Own Player: " << *own_player_ids.begin() << " / " 
		  << own_player_ids.size() << std::endl;
      }
#endif
      // communicate move
      if( local )
      {
	// send response
	BGP::Msg_Move msg(player_id, move_sequence);
	msg_net_client->send_message(&msg);
      }
      else
      {
	if( awaiting_move && !sequence_available )
	{
	  sequence = move_sequence;
	  sequence_available = true;
	  awaiting_move = false;
	  if( delay_continue_game )
	  {
	    delayed_move_visualization = true;
	    start_timer(1);
	  }
	  else
	  {
	    ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE, ANIMATION_DONE );
	    // triggers continue_game() after done
	  }
	}
	else
	{
	  pending_moves.push_back(std::pair<int/*player_id*/,Move_Sequence>
				  (player_id,move_sequence));
	}
      }
    }
    return true;
  }

  void Network_Manager_BGP100a_Client::process_deferred_messages()
  {
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> > messages 
      = deferred_messages;
    deferred_messages.clear();
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> >::iterator it;
    for( it=messages.begin(); it!=messages.end(); ++it )
    {
      Message_Network<BGP::Message>* connection = it->first;
      BGP::Message* message = it->second;
      process_message(connection,message);
    }
  }

  BEGIN_EVENT_TABLE(Network_Manager_BGP100a_Client, wxEvtHandler)				
    EVT_TIMER(NETWORK_TIMER, Network_Manager_BGP100a_Client::on_timer)	//**/
    EVT_TIMER(ANIMATION_DONE, Network_Manager_BGP100a_Client::on_done)	//**/
  END_EVENT_TABLE()							//**/

}

