/*
 * msg_net.hpp
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

#ifndef __HOLTZ_MESSAGE_NET__
#define __HOLTZ_MESSAGE_NET__

#include "line_net.hpp"

#include <string>

namespace holtz
{
  template<class Message> class Message_Network;
  template<class Message> class Message_Network_Server;

  template<class Message>
  class Message_Network_Handler
  {
  public:
    virtual ~Message_Network_Handler() {}
  protected:
    friend class Message_Network<Message>;
    // is called when message arrives
    virtual void process_message( Message_Network<Message> *sender, 
				  Message *message ) = 0;    
				// message must be deleted,
				// message==0: invalid message arrived
    // is called when connection is established (for Message_Network_Client only)
    virtual void on_connect( Message_Network<Message> *sender ) = 0;
    // is called when connection was closed or couldn't be established
    virtual void on_lost( Message_Network<Message> *sender ) = 0;
    // is called when an error occured
    virtual void on_error( Message_Network<Message> *sender ) = 0;
  };

  template<class Message>
  class Message_Network : public Line_Network_Handler
  {
  public:
    // line_network will be destroyed by Message_Network
    Message_Network( Line_Network *line_network, 
		     Message_Network_Handler<Message> *handler = 0 );
    ~Message_Network();
    void send_message( Message *message ); // message won't be deleted
    void flush();
    bool is_connected() { return line_network->is_connected(); }
    // the following functions return 0 or "" if not connected
    std::string get_remote_host()	{ return line_network->get_remote_host(); }
    int get_remote_port()		{ return line_network->get_remote_port(); }
    std::string get_local_host()	{ return line_network->get_local_host(); }
    int get_local_port()		{ return line_network->get_local_port(); }

    void set_handler( Message_Network_Handler<Message> *handler );
    void set_auto_flush( bool auto_flush ) { line_network->set_auto_flush(auto_flush); }
    Line_Network *get_line_network() { return line_network; }
    wxSocketBase *get_socket() { return line_network->get_socket(); }
  protected:
    virtual void process_line( Line_Network *connection, std::string line ); 
				// line has no '\n' at end
    // is called when connection is established (for Line_Network_Client only)
    virtual void on_connect( Line_Network *connection );
    // is called when connection was closed or couldn't be established
    virtual void on_lost( Line_Network *connection );
    // is called when an error occured
    virtual void on_error( Line_Network *connection );
  private:
    Line_Network *line_network;
    Message_Network_Handler<Message> *handler;
  };

  template<class Message>
  class Message_Network_Client : public Message_Network<Message>
  {
  public:
    Message_Network_Client( std::string host, int port, Message_Network_Handler<Message> *handler, 
			    bool auto_flush = true );
  };

  template<class Message>
  class Message_Network_Server_Handler
  {
  public:
    virtual ~Message_Network_Server_Handler() {}
  protected:
    friend class Message_Network_Server<Message>;
    virtual void new_connection( Message_Network<Message> *message_network ) = 0;  
				// message_network must be deleted!
				// ->set_handler() should be called
  };

  template<class Message>
  class Message_Network_Server : private Line_Network_Server_Handler
  {
  public:
    Message_Network_Server( Message_Network_Server_Handler<Message> *server_handler, 
			    bool auto_flush = true );
    ~Message_Network_Server();
  
    bool bind( int port );	// returns true if bind to port succeded
    int get_port() { return line_server->get_port(); } // returns 0 if not bound

    wxSocketBase *get_socket() { return line_server->get_socket(); }
  protected:
    virtual void new_connection( Line_Network *line_network );
  private:
    Message_Network_Server_Handler<Message> *server_handler;
  
    Line_Network_Server *line_server;
  };
}

#include "msg_net_templ.cpp"

#endif
