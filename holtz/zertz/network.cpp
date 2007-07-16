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

#include "network.hpp"
#include "util.hpp"
#include "wxmain.hpp"

#include <iostream>

namespace zertz
{
  using namespace holtz;

//============================================================================================================
//============================================================================================================
//  Server
//============================================================================================================
//============================================================================================================

  Network_Manager_BGP100a_Server::Network_Manager_BGP100a_Server
  ( Game_Manager &game_manager, Game_UI_Manager &ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), 
      display_handler(0), 
      game( game_manager.get_game() ),
      timer(this,NETWORK_TIMER),
      msg_net_server(this),
      max_connections(15), response_timeout(6000/*ms*/),
      current_id(11), current_player_id(101), game_phase(GAME_INIT), 
      clients_ready(0), clients_player_setup_ack(0)
  {
  }

  Network_Manager_BGP100a_Server::~Network_Manager_BGP100a_Server()
  {
    if( display_handler )
      display_handler->aborted();

    close_connections();
  }

  // connection commands
  bool Network_Manager_BGP100a_Server::setup_server( int port )
  {
    close_connections();
    return msg_net_server.bind(port);
  }

  //---------------------------------------------
  //---------------------------------------------
  // commands inherited from Basic_Network_Server
  //---------------------------------------------
  //---------------------------------------------

  void Network_Manager_BGP100a_Server::close_connections()
  {
    std::map<Message_Network<BGP::Message>*,Connection_State>::iterator it;
    for( it=msg_net_connection_state.begin();
	 it!=msg_net_connection_state.end(); ++it )
    {
      Message_Network<BGP::Message> *connection = it->first;
      BGP::Msg_Disconnect msg;
      connection->send_message(&msg);
      connection->flush();
      delete connection;
    }
    msg_net_connection_state.clear();
    player_id_connection.clear();
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
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_change_board( const Game &game )
  {
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
  bool Network_Manager_BGP100a_Server::add_player( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Server::remove_player( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Server::player_up( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Server::player_down( const Player & )
  {
  }
  // ready with adding players
  void Network_Manager_BGP100a_Server::ready()
  {
  }
  //-------------------------------------------
  // game commands
  //-------------------------------------------

  // is everyone ready and number of players ok?
  Game_Setup_Manager::Game_State Network_Manager_BGP100a_Server::can_start()
  {
  }
  // call only when can_start() == true
  void Network_Manager_BGP100a_Server::start_game()
  {
  }
  // request to play new game
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_new_game()
  {
  }
  // request to undo n half moves
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Server::ask_undo_moves(int n)
  {
  }
  // force new game (may close connections)
  void Network_Manager_BGP100a_Server::force_new_game()
  {
  }
  // stop game
  void Network_Manager_BGP100a_Server::stop_game()
  {
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Input
  //-------------------------------------------
  //-------------------------------------------

  Player_Input::Player_State Network_Manager_BGP100a_Server::determine_move() throw(Exception)
  {
  }
  Move_Sequence Network_Manager_BGP100a_Server::get_move()
  {
  }
  long Network_Manager_BGP100a_Server::get_used_time()
  {
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Output
  //-------------------------------------------
  //-------------------------------------------

  void Network_Manager_BGP100a_Server::report_move( const Move_Sequence & )
  {
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
	  disconnect(connection);
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
	      // supported protocols: BGP 1.00a
	      if( protocol.name == "BGP" && protocol.number== 1.00 &&
		  protocol.letter == 'a' )
	      {
		// accept protocol
		conn_state.state = BGP_HANDSHAKE;
		conn_state.nick_name = det_msg->get_host_nick();
		// send response
		BGP::Msg_Accept msg;
		connection->send_message(&msg);
		// set timeout
		conn_state.timer.start(response_timeout);
	      } else {
		// protocol mismatch => list supported protocols
		std::list<BGP::Protocol> supported_protocols;
		supported_protocols.push_back(BGP::Protocol("BGP",1.00,'a'));
		// send response
		BGP::Msg_List_Protocols msg(supported_protocols);
		connection->send_message(&msg);
		// set timeout
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
	      rooms.push_back(BGP::Room("standard room",get_client_count(),true,get_bgp_phase()));
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
	case BGP_ERROR:
	{
	  switch( message->get_type() )
	  {
	    case BGP::msg_get_phase:
	    {
	      // reintegrate connection
	      // todo: send phase
	      break;
	    }
	    default: invalid = true; break;
	  }
	  break;
	}
	default: invalid = true; break;
      }

      //------------------------------
      // error handling:
    }
    else
    {
      invalid = true;
    }
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
	  conn_state.state = BGP_ERROR;
	  // send response
	  BGP::Msg_Error msg;
	  connection->send_message(&msg);
	  break;
      }
    }
  }
  // is called when connection is established (for Message_Network_Client only)
  void Network_Manager_BGP100a_Server::on_connect( Message_Network<BGP::Message> *connection )
  {
    assert(false);		// shouldn't happen for server
  }
  // is called when connection was closed or couldn't be established
  void Network_Manager_BGP100a_Server::on_lost( Message_Network<BGP::Message> *connection )
  {
    Connection_State &conn_state = msg_net_connection_state[connection];
    if( connection_handler )
      connection_handler->closed_connection(conn_state.id);
#ifndef __WXMSW__
    std::cerr << "Server Error - Connection Lost to " << conn_state.name << std::endl;
#endif
    std::list<int> &player_ids = msg_net_connection_state[connection].controlled_player_ids;
    for(std::list<int>::iterator it = player_ids.begin(); it != player_ids.end(); ++it )
      player_id_connection.erase(*it);
    msg_net_connection_state.erase(connection);
    delete connection;
  }
  // is called when an error occured
  void Network_Manager_BGP100a_Server::on_error( Message_Network<BGP::Message> *connection )
  {
    Connection_State &conn_state = msg_net_connection_state[connection];
#ifndef __WXMSW__
    std::cerr << "Server Error - Low level message error from " << conn_state.name << std::endl;
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
	conn_state.state = BGP_ERROR;
	// send response
	BGP::Msg_Error msg;
	connection->send_message(&msg);
	break;
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
    if( get_client_count() >= max_connections )
    {
      delete connection;
      return;
    }
    connection->set_handler(this);

    Connection_State &conn_state = msg_net_connection_state[connection]; // this inserts automatically
    conn_state.id = current_id++;
    conn_state.state = BGP_CONNECTED;
    conn_state.name = connection->get_remote_host() + ":" + long_to_string(connection->get_remote_port());
    id_connection[conn_state.id] = connection;

    conn_state.timer.init(this,connection);
    conn_state.timer.start(response_timeout);
    // wait for helo
  }

  //----------------
  //----------------
  // event handlers
  //----------------
  //----------------
    
  void Network_Manager_BGP100a_Server::on_timer(wxTimerEvent& event)
  {
  }

  void Network_Manager_BGP100a_Server::on_connection_timer( Message_Network<BGP::Message> *connection )
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

  void Network_Manager_BGP100a_Server::continue_game()
  {
    game_manager.continue_game();
  }

  void Network_Manager_BGP100a_Server::disconnect(Message_Network<BGP::Message>* connection)
  {
    Connection_State &conn_state = msg_net_connection_state[connection];
    if( connection_handler )
      connection_handler->closed_connection(conn_state.id);

    BGP::Msg_Disconnect msg;
    connection->send_message(&msg);
    connection->flush();
    std::list<int> &player_ids = msg_net_connection_state[connection].controlled_player_ids;
    for(std::list<int>::iterator it = player_ids.begin(); it != player_ids.end(); ++it )
      player_id_connection.erase(*it);
    msg_net_connection_state.erase(connection);
    delete connection;
  }

  void Network_Manager_BGP100a_Server::start_timer(int milliseconds)
  {
    timer.Start(milliseconds,wxTIMER_ONE_SHOT);
  }

  unsigned Network_Manager_BGP100a_Server::get_client_count()
  {
    return (unsigned)msg_net_connection_state.size();
  }

  BGP::Phase_Type Network_Manager_BGP100a_Server::get_bgp_phase()
  {
    if( game_phase == GAME_PLAYING )
      return BGP::phase_playing;
    else
      return BGP::phase_setup;
  }

  BEGIN_EVENT_TABLE(Network_Manager_BGP100a_Server, wxEvtHandler)				
    EVT_TIMER(NETWORK_TIMER, Network_Manager_BGP100a_Server::on_timer)	//**/
  END_EVENT_TABLE()							//**/

//============================================================================================================
//============================================================================================================
//  Client
//============================================================================================================
//============================================================================================================

  Network_Manager_BGP100a_Client::Network_Manager_BGP100a_Client
  ( Game_Manager &game_manager, Game_UI_Manager &ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), 
      display_handler(0), 
      game( game_manager.get_game() ),
      timer(this,NETWORK_TIMER),
      msg_net_client(0),
      response_timeout(6000/*ms*/),
      game_phase(GAME_INIT)
  {
    conn_state.state = BGP_UNCONNECTED;
  }

  Network_Manager_BGP100a_Client::~Network_Manager_BGP100a_Client()
  {
    if( display_handler )
      display_handler->aborted();

    close_connection();
  }

  //--------------------
  // connection commands
  //--------------------

  // returns true for success
  bool Network_Manager_BGP100a_Client::connect_to_server( std::string host, int port )
  {
    close_connection();
    conn_state.state = BGP_UNCONNECTED;
    conn_state.name = host + ":" + long_to_string(port);
    msg_net_client = new Message_Network_Client<BGP::Message>(host,port,this);
    return true;		// on_lost will be called on failure
  }

  void Network_Manager_BGP100a_Client::close_connection()
  {
    conn_state.state = BGP_UNCONNECTED;
    if( msg_net_client != 0 )
    {
      BGP::Msg_Disconnect msg;
      msg_net_client->send_message(&msg);
      msg_net_client->flush();
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
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_change_board( const Game &game )
  {
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
  bool Network_Manager_BGP100a_Client::add_player( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Client::remove_player( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Client::player_up( const Player & )
  {
  }
  bool Network_Manager_BGP100a_Client::player_down( const Player & )
  {
  }
  // ready with adding players
  void Network_Manager_BGP100a_Client::ready()
  {
  }
  //-------------------------------------------
  // game commands
  //-------------------------------------------

  // is everyone ready and number of players ok?
  Game_Setup_Manager::Game_State Network_Manager_BGP100a_Client::can_start()
  {
  }
  // call only when can_start() == true
  void Network_Manager_BGP100a_Client::start_game()
  {
  }
  // request to play new game
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_new_game()
  {
  }
  // request to undo n half moves
  Game_Setup_Manager::Answer_Type Network_Manager_BGP100a_Client::ask_undo_moves(int n)
  {
  }
  // force new game (may close connections)
  void Network_Manager_BGP100a_Client::force_new_game()
  {
  }
  // stop game
  void Network_Manager_BGP100a_Client::stop_game()
  {
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Input
  //-------------------------------------------
  //-------------------------------------------

  Player_Input::Player_State Network_Manager_BGP100a_Client::determine_move() throw(Exception)
  {
  }
  Move_Sequence Network_Manager_BGP100a_Client::get_move()
  {
  }
  long Network_Manager_BGP100a_Client::get_used_time()
  {
  }

  //-------------------------------------------
  //-------------------------------------------
  // commands inherited from Player Output
  //-------------------------------------------
  //-------------------------------------------

  void Network_Manager_BGP100a_Client::report_move( const Move_Sequence & )
  {
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
    assert(connection == msg_net_client);
#ifndef __WXMSW__
    // !!! Debug output
    std::cerr << "Client received: "; message->print(std::cerr);
#endif

    //------------------------------
    // non state dependent messages:

    if( message->get_type() == BGP::msg_disconnect )
    {
      conn_state.state = BGP_UNCONNECTED;
      disconnect();
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

    bool invalid = false;
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
	    // supported protocols: BGP 1.00a
	    for( std::list<BGP::Protocol>::iterator it=protocols.begin(); it!=protocols.end(); ++it )
	    {
	      BGP::Protocol &protocol = *it;
	      if( protocol.name == "BGP" && protocol.number== 1.00 &&
		  protocol.letter == 'a' )
	      {
		match = true;
		// send response
		BGP::Msg_Helo msg(protocol,connection->get_local_host());
		connection->send_message(&msg);
		// setup timeout
		start_timer(response_timeout);
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
#ifndef __WXMSW__
	    // !!! Debug
	    std::cerr << "Client Success:" << std::endl;
#endif
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
	  case BGP::msg_error:
	  {
	    // restart by querying phase
	    conn_state.state = BGP_GET_PHASE;
	    BGP::Msg_Get_Phase msg;
	    connection->send_message(&msg);
	    start_timer(response_timeout);
	    break;
	  }
	  default: invalid = true; break;
	}
	break;
      }
      default: invalid = true; break;
    }

    //------------------------------
    // error handling:

    if( invalid )
    {
#ifndef __WXMSW__
      std::cerr << "Client Error - Invalid network message from " << conn_state.name << " in state " 
		<< to_string(conn_state.state) << ": ";
      message->print(std::cerr);
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
    assert(connection == msg_net_client);
    if(conn_state.state == BGP_UNCONNECTED)
    {
      conn_state.state = BGP_HANDSHAKE;
      // send request
      BGP::Msg_Helo msg(BGP::Protocol("BGP",1.00,'a'), connection->get_local_host());
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
    assert(connection == msg_net_client);
#ifndef __WXMSW__
    std::cerr << "Client Error - Connection Lost!" << std::endl;
#endif
    conn_state.state = BGP_UNCONNECTED;
    delete msg_net_client;
    msg_net_client = 0;
  }
  // is called when an error occured
  void Network_Manager_BGP100a_Client::on_error( Message_Network<BGP::Message> *connection )
  {
    assert(connection == msg_net_client);
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
    
  void Network_Manager_BGP100a_Client::on_timer(wxTimerEvent& event)
  {
    Message_Network<BGP::Message> *connection = msg_net_client;
#ifndef __WXMSW__
    std::cerr << "Client Error - Timeout triggered" << std::endl;
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
    }
    assert(false);
    return "<invalid>";
  }

  void Network_Manager_BGP100a_Client::continue_game()
  {
    game_manager.continue_game();
  }

  void Network_Manager_BGP100a_Client::disconnect()
  {
    conn_state.state = BGP_UNCONNECTED;
    BGP::Msg_Disconnect msg;
    msg_net_client->send_message(&msg);
    msg_net_client->flush();
    delete msg_net_client;
    msg_net_client = 0;
  }

  void Network_Manager_BGP100a_Client::start_timer(int milliseconds)
  {
    timer.Start(milliseconds,wxTIMER_ONE_SHOT);
  }

  void Network_Manager_BGP100a_Client::stop_timer()
  {
    timer.Stop();
  }

  BGP::Phase_Type Network_Manager_BGP100a_Client::get_bgp_phase()
  {
    if( game_phase == GAME_PLAYING )
      return BGP::phase_playing;
    else
      return BGP::phase_setup;
  }

  BEGIN_EVENT_TABLE(Network_Manager_BGP100a_Client, wxEvtHandler)				
    EVT_TIMER(NETWORK_TIMER, Network_Manager_BGP100a_Client::on_timer)	//**/
  END_EVENT_TABLE()							//**/

}

