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

#include "wxzertz.hpp"
#include "util.hpp"
#ifndef __OLD_GCC__
  #include <sstream>
#else
  #include <strstream>
#endif

namespace zertz
{
  //BEGIN_EVENT_TABLE(Network_Manager, wxEvtHandler)				
  //EVT_SOCKET(NETWORK_EVENT, Network_Manager::on_network)
  //EVT_TIMER(ANIMATION_DONE, Network_Manager::on_done)		//**/
  //END_EVENT_TABLE()						//**/

  Network_Connection_Handler::~Network_Connection_Handler()
  {
  }

  Network_Manager::Network_Manager( Game_Manager &game_manager, Game_UI_Manager &ui_manager )
    : game_manager(game_manager), ui_manager(ui_manager), 
      server(0), client(0), connection_handler(0), 
      display_handler(0), mode(mode_undefined),
      state(begin), current_id(101), 
      game( game_manager.get_game() ),
      max_clients(15), clients_ready(0), clients_player_setup_ack(0),
      max_string_size(60),
      timeout_sequencial_read(700) // 700 milliseconds
  {
    // connect event functions
    Connect( NETWORK_EVENT, wxEVT_SOCKET, 
	     (wxObjectEventFunction) (wxEventFunction) (wxSocketEventFunction) 
	     &Network_Manager::on_network );
    Connect( ANIMATION_DONE, wxEVT_TIMER, 
	     (wxObjectEventFunction) (wxEventFunction) (wxTimerEventFunction) 
	     &Network_Manager::on_done );
  }

  Network_Manager::~Network_Manager()
  {
    if( display_handler )
      display_handler->aborted();

    close_connection();
  }
  
  bool Network_Manager::setup_server( wxIPV4address port ) throw(Network_Exception)
  {
    close_connection();		// close previous connection if any

    if( mode == mode_undefined )
    {
      state = begin;
      assert( !server );
      mode = mode_server;
      
      server = new wxSocketServer(port,wxSOCKET_NOWAIT);
      server->SetEventHandler(*this,NETWORK_EVENT);
      server->SetNotify(wxSOCKET_CONNECTION_FLAG);
      server->Notify(TRUE);
      
      if( server->Ok() )
      {
	//server->GetLocal(localhost);
	return true;
      }
      else
      {
	server->Destroy();
	server = 0;
	mode = mode_undefined;
      }
    }
    return false;
  }

  bool Network_Manager::setup_client( wxIPV4address host ) throw(Network_Exception)
  {
    close_connection();		// close previous connection if any

    if( mode == mode_undefined )
    {
      state = begin;
      mode = mode_connecting;

      if( !client )
	client = new wxSocketClient(wxSOCKET_NOWAIT);
      
      client->SetEventHandler(*this,NETWORK_EVENT);
      client->SetNotify(wxSOCKET_CONNECTION_FLAG | wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG);
      client->Notify(TRUE);
      
      client->Connect(host,false);
      bool success = client->WaitOnConnect(10);	// wait 10 seconds

#ifndef __WXMSW__
      if( !success )
      {
	std::cerr << "Could not connect to server" << std::endl;
      }
      else
	std::cerr << "Connected to server" << std::endl;
#endif

      if( !client->IsConnected() )
      {
	client->Destroy();
	client = 0;
	mode = mode_undefined;
	return false;
      }
      mode = mode_client;	// already set by on_connect or received handshake
    }
    return true;
  }

  void Network_Manager::set_connection_handler( Network_Connection_Handler *handler )
  {
    connection_handler = handler;
  }
  
  void Network_Manager::close_connection()
  {
    switch( mode )
    {
      case mode_server:
      {
	// send setup of all players to all clients
	std::map<wxSocketBase*,Client>::iterator client_it;
	for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
	{
	  Client &c = client_it->second;
	  if( c.socket )
	  {
	    write_message_type( *c.socket, msg_abort_game );
	    c.socket->Destroy();
	  }
	}
	clients.clear();

	mode = mode_undefined;
	server->Close();
	server->Destroy();
	server = 0;

	id_client.clear();
	players.clear();
	own_players.clear();
	id_own_player.clear();
	clients_ready = 0;
	clients_player_setup_ack = 0;
	state = begin;
      }
      break;
      case mode_client:
      {
	write_message_type( *client, msg_abort_game );
	mode = mode_undefined;
	client->Destroy();
	client = 0;

	id_client.clear();
	players.clear();
	own_players.clear();
	id_own_player.clear();
	clients_ready = 0;
	clients_player_setup_ack = 0;
	state = begin;
      }
      break;
      case mode_connecting:
      case mode_undefined:
	break;
    }
  }

  bool Network_Manager::may_disconnect( wxSocketBase *socket )
  {
    if( mode == mode_server )
    {
      Client &c = clients[socket];
      if( c.socket )
      {
	if( (state == begin) || (state == is_ready) )
	  return true;
      }
      else
      {
	clients.erase( socket ); // erase entry if just added
      }
    }
    if( mode == mode_client )
    {
      if( (state == begin) || (state == is_ready) )
	return socket == client;
    }
    return false;
  }

  void Network_Manager::disconnect( wxSocketBase *socket )
  {
    if( mode == mode_server )
    {
      Client &c = clients[socket];
      if( c.socket )
      {
	write_message_type( *c.socket, msg_abort_game );

	// erase pointers to client objects
	std::list<std::list<Player>::iterator>::iterator player;
	for( player = c.players.begin(); player != c.players.end(); ++player )
	{
	  report_player_removed( &**player );
	  
	  id_player.erase( (*player)->id ); 
	  players.erase( *player );
	  id_client.erase( (*player)->id );
	}
	c.socket->Destroy();
	clients.erase( socket );
	recount_client_stat();
	check_state();
      }
      else
      {
	clients.erase( socket ); // erase entry if just added
      }
    }
    if( mode == mode_client )
    {
      close_connection();
    }
  }

  Game_Setup_Manager::Type Network_Manager::get_type()
  {
    if( mode == mode_server ) 
      return Game_Setup_Manager::server;
    else
      return Game_Setup_Manager::client;
  }

  void Network_Manager::set_display_handler( Game_Setup_Display_Handler *handler )
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

      // tell handler which board is currently active
      display_handler->set_board( game );
    }
    */
  }

  // board commands
  Game_Setup_Manager::Answer_Type Network_Manager::ask_change_board( const Game &new_game )
  {
    if( mode == mode_server )
    {
      game = new_game;
      report_ruleset_change();
      return accept;
    }
    if( mode == mode_client )
    {
      write_message_type( *client, msg_board );
      write_board( *client, new_game );
      return wait_for_answer;
    }
    return deny;
  }

  const Game &Network_Manager::get_board()
  {
    return game;
  }

  const std::list<Player> &Network_Manager::get_players()
  {
    return players;
  }

  // player commands
  bool Network_Manager::add_player( const Player &player )
  {
    if( mode == mode_server )
    {
      if( state == begin )
      {
	if( players.size() < game.get_max_players() )
	{
	  std::list<Player_Output*> outlist;
	  outlist.push_back(this);
	  
	  int id = current_id; ++current_id;
	  
	  std::list<Player>::iterator p = players.insert(players.end(), player);
	  p->id = id;
	  p->outputs.clear();
	  p->outputs.push_back( this );
	  id_player[id] = p;
	  
	  report_player_added( &*p );
	  return true;
	}
	else
	  ui_manager.report_error( _("Too many players!"), _("Add failed") );
      }
    }
    else
    {
      if( mode == mode_client )
      {
	if( state == handshake )
	{
	  requested_player_name = player.name;
	  requested_player_type = player.type;
	  state = request_player;
	  
	  write_message_type( *client, msg_player_request );
	  write_string( *client, player.name );
	  write_int( *client, player.type );
	  
	  return true;
	}
      }
    }
    return false;
  }
  bool Network_Manager::remove_player( const Player &player )
  {
    if( mode == mode_server )
    {
      if( (state == begin) || (state == is_ready) )
      {
	if( id_player.find(player.id) != id_player.end() )
	{
	  std::list<Player>::iterator player_it = id_player[player.id];
	  report_player_removed( &*player_it );
	  
	  players.erase(player_it);
	  id_player.erase( player.id ); 
	  Client *client = id_client[player.id];
	  id_client.erase( player.id );
	  // erase player from list of players hosted by this player
	  if( client )
	  {
	    std::list<std::list<Player>::iterator>::iterator client_player;
	    for( client_player = client->players.begin();
		 client_player != client->players.end(); ++client_player )
	    {
	      if( *client_player == player_it )
	      {
		client->players.erase(client_player);
		break;
	      }
	    }
	  }
	  return true;
	}
      }
    }
    if( mode == mode_client )
    {
      if( (state == begin) || (state == handshake) || (state == request_player) )
      {
	write_message_type( *client, msg_player_removed );
	write_int( *client, player.id );
	return true;
      }
    }
    return false;
  }
  bool Network_Manager::player_up( const Player &player )
  {
    if( mode == mode_server )
    {
      if( (state == begin) || (state == is_ready) )
      {
	if( id_player.find(player.id) != id_player.end() )
	{
	  if( id_player[ player.id ] != players.begin() ) 
	  {
	    std::list<Player>::iterator player1 = id_player[player.id];
	    std::list<Player>::iterator player2 = player1; --player2;
	    
	    report_player_up( &*player1 );

	    Player p1 = *player1;
	    players.erase(player1);
	    player1 = players.insert(player2,p1);	// insert player before player 2
    
	    id_player[p1.id] = player1;
	    return true;
	  }
	}
      }
    }
    if( mode == mode_client )
    {
      if( (state == begin) || (state == handshake) || (state == request_player) )
      {
	write_message_type( *client, msg_player_up );
	write_int( *client, player.id );
	return true;
      }
    }
    return false;
  }
  bool Network_Manager::player_down( const Player &player )
  {
    if( mode == mode_server )
    {
      if( (state == begin) || (state == is_ready) )
      {
	if( id_player.find(player.id) != id_player.end() )
	{
	  if( id_player[ player.id ] != --players.end() ) 
	  {
	    std::list<Player>::iterator player1 = id_player[player.id];
	    std::list<Player>::iterator player2 = player1; ++player2;
	    
	    report_player_down( &*player1 );

	    Player p2 = *player2;
	    players.erase(player2);
	    player2 = players.insert(player1,p2); // insert player 2 before player
    
	    id_player[p2.id] = player2;
	    return true;
	  }
	}
      }
    }
    if( mode == mode_client )
    {
      if( (state == begin) || (state == handshake) || (state == request_player) )
      {
	write_message_type( *client, msg_player_down );
	write_int( *client, player.id );
	return true;
      }
    }
    return false;
  }
  void Network_Manager::ready()      // ready with adding players
  {
    if( mode == mode_server )
    {
      if( state == begin )
      {
	state = is_ready;
	check_state();
      }
    }
    else
    {
      if( mode == mode_client )
      {
	if( state == handshake )
	{
	  state = is_ready;
	  write_message_type( *client, msg_ready );
	}
      }
    }
  }

  // is everyone ready and number of players ok?
  Game_Setup_Manager::Game_State Network_Manager::can_start() 
  {
    // check if all clients are ready
    if( mode == mode_server )
    {
      if( players.size() < game.get_min_players() )
      {
	return too_few_players;
      }

      check_state();

      if( (state == players_ready) || (state == game_started) )
      {
	return everyone_ready;
      }
    }
    else if( mode == mode_client )
    {
      if( state == game_started )
	return everyone_ready;
    }
    return not_ready;
  }

  void Network_Manager::start_game() // call only when can_start() == true
  {
    game_manager.set_board( game );
    game_manager.set_players( players );
    game_manager.start_game();
  }

  Game_Setup_Manager::Answer_Type Network_Manager::ask_new_game() // request to play new game
  {
    if( mode == mode_server )
    {
      state = begin;
      clients_ready = 0;
      clients_player_setup_ack = 0;

      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	c.state = handshake;	// already connected clients are in state handshake
	write_message_type( *c.socket, msg_new_game );      
      }
    }
    if( mode == mode_client )
    {
      state = handshake;
      write_message_type( *client, msg_new_game );
    }
    return wait_for_answer;
  }

  Game_Setup_Manager::Answer_Type Network_Manager::ask_undo_moves(int n) // request to play new game
  {
    //!!! implement this !!!
    return deny;
  }
  
  void Network_Manager::force_new_game()   // force new game (may close connections)
  {
    if( display_handler )	// there should be a display handler that may display a game setup dialog
      display_handler->game_setup(); // Attention: this opens a dialog which might destroy this object
  }

  void Network_Manager::stop_game()  // stop game
  {
    state = game_stop;		// avoid processing of network commands
  }

  // Player_Input functions
  Player_Input::Player_State Network_Manager::determine_move() throw(Exception)
  {
    if( state == move_received )
    {
      return finished;
    }
    assert( state == game_started );
    state = accept_move;
    sequence.clear();

    return wait_for_event;
  }

  Move_Sequence Network_Manager::get_move()
  {
    assert( state == move_received );

    // moue should be already done
    //sequence.do_sequence( game );

    state = game_started;
    return sequence;
  }

  long Network_Manager::get_used_time()
  {
    return 1;			// !!! measure time
  }

  // Player_Output functions
  void Network_Manager::report_move( const Move_Sequence &sequence )
  {
    switch( mode )
    {
      case mode_server:
      {
	// send setup of all players to all clients
	std::map<wxSocketBase*,Client>::iterator client_it;
	for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
	{
	  Client &c = client_it->second;
	  if( c.socket )
	  {
	    write_message_type( *c.socket, msg_report_move );
	
	    write_move( *c.socket, sequence );
	  }
	}
      }
      break;
      case mode_client:
      {
	write_message_type( *client, msg_report_move );
	write_move( *client, sequence );
      }
      break;
      case mode_connecting:
      case mode_undefined:
	break;
    }
  }

  void Network_Manager::on_network( wxSocketEvent &event )
  {
    switch( event.GetSocketEvent() )
    {
      case wxSOCKET_CONNECTION:
	on_connect( *event.GetSocket() );
	break;
      case wxSOCKET_INPUT:
	on_input( *event.GetSocket() );
	break;
      case wxSOCKET_OUTPUT:
	on_output( *event.GetSocket() );
	break;
      case wxSOCKET_LOST:
	on_lost( *event.GetSocket() );
	break;
    }
  }

  void Network_Manager::on_done( wxTimerEvent & )
  {
    state = move_received;
    continue_game();
  }

  void Network_Manager::on_connect( wxSocketBase& sock )
  {
    if( mode == mode_connecting )
    {
      mode = mode_client;
    }
    if( mode == mode_server )
    {
      assert(server);
     
      if( clients.size() == 0 )	// first client
      {
	// determine local network adress:
	//sock.GetLocal(localhost);
      }

      if( clients.size() < max_clients )
      {
	wxSocketBase *socket;
	socket = server->Accept(false);
	if( socket )
	{
	  Client &c = clients[socket];
	  c.socket = socket;
	  
	  socket->SetEventHandler(*this, NETWORK_EVENT);
	  socket->SetNotify(wxSOCKET_INPUT_FLAG | wxSOCKET_OUTPUT_FLAG | wxSOCKET_LOST_FLAG);
	  socket->Notify(TRUE);

	  if( connection_handler )
	  {
	    wxIPV4address host;
	    socket->GetPeer(host);
	    connection_handler->new_connection( host, socket );
	  }
	}
      }
      else
      {
	write_message_type( sock, msg_handshake_deny );
      }
    }
  }

  void Network_Manager::on_input( wxSocketBase& sock )
  {
    // disable input events
    sock.SetNotify(wxSOCKET_LOST_FLAG);
    
    Message_Type message_type = read_message_type(sock);

    switch( message_type )
    {
      case msg_handshake:
      {
	if( mode == mode_server )
	{
	  // client accepted handschake
	  Client &c = clients[ &sock ];
	  if( c.socket )
	  {
	    if( c.state == begin )
	    {
	      assert( c.socket == &sock );
	      c.state = handshake;

	      report_players( *c.socket );
	      report_ruleset_change();
	    }
	    else
	    {
	      write_message_type( sock, msg_illegal_request );
	    }
	  }
	  else
	  {
	    // unknown client...
	    clients.erase( &sock ); // erase entry if just added
	  }
	}
	if( mode == mode_connecting ) // handshake message prooves that client is connected
	  mode = mode_client;
	if( mode == mode_client )
	{
	  // server offers handschake
	  if( state == begin )
	  {
	    if( &sock == static_cast<wxSocketBase*>(client) )
	    {
	      state = handshake;
	      write_message_type( sock, msg_handshake ); // return handshake
	    }
	  }
	}
      }
      break;
      case msg_handshake_deny:
      {
	if( mode == mode_client )
	{
	  assert( &sock == static_cast<wxSocketBase*>(client) );
	  // server denied handshake
	  close_connection();
	}
      }
      break;
      case msg_player_request:
      {
	// ***********
	// parameters:

	// get name
	std::string name = read_string(sock);
	// get type
	Player::Player_Type type = Player::Player_Type(read_int(sock));

	if( mode == mode_server )
	{
	  // client requests to add player
	  if( ((state == begin) || (state == is_ready)) &&
	      (players.size() < game.get_max_players() ) )
	  {
	    Client &c = clients[&sock];
	    if( c.socket && (c.state == handshake) ) // is it a connected client?
	    {
	      // ******
	      // accept

	      switch(type)
	      {
		default: type = Player::unknown; break;
		case Player::user:
		case Player::ai:
		  break;
	      }
	      // get host
	      std::string host;
	      wxIPV4address addr;
	      if( sock.GetPeer(addr) )
	      {
		host = wxstr_to_str(addr.Hostname());
	      }

	      Player player( name, current_id, this, Player::no_output, host, type );

	      std::list<Player>::iterator p = players.insert(players.end(), player);
	      c.players.push_back( p );
	      id_player[current_id] = p;
	      id_client[current_id] = &c;

	      write_message_type( sock, msg_player_ack );
	      // return id for player to client
	      write_int( sock, current_id );

	      report_player_added( &*p );
	  
	      ++current_id;

	      // !!! player request might cause everyone to be ready
	    }
	    else
	    {
#ifndef __WXMSW__
	      if( !c.socket )
	      {
		std::cerr << std::endl << "error: unknown client" << std::endl;
		clients.erase( &sock ); // erase entry if just added
	      }
	      if( c.state != handshake )
		std::cerr << std::endl << "error: client not in state handshake" << std::endl;
#endif
	    }
	  }
	  else
	  {
#ifndef __WXMSW__
	    if( (state != begin) && (state != is_ready) )
	      std::cerr << std::endl << "error: wrong state" << std::endl;
	    if( players.size() < game.get_max_players() )
	      std::cerr << std::endl << "error: too many players" << std::endl;
#endif
	    // *****
	    // deny
	    write_message_type( sock, msg_player_deny );
	  }
	}
	else
	{
#ifndef __WXMSW__
	  std::cerr << std::endl << "error: client can't receive that message type" << std::endl;
#endif
	}
      }
      break;
      case msg_player_ack:
      {
	if( mode == mode_client )
	{
	  assert( &sock == static_cast<wxSocketBase*>(client) );
	  if( state == request_player )
	  {
	    state = handshake;
	    int id = read_int( sock );
	    Player player( requested_player_name, id, 0, Player::no_output,
			   "", requested_player_type );

	    std::list<Player>::iterator p = own_players.insert(own_players.end(), player);
	    id_own_player[id] = p;

	    // report that player was accepted
	    ui_manager.show_status_text( _("Player added successfully") );
	  }
	}
      }
      break;
      case msg_player_deny:
      {
	if( mode == mode_client )
	{
	  assert( &sock == static_cast<wxSocketBase*>(client) );
	  if( state == request_player )
	  {
	    state = handshake;
	    // report that player was denied
	    ui_manager.report_error( _("Adding of Player denied!"), _("Add failed") );
	  }
	}
      }
      break;
      case msg_ready:
      {
	if( mode == mode_server )
	{
	  // client tells to be ready with player adding
	  if( (state == begin) || (state == is_ready ) )
	  {
	    Client &c = clients[&sock];
	    if( c.socket )	// is it a connected client?
	    {
	      if( c.state == handshake )
	      {
		c.state = is_ready;
		++clients_ready;
	      
		check_state();
	      }
	    }
	    else
	    {
	      // unknown client...
	      clients.erase( &sock ); // erase entry if just added
	    }
	  }
	}
      }
      break;
      case msg_player_added:
      {
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    Player player = read_player( *client );
	    if( player.host == "" ) // is player hosted by server?
	    {
	      wxIPV4address adr;
	      sock.GetPeer( adr );      
	      player.host = wxstr_to_str(adr.Hostname());
	    }
	  
	    std::list<Player>::iterator p = players.insert(players.end(), player);
	    id_player[ player.id ] = p;

	    report_player_added( &*p );
	  }
	}
      }
      break;
      case msg_player_removed:
      {
	int id = read_int( sock );
	if( mode == mode_server )
	{
	  if( (state == begin) || (state == is_ready ) )
	  {
	    // if client is responsable for player
	    if( id_client[id]->socket == &sock )
	    {
	      remove_player( *id_player[id] );
	    }
	    else
	    {
	      write_message_type( sock, msg_player_change_deny );
	    }
	  }
	}
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    Player *player = &*id_player[ id ];
	    if( player )
	    {
	      report_player_removed( player );
	      // don't erase player from lists as it will be cleared before final setup is transmitted
	    }
	  }   
	}
      }
      break;
      case msg_player_up:
      {
	int id = read_int( sock );
	if( mode == mode_server )
	{
	  if( (state == begin) || (state == is_ready) )
	  {
	    // if client is responsable for player
	    if( id_client[id]->socket == &sock )
	    {
	      player_up( *id_player[id] );
	    }
	    else
	    {
	      write_message_type( sock, msg_player_change_deny );
	    }
	  }
	}
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || (state == is_ready) )
	  {
	    std::list<Player>::iterator player = id_player[id];
	    std::list<Player>::iterator player2 = player; --player2;
	    
	    report_player_up( &*player );

	    Player p1 = *player;
	    players.erase(player);
	    player = players.insert(player2,p1);	// insert player before player 2
    
	    id_player[p1.id] = player;
	  }
	}
      }
      break;
      case msg_player_down:
      {
	int id = read_int( sock );
	if( mode == mode_server )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    // if client is responsable for player
	    if( id_client[id]->socket == &sock )
	    {
	      player_down( *id_player[id] );
	    }
	    else
	    {
	      write_message_type( sock, msg_player_change_deny );
	    }
	  }
	}
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    std::list<Player>::iterator player = id_player[id];
	    std::list<Player>::iterator player2 = player; ++player2;
	    
	    report_player_down( &*player );

	    Player p2 = *player2;
	    players.erase(player2);
	    player2 = players.insert(player,p2); // insert player 2 before player
    
	    id_player[p2.id] = player2;
	  }
	}
      }
      break;
      case msg_player_change_deny:
      {
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    if( display_handler )
	      display_handler->player_change_denied();
	  }
	}
      }
      break;
      case msg_player_setup: 
      {
	if( mode == mode_client )
	{
	  assert( &sock == static_cast<wxSocketBase*>(client) );
	  if( state == is_ready )
	  {
	    state = players_ready;

	    players.clear();
	    id_player.clear();
	    int num_players = read_int( sock );
	  
	    for( int i = 0; i < num_players; ++i )
	    {
	      Player player = read_player( sock );
	      if( player.host == "" ) // is player hosted by server?
	      {
		wxIPV4address adr;
		sock.GetPeer( adr );
		player.host = wxstr_to_str(adr.Hostname());
	      }

	      if( id_own_player.find(player.id) != id_own_player.end() )
	      {
		Player_Input *input;
		if( player.type == Player::ai )
		  input = game_manager.get_player_ai();
		else
		  input = ui_manager.get_user_input();
		  
		player.input = input;
		player.host = "";
		  
		std::list<Player_Output*> outlist;
		outlist.push_back(this);
		player.outputs = outlist;
	      }
	      players.push_back( player );
	    }

	    write_message_type( sock, msg_player_setup_ack );
	  }
	}
      }
      break;
      case msg_player_setup_ack:
      {
	if( mode == mode_server )
	{
	  // client has recognized player setup
	  if( state == players_ready )
	  {
	    Client &c = clients[&sock];
	    if( c.socket )	// is it a connected client?
	    {
	      if( c.state == is_ready )
	      {
		c.state = players_ready;

		++clients_player_setup_ack;

		if( clients_player_setup_ack >= clients.size() )
		{
		  // send that game started to all clients
		  std::map<wxSocketBase*,Client>::iterator client_it;
		  for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
		  {
		    Client &c = client_it->second;
		    if( c.socket )
		    {
		      assert( c.state == players_ready );
		      c.state = game_started;
		    
		      write_message_type( *c.socket, msg_start_game );
		    }
		  }
		  state = game_started;

		  continue_game();
		}
	      }
	    }
	    else
	    {
	      // unknown client...
	      clients.erase( &sock ); // erase entry if just added
	    }
	  }
	}
      }
      break;
      case msg_board:
      {
	Game new_game = read_board( sock );
	if( mode == mode_server )
	{
	  if( (state == begin) || (state == is_ready) )
	  {
	    Client &c = clients[&sock];
	    if( c.socket )	// is it a connected client?
	    {
	      wxIPV4address host;
	      sock.GetPeer(host);

	      if( display_handler )
	      {
		if( display_handler->ask_change_board( new_game, host.Hostname() ) )
		{
		  ask_change_board( new_game );
		}
		else
		{
		  write_message_type( sock, msg_board_deny );
		  report_ruleset_change();
		}
	      }
	    }
	    else
	    {
	      // unknown client...
	      clients.erase( &sock ); // erase entry if just added
	    }
	  }
	}
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    game = new_game;
	    report_ruleset_change();
	  }
	}
      }
      break;
      case msg_board_deny:
      {
	if( mode == mode_client )
	{
	  if( (state == begin) || (state == handshake) || (state == request_player) || 
	      (state == is_ready) )
	  {
	    if( display_handler )
	      display_handler->board_change_denied();
	  }
	}
      }
      break;
      case msg_start_game:
      {
	// !!! when AIs are playing, we have to garantee that all
	// players are listening before the first player is sending his
	// move

	if( mode == mode_client )
	{
	  assert( &sock == static_cast<wxSocketBase*>(client) );
	  if( state == players_ready )
	  {
	    state = game_started;

	    continue_game();
	  }
	}
      }
      break;
      case msg_report_move:
      {
	if( mode == mode_server )
	{
	  // client reports a move
	  if( state == accept_move )
	  {
	    Client &c = clients[&sock];
	    if( c.socket )	// is it a connected client?
	    {
	      if( c.state == game_started )
	      {
		if( id_client[game_manager.get_game().current_player->id] == &c ) 
				// is the client responsible for player ?
		{
		  // read move
		  sequence = read_move( sock );
		  state = move_received;

		  // send move to clients, that didn't know that move:
		  std::map<wxSocketBase*,Client>::iterator client_it;
		  for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
		  {
		    Client &dest_client = client_it->second;
		    if( dest_client.socket != c.socket )
		    {
		      if( dest_client.socket )
		      {
			write_message_type( *dest_client.socket, msg_report_move );
		  
			write_move( *dest_client.socket, sequence );
		      }
		    }
		  }
		  state = wait_for_move;
		  ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE );
		}
	      }
	    }
	    else
	    {
	      // unknown client...
	      clients.erase( &sock ); // erase entry if just added
	    }
	  }
	}
	else
	{
	  // client reports a move
	  if( state == accept_move )
	  {
	    if( &sock == static_cast<wxSocketBase*>(client) )
	    {
	      // read move
	      sequence = read_move( sock );
	      state = move_received;
	    
	      ui_manager.do_move_slowly( sequence, this, ANIMATION_DONE );
	    }
	  }
	}
      }
      break;
      case msg_undo_request:
      {
      }
      break;
      case msg_undo_ack:
      {
      }
      break;
      case msg_undo_deny:
      {
      }
      break;
      case msg_abort_game:
      {
	if( connection_handler )
	  connection_handler->closed_connection( &sock );

	if( mode == mode_server )
	{
	  disconnect( &sock );
	}
	if( mode == mode_client )
	{
	  ui_manager.report_error( _("Server disconnected"), _("Disconnection") );

	  if( (state == begin) || (state == handshake) || (state == request_player) )
	  {
	    if( display_handler )
	      display_handler->aborted();
	  }
	}
	// in the future network players should be overtaken by AIs
      }
      break;
      case msg_new_game: 
      {
	if( (state == game_started) || (state == accept_move) || (state == move_received) ||
	    (state == game_stop ) )
	{
	  wxIPV4address host;
	  sock.GetPeer(host);
	  if( display_handler )
	  {
	    if( display_handler->ask_new_game( host.Hostname() ) )
	    {
	      force_new_game(); // Attention: this opens a dialog which might destroy this object
	    }
	  }
	}
      }
      break;
      case msg_illegal_request:
      {
      }
      break;
    }
    
    // enable input events
    sock.SetNotify(wxSOCKET_LOST_FLAG || wxSOCKET_INPUT_FLAG);
  }

  void Network_Manager::on_output( wxSocketBase& sock ) // start output
  {
    if( mode == mode_server )
    {
      write_message_type( sock, msg_handshake );
    }
  }

  void Network_Manager::on_lost( wxSocketBase& sock )
  {
    if( mode == mode_server )
    {
      switch(state)
      {
	case begin:
	case is_ready:
	{
	  std::map<wxSocketBase*,Client>::iterator client;
	  client = clients.find(&sock);
	  if( client != clients.end() )
	  {
	    Client &c = client->second;
	    // remove players of host
	    std::list<std::list<Player>::iterator>::iterator player;
	    for( player = c.players.begin(); player != c.players.end(); ++player )
	    {
	      players.erase(*player);
	    }
	    // remove host
	    clients.erase(client);
	  }
	}
	break;
	case players_ready:
	case game_started:
	case accept_move:
	case move_received:
	case game_stop:
	{
	  std::map<wxSocketBase*,Client>::iterator client;
	  client = clients.find(&sock);
	  if( client != clients.end() )
	  {
	    Client &c = client->second;
	    // remove players of host
	    std::list<std::list<Player>::iterator>::iterator player;
	    for( player = c.players.begin(); player != c.players.end(); ++player )
	    {
	      players.erase(*player); // should replace player with ai
	    }
	    // remove host
	    clients.erase(client);
	  }
	  //wxMessageBox(_("Connection lost!"), _("Network Message"), wxOK | wxICON_ERROR, &game_window);
	}
	break;
	case wait_for_move:
	  break;
	case handshake:
	case request_player:
	  assert( false );	// no state for server!
      }
    }
    if( mode == mode_client )
    {
      client->Destroy();		// destroy client after only connection is lost
      client = 0;

      mode = mode_undefined;
      state = begin;
      //wxMessageBox(_("Connection lost"), _("Network Message"), wxOK | wxICON_INFORMATION, &game_window);
    }
  }

  void Network_Manager::recount_client_stat()
  {
    if( mode == mode_server )
    {
      clients_ready = 0;
      clients_player_setup_ack = 0;
      
      // send setup of all players to all clients
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  if( c.state == is_ready )
	  {
	    ++clients_ready;
	  }
	  if( c.state == players_ready )
	  {
	    ++clients_player_setup_ack;
	  }
	}
      }	
    }
  }
  // forces some actions in certain states
  void Network_Manager::check_state()
  {
    // check if all clients are ready
    if( mode == mode_server )
    {
      if( state == is_ready )
      {
	if( clients_ready >= clients.size() )
	{
	  if( players.size() >= game.get_min_players() )
	  {
	    state = players_ready;

	    // make sure every client knows the current ruleset
	    report_ruleset_change();
	    
	    // send setup of all players to all clients
	    std::map<wxSocketBase*,Client>::iterator client_it;
	    for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
	    {
	      Client &c = client_it->second;
	      if( c.socket )
	      {
		write_message_type( *c.socket, msg_player_setup );
		
		assert( c.state == is_ready );
		
		write_int( *c.socket, players.size() );
		std::list<Player>::iterator player;
		for( player = players.begin(); player != players.end(); ++player )
		{
		  write_player( *c.socket, *player );
		}
	      }
	    }
	    /*
	    // insert players in game
	    if( gui_connected )
	      game_window->new_game( players, *ruleset );
	    else
	      game.players = players;

	    if( !clients.size() ) // if no clients
	    {
	      continue_game();	// don't wait for answers on player_setup request
	    }
	    */
	  }
	}
      }
    }
  }

  void Network_Manager::write_string( wxSocketBase& sock, std::string str )
  {
#ifndef __WXMSW__	
    std::cerr << str << " ";
#endif

    unsigned size = str.size();
    sock.Write( &size, sizeof(size) );
    sock.Write( str.c_str(), size );      
  }
  std::string Network_Manager::read_string( wxSocketBase& sock )
  {
    std::string ret;

    unsigned size;
    sock.WaitForRead( 0, timeout_sequencial_read );
    sock.Read( &size, sizeof(size) );
    if( size > max_string_size )
      size = max_string_size;

    char *buf = new char[size + 1];
    if( buf != 0 )
    {
      sock.WaitForRead( 0, timeout_sequencial_read );
      sock.Read(buf,size);
      buf[size] = 0;		// terminate C-string
      ret = buf;
      delete[] buf;
    }

#ifndef __WXMSW__
    std::cerr << ret << " ";
#endif

    return ret;
  }

  void Network_Manager::write_int( wxSocketBase& sock, int i )
  {
#ifndef __WXMSW__
    std::cerr << i << " ";
#endif
    sock.Write( &i, sizeof(i) );
  }

  int Network_Manager::read_int( wxSocketBase& sock )
  {
    int i;
    sock.WaitForRead( 0, timeout_sequencial_read );
    sock.Read( &i, sizeof(i) );

#ifndef __WXMSW__
    std::cerr << i << " ";
#endif

    return i;
  }

  void Network_Manager::write_message_type( wxSocketBase& sock, Message_Type type )
  {
#ifndef __WXMSW__
    std::cerr << std::endl << "------------------------------------" << std::endl;
    std::cerr << "Send: ";
    switch( type )
    {
      case msg_handshake: std::cerr << "handshake "; break;
      case msg_handshake_deny: std::cerr << "handshake_deny "; break;
      case msg_player_request: std::cerr << "player_request "; break;
      case msg_player_ack: std::cerr << "player_ack "; break;
      case msg_player_deny: std::cerr << "player_deny "; break;
      case msg_ready: std::cerr << "ready "; break;
      case msg_player_added: std::cerr << "player_added "; break;
      case msg_player_removed: std::cerr << "player_removed "; break;
      case msg_player_up: std::cerr << "player_up "; break;
      case msg_player_down: std::cerr << "player_down "; break;
      case msg_player_change_deny: std::cerr << "player_change_deny "; break;
      case msg_player_setup: std::cerr << "player_setup "; break; 
      case msg_player_setup_ack: std::cerr << "player_setup_ack "; break;
      case msg_board: std::cerr << "ruleset "; break;
      case msg_board_deny: std::cerr << "ruleset_deny "; break;
      case msg_start_game: std::cerr << "start_game "; break;
      case msg_report_move: std::cerr << "report_move "; break;
      case msg_undo_request: std::cerr << "undo_request "; break;
      case msg_undo_ack: std::cerr << "undo_ack "; break;
      case msg_undo_deny: std::cerr << "undo_deny "; break;
      case msg_abort_game: std::cerr << "abort_game "; break;
      case msg_new_game: std::cerr << "new_game "; break;
      case msg_illegal_request: std::cerr << "illegal_request "; break;
      default: std::cerr << "!!! wrong message type !!! "; break;
    }
#endif
    sock.Write( &type, sizeof(type) );
  }

  Network_Manager::Message_Type Network_Manager::read_message_type( wxSocketBase& sock )
  {
    Message_Type type;
    sock.Read( &type, sizeof(type) );

#ifndef __WXMSW__
    std::cerr << std::endl << "------------------------------------" << std::endl;
    std::cerr << "Received: ";
    switch( type )
    {
      case msg_handshake: std::cerr << "handshake "; break;
      case msg_handshake_deny: std::cerr << "handshake_deny "; break;
      case msg_player_request: std::cerr << "player_request "; break;
      case msg_player_ack: std::cerr << "player_ack "; break;
      case msg_player_deny: std::cerr << "player_deny "; break;
      case msg_ready: std::cerr << "ready "; break;
      case msg_player_added: std::cerr << "player_added "; break;
      case msg_player_removed: std::cerr << "player_removed "; break;
      case msg_player_up: std::cerr << "player_up "; break;
      case msg_player_down: std::cerr << "player_down "; break;
      case msg_player_change_deny: std::cerr << "player_change_deny "; break;
      case msg_player_setup: std::cerr << "player_setup "; break; 
      case msg_player_setup_ack: std::cerr << "player_setup_ack "; break;
      case msg_board: std::cerr << "ruleset "; break;
      case msg_board_deny: std::cerr << "ruleset_deny "; break;
      case msg_start_game: std::cerr << "start_game "; break;
      case msg_report_move: std::cerr << "report_move "; break;
      case msg_undo_request: std::cerr << "undo_request "; break;
      case msg_undo_ack: std::cerr << "undo_ack "; break;
      case msg_undo_deny: std::cerr << "undo_deny "; break;
      case msg_abort_game: std::cerr << "abort_game "; break;
      case msg_new_game: std::cerr << "new_game "; break;
      case msg_illegal_request: std::cerr << "illegal_request "; break;
      default: std::cerr << "!!! wrong message type !!! "; break;
    }
#endif

    return type;
  }

  void Network_Manager::write_player( wxSocketBase& sock, Player player )
  {
    write_int( sock, player.id );
    write_string( sock, player.name );
    write_string( sock, player.host );
    write_int( sock, player.type );
    write_int( sock, player.help_mode );
  }

  Player Network_Manager::read_player( wxSocketBase& sock )
  {
    int id		= read_int( sock );
    std::string name	= read_string( sock );
    std::string host	= read_string( sock );
    Player::Player_Type type = Player::Player_Type( read_int( sock ) );
    Player::Help_Mode help_mode = Player::Help_Mode( read_int( sock ) );

    // correct type value
    switch( type )
    {
      default: type = Player::unknown; break;
      case Player::ai:
      case Player::user:
	// type ok
	break;
    }
    // correct help_mode
    switch( help_mode )
    {
      default: help_mode = Player::no_help; break;
      case Player::no_help:
      case Player::show_possible_moves:
      case Player::show_hint:
	// type ok
	break;
    }

    std::list<Player_Output*> outputs; outputs.push_back(this);
    return Player( name, id, this, outputs, host, type );
  }
  
  void Network_Manager::write_move( wxSocketBase& sock, Move_Sequence sequence )
  {
#ifndef __WXMSW__
    std::cerr << sequence << " ";
#endif

#ifndef __OLD_GCC__
    std::ostringstream os;
    os << sequence;

    unsigned size = os.str().size();

    write_int( sock, size );
    sock.WriteMsg( os.str().c_str(), size );
#else
    char buf[4096];
    std::ostrstream os(buf,4096);
    os << sequence;

    unsigned size = os.pcount();
    write_int( sock, size );
    sock.WriteMsg( os.str(), size );
#endif
  }
  Move_Sequence Network_Manager::read_move( wxSocketBase& sock )
  {
    unsigned size = read_int( sock );
    if( size > max_string_size*20 ) size = max_string_size*20;

    char *buf = new char[size + 1];
    sock.WaitForRead( 0, timeout_sequencial_read * 3 );	// move sequence might get quite long
    sock.ReadMsg( buf, size );
    buf[size] = 0;		// terminate string
    std::string str = buf;
#ifndef __OLD_GCC__
    std::istringstream is(str);
#else
    std::istrstream is(str.c_str());
#endif

    Move_Sequence seq;
    is >> seq;

#ifndef __WXMSW__
    std::cerr << seq << " ";
#endif

    return seq;
  }

  void Network_Manager::write_board_base( wxSocketBase& sock, Board board )
  {
    write_int( sock, int(board.board_type) );
    if( board.board_type == Board::custom )
    {
      assert(false);
    }
  }

  Board Network_Manager::read_board_base( wxSocketBase& sock )
  {
    int i = read_int( sock );
    
    switch( Board::Board_Type(i) )
    {
      case Board::s37_rings: 
	return Board( (const int*) board_37, 
		      sizeof(board_37[0]) / sizeof(board_37[0][0]),
		      sizeof(board_37)    / sizeof(board_37[0]),
		      Board::s37_rings );
      case Board::s40_rings: 
	return Board( (const int*) board_40, 
		      sizeof(board_40[0]) / sizeof(board_40[0][0]),
		      sizeof(board_40)    / sizeof(board_40[0]),
		      Board::s40_rings );
      case Board::s44_rings: 
	return Board( (const int*) board_44, 
		      sizeof(board_44[0]) / sizeof(board_44[0][0]),
		      sizeof(board_44)    / sizeof(board_44[0]),
		      Board::s44_rings );
      case Board::s48_rings: 
	return Board( (const int*) board_48, 
		      sizeof(board_48[0]) / sizeof(board_48[0][0]),
		      sizeof(board_48)    / sizeof(board_48[0]),
		      Board::s48_rings );
      case Board::s61_rings: 
	return Board( (const int*) board_61, 
		      sizeof(board_61[0]) / sizeof(board_61[0][0]),
		      sizeof(board_61)    / sizeof(board_61[0]),
		      Board::s61_rings );
      case Board::custom:
	//read board fields
	//break;
      default:
#ifndef __WXMSW__
	std::cerr << "!!! Wrong board type !!!" << std::endl;
#endif
	// shouldn't happen
	break;
    }
    // return standard board
    return Board( (const int*) board_37, 
		  sizeof(board_37[0]) / sizeof(board_37[0][0]),
		  sizeof(board_37)    / sizeof(board_37[0]),
		  Board::custom );
  }

  void Network_Manager::write_common_stones( wxSocketBase& sock, Common_Stones common_stones )
  {
    write_int( sock, int(common_stones.get_type()) );
    if( common_stones.get_type() == Common_Stones::custom )
    {
      write_int( sock, common_stones.stone_count[ Stones::white_stone ] );
      write_int( sock, common_stones.stone_count[ Stones::grey_stone  ] );
      write_int( sock, common_stones.stone_count[ Stones::black_stone ] );
    }
  }

  Common_Stones Network_Manager::read_common_stones( wxSocketBase& sock )
  {
    int type = read_int(sock);
    switch( Common_Stones::Common_Stones_Type( type ) )
    {
      case Common_Stones::standard:   return Standard_Common_Stones();
      case Common_Stones::tournament: return Tournament_Common_Stones();
      case Common_Stones::custom:
      {
	int num_white = read_int(sock);
	int num_grey  = read_int(sock);
	int num_black = read_int(sock);
	return Custom_Common_Stones( num_white, num_grey, num_black );
      }
    }
    return Custom_Common_Stones( -1, -1, -1 );
  }

  void Network_Manager::write_win_condition( wxSocketBase& sock, Win_Condition *win_condition )
  {
    write_int( sock, int(win_condition->get_type()) );
    if( win_condition->get_type() == Win_Condition::generic )
    {
      Generic_Win_Condition *gwc = dynamic_cast<Generic_Win_Condition*>(win_condition);
      write_int( sock, gwc->num_white );
      write_int( sock, gwc->num_grey );
      write_int( sock, gwc->num_black );
      write_int( sock, gwc->num_all );
    }
  }

  Win_Condition *Network_Manager::read_win_condition( wxSocketBase& sock )
  {
    int type = read_int(sock);
    switch( Win_Condition::Win_Condition_Type( type ) )
    {
      case Win_Condition::standard:   return new Standard_Win_Condition(); 
      case Win_Condition::tournament: return new Tournament_Win_Condition(); 
      case Win_Condition::generic:
      {
	int num_white = read_int(sock);
	int num_grey  = read_int(sock);
	int num_black = read_int(sock);
	int num_all   = read_int(sock);
	return new Generic_Win_Condition( num_white, num_grey, num_black, num_all ); 
      }
      case Win_Condition::full_custom: 
	break;
    }
    return 0;
  }

  void Network_Manager::write_ruleset( wxSocketBase& sock, const Ruleset &ruleset )
  {
    write_board_base( sock, ruleset.board );
    write_common_stones( sock, ruleset.common_stones );
    write_win_condition( sock, ruleset.win_condition );
  }

  Ruleset *Network_Manager::read_ruleset( wxSocketBase& sock )
  {
    Board board			 = read_board_base( sock );
    Common_Stones common_stones  = read_common_stones( sock );
    Win_Condition *win_condition = read_win_condition( sock );

    return new Custom_Ruleset( board, common_stones, win_condition, 
			       new Standard_Coordinate_Translator(board) );
  }

  void Network_Manager::write_board( wxSocketBase& sock, const Game &game )
  {
    write_ruleset( sock, *game.ruleset );
    // write_moves
  }

  Game Network_Manager::read_board( wxSocketBase& sock )
  {
    Ruleset *ruleset = read_ruleset( sock );
    Game game( *ruleset );
    delete ruleset;
    // read_moves

    return game;
  }
  
  void Network_Manager::continue_game()
  {
    game_manager.continue_game();
  }

  void Network_Manager::report_player_added( Player *player )
  {
    if( mode == mode_server )
    {
      // tell all clients that a player was added
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  write_message_type( *c.socket, msg_player_added );
	  write_player (*c.socket, *player );
	}
      }
    }

    if( display_handler )
      display_handler->player_added(*player);
  }
  void Network_Manager::report_player_removed( Player *player )
  {
    if( mode == mode_server )
    {
      // tell all clients that a player was removed
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  write_message_type( *c.socket, msg_player_removed );
	  write_int (*c.socket, player->id );
	}
      }
    }

    if( display_handler )
      display_handler->player_removed(*player);
  }

  void Network_Manager::report_player_up( Player *player )
  {
    if( mode == mode_server )
    {
      // tell all clients that a player moved up
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  write_message_type( *c.socket, msg_player_up );
	  write_int (*c.socket, player->id );
	}
      }
    }

    if( display_handler )
      display_handler->player_up(*player);
  }
  void Network_Manager::report_player_down( Player *player )
  {
    if( mode == mode_server )
    {
      // tell all clients that a player moved down
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  write_message_type( *c.socket, msg_player_down );
	  write_int (*c.socket, player->id );
	}
      }
    }

    if( display_handler )
      display_handler->player_down(*player);
  }

  void Network_Manager::report_players( wxSocketBase &sock )
  {
    if( mode == mode_server )
    {
      std::list<Player>::iterator player;
      for( player = players.begin(); player != players.end(); ++player )
      {
	write_message_type( sock, msg_player_added );
	write_player( sock, *player );
      }
    }
  }

  void Network_Manager::report_ruleset_change()
  {
    if( mode == mode_server )
    {
      // tell all clients that the ruleset was changed
      std::map<wxSocketBase*,Client>::iterator client_it;
      for( client_it = clients.begin(); client_it != clients.end(); ++client_it )
      {
	Client &c = client_it->second;
	if( c.socket )
	{
	  write_message_type( *c.socket, msg_board );
	  write_board( *c.socket, game );
	}
      }
    }

    if( display_handler )
    {
      display_handler->set_board( game );
    }
  }

  Network_Manager::Client::Client()
    : socket(0), state(begin)
  {
  }
}

