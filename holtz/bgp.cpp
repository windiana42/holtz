/*
 * bgp.cpp
 * 
 * Board Game Protocol classes
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

#include "bgp.hpp"

#include "util.hpp"

namespace holtz
{
  // ----------------------------------------------------------------------------
  // BGP_Message
  // ----------------------------------------------------------------------------

  BGP_Message::BGP_Message( Message_Type type )
    : type(type)
  {
  }

  BGP_Message *BGP_Message::read_from_line( std::string line ) // returns 0 for parse error
  {
    int num = string_to_long(line).first; // read first number from ascii string
    switch( Message_Type(num) )
      {
      case text_message:
	{
	  Message_Text *msg = new Message_Text();
	  if( msg->read_from_line(line) )
	    return msg;
	}
      case distance_message:
	{
	  Message_Distance *msg = new Message_Distance();
	  if( msg->read_from_line(line) )
	    return msg;
	}
      case connect_message:
	{
	  Message_Connect *msg = new Message_Connect();
	  if( msg->read_from_line(line) )
	    return msg;
	}
      case get_server_message:
	{
	  return new BGP_Message(get_server_message);
	}
      case tell_server_message:
	{
	  Message_Tell_Server *msg = new Message_Tell_Server();
	  if( msg->read_from_line(line) )
	    return msg;
	}
      }
    return 0;
  }

  // ----------------------------------------------------------------------------
  // Message_Text
  // ----------------------------------------------------------------------------

  Message_Text::Message_Text( std::string sender, std::string text )
    : BGP_Message(text_message), sender(sender), text(text)
  {
  }

  Message_Text::Message_Text()
    : BGP_Message(text_message)
  {
  }

  bool Message_Text::read_from_line( std::string line ) // returns false on parse error
  {
    std::istringstream is(line);

    int msg_number;
    is >> msg_number >> unescape(sender) >> unescape(text);
    return true;
  }

  std::string Message_Text::write_to_line()
  {
    return escape(sender) + escape(text);
  }

  // ----------------------------------------------------------------------------
  // Message_Distance
  // ----------------------------------------------------------------------------

  Message_Distance::Message_Distance( std::string origin, int distance )
    : BGP_Message(distance_message), origin(origin), distance(distance)
  {
  }

  Message_Distance::Message_Distance()
    : BGP_Message(distance_message)
  {
  }

  bool Message_Distance::read_from_line( std::string line ) // returns false on parse error
  {
    std::istringstream is(line);
    std::escape_istream eis(is);

    int msg_number;
    eis >> msg_number >> origin >> distance;
    //is >> msg_number >> unescape(origin) >> distance;

    return true;
  }

  std::string Message_Distance::write_to_line()
  {
    std::ostringstream os;
    std::escape_ostream eos(os);

    eos << origin << distance;

    return os.str();
  }

  // ----------------------------------------------------------------------------
  // Message_Connect
  // ----------------------------------------------------------------------------

  Message_Connect::Message_Connect( std::string host, int port )
    : BGP_Message(connect_message), host(host), port(port)
  {
  }

  Message_Connect::Message_Connect()
    : BGP_Message(connect_message)
  {
  }

  bool Message_Connect::read_from_line( std::string line ) // returns false on parse error
  {
    std::istringstream is(line);
    std::escape_istream eis(is);

    int msg_number;
    eis >> msg_number >> host >> port;

    return true;
  }

  std::string Message_Connect::write_to_line()
  {
    return escape(host) + long_to_string(port);
  }

  // ----------------------------------------------------------------------------
  // Message_Tell_Server
  // ----------------------------------------------------------------------------

  Message_Tell_Server::Message_Tell_Server( int port )
    : BGP_Message(tell_server_message), port(port)
  {
  }

  Message_Tell_Server::Message_Tell_Server()
    : BGP_Message(tell_server_message)
  {
  }

  bool Message_Tell_Server::read_from_line( std::string line ) // returns false on parse error
  {
    unsigned digits = string_to_long(line).second; // read first number from ascii string
    if( !digits ) return false;

    std::string msg_text = line.substr( digits + 1 ); // remove message number

    port = string_to_long(msg_text).first;
    return true;
  }

  std::string Message_Tell_Server::write_to_line()
  {
    return long_to_string(port);
  }
}
