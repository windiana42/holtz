/*
 * msg_net_templ.cpp
 * 
 * Netzwork access with message classes
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

#ifndef __HOLTZ_MESSAGE_NET_TEMPLATE_IMPLEMENTATION__
#define __HOLTZ_MESSAGE_NET_TEMPLATE_IMPLEMENTATION__

#include "msg_net.hpp"

namespace holtz
{
  // ----------------------------------------------------------------------------
  // Message_Network
  // ----------------------------------------------------------------------------

  // line_network will be destroyed by Message_Network
  template<class Message> Message_Network<Message>::Message_Network
  ( Line_Network *line_network, Message_Network_Handler<Message> *handler )
    : line_network(line_network), handler(handler)
  {
    // if handler is set to 0, tell line_network to buffer the lines
    if( !handler )
      line_network->set_handler(0);
    else
    {
      // make sure that on_connect is called when connection is already established (Client only)
      switch( line_network->get_connection_state() )
      {
	case Line_Network::connected:  
	  handler->on_connect(this);
	  break;
	case Line_Network::lost:  
	  handler->on_lost(this);
	  break;
	default:
	  break;
      }
      line_network->set_handler(this);
    }
  }

  template<class Message>
  Message_Network<Message>::~Message_Network()
  {
    line_network->destroy();
  }

  template<class Message>
  void Message_Network<Message>::send_message( Message *message ) 
  {
#ifndef __WXMSW__
    // !!! Debug output
    std::cerr << "send: "; message->print(std::cerr);
#endif
    line_network->send_line( long_to_string(message->get_type()) + ' ' + message->write_to_line() );
  }

  template<class Message>
  void Message_Network<Message>::flush()
  {
    line_network->flush();
  }

  template<class Message>
  void Message_Network<Message>::set_handler
  ( Message_Network_Handler<Message> *new_handler )
  {
    Message_Network_Handler<Message> *old_handler = handler;
    handler = new_handler;
    // if handler was 0, tell line_network to send lines to this object
    if( (new_handler) && (!old_handler) )
      line_network->set_handler(this);
    // if new_handler is set to 0, tell line_network to buffer the lines
    if( (!new_handler) && (old_handler) )
      line_network->set_handler(0);
  }

  template<class Message>
  void Message_Network<Message>::process_line( Line_Network *connection, std::string line )
  {
    assert( connection == line_network );
    assert( handler );
    handler->process_message( this, Message::read_from_line(line) );
  }
  // is called when connection is established (for Line_Network_Client only)
  template<class Message>
  void Message_Network<Message>::on_connect( Line_Network *connection )
  {
    assert( connection == line_network );
    if( handler )
      handler->on_connect(this);
  }
  // is called when connection was closed or couldn't be established
  template<class Message>
  void Message_Network<Message>::on_lost( Line_Network *connection )
  {
    assert( connection == line_network );
    if( handler )
      handler->on_lost(this);
  }
  // is called when an error occured
  template<class Message>
  void Message_Network<Message>::on_error( Line_Network *connection )
  {
    assert( connection == line_network );
    if( handler )
      handler->on_error(this);
  }

  // ----------------------------------------------------------------------------
  // Message_Network_Client
  // ----------------------------------------------------------------------------

  template<class Message>
  Message_Network_Client<Message>::Message_Network_Client
  ( std::string host, int port, Message_Network_Handler<Message> *handler, bool auto_flush )
    : Message_Network<Message>( new Line_Network_Client( host, port, 0, auto_flush ), handler )
  {
  }

  // ----------------------------------------------------------------------------
  // Message_Network_Server
  // ----------------------------------------------------------------------------

  template<class Message>
  Message_Network_Server<Message>::Message_Network_Server
  ( Message_Network_Server_Handler<Message> *server_handler, bool auto_flush )
    : server_handler(server_handler)
  {
    line_server = new Line_Network_Server( this, auto_flush );
  }

  template<class Message>
  Message_Network_Server<Message>::~Message_Network_Server()
  {
    line_server->destroy();
  }
  
  // returns true if bind to port succeded
  template<class Message>
  bool Message_Network_Server<Message>::bind( int port )	
  {
    return line_server->bind(port);
  }

  template<class Message>
  void Message_Network_Server<Message>::new_connection( Line_Network *line_network )
  {
    Message_Network<Message> *network = 
      new Message_Network<Message>( line_network, 0 /*no handler yet*/ );
    server_handler->new_connection( network );  // will call set_handler
    //line_network->set_handler(network);		// server_handler will call set_handler
  }
}

#endif
