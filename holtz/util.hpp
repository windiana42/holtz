/*
 * util.hpp
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

#ifndef __HOLTZ_UTIL__
#define __HOLTZ_UTIL__

#include <stdlib.h>
#include <time.h>
#include <assert.h>

#include <string>
#include <map>

#include <wx/wx.h>

#ifndef __OLD_GCC__
  #include <sstream>
#else
  #include <strstream>
#endif

namespace holtz
{
  inline int random( int min, int max )
  {
    assert( max > min );
    return (rand() % (max - min + 1)) + min;
  }

  inline int random( int num )
  {
    assert( num > 0 );
    return random( 0, num - 1 );
  }

  void randomize();

  unsigned next_prime( unsigned start );

  std::string long_to_string( long );
  std::pair<long,unsigned /*digits*/> string_to_long( std::string, int base = 10 );

  inline std::string wxstr_to_str( const wxString &str )
  {
#if wxUSE_UNICODE
    return str.mb_str( wxConvUTF8 ).data();
#else
    return str.c_str();
#endif
  }

  inline wxString str_to_wxstr( const std::string &str )
  {
#ifdef wxUSE_UNICODE
    return wxString(str.c_str(), wxConvUTF8);
#else
    return str.c_str();
#endif
  }
}
namespace std
{
  // returns an escaped string for transmission over streams (includes space separator at end)
  // escaped strings are quoted ("str"); quotes and backslashes are escaped
  std::string escape( const std::string str );

  // allows reading escaped strings
  // usage: cin >> unescape(str);
  class Escaped_String
  {
  public:
    Escaped_String( string &str );
  private:
    string &str;
    friend istream &operator>>( istream &, Escaped_String );
  };
  istream &operator>>( istream &, Escaped_String );
  inline Escaped_String unescape( std::string &str ) { return Escaped_String(str); }

  // allows reading escaped strings from any istream with operator>>  
  class escape_istream 
  {
  public:
    escape_istream( istream & );

    inline escape_istream &operator>>( string &str )
    {
      is >> unescape(str);
      return *this;
    }
    template<class T>
    inline escape_istream &operator>>( T &obj )
    { 
      is >> obj;
      return *this;
    }
  private:
    istream &is;
  };

  // for writing escaped strings to any ostream with operator<<
  // includes separator spaces after any item
  class escape_ostream 
  {
  public:
    escape_ostream( ostream & );

    template<class T>
    inline escape_ostream &operator<<( T obj )
    { 
      os << obj << ' ';
      return *this;
    }
    inline escape_ostream &operator<<( const string &str )
    {
      os << escape(str);	// escape includes seperator space at the end
      return *this;
    }
  private:
    ostream &os;
  };

  // returns true if pattern matches inside string
  inline bool contains( std::string str, std::string pattern )
  {
    return str.find(pattern) != std::string::npos;
  }
  inline bool contains( std::string str, char pattern )
  {
    return str.find(pattern) != std::string::npos;
  }

  // returns true if pattern matches exactly once
  inline bool contains_once( string str, string pattern )
  {
    string::size_type pos1 = str.find(pattern);
    string::size_type pos2 = str.rfind(pattern);
    return ( pos1 == pos2 ) && (pos1 != string::npos);
  }
  inline bool contains_once( string str, char pattern )
  {
    string::size_type pos1 = str.find(pattern);
    string::size_type pos2 = str.rfind(pattern);
    return ( pos1 == pos2 ) && (pos1 != string::npos);
  }

  // returns string before pattern
  inline string before_first( string str, string pattern )
  {
    return str.substr( 0, str.find(pattern) );
  }
  inline string before_first( string str, char pattern )
  {
    return str.substr( 0, str.find(pattern) );
  }

  // returns string after pattern
  inline string after_first( string str, string pattern )
  {
    // if pattern can't be found find returns -1 which returns the complete string
    return str.substr( str.find(pattern) + pattern.size(), string::npos );
  }
  inline string after_first( string str, char pattern )
  {
    // if pattern can't be found find returns -1 which returns the complete string
    return str.substr( str.find(pattern) + 1, string::npos );
  }

  // returns string before pattern
  inline string before_last( string str, string pattern )
  {
    return str.substr( 0, str.rfind(pattern) );
  }
  inline string before_last( string str, char pattern )
  {
    return str.substr( 0, str.rfind(pattern) );
  }

  // returns string after pattern
  inline string after_last( string str, string pattern )
  {
    // if pattern can't be found rfind returns -1 which returns the complete string
    return str.substr( str.rfind(pattern) + pattern.size(), string::npos );
  }
  inline string after_last( string str, char pattern )
  {
    // if pattern can't be found rfind returns -1 which returns the complete string
    return str.substr( str.rfind(pattern) + 1, string::npos );
  }

  // replace pattern with <replace> in str and return number of occurances
  int replace( string &str, string pattern, string replace, 
	       string::size_type from=0, string::size_type to=string::npos );
}
#endif
