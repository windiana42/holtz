/*
 * pbm.hpp
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

#ifndef __DVONN_PBM__
#define __DVONN_PBM__

#include <iostream>

#include "dvonn.hpp"

namespace dvonn
{
  struct PBM_Content
  {
    int id;
    int from, to;
    std::string player1, player2;
  };

  bool operator<  ( const PBM_Content&, const PBM_Content& );
  bool operator>  ( const PBM_Content&, const PBM_Content& );
  bool operator<= ( const PBM_Content&, const PBM_Content& );
  bool operator>= ( const PBM_Content&, const PBM_Content& );

  // scans a PBM message for game id, moves and player names
  PBM_Content scan_pbm_file( std::istream &is );

  // loads a PBM message from mail and returns the number of half moves it loaded into the game
  // movenumber 0 is a virtual move that initializes the game
  int load_pbm_file( std::istream &is, Game &game, int from = 0, int to = -1 );
}

#endif
