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
}
#endif
