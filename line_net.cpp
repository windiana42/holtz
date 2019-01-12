/*
 * line_net.cpp
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

#include "line_net.hpp"

#include <wx/app.h>

namespace holtz
{

  // ----------------------------------------------------------------------------
  // Line_Network_Handler
  // ----------------------------------------------------------------------------

  Line_Network_Handler::~Line_Network_Handler()
  {
  }

  // ----------------------------------------------------------------------------
  // Line_Network
  // ----------------------------------------------------------------------------

  Line_Network::Line_Network( wxSocketBase *socket, Line_Network_Handler *handler, bool auto_flush )
    : connection_state(none), line_handler(handler), socket(socket),  
      destroyed(false), auto_flush(auto_flush)
  {
    Connect( -1, wxEVT_SOCKET, 
	     (wxObjectEventFunction) (wxEventFunction) (wxSocketEventFunction) 
	     &Line_Network::on_network );

    if( socket )
    {
      if( socket->IsConnected() )
	connection_state = connected;
      socket->SetEventHandler(*this,-1);
      socket->SetNotify( wxSOCKET_CONNECTION_FLAG | wxSOCKET_OUTPUT_FLAG | 
			 wxSOCKET_INPUT_FLAG | wxSOCKET_LOST_FLAG );
      socket->Notify(TRUE);
      socket->SetFlags(wxSOCKET_NOWAIT);
    }
  }
  Line_Network::~Line_Network()
  {
    assert(destroyed);
  }
  void Line_Network::destroy()
  {
    destroyed = true;
    socket->Close();
    socket->Destroy();
    socket = 0;
    //wxPostDelete(this);

    //!!!figure out a way to delete this object after events are
    //processed and no half processed event is still on the stack
  }

  void Line_Network::send_line( std::string line )
  {
    send_buffer += line;
    if( send_buffer[send_buffer.size() - 1] != '\n' )
      send_buffer += '\n';
    else			// if string ends with '\n'
    {
      // remove '/r' if there is one before the '\n'
      if( send_buffer[send_buffer.size() - 2] == '\r' )
      {
	send_buffer.erase( send_buffer.size() - 2, 1 );
      }
    }

    if( !socket || destroyed || (connection_state != connected)) return; 

    if( auto_flush )
      flush();
    else
      do_send();
  }

  void Line_Network::flush()
  {
    if( !socket || destroyed || (connection_state != connected)) return; 

    wxSocketFlags flags = socket->GetFlags();
    // switch to blocking and send until done
    socket->SetFlags(wxSOCKET_WAITALL);
    while( send_buffer.size() )
    {
      do_send();
      if( send_buffer.size() && socket->Error())	// critical or non-critical error
      {
	socket->SetFlags(flags); // restore flags
	wxTheApp->Yield(true);	// allow to process on_lost events
	if(!socket) return;
	socket->SetFlags(wxSOCKET_WAITALL);
      }
    }
    if(!socket) return;
    socket->SetFlags(flags); // restore flags
  }

  std::string Line_Network::get_remote_host()	// returns "" if not connected
  {
    if( !socket || destroyed ) return "";

    wxIPV4address address;
    if( !socket->GetPeer(address) ) return "";
    //return wxstr_to_str(address.Hostname()); takes time and doesn't always give good answer
    return wxstr_to_str(address.IPAddress());
  }

  int Line_Network::get_remote_port()		// returns 0 if not bound
  {
    if( !socket || destroyed ) return 0;

    wxIPV4address address;
    if( !socket->GetPeer(address) ) return 0;
    return address.Service();
  }

  std::string Line_Network::get_local_host()	// returns "" if not connected
  {
    if( !socket || destroyed ) return "";

    wxIPV4address address;
    if( !socket->GetLocal(address) ) return "";
    return wxstr_to_str(address.Hostname());
  }

  int Line_Network::get_local_port()		// returns 0 if not bound
  {
    if( !socket || destroyed ) return 0;
    
    wxIPV4address address;
    if( !socket->GetLocal(address) ) return 0;
    return address.Service();
  }

  void Line_Network::on_network( wxSocketEvent &event )
  {
    if( !socket || destroyed ) return;

    switch( event.GetSocketEvent() )
    {
      case wxSOCKET_CONNECTION:
	if( line_handler )
	  line_handler->on_connect(this);
	connection_state = connected;
	flush();
	break;
      case wxSOCKET_INPUT:
	on_input();
	break;
      case wxSOCKET_OUTPUT:
	do_send();
	break;
      case wxSOCKET_LOST:
#ifndef __WXMSW__
	std::cerr << "Debug: connection lost" << std::endl;
#endif
	if( line_handler )
	  line_handler->on_lost(this);
	connection_state = lost;
	break;
    }
  }

  void Line_Network::on_input( )
  {
    if( !socket || destroyed ) return;

    char buffer[2048];
    do{
      assert(socket);
      socket->Read(buffer, 2048);
      assert(socket);
      receive_buffer.append(buffer, socket->LastCount());
    }while( socket->LastCount() > 0 );
    
    std::string::size_type pos;
    if( line_handler )
    {
      while( (pos = receive_buffer.find('\n', 0)) != std::string::npos )
      {
	std::string line = receive_buffer.substr(0,pos);
	receive_buffer = receive_buffer.substr(pos+1);
	line_handler->process_line( this, line );
	if(!line_handler) break;  // line_handler may have been unregistered
      }
    }
  }

  void Line_Network::do_send()
  {
    if( !socket || destroyed || (connection_state != connected) )
    {
      if( line_handler )
	line_handler->on_error(this);
      send_buffer = "";
      return;
    }

    if( !send_buffer.size() ) return;

    const char* buf = send_buffer.c_str();
    socket->Write(buf, send_buffer.size());
    if( !socket )  // may be Write yields => recheck socket
    {
      if( line_handler )
	line_handler->on_error(this);
      send_buffer = "";
      return;
    }
    if( socket->Error() && (socket->LastError() != wxSOCKET_WOULDBLOCK) )
    {
#ifndef __WXMSW__
      std::cerr << "socket error occured: " << socket->LastError() << std::endl;
#endif
      if( line_handler )
	line_handler->on_error(this);
      send_buffer = "";
    }
    else
    {
      send_buffer = send_buffer.substr( socket->LastCount() );
    }
  }

  void Line_Network::set_handler( Line_Network_Handler *handler )
  {
    line_handler = handler;
    if( handler )
    {
      on_input();
    }
  }

  // ----------------------------------------------------------------------------
  // Line_Network_Client
  // ----------------------------------------------------------------------------

  Line_Network_Client::Line_Network_Client( std::string host, int port,
					    Line_Network_Handler *handler, bool auto_flush )
    : Line_Network( client_socket = new wxSocketClient(), handler, auto_flush )
  {
    wxIPV4address address;
    address.Hostname( str_to_wxstr(host) );
    address.Service( port );

    connection_state = connecting;
    client_socket->Connect( address, false /*no wait for connect*/ );
  }

  // ----------------------------------------------------------------------------
  // Line_Network_Server
  // ----------------------------------------------------------------------------

  Line_Network_Server::Line_Network_Server( Line_Network_Server_Handler *handler, 
					    bool auto_flush )
    : server_handler(handler), server_socket(0), destroyed(false), auto_flush(auto_flush)
  {
    Connect( -1, wxEVT_SOCKET, 
	     (wxObjectEventFunction) (wxEventFunction) (wxSocketEventFunction) 
	     &Line_Network_Server::on_network );
  }

  Line_Network_Server::~Line_Network_Server()
  {
    assert( destroyed );
  }

  void Line_Network_Server::destroy()
  {
    if( server_socket )
    {
      server_socket->Close();
      server_socket->Destroy();
      server_socket = 0;
    }
    destroyed = true;

#if wxUSE_GUI
    if ( !wxPendingDelete.Member(this) )
      wxPendingDelete.Append(this);
#else
    delete this;
#endif
  }

  bool Line_Network_Server::bind( int port )		// returns true if bind to port succeded
  {
    if( server_socket )
      server_socket->Destroy();

    wxIPV4address address;
    address.AnyAddress();
    address.Service( port );
    server_socket = new wxSocketServer( address );

    server_socket->SetEventHandler(*this,-1);
    server_socket->SetNotify( wxSOCKET_CONNECTION_FLAG );
    server_socket->Notify(TRUE);
    server_socket->SetFlags(wxSOCKET_NOWAIT | wxSOCKET_REUSEADDR);

    return server_socket->Ok();
  }

  int Line_Network_Server::get_port() // returns 0 if not bound
  {
    if( !server_socket || destroyed ) return 0;
    
    wxIPV4address address;
    if( !server_socket->GetLocal(address) ) return 0;
    return address.Service();
  }

  void Line_Network_Server::on_network( wxSocketEvent &event )
  {
    if( !server_socket || destroyed ) return;

    switch( event.GetSocketEvent() )
    {
      case wxSOCKET_CONNECTION:
      {
	wxSocketBase *socket = server_socket->Accept(false);
	if( server_socket->Error() )
	{
	  socket = 0;
	}
	else if( socket )
	{
	  Line_Network *line_network = new Line_Network( socket, 0, auto_flush );
	  server_handler->new_connection( line_network );
	}
	break;
      }
      default:
	break;
    }
  }
}


