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

namespace holtz
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

    is >> str;
    do				// search for "Summary of"
    {
      while( str != "Summary" )
      {
	if( !is )		// eof?
	{
#ifndef __WXMSW__
	  std::cout << "last string read: \"" << str << "\""<< std::endl;
#endif
	  return invalid;
	}

	is >> str;
      }
      is >> str;
    }while( str != "of" );

#ifndef __WXMSW__
    std::cout << "  \"Summary of\" found" << std::endl;
#endif

    is >> str; if( str != "Zertz" ) return invalid;
    is >> str; if( str != "Board" ) return invalid;
    is >> ret.id; if( ret.id < 0  ) return invalid;
    is >> str;
    do				// search for "Player 2"
    {
      while( str != "Player" )
      {
	if( !is ) return invalid;	// eof?
	is >> str;
      }
      is >> str;
    }while( str != "2" );

#ifndef __WXMSW__
    std::cout << "  \"Player 2\" found" << std::endl;
#endif

    // get player names
    is >> ret.player1;
    is >> ret.player2;
    is >> ret.from;

    int move_num = ret.from;
    for( int current_move_num = ret.from; ; ++current_move_num )
    {
      char next;
      // dump move
      is >> next;
      if( next == 'x' )		// knock out move
	is >> str;
      else			// set move
      {
	is >> str;

	is >> next;
	if( next == 'x' )		// some fields removed?
	  is >> str;
	else
	  is.unget();
      }

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
      if( isdigit(next ) )
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

    is >> str;
    do				// search for "Summary of"
    {
      while( str != "Summary" )
      {
	if( !is )		// eof?
	{
#ifndef __WXMSW__
	  std::cout << "last string read: \"" << str << "\""<< std::endl;
#endif
	  return -1;
	}

	is >> str;
      }
      is >> str;
    }while( str != "of" );

#ifndef __WXMSW__
    std::cout << "  \"Summary of\" found" << std::endl;
#endif

    is >> str; 
    if( str == "Zertz" )
    {
      if( from == 1 )		// if this is the first call of load_game setup a new game
      {
	Ruleset *ruleset = new Tournament_Ruleset();
	ruleset->min_players = 2;	// exact 2 players
	ruleset->max_players = 2;	// exact 2 players
	game.reset_game( *ruleset );
	delete ruleset;
      }
    }
    else
      return -1;

    is >> str; if( str != "Board" ) return -1;
    is >> board_num;

    is >> str;
    do				// search for "Player 2"
    {
      while( str != "Player" )
      {
	if( !is ) return -1;	// eof?

	is >> str;
      }
      is >> str;
    }while( str != "2" );

#ifndef __WXMSW__
    std::cout << "  \"Player 2\" found" << std::endl;
#endif

    std::string name;
    is >> name; game.add_player( Player( name, 50, 0 ) );
    is >> name; game.add_player( Player( name, 51, 0 ) );

#ifndef __WXMSW__
    std::cout << "Players: " << game.players.size() <<  std::endl;
#endif

    Standard_Move_Translator move_translator( game.ruleset->coordinate_translator, &game.board );

    int num_moves_read = 0;
    for( int current_move_num = from; (current_move_num < to) || (to < 0); ++current_move_num )
    {
      int move_num;
      Sequence sequence;
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
