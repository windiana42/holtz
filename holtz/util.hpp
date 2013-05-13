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
#include <set>
#include <list>
#include <vector>
#include <algorithm>

#include <wx/wx.h>

#ifdef __OLD_GCC__
  #include <strstream>
#else
  #include <sstream>
#endif

namespace holtz
{
  static const double epsilon = 1e-8;

  template<typename T> inline T min(T t1, T t2) { return t1 < t2 ? t1 : t2; }
  template<typename T> inline T max(T t1, T t2) { return t1 > t2 ? t1 : t2; }
  template<typename T> inline T min(T t1, T t2, T t3) { return min(min(t1,t2),t3); }
  template<typename T> inline T max(T t1, T t2, T t3) { return max(max(t1,t2),t3); }

  inline int random( int min, int max )
  {
    assert( max >= min );
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
  // convert double to string with a certain number of digits after the decimal point
  // and with an asserted maximum number of digits before the point
  std::string double_to_string( double, int post_point_digits=-1, 
				int assert_max_pre_point_digits=-1 );

  inline std::string wxstr_to_str( const wxString &str )
  {
#if wxUSE_UNICODE
    return str.mb_str( wxConvUTF8 ).data();
#else
    return str.mb_str( wxConvUTF8 );
    //return str.c_str();
#endif
  }

  inline wxString str_to_wxstr( const std::string &str )
  {
#ifdef wxUSE_UNICODE
    return wxString(str.c_str(), wxConvUTF8);
#else
    return wxString(str.c_str(), wxConvUTF8);
#endif
  }

  inline bool is_equal( float v1, float v2 )
  {
    return fabs(v1 - v2) < epsilon;
  }
}
namespace std
{
#ifdef __OLD_GCC__
  class istringstream : public istrstream
  {
  public:
    istringstream( std::string );
  private:
    std::string str;
  };
#endif

  // returns an escaped string for transmission over streams (includes space separator at end)
  // escaped strings are quoted ("str"); quotes and backslashes are escaped
  std::string escape( const std::string str );

  // allows reading escaped strings
  // usage: cin >> unescape(str);
  class Escaped_String
  {
  public:
    Escaped_String( string &str );
    bool did_error_occur()	{ return error; }
    void set_error()		{ error=true; }
    void reset_error()		{ error=false; }
  private:
    string &str;
    bool error;
    friend istream &operator>>( istream &, Escaped_String& );
  };
  istream &operator>>( istream &, Escaped_String& );
  inline Escaped_String unescape( std::string &str ) { return Escaped_String(str); }

  // allows reading escaped strings from any istream with operator>>  
  class escape_istream 
  {
  public:
    escape_istream( istream & );

    inline escape_istream &operator>>( string &str )
    {
      Escaped_String estr(str);
      is >> estr;
      error = error || estr.did_error_occur();
      error = error || is.fail();
      return *this;
    }
    inline escape_istream &operator>>( bool &var )
    {
      int i;
      is >> i;
      if( (i < 0) || (i > 1) )	// i should be '1' for true or '0' for false
	error = true;
      else
	var = i ? true : false;		
      error = error || is.fail();
      return *this;
    }
    inline escape_istream &operator>>( int &i )
    { 
      is >> i;
      error = error || is.fail();
      return *this;
    }
    template<class T>
    inline escape_istream &operator>>( T &obj )
    { 
      is >> obj;
      error = error || is.fail();
      return *this;
    }
    bool did_error_occur()	{ return error; }
    void set_error()		{ error=true; }
    void reset_error()		{ error=false; }
  private:
    istream &is;
    bool error;
  };

  // for writing escaped strings to any ostream with operator<<
  // includes separator spaces after any item
  class escape_ostream 
  {
  public:
    escape_ostream( ostream & );

    inline escape_ostream &operator<<( const string &str )
    {
      os << escape(str);	// escape includes seperator space at the end
      return *this;
    }
    inline escape_ostream &operator<<( bool var )
    {
      os << (var ? 1:0) << ' ';	// output boolean as numbers
      return *this;
    }
    template<class T>
    inline escape_ostream &operator<<( T obj )
    { 
      os << obj << ' ';
      return *this;
    }
  private:
    ostream &os;
  };

  // tests whether map or set includes an element
  /*
  template<typename Container, typename Element>
  inline bool does_include( Container c, Element e )
  {
    return c.find(e) != c.end();
  }
  */
  template<typename Key, typename Element>
  inline bool does_include( map<Key,Element> c, Key e )
  {
    return c.find(e) != c.end();
  }

  template<typename Element>
  inline bool does_include( set<Element> c, Element e )
  {
    return c.find(e) != c.end();
  }

  // tests whether list includes an element
  template<typename Element>
  inline bool does_include( list<Element> c, Element e )
  {
    return find(c.begin(),c.end(),e) != c.end();
  }

  // tests whether vector includes an element
  template<typename Element>
  inline bool does_include( vector<Element> c, Element e )
  {
    return find(c.begin(),c.end(),e) != c.end();
  }

  // array operator on lists
  template<typename Element>
  inline typename list<Element>::iterator at_iterator( list<Element> &c, unsigned idx )
  {
    assert( c.size() > idx );
    typename list<Element>::iterator it = c.begin();
    for(unsigned i=0; i<idx; ++i )
      ++it;
    return it;
  }

  // array operator on lists
  /* overloading with const version doesn't seem to work well
  template<typename Element>
  inline typename list<Element>::const_iterator at_iterator( const list<Element> &c, unsigned idx )
  {
    assert( c.size() > idx );
    typename list<Element>::const_iterator it = c.begin();
    for(unsigned i=0; i<idx; ++i )
      ++it;
    return it;
  }
  */

  // array operator on lists
  template<typename Element>
  inline Element &at( list<Element> &c, unsigned idx )
  {
    assert( c.size() > idx );
    typename list<Element>::iterator it = c.begin();
    for(unsigned i=0; i<idx; ++i )
      ++it;
    return *it;
  }

  // array operator on lists
  /* overloading with const version doesn't seem to work well
  template<typename Element>
  inline Element const &at( const list<Element> &c, unsigned idx )
  {
    assert( c.size() > idx );
    typename list<Element>::const_iterator it = c.begin();
    for(unsigned i=0; i<idx; ++i )
      ++it;
    return *it;
  }
  */

  // tests whether map or set includes an element
  template<typename Container, typename Element>
  inline bool erase( Container c, Element e )
  {
    typename Container::iterator it;
    for( it=c.begin(); it!=c.end(); ++it )
    {
      if( *it == e )
      {
	c.erase(it);
	return true;
      }
    }
    return false;
  }

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

  template<typename T>
  vector<T> list_to_vector(const list<T> &l) {
    vector<T> ret(l.size());
    typename list<T>::const_iterator it; int i;
    for( it = l.begin(), i=0; it != l.end(); ++it, ++i )
      ret.at(i) = *it;
    return ret;
  }

  template<typename T>
  list<T> vector_to_list(const vector<T> &v) {
    list<T> ret;
    typename vector<T>::const_iterator it;
    for( it = v.begin(); it != v.end(); ++it )
      ret.push_back(*it);
    return ret;
  }
}
#endif
