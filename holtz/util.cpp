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
}
