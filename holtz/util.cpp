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

#include <iostream>
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
    unsigned int seed = (unsigned int)time(0);
    std::cout << "Initial random seed is: " << seed << std::endl;
    srand(seed);
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
#else
    char buf[4096];
    std::ostrstream os(buf,4096);
#endif
    os << l;
    str = os.str();
    return str;
  }

  std::pair<long,unsigned /*digits*/> string_to_long( std::string str, int base )
  {
    const char *start = str.c_str();
    char *end;
    long num = strtol( start, &end, base );

    return std::pair<long,unsigned /*digits*/>( num, end - start );
  }

  std::string double_to_string( double number, int post_point_digits, 
				int assert_max_pre_point_digits )
  {
    std::string str;
#ifndef __OLD_GCC__
    std::ostringstream os;
#else
    char buf[4096];
    std::ostrstream os(buf,4096);
#endif
    if( post_point_digits >= 0 )
    {
      int pre_point_digits=0;
      double compare=1;
      while( compare < number )
      {
	compare *= 10; 
	++pre_point_digits;
      }
      if( assert_max_pre_point_digits >= 0 )
	assert( pre_point_digits <= assert_max_pre_point_digits );

      os.precision(post_point_digits + pre_point_digits);
    }
    os << number;
    str = os.str();
    return str;
  }

}
namespace std
{
#ifdef __OLD_GCC__
  istringstream::istringstream( std::string str )
    : istrstream((this->str=str).c_str(), str.size())
  {
  }
#endif

  // returns an escaped string for transmission over streams
  string escape( const string str )
  {
    string ret = str;

    bool escape = false;
    if( (ret.size() == 0) || (ret[0] == '\"') )
      escape = true;
    else
    {
      for( unsigned i=0; i<ret.size(); ++i )
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
    : str(str), error(false)
  {
  }
  istream &operator>>( istream &is, Escaped_String &estr )
  {
    bool ok = false;
    string &dest = estr.str;

    char c;

    is >> c;
    if( c == '"' )
    {
      dest = "";
      while(is.good())
      {
	is.get(c);
	if( c == '"' ) 
	{
	  ok = true;
	  break;
	}
	if( c == '\\' )
	  is.get(c);
	dest += c;
      }
    }
    else
    {
      is.putback(c);
      static_cast<istringstream&>(is) >> dest;
      ok = true;
    }

    if( !ok ) 
      estr.set_error();

    return is;
  }
  
  escape_istream::escape_istream( istream &is ) : is(is), error(false)
  {
  }

  escape_ostream::escape_ostream( ostream &os ) : os(os)
  {
  }

  // replace pattern with <replace> in str and return number of occurances
  int replace( string &str, string pattern, string replace, 
	       string::size_type from, string::size_type to )
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
