/*
 * pbm.hpp
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

#ifndef __DVONN_LITTLEGOLEM__
#define __DVONN_LITTLEGOLEM__

#include <iostream>

#include "dvonn.hpp"

namespace dvonn
{
  struct LG_Content
  {
    std::string event, site;
    int moves;
    std::string white_name, black_name;
  };

  // scans a PBM message for game id, moves and player names
  LG_Content scan_littlegolem_file( std::istream &is );

  // loads a PBM message from mail and returns the number of half moves it loaded into the game
  // movenumber 0 is a virtual move that initializes the game
  int load_littlegolem_file( std::istream &is, Game &game );
}

#endif
