/*
 * pbm.cpp
 * 
 * functions for PBM Server support
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

#include "pbm.hpp"

#include <iostream>

namespace dvonn
{
  bool operator<  ( const PBM_Content& c1, const PBM_Content& c2 )
  {
    if( c1.from < c2.from ) return true;
    if( c2.from > c2.from ) return false;
    if( c1.to   < c2.to   ) return true;
    if( c2.to   > c2.to   ) return false;
    return false;
  }
  bool operator>  ( const PBM_Content& c1, const PBM_Content& c2 )
  {
    if( c1.from > c2.from ) return true;
    if( c2.from < c2.from ) return false;
    if( c1.to   > c2.to   ) return true;
    if( c2.to   < c2.to   ) return false;
    return false;
  }
  bool operator<= ( const PBM_Content& c1, const PBM_Content& c2 )
  {
    return !(c1 > c2);
  }
  bool operator>= ( const PBM_Content& c1, const PBM_Content& c2 )
  {
    return !(c1 < c2);
  }

  PBM_Content scan_pbm_file( std::istream &is )
  {
    PBM_Content ret, invalid; invalid.id = -1; // invalid

    std::string str; 

    bool ok = false, new_game = false;
    is >> str;
    while( is )
    {
      // search for "Summary of Zertz Board <board_num>"
      if( str == "Summary" )
      {
	is >> str;
	if( str == "of" )
	{
#ifndef __WXMSW__
	  std::cout << "  \"Summary of\" found" << std::endl;
#endif

	  is >> str; 
	  if( str == "Dvonn" )
	  {
	    is >> str; 
	    if( str == "Board" )
	    {
	      is >> ret.id;
	      ok = true;
	      break;
	    }
	  }
	}
      }
      // check for "A new Dvonn board (<board_num>)"
      else if( str == "A" )
      {
#ifndef __WXMSW__
	std::cout << "  \"A\" found" << std::endl;
#endif
	is >> str;
	if( str == "new" )
	{
	  is >> str;
	  if( str == "Dvonn" )
	  {
	    is >> str;
	    if( str == "board" )
	    {
#ifndef __WXMSW__
	      std::cout << "  \"A new Dvonn board\" found" << std::endl;
#endif
	      char c;
	      is >> c;
	      if( c == '(' )
	      {
		is >> ret.id;
		if( ret.id > 0 )
		{
		  is >> c;
		  if( c == ')' )
		  {
		    new_game = true;
		    ok = true;
		    break;
		  }
		}
	      }
	    }
	  }
	}
      }
      else
	is >> str;
    }

    if( !ok )
    {
#ifndef __WXMSW__
      std::cout << "last string read: \"" << str << "\""<< std::endl;
#endif
      return invalid;
    }

    is >> str;
    while( str != "Black(b)" )
    {
      if( !is ) return invalid;	// eof?
      is >> str;
    }

#ifndef __WXMSW__
    std::cout << "  \"Black(b)\" found" << std::endl;
#endif

    // get player names
    is >> ret.player1;
    is >> ret.player2;

    if( new_game )		// new game
    {
      ret.from = 0;
      ret.to = 0;
      return ret;
    }

    is >> ret.from;
    int current_move_num = ret.from;
    if( ret.from == 1 )		// add the virtual move with number 0
      ret.from = 0;

    int move_num;
    for( ; ; ++current_move_num )
    {
      char next;
      // dump move
      is >> str;

      // check for automove
      is >> next;
      if( next == '(' )
      {
	is >> str;
	if( str != "Auto)" )
	  return invalid;
      }
      else
	is.unget();

      is >> next;
      if( isdigit(next) )
      {
	is.unget();
	is >> move_num;
	if( move_num != current_move_num + 1 ) return invalid;
      }
      else
      {
	ret.to = current_move_num;
	return ret;
      }
    }
    return invalid;
  }

  int load_pbm_file( std::istream &is, Game &game, int from, int to )
  {
    std::string str; int board_num;

    bool ok = false;

    is >> str;
    while( is )
    {
      // search for "Summary of Zertz Board <board_num>"
      if( str == "Summary" )
      {
	is >> str;
	if( str == "of" )
	{
#ifndef __WXMSW__
	  std::cout << "  \"Summary of\" found" << std::endl;
#endif

	  is >> str; 
	  if( str == "Dvonn" )
	  {
	    is >> str; 
	    if( str == "Board" )
	    {
	      is >> board_num;
	      ok = true;
	      break;
	    }
	  }
	}
      }
      // check for "A new Zertz board (<board_num>)"
      else if( str == "A" )
      {
#ifndef __WXMSW__
	std::cout << "  \"A\" found" << std::endl;
#endif
	is >> str;
	if( str == "new" )
	{
	  is >> str;
	  if( str == "Dvonn" )
	  {
	    is >> str;
	    if( str == "board" )
	    {
#ifndef __WXMSW__
	      std::cout << "  \"A new Dvonn board\" found" << std::endl;
#endif
	      char c;
	      is >> c;
	      if( c == '(' )
	      {
		is >> board_num;
		if( board_num > 0 )
		{
		  is >> c;
		  if( c == ')' )
		  {
		    ok = true;
		    break;
		  }
		}
	      }
	    }
	  }
	}
      }
      else
	is >> str;
    }

    if( !ok )
    {
#ifndef __WXMSW__
      std::cout << "last string read: \"" << str << "\""<< std::endl;
#endif
      return -1;
    }

    is >> str;
    while( str != "Black(b)" )
    {
      if( !is ) return -1;	// eof?
      
      is >> str;
    }

#ifndef __WXMSW__
    std::cout << "  \"Black(b)\" found" << std::endl;
#endif

    std::string name1, name2;
    is >> name1; 
    is >> name2; 

    int num_moves_read = 0;
    if( from == 0 )		// if this is the first call of load_game setup a new game
    {
      Ruleset *ruleset;
      ruleset = new Standard_Ruleset();

      ruleset->min_players = 2;	// exact 2 players
      ruleset->max_players = 2;	// exact 2 players
      game.reset_game( *ruleset );
      std::vector<Player> players;
      players.push_back( Player( name1, 50, 0 ) );
      players.push_back( Player( name2, 51, 0 ) );
      game.set_players( players );
      delete ruleset;
      ++num_moves_read;		// initializing game counts as move "0"
      ++from;			// start from real move "1"
    }

#ifndef __WXMSW__
    std::cout << "Players: " << game.players.size() <<  std::endl;
#endif

    Standard_Move_Translator move_translator( game.ruleset->coordinate_translator );

    for( int current_move_num = from; (current_move_num < to) || (to < 0); ++current_move_num )
    {
      int move_num;
      Move_Sequence sequence;
      do
      {
	char next;
	is >> next;
      
	if( isdigit(next) )
	  is.unget();
	else
	{
	  if( next == '(' )
	  {
	    is >> str;
	    if( str != "Auto)" )
	      return num_moves_read;
	  }
	  else
	  {
	    return num_moves_read;
	  }
	}

	move_num = -1; is >> move_num;
	if( move_num < 1 ) return num_moves_read;
#ifndef __WXMSW__
	std::cout << "Reading move: " << move_num <<  std::endl;
#endif
	// read move
	sequence = move_translator.decode( is );
      }while( move_num < current_move_num );

      if( (move_num > to) && (to > 0) ) return num_moves_read;

      if( !sequence.check_sequence(game) ) 
      {
#ifndef __WXMSW__
	std::cout << "Illegal Move " << num_moves_read << ": " << sequence << std::endl;
#endif
	return num_moves_read;
      }
      game.do_move( sequence );
      ++num_moves_read;
#ifndef __WXMSW__
      std::cout << "Move " << current_move_num << ": " << sequence << std::endl;
      std::cout << "Current Player: " << game.current_player->name << std::endl;
#endif
    }
    return num_moves_read;
  }
}
