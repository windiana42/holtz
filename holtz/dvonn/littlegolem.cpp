/*
 * littlegolem.cpp
 * 
 * functions to read Dvonn files from littlegolem
 * 
 * Copyright (c) 2005 by Martin Trautmann (martintrautmann@gmx.de) 
 * 
 * This file may be distributed and/or modified under the terms of the 
 * GNU General Public License version 2 as published by the Free Software 
 * Foundation. 
 * 
 * This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 * WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * 
 */

#include "littlegolem.hpp"

#include <iostream>

namespace dvonn
{
  LG_Content scan_littlegolem_file( std::istream &is )
  {
    LG_Content ret, invalid; invalid.moves = -1; // invalid
    
    std::string str;
    char ch; 

    is >> str;
    if( str != "Event" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;
    
    getline(is,str,']');
    ret.event = str;

    /*
    is >> ch;
    if( ch != ']' ) 
      return invalid;
    */

    is >> str;
    if( str != "Site" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.site = str;

    is >> str;
    if( str != "White" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.white_name = str;

    is >> str;
    if( str != "Black" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.black_name = str;

    ret.moves = 0;
    int move_num, cmp_num;
    for( move_num = 1; is; ++move_num )
    {
      is >> cmp_num;
      if( !is )			// there might be white spaces at EOF
	return ret;

      if( move_num != cmp_num )
	return invalid;

      is >> ch;
      if( ch != '.' )
	return invalid;

      is >> str;		// move white
      ++ret.moves;
      is >> str;		// move black
      if( !is )			// last read was EOF
	return ret;
      else
	++ret.moves;
    }
    return invalid;
  }

  int load_littlegolem_file( std::istream &is, Game &game )
  {
    LG_Content ret;
    int invalid = -1;
    
    std::string str;
    char ch; 

    is >> str;
    if( str != "Event" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;
    
    getline(is,str,']');
    ret.event = str;

    /*
    is >> ch;
    if( ch != ']' ) 
      return invalid;
    */

    is >> str;
    if( str != "Site" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.site = str;

    is >> str;
    if( str != "White" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.white_name = str;

    is >> str;
    if( str != "Black" ) 
      return invalid;

    is >> ch;
    if( ch != '[' ) 
      return invalid;

    getline(is,str,']');
    ret.black_name = str;

    Ruleset *ruleset;
    ruleset = new Standard_Ruleset();

    ruleset->min_players = 2;	// exact 2 players
    ruleset->max_players = 2;	// exact 2 players
    game.reset_game( *ruleset );
    std::vector<Player> players;
    players.push_back( Player( ret.white_name, 50, 0 ) );
    players.push_back( Player( ret.black_name, 51, 0 ) );
    game.set_players( players );
    delete ruleset;
    Standard_Move_Translator move_translator( game.ruleset->coordinate_translator );
    Move_Sequence sequence;

    ret.moves = 0;
    int move_num, cmp_num;
    for( move_num = 1; is; ++move_num )
    {
      is >> cmp_num;
      if( !is )			// there might be white spaces at EOF
	return ret.moves;

      if( move_num != cmp_num )
	return invalid;

      is >> ch;
      if( ch != '.' )
	return invalid;

      // read white move
      sequence = move_translator.decode( is );
      if( !sequence.is_empty() ) // pass and resign cause empty moves
      {
	if( game.current_player->stone_type != Stones::white_stone )
	{
#ifndef __WXMSW__
	  std::cout << "Illegal Move " << ret.moves << ": " << "It should be the turn of white" << std::endl;
#endif
	  return ret.moves;
	}
	if( !sequence.check_sequence(game) ) 
	{
#ifndef __WXMSW__
	  std::cout << "Illegal Move " << ret.moves << ": " << sequence << std::endl;
#endif
	  return ret.moves;
	}
	game.do_move( sequence );
	++ret.moves;
#ifndef __WXMSW__
	std::cout << "Move " << ret.moves << ": " << sequence << std::endl;
	std::cout << "Current Player: " << game.current_player->name << std::endl;
#endif
      }

      if( !is )
	return ret.moves;

      // read black move
      sequence = move_translator.decode( is );
      if( !sequence.is_empty() ) // EOF, pass and resign cause empty moves
      {
	if( game.current_player->stone_type != Stones::black_stone )
	{
#ifndef __WXMSW__
	  std::cout << "Illegal Move " << ret.moves << ": " << "It should be the turn of white" << std::endl;
#endif
	  return ret.moves;
	}
	if( !sequence.check_sequence(game) ) 
	{
#ifndef __WXMSW__
	  std::cout << "Illegal Move " << ret.moves << ": " << sequence << std::endl;
#endif
	  return ret.moves;
	}
	game.do_move( sequence );
	++ret.moves;
#ifndef __WXMSW__
	std::cout << "Move " << ret.moves << ": " << sequence << std::endl;
	std::cout << "Current Player: " << game.current_player->name << std::endl;
#endif
      }

      if( !is )
	return ret.moves;
    }

    return ret.moves;
  }
}
