/*
 * util.cpp
 * 
 * utility functions
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

#include "util.hpp"

#ifndef __OLD_GCC__
  #include <sstream>
#else
  #include <strstream>
#endif

#include <ctype.h>

namespace holtz
{
  void randomize()
  {
    srand( time( 0 ) );
  }

  unsigned next_prime( unsigned start )
  {
    if( !(start & 1) )
      ++start;

    unsigned i;
    bool found = false;
    for( i = start; true; i += 2 )
    {
      found = true;
      unsigned add = 5, factor_square = 9;
      for( unsigned factor = 3; factor_square <= i; ++factor, factor_square += add )
      {
	if( i % factor == 0 )
	{
	  found = false;
	  break;
	}
	add += 2;
      }
      if( found )
	break;
    }
    return i;
  }

  std::string long_to_string( long l )
  {
    std::string str;
#ifndef __OLD_GCC__
    std::ostringstream os;
    os << l;
    str = os.str();
#else
    char buf[4096];
    std::ostrstream os(buf,4096);
    os << l;
    str = os.str();
#endif
    return str;
  }

  std::pair<long,unsigned /*digits*/> string_to_long( std::string str, int base )
  {
    const char *start = str.c_str();
    char *end;
    long num = strtol( start, &end, base );

    return std::pair<long,unsigned /*digits*/>( num, end - start );
  }
}
namespace std
{
  // returns an escaped string for transmission over streams
  string escape( const string str )
  {
    string ret = str;

    bool escape = false;
    if( ret[0] == '\"' )
      escape = true;
    else
    {
      for( int i=0; i<ret.size(); ++i )
	if( isspace( ret[i] ) )
	{
	  escape = true;
	  break;
	}
    }

    if( escape )
    {
      replace( ret, "\\", "\\\\" );
      replace( ret, "\"", "\\\"" );
      ret = '\"' + ret + '\"';
    }

    return ret + ' ';
  }

  Escaped_String::Escaped_String( string &str )
    : str(str)
  {
  }
  istream &operator>>( istream &is, Escaped_String estr )
  {
    string &dest = estr.str;

    char c;

    is >> c;
    if( c == '"' )
    {
      dest = "";
      while(is)
      {
	is.get(c);
	if( c == '"' ) 
	  break;
	if( c == '\\' )
	  is.get(c);
	dest += c;
      }
    }
    else
    {
      is.putback(c);
      static_cast<istringstream&>(is) >> dest;
    }
    return is;
  }
  
  escape_istream::escape_istream( istream &is ) : is(is)
  {
  }

  escape_ostream::escape_ostream( ostream &os ) : os(os)
  {
  }

  // replace pattern with <replace> in str and return number of occurances
  int replace( string &str, string pattern, string replace, 
	       string::size_type from=0, string::size_type to=string::npos )
  {
    int replaces = 0;
    string::size_type pos,last_pos=from;
    for(;;)
    {
      pos = str.find(pattern, last_pos);
      if( (pos == string::npos) || ((to != string::npos) && (pos >= to)) ) break;
      str.replace( pos, pattern.size(), replace );
      last_pos = pos + replace.size();
      replaces++;
    }
    return replaces;
  }
}
