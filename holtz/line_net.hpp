/*
 * line_net.hpp
 * 
 * Netzwork access line by line
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

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/socket.h>
#include <string>

#ifndef __HOLTZ_LINE_NETZWORK__
#define __HOLTZ_LINE_NETZWORK__

#include "util.hpp"

namespace holtz
{
  class Line_Network;

  class Line_Network_Handler
  {
  public:
    virtual ~Line_Network_Handler();
  protected:
    friend class Line_Network;
    virtual void process_line( Line_Network *connection, std::string line ) = 0; 
				// line has no '\n' at end
    // is called when connection is established (for Line_Network_Client only)
    virtual void on_connect( Line_Network *connection ) = 0;
    // is called when connection was closed or couldn't be established
    virtual void on_lost( Line_Network *connection ) = 0;
    // is called when an error occured
    virtual void on_error( Line_Network *connection ) = 0;
  };


  class Line_Network : public wxEvtHandler
  {
  public:
    enum Connection_Type { none, connecting, connected, lost };

    Line_Network( wxSocketBase *socket, Line_Network_Handler *handler = 0, 
		  bool auto_flush = true );
    ~Line_Network();
    void destroy();

    // send line over network connection ('\n' or '\r\n' at the end is removed)
    void send_line( std::string line );
    void flush();
    bool is_connected() { if(!socket) return false; return socket->IsConnected(); }
    std::string get_remote_host();	// returns "" if not connected
    int get_remote_port();		// returns 0 if not bound
    std::string get_local_host();	// returns "" if not connected
    int get_local_port();		// returns 0 if not bound

    void set_handler( Line_Network_Handler *handler );
    void set_auto_flush( bool af ) { auto_flush = af; }
    wxSocketBase *get_socket() { return socket; }
    Connection_Type get_connection_state() { return connection_state; }
  protected:
    Connection_Type connection_state;
  private:
    void on_network( wxSocketEvent &event );
    void on_input();
    void do_send();

    Line_Network_Handler *line_handler;
    wxSocketBase *socket;
    std::string send_buffer, receive_buffer;

    bool destroyed;		// whether destroy() was called
    bool auto_flush;
  };

  class Line_Network_Client : public Line_Network
  {
  public:
    Line_Network_Client( std::string host, int port, Line_Network_Handler *handler, 
			 bool auto_flush = true );
  private:
    wxSocketClient *client_socket;
  };

  class Line_Network_Server_Handler
  {
  public:
    virtual ~Line_Network_Server_Handler() {}
  protected:
    friend class Line_Network_Server;
    virtual void new_connection( Line_Network *line_network ) = 0; // line_network must be destroyed
  };

  class Line_Network_Server : public wxEvtHandler
  {
  public:
    Line_Network_Server( Line_Network_Server_Handler *handler, 
			 bool auto_flush = true );
    ~Line_Network_Server();
    void destroy();

    bool bind( int port );	// returns true if bind to port succeded
    int get_port();		// returns 0 if not bound

    wxSocketBase *get_socket() { return server_socket; }
  private:
    void on_network( wxSocketEvent &event );

    Line_Network_Server_Handler *server_handler;

    wxSocketServer *server_socket;
    bool destroyed;		// whether destroy() was called
    bool auto_flush;
  };

  enum 
  {
    NETWORK_EVENT=200
  };
}
#endif
