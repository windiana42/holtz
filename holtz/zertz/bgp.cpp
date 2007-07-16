/*
 * bgp.cpp
 * 
 * Board Game Protocol classes
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

#include "bgp.hpp"

#include "util.hpp"

#include <string>
#include <list>
#include <iostream>
#ifndef __OLD_GCC__
#include <sstream>
#endif

namespace zertz
{
  using namespace holtz;
  namespace BGP
  {

    // ============================================================================
    // ----------------------------------------------------------------------------
    // general help functions
    // ----------------------------------------------------------------------------
    // ============================================================================

    std::string to_string(Message_Type type)
    {
      switch(type)
      {
        case msg_helo: return "helo";
        case msg_list_protocols: return "list_protocols";
        case msg_disconnect: return "disconnect";
        case msg_accept: return "accept";
        case msg_deny: return "deny";
        case msg_defer: return "defer";
        case msg_try_again: return "try_again";
        case msg_get_rooms: return "get_rooms";
        case msg_tell_rooms: return "tell_rooms";
        case msg_choose_room: return "choose_room";
        case msg_get_phase: return "get_phase";
        case msg_tell_phase: return "tell_phase";
        case msg_get_game: return "get_game";
        case msg_tell_game: return "tell_game";
        case msg_get_setup: return "get_setup";
        case msg_tell_setup: return "tell_setup";
        case msg_get_moves: return "get_moves";
        case msg_tell_moves: return "tell_moves";
        case msg_get_situation: return "get_situation";
        case msg_tell_situation: return "tell_situation";
        case msg_request_final_setup: return "request_final_setup";
        case msg_request_move_reminder: return "request_move_reminder";
        case msg_setup: return "setup";
        case msg_get_setup_and_changes: return "get_setup_and_changes";
        case msg_get_moves_and_play: return "get_moves_and_play";
        case msg_get_situation_and_play: return "get_situation_and_play";
        case msg_add_player: return "add_player";
        case msg_accept_player: return "accept_player";
        case msg_setup_change: return "setup_change";
        case msg_ask_setup_change: return "ask_setup_change";
        case msg_ask_capable: return "ask_capable";
        case msg_incapable: return "incapable";
        case msg_ready: return "ready";
        case msg_start_game: return "start_game";
        case msg_your_turn: return "your_turn";
        case msg_move: return "move";
        case msg_undo: return "undo";
        case msg_ask_undo: return "ask_undo";
        case msg_new_game: return "new_game";
        case msg_ask_new_game: return "ask_new_game";
        case msg_chat: return "chat";
        case msg_set_nick: return "set_nick";
        case msg_nick_change: return "nick_change";
        case msg_ping: return "ping";
        case msg_pong: return "pong";
        case msg_error: return "error";
      }
    }

    // ============================================================================
    // ----------------------------------------------------------------------------
    // get/put help functions
    // ----------------------------------------------------------------------------
    // ============================================================================

    // ----------------------------------------------------------------------------
    // <board> : is array of <field_type>
    // ----------------------------------------------------------------------------
    //   <rows> <columns> { {<field_type>}*(row) }*(column)

    bool read_board( std::escape_istream &eis, Board &board )
    {
      int columns, rows;
      eis >> rows >> columns;
      std::vector< std::vector<Field_State_Type> > fields;
      fields.resize(rows);
      for( int row=0; row<rows; ++row )
	fields[row].resize(columns);

      for( int col=0; col<columns; ++col )
      {
	for( int row=0; row<rows; ++row )
	{
	  int field_type;
	  eis >> field_type;
	  switch( Field_Type(field_type) )
	  {
	    case field_removed:
	    case field_empty:
	    case field_white:
	    case field_grey:
	    case field_black:
	    {
	      fields[row][col] = Field_State_Type(field_type);
	      continue;		// field type ok => continue loop
	    }
	  }
	  return false;		// if field type is invalid
	}
      }
      board = Board(fields);
      return true;
    }

    void write_board( std::escape_ostream &eos, const Board &board )
    {
      int columns=board.get_y_size(), rows=board.get_x_size();
      eos << rows << columns;

      for( int col=0; col<columns; ++col )
      {
	for( int row=0; row<rows; ++row )
	{
	  eos << board.field[row][col];
	}
      }
    }

    // ----------------------------------------------------------------------------
    // <start_board> : is one of
    // ----------------------------------------------------------------------------
    //   board_37
    //   board_40
    //   board_44
    //   board_48
    //   board_61
    //   board_custom <board>
    
    bool read_start_board( std::escape_istream &eis, Board &start_board )
    {
      int type;
      eis >> type;
      switch(Start_Board_Type(type))
      {
	case board_37_rings:
	{
	  start_board = Board( (const int*) board_37, 
			       sizeof(board_37[0]) / sizeof(board_37[0][0]),
			       sizeof(board_37)    / sizeof(board_37[0]),
			       Board::s37_rings );
	  return true;
	}
	case board_40_rings:
	{
	  start_board = Board( (const int*) board_40, 
			       sizeof(board_40[0]) / sizeof(board_40[0][0]),
			       sizeof(board_40)    / sizeof(board_40[0]),
			       Board::s40_rings );
	  return true;
	}
	case board_44_rings:
	{
	  start_board = Board( (const int*) board_44, 
			       sizeof(board_44[0]) / sizeof(board_44[0][0]),
			       sizeof(board_44)    / sizeof(board_44[0]),
			       Board::s44_rings );
	  return true;
	}
	case board_48_rings:
	{
	  start_board = Board( (const int*) board_48, 
			       sizeof(board_48[0]) / sizeof(board_48[0][0]),
			       sizeof(board_48)    / sizeof(board_48[0]),
			       Board::s48_rings );
	  return true;
	}
	case board_61_rings:
	{
	  start_board = Board( (const int*) board_61, 
			       sizeof(board_61[0]) / sizeof(board_61[0][0]),
			       sizeof(board_61)    / sizeof(board_61[0]),
			       Board::s61_rings );
	  return true;
	}
	case board_custom:
	{
	  return read_board( eis, start_board );
	}
      }
      return false;
    }

    void write_start_board( std::escape_ostream &eos, const Board &start_board )
    {
      eos << start_board.get_type();
      if( start_board.get_type() == Board::custom )
      {
	write_board( eos, start_board );
      }
    }
    
    // ----------------------------------------------------------------------------
    // <common_stones> : is one of
    // ----------------------------------------------------------------------------
    //   stones_standard
    //   stones_tournament
    //   stones_custom <white_stones> <grey_stones> <black_stones>

    bool read_common_stones( std::escape_istream &eis, Common_Stones &common_stones )
    {
      int type;
      eis >> type;
      switch(Common_Stones_Type(type))
      {
	case stones_standard:
	{
	  common_stones = Standard_Common_Stones();
	  return true;
	}
	case stones_tournament:
	{
	  common_stones = Tournament_Common_Stones();
	  return true;
	}
	case stones_custom:
	{
	  int white_stones, grey_stones, black_stones;

	  eis >> white_stones >> grey_stones >> black_stones;

	  common_stones = Custom_Common_Stones( white_stones, grey_stones, black_stones );
	  return true;
	}
      }
      return false;
    }

    void write_common_stones( std::escape_ostream &eos, Common_Stones common_stones )
    {
      eos << common_stones.get_type();
      if( common_stones.get_type() == Common_Stones::custom )
      {
	eos << common_stones.stone_count[Stones::white_stone];
	eos << common_stones.stone_count[Stones::grey_stone];
	eos << common_stones.stone_count[Stones::black_stone];
      }
    }
    
    // ----------------------------------------------------------------------------
    // <win_condition> : is one of
    // ----------------------------------------------------------------------------
    //   win_standard
    //   win_tournament
    //   win_generic <white_stones> <grey_stones> <black_stones> <all_stones> 

    bool read_win_condition( std::escape_istream &eis, Win_Condition *&win_condition )
    {
      int type;
      eis >> type;
      switch(Win_Condition_Type(type))
      {
	case win_standard:
	{
	  win_condition = new Standard_Win_Condition();
	  return true;
	}
	case win_tournament:
	{
	  win_condition = new Tournament_Win_Condition();
	  return true;
	}
	case win_generic:
	{
	  int white_stones, grey_stones, black_stones, all_stones;

	  eis >> white_stones >> grey_stones >> black_stones >> all_stones;

	  win_condition = new Generic_Win_Condition( white_stones, grey_stones, black_stones, 
						     all_stones );
	  return true;
	}
      }
      return false;
    }

    void write_win_condition( std::escape_ostream &eos, Win_Condition *win_condition )
    {
      assert( win_condition->get_type() != Win_Condition::full_custom );
      eos << win_condition->get_type();
      if( win_condition->get_type() == Win_Condition::generic )
      {
	Generic_Win_Condition *gwin = dynamic_cast<Generic_Win_Condition*>(win_condition);
	if( gwin )
	{
	  eos << gwin->num_white;
	  eos << gwin->num_grey;
	  eos << gwin->num_black;
	  eos << gwin->num_all;
	}
      }
    }
    
    // ----------------------------------------------------------------------------
    // <ruleset> : is one of
    // ----------------------------------------------------------------------------
    //   ruleset_standard
    //   ruleset_tournament
    //   ruleset_custom <start_board> <common_stones> <win_condition> <min_players> <max_players>

    bool read_ruleset( std::escape_istream &eis, Ruleset &ruleset )
    {
      int type;
      eis >> type;
      switch(Ruleset_Type(type))
      {
	case ruleset_standard:
	{
	  ruleset = Standard_Ruleset();
	  return true;
	}
	case ruleset_tournament:
	{
	  ruleset = Tournament_Ruleset();
	  return true;
	}
	case ruleset_custom:
	{
	  Board board;
	  Common_Stones common_stones;
	  Win_Condition *win_condition;
	  unsigned min_players, max_players;

	  if( !read_board(eis,board) ) return false;
	  if( !read_common_stones(eis,common_stones) ) return false;
	  if( !read_win_condition(eis,win_condition) ) return false;
	  eis >> min_players >> max_players;

	  ruleset = Custom_Ruleset( board, common_stones, win_condition, 
				    new Standard_Coordinate_Translator(board), true,
				    min_players, max_players );
	  return true;
	}
      }
      return false;
    }

    void write_ruleset( std::escape_ostream &eos, const Ruleset &ruleset )
    {
      eos << ruleset.get_type();
      if( ruleset.get_type() == Ruleset::custom )
      {
	write_board( eos, ruleset.board );
	write_common_stones( eos, ruleset.common_stones );
	write_win_condition( eos, ruleset.win_condition );
	eos << ruleset.min_players << ruleset.max_players;
      }
    }

    // ----------------------------------------------------------------------------
    // <players> : consists of
    // ----------------------------------------------------------------------------
    //   <player_cnt> { <player_id> <player_name> <white_stones> <grey_stones> <black_stones> }*

    bool read_players( std::escape_istream &eis, std::list<Player> &players )
    {
      players.clear();

      int player_cnt;
      eis >> player_cnt;

      int player_id;
      std::string player_name;
      int white_stones, grey_stones, black_stones;

      for( int i=0; i<player_cnt; ++i )
      {
	eis >> player_id >> player_name >> white_stones >> grey_stones >> black_stones;

	// check input
	if( (white_stones < 0) || (grey_stones < 0) || (black_stones < 0) )
	  return false;
	std::list<Player>::const_iterator i;
	for( i = players.begin(); i != players.end(); ++i )
	{
	  if( i->id == player_id )
	    return false;
	}
	
	// input ok:
	Player player(player_name, player_id);
	player.stones.stone_count[Stones::white_stone] = white_stones;
	player.stones.stone_count[Stones::grey_stone ] = grey_stones;
	player.stones.stone_count[Stones::black_stone] = black_stones;
	players.push_back( player );
      }
      return true;
    }

    void write_players( std::escape_ostream &eos, const std::list<Player> &players )
    {
      eos << players.size();

      std::list<Player>::const_iterator i;
      for( i = players.begin(); i != players.end(); ++i )
      {
	eos << i->id << i->name;
	eos << i->stones.stone_count.find(Stones::white_stone)->second;
	eos << i->stones.stone_count.find(Stones::grey_stone)->second;
	eos << i->stones.stone_count.find(Stones::black_stone)->second;
      }
    }

    // ----------------------------------------------------------------------------
    // <player_setting> : consists of
    // ----------------------------------------------------------------------------
    //   <player_name> <host_nick> <player_type> <help_mode>

    bool read_player_setting( std::escape_istream &eis, Player &player )
    {
      std::string player_name, host_nick;
      int player_type, help_mode;
      eis >> player_name >> host_nick >> player_type >> help_mode;

      // check input
      switch(Player_Type(player_type))
      {
	case player_unknown:
	case player_user:
	case player_ai:
	{
	  switch(Help_Mode_Type(help_mode))
	  {
	    case help_none:
	    case help_show_possible_moves:
	    case help_ai_hints:
	    {
	      player = Player( player_name );
	      player.host = host_nick;
	      player.type = Player::Player_Type(player_type);
	      player.help_mode = Player::Help_Mode(help_mode);
	      return true;
	    }
	  }
	}
      }
      return false;
    }
    
    void write_player_setting( std::escape_ostream &eos, const Player &player )
    { 
      eos << player.name << player.host << player.type << player.help_mode;
    }

    // ----------------------------------------------------------------------------
    // <player_settings> : is array of <player_setting>
    // ----------------------------------------------------------------------------
    //   <player_cnt> { <player_id> <player_settings> }*

    bool read_player_settings( std::escape_istream &eis, std::list<Player> &players )
    {
      players.clear();

      int player_cnt;
      eis >> player_cnt;

      int player_id;
      Player player;

      for( int i=0; i<player_cnt; ++i )
      {
	eis >> player_id;
	if( !read_player_setting(eis,player) ) return false;
	player.id = player_id;

	// check input
	std::list<Player>::const_iterator i;
	for( i = players.begin(); i != players.end(); ++i )
	{
	  if( i->id == player_id )
	    return false;
	}
	
	// input ok:
	players.push_back( player );
      }
      return true;
    }
    
    void write_player_settings( std::escape_ostream &eos, const std::list<Player> &players )
    {
      eos << players.size();

      std::list<Player>::const_iterator i;
      for( i = players.begin(); i != players.end(); ++i )
      {
	eos << i->id;
	write_player_setting(eos,*i);
      }
    }

    // ----------------------------------------------------------------------------
    // <setup_change> : is one of
    // ----------------------------------------------------------------------------
    /*
    enum Setup_Change_Type
    {
      change_board=0,		// board <start_board>
				// = changing the board empties initial moves
      change_common_stones,	// common_stones <common_stones>
      change_win_condition,	// win_condition <win_condition>
      change_min_players,	// min_players <n>
      change_max_players,	// max_players <n>
      change_initial_players,	// initial_players <initial_players>
      change_moves,		// moves <moves>
				// = set up initial moves

      change_insert_player=20,	// insert_player <id> <n> <player_settings>
				// = insert new player with id at n-th position (count from zero)
      change_remove_player,	// remove_player <id>
      change_move_player_up,	// move_player_up <id>
      change_move_player_down,	// move_player_down <id>
    };
    */

    bool read_setup_change( std::escape_istream &eis, Setup_Change *&setup_change )
    {
      int setup_change_type;
      eis >> setup_change_type;

      // check input
      switch(Setup_Change_Type(setup_change_type))
      {
	case change_board:
	{
	  Setup_Change_Board *change = new Setup_Change_Board();
	  if( !read_start_board(eis,change->board) )
	  {
	    delete change;
	    return false;
	  }
	  setup_change = change;
	  return true;
	}
	break;
	case change_common_stones:
	{
	  Setup_Change_Common_Stones *change = new Setup_Change_Common_Stones();
	  if( !read_common_stones(eis,change->common_stones) )
	  {
	    delete change;
	    return false;
	  }
	  setup_change = change;
	  return true;
	}
	break;
      }
      return false;
    }
    
    void write_setup_change( std::escape_ostream &eos, Setup_Change *setup_change )
    { 
      eos << setup_change->get_type();

      switch(setup_change->get_type())
      {
	case change_board:
	{
	  assert( dynamic_cast<Setup_Change_Board*>(setup_change) );
	  Setup_Change_Board *change = static_cast<Setup_Change_Board*>(setup_change);
	  write_start_board(eos,change->board);
	  return;
	}
	break;
	case change_common_stones:
	{
	  assert( dynamic_cast<Setup_Change_Common_Stones*>(setup_change) );
	  Setup_Change_Common_Stones *change = static_cast<Setup_Change_Common_Stones*>(setup_change);
	  write_common_stones(eos,change->common_stones);
	  return;
	}
	break;
      }
      assert(false);
    }

    // ----------------------------------------------------------------------------
    // <move_sequence> : is array of <move>
    // ----------------------------------------------------------------------------
    //   <n> {<move>}* <move=finish_move>

    bool read_move_sequence( std::escape_istream &eis, Move_Sequence &move_sequence )
    {
      return move_sequence.input(eis);
    }
    
    void write_move_sequence( std::escape_ostream &eos, const Move_Sequence &move_sequence )
    {
      eos << move_sequence;	// move_sequence has it's own operator
    }

    // ----------------------------------------------------------------------------
    // <moves> : is array of <move_sequence>
    // ----------------------------------------------------------------------------
    //   <n> {<move_sequence>}*

    bool read_moves( std::escape_istream &eis, std::list<Move_Sequence> &moves )
    {
      moves.clear();

      int move_cnt;
      eis >> move_cnt;

      Move_Sequence move_sequence;

      for( int i=0; i<move_cnt; ++i )
      {
	if( !read_move_sequence(eis,move_sequence) ) return false;
	
	moves.push_back( move_sequence );
      }
      return true;
    }
    
    void write_moves( std::escape_ostream &eos, const std::list<Move_Sequence> &moves )
    {
      eos << moves.size();
      
      std::list<Move_Sequence>::const_iterator i;
      for( i = moves.begin(); i != moves.end(); ++i )
      {
	write_move_sequence(eos,*i);
      }
    }

    // ============================================================================
    // ----------------------------------------------------------------------------
    // Storage classes
    // ----------------------------------------------------------------------------
    // ============================================================================

    // ----------------------------------------------------------------------------
    // Setup_Change
    // ----------------------------------------------------------------------------

    Setup_Change::Setup_Change(Setup_Change_Type type)
      : type(type)
    {
    }

    // ----------------------------------------------------------------------------
    // Setup_Change_Board
    // ----------------------------------------------------------------------------

    Setup_Change_Board::Setup_Change_Board()
      : Setup_Change(change_board)
    {
    }

    // ----------------------------------------------------------------------------
    // Setup_Change_Common_Stones
    // ----------------------------------------------------------------------------

    Setup_Change_Common_Stones::Setup_Change_Common_Stones()
      : Setup_Change(change_common_stones)
    {
    }

    // ============================================================================
    // ----------------------------------------------------------------------------
    // Message classes
    // ----------------------------------------------------------------------------
    // ============================================================================

    // ----------------------------------------------------------------------------
    // Message
    // ----------------------------------------------------------------------------
    
    Message::Message( Message_Type type )
      : type(type)
    {
    }

    Message *Message::read_from_line( std::string line ) // returns 0 for parse error
    {
      int num = string_to_long(line).first; // read first number from ascii string
      switch( Message_Type(num) )
      {
	case msg_helo:
	{
	  Msg_Helo *msg = new Msg_Helo();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_list_protocols:
	{
	  Msg_List_Protocols *msg = new Msg_List_Protocols();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_disconnect:
	{
	  return new Msg_Disconnect();
	}
	case msg_accept:
	{
	  return new Msg_Accept();
	}
	case msg_deny:
	{
	  return new Msg_Deny();
	}
	case msg_defer:
	{
	  Msg_Defer *msg = new Msg_Defer();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_try_again:
	{
	  Msg_Try_Again *msg = new Msg_Try_Again();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_rooms:
	{
	  return new Msg_Get_Rooms();
	}
	case msg_tell_rooms:
	{
	  Msg_Tell_Rooms *msg = new Msg_Tell_Rooms();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_choose_room:
	{
	  Msg_Choose_Room *msg = new Msg_Choose_Room();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_phase:
	{
	  return new Msg_Get_Phase();
	}
	case msg_tell_phase:
	{
	  Msg_Tell_Phase *msg = new Msg_Tell_Phase();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_game:
	{
	  return new Msg_Get_Game();
	}
	case msg_tell_game:
	{
	  Msg_Tell_Game *msg = new Msg_Tell_Game();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_setup:
	{
	  return new Msg_Get_Setup();
	}
	case msg_tell_setup:
	{
	  Msg_Tell_Setup *msg = new Msg_Tell_Setup();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_moves:
	{
	  return new Msg_Get_Moves();
	}
	case msg_tell_moves:
	{
	  Msg_Tell_Moves *msg = new Msg_Tell_Moves();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_get_situation:
	{
	  return new Msg_Get_Situation();
	}
	case msg_tell_situation:
	{
	  Msg_Tell_Situation *msg = new Msg_Tell_Situation();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_request_final_setup:
	{
	  return new Msg_Request_Final_Setup();
	}
	case msg_request_move_reminder:
	{
	  return new Msg_Request_Move_Reminder();
	}
	case msg_setup:
	{
	  return new Msg_Setup();
	}
	case msg_get_setup_and_changes:
	{
	  return new Msg_Get_Setup_And_Changes();
	}
	case msg_get_moves_and_play:
	{
	  return new Msg_Get_Moves_And_Play();
	}
	case msg_get_situation_and_play:
	{
	  return new Msg_Get_Situation_And_Play();
	}
	case msg_add_player:
	{
	  Msg_Add_Player *msg = new Msg_Add_Player();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_accept_player:
	{
	  Msg_Accept_Player *msg = new Msg_Accept_Player();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_setup_change:
	{
	  Msg_Setup_Change *msg = new Msg_Setup_Change();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_ask_setup_change:
	{
	  Msg_Ask_Setup_Change *msg = new Msg_Ask_Setup_Change();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_ask_capable:
	{
	  Msg_Ask_Capable *msg = new Msg_Ask_Capable();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_incapable:
	{
	  Msg_Incapable *msg = new Msg_Incapable();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_ready:
	{
	  return new Msg_Ready();
	}
	case msg_start_game:
	{
	  return new Msg_Start_Game();
	}
	case msg_your_turn:
	{
	  return new Msg_Your_Turn();
	}
	case msg_move:
	{
	  Msg_Move *msg = new Msg_Move();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_undo:
	{
	  Msg_Undo *msg = new Msg_Undo();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_ask_undo:
	{
	  Msg_Ask_Undo *msg = new Msg_Ask_Undo();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_new_game:
	{
	  return new Msg_New_Game();
	}
	case msg_ask_new_game:
	{
	  Msg_Ask_New_Game *msg = new Msg_Ask_New_Game();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_chat:
	{
	  Msg_Chat *msg = new Msg_Chat();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_set_nick:
	{
	  Msg_Set_Nick *msg = new Msg_Set_Nick();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_nick_change:
	{
	  Msg_Nick_Change *msg = new Msg_Nick_Change();
	  if( msg->read_from_line(line) )
	    return msg;
	  delete msg;
	  break;
	}
	case msg_ping:
	{
	  return new Msg_Ping();
	}
	case msg_pong:
	{
	  return new Msg_Pong();
	}
	case msg_error:
	{
	  return new Msg_Error();
	}
      }
      return 0;
    }

    // ----------------------------------------------------------------------------
    // Msg_Helo
    // ----------------------------------------------------------------------------

    Msg_Helo::Msg_Helo( Protocol protocol, std::string host_nick )
      : Message(msg_helo), protocol(protocol), host_nick(host_nick)
    {
    }
    
    Msg_Helo::Msg_Helo()
      : Message(msg_helo)
    {
    }

    bool Msg_Helo::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number >> protocol.name >> protocol.number >> protocol.letter >> host_nick;
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Helo::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      // floating point format for protocol number: ####.##
      eos << protocol.name << double_to_string(protocol.number,2,4) << protocol.letter << host_nick;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_List_Protocols
    // ----------------------------------------------------------------------------

    Msg_List_Protocols::Msg_List_Protocols( std::list<Protocol> protocols )
      : Message(msg_list_protocols), protocols(protocols)
    {
    }
    
    Msg_List_Protocols::Msg_List_Protocols()
      : Message(msg_list_protocols)
    {
    }

    bool Msg_List_Protocols::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, protocol_cnt;
      eis >> msg_number >> protocol_cnt;
      protocols.clear();
      for( int i=0; i<protocol_cnt; ++i )
      {
	Protocol protocol;
	eis >> protocol.name >> protocol.number >> protocol.letter;
	protocols.push_back( protocol );
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_List_Protocols::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << protocols.size();
      std::list<Protocol>::iterator i;
      for( i = protocols.begin(); i != protocols.end(); ++i )
      {
	Protocol &protocol = *i;
	// floating point format for protocol number: ####.##
	eos << protocol.name << double_to_string(protocol.number,2,4) << protocol.letter;
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Disconnect
    // ----------------------------------------------------------------------------

    Msg_Disconnect::Msg_Disconnect() : Message(msg_disconnect) {}

    // ----------------------------------------------------------------------------
    // Msg_Accept
    // ----------------------------------------------------------------------------

    Msg_Accept::Msg_Accept() : Message(msg_accept) {}

    // ----------------------------------------------------------------------------
    // Msg_Deny
    // ----------------------------------------------------------------------------

    Msg_Deny::Msg_Deny() : Message(msg_deny) {}

    // ----------------------------------------------------------------------------
    // Msg_Defer
    // ----------------------------------------------------------------------------

    Msg_Defer::Msg_Defer( int id )
      : Message(msg_defer), id(id)
    {
    }
    
    Msg_Defer::Msg_Defer()
      : Message(msg_defer)
    {
    }

    bool Msg_Defer::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number >> id;
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Defer::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      // floating point format for protocol number: ####.##
      eos << id;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Try_Again
    // ----------------------------------------------------------------------------

    Msg_Try_Again::Msg_Try_Again( int id )
      : Message(msg_try_again), id(id)
    {
    }
    
    Msg_Try_Again::Msg_Try_Again()
      : Message(msg_try_again)
    {
    }

    bool Msg_Try_Again::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number >> id;
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Try_Again::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      // floating point format for protocol number: ####.##
      eos << id;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Rooms
    // ----------------------------------------------------------------------------

    Msg_Get_Rooms::Msg_Get_Rooms() : Message(msg_get_rooms) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Rooms
    // ----------------------------------------------------------------------------

    Msg_Tell_Rooms::Msg_Tell_Rooms( std::list<Room> rooms )
      : Message(msg_tell_rooms), rooms(rooms)
    {
    }
    
    Msg_Tell_Rooms::Msg_Tell_Rooms()
      : Message(msg_tell_rooms)
    {
    }

    bool Msg_Tell_Rooms::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, room_cnt;
      eis >> msg_number >> room_cnt;
      rooms.clear();
      for( int i=0; i<room_cnt; ++i )
      {
	Room room;
	int room_phase;
	eis >> room.name >> room.host_cnt >> room.is_open >> room_phase;
	room.phase = Phase_Type(room.phase);
	switch( room.phase )
	{
	  case phase_setup:
	  case phase_playing:
	    rooms.push_back( room );
	    continue;
	}
	return false;		// if phase is invalid
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Tell_Rooms::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << rooms.size();
      std::list<Room>::iterator i;
      for( i = rooms.begin(); i != rooms.end(); ++i )
      {
	Room &room = *i;
	eos << room.name << room.host_cnt << room.is_open << room.phase;
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Choose_Room
    // ----------------------------------------------------------------------------

    Msg_Choose_Room::Msg_Choose_Room( Room room )
      : Message(msg_choose_room), room(room)
    {
    }
    
    Msg_Choose_Room::Msg_Choose_Room()
      : Message(msg_choose_room)
    {
    }

    bool Msg_Choose_Room::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, room_phase;
      eis >> msg_number >> room.name >> room.host_cnt >> room.is_open >> room_phase;
      room.phase = Phase_Type(room_phase);

      switch( room.phase )
      {
	case phase_setup:
	case phase_playing:
	{
	  if( eis.did_error_occur() )
	    return false;
	  else
	    return true;
	}
      }
      return false;		// if phase is invalid
    }
    
    std::string Msg_Choose_Room::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << room.name << room.host_cnt << room.is_open << room.phase;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Phase
    // ----------------------------------------------------------------------------

    Msg_Get_Phase::Msg_Get_Phase() : Message(msg_get_phase) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Phase
    // ----------------------------------------------------------------------------

    Msg_Tell_Phase::Msg_Tell_Phase( Phase_Type phase )
      : Message(msg_tell_phase), phase(phase)
    {
    }
    
    Msg_Tell_Phase::Msg_Tell_Phase()
      : Message(msg_tell_phase)
    {
    }

    bool Msg_Tell_Phase::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, current_phase;
      eis >> msg_number >> current_phase;
      phase = Phase_Type(current_phase);
      switch( phase )
      {
	case phase_setup:
	case phase_playing:
	{
	  if( eis.did_error_occur() )
	    return false;
	  else
	    return true;
	}
      }
      return false;		// if phase is invalid
    }
    
    std::string Msg_Tell_Phase::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << phase;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Game
    // ----------------------------------------------------------------------------

    Msg_Get_Game::Msg_Get_Game() : Message(msg_get_game) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Game
    // ----------------------------------------------------------------------------

    Msg_Tell_Game::Msg_Tell_Game( std::string game )
      : Message(msg_tell_game), game(game)
    {
    }
    
    Msg_Tell_Game::Msg_Tell_Game()
      : Message(msg_tell_game)
    {
    }

    bool Msg_Tell_Game::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number >> game;
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Tell_Game::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      // floating point format for protocol number: ####.##
      eos << game;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Setup
    // ----------------------------------------------------------------------------

    Msg_Get_Setup::Msg_Get_Setup() : Message(msg_get_setup) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Setup
    // ----------------------------------------------------------------------------

    Msg_Tell_Setup::Msg_Tell_Setup( Setup setup )
      : Message(msg_tell_setup), setup(setup)
    {
    }
    
    Msg_Tell_Setup::Msg_Tell_Setup()
      : Message(msg_tell_setup)
    {
    }

    bool Msg_Tell_Setup::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      if( !read_ruleset( eis, setup.ruleset ) ) return false;
      if( !read_players( eis, setup.initial_players ) ) return false;
      if( !read_player_settings( eis, setup.current_players ) ) return false;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Tell_Setup::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      write_ruleset( eos, setup.ruleset );
      write_players( eos, setup.initial_players );
      write_player_settings( eos, setup.current_players );

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Moves
    // ----------------------------------------------------------------------------

    Msg_Get_Moves::Msg_Get_Moves() : Message(msg_get_moves) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Moves
    // ----------------------------------------------------------------------------

    Msg_Tell_Moves::Msg_Tell_Moves( std::list<Move_Sequence> moves )
      : Message(msg_tell_moves), moves(moves)
    {
    }
    
    Msg_Tell_Moves::Msg_Tell_Moves()
      : Message(msg_tell_moves)
    {
    }

    bool Msg_Tell_Moves::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      if( !read_moves( eis, moves ) ) return false;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Tell_Moves::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      write_moves( eos, moves );

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Get_Situation
    // ----------------------------------------------------------------------------

    Msg_Get_Situation::Msg_Get_Situation() : Message(msg_get_situation) {}

    // ----------------------------------------------------------------------------
    // Msg_Tell_Situation
    // ----------------------------------------------------------------------------

    Msg_Tell_Situation::Msg_Tell_Situation( const Board &board, 
					    const std::list<Player> &players )
      : Message(msg_tell_situation), board(board), players(players)
    {
    }
    
    Msg_Tell_Situation::Msg_Tell_Situation()
      : Message(msg_tell_situation)
    {
    }

    bool Msg_Tell_Situation::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      if( !read_board( eis, board ) ) return false;
      if( !read_players( eis, players ) ) return false;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Tell_Situation::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      write_board( eos, board );
      write_players( eos, players );

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Request_Final_Setup
    // ----------------------------------------------------------------------------

    Msg_Request_Final_Setup::Msg_Request_Final_Setup() : Message(msg_request_final_setup) {}

    // ----------------------------------------------------------------------------
    // Msg_Request_Move_Reminder
    // ----------------------------------------------------------------------------

    Msg_Request_Move_Reminder::Msg_Request_Move_Reminder() : Message(msg_request_move_reminder) {}

    // ----------------------------------------------------------------------------
    // Msg_Setup
    // ----------------------------------------------------------------------------

    Msg_Setup::Msg_Setup() : Message(msg_setup) {}

    // ----------------------------------------------------------------------------
    // Msg_Get_Setup_And_Changes
    // ----------------------------------------------------------------------------

    Msg_Get_Setup_And_Changes::Msg_Get_Setup_And_Changes() : Message(msg_get_setup_and_changes) {}

    // ----------------------------------------------------------------------------
    // Msg_Get_Moves_And_Play
    // ----------------------------------------------------------------------------

    Msg_Get_Moves_And_Play::Msg_Get_Moves_And_Play() : Message(msg_get_moves_and_play) {}

    // ----------------------------------------------------------------------------
    // Msg_Get_Situation_And_Play
    // ----------------------------------------------------------------------------

    Msg_Get_Situation_And_Play::Msg_Get_Situation_And_Play() : Message(msg_get_situation_and_play) {}

    // ----------------------------------------------------------------------------
    // Msg_Add_Player
    // ----------------------------------------------------------------------------

    Msg_Add_Player::Msg_Add_Player( Player player )
      : Message(msg_add_player), player(player)
    {
    }
    
    Msg_Add_Player::Msg_Add_Player()
      : Message(msg_add_player)
    {
    }

    bool Msg_Add_Player::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      if( !read_player_setting( eis, player ) ) return false;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Add_Player::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      write_player_setting( eos, player );

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Accept_Player
    // ----------------------------------------------------------------------------

    Msg_Accept_Player::Msg_Accept_Player( int id )
      : Message(msg_accept_player), id(id)
    {
    }
    
    Msg_Accept_Player::Msg_Accept_Player()
      : Message(msg_accept_player)
    {
    }

    bool Msg_Accept_Player::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> id;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Accept_Player::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << id;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Setup_Change
    // ----------------------------------------------------------------------------

    Msg_Setup_Change::Msg_Setup_Change( std::list<Setup_Change*> changes )
      : Message(msg_setup_change), changes(changes)
    {
    }
    
    Msg_Setup_Change::Msg_Setup_Change()
      : Message(msg_setup_change)
    {
    }

    Msg_Setup_Change::~Msg_Setup_Change()
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
    }

    bool Msg_Setup_Change::read_from_line( std::string line ) // returns false on parse error
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
      changes.clear();

      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, change_cnt;
      eis >> msg_number >> change_cnt;
      for( int i=0; i<change_cnt; ++i )
      {
	Setup_Change *change;
	if( !read_setup_change(eis,change) ) return false;
	changes.push_back( change );
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Setup_Change::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << changes.size();
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	write_setup_change(eos,*i);
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Ask_Setup_Change
    // ----------------------------------------------------------------------------

    Msg_Ask_Setup_Change::Msg_Ask_Setup_Change( std::list<Setup_Change*> changes )
      : Message(msg_ask_setup_change), changes(changes)
    {
    }
    
    Msg_Ask_Setup_Change::Msg_Ask_Setup_Change()
      : Message(msg_ask_setup_change)
    {
    }

    Msg_Ask_Setup_Change::~Msg_Ask_Setup_Change()
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
    }

    bool Msg_Ask_Setup_Change::read_from_line( std::string line ) // returns false on parse error
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
      changes.clear();

      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, change_cnt;
      eis >> msg_number >> change_cnt;
      for( int i=0; i<change_cnt; ++i )
      {
	Setup_Change *change;
	if( !read_setup_change(eis,change) ) return false;
	changes.push_back( change );
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Ask_Setup_Change::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << changes.size();
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	write_setup_change(eos,*i);
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Ask_Capable
    // ----------------------------------------------------------------------------

    Msg_Ask_Capable::Msg_Ask_Capable( std::list<Setup_Change*> changes )
      : Message(msg_ask_capable), changes(changes)
    {
    }
    
    Msg_Ask_Capable::Msg_Ask_Capable()
      : Message(msg_ask_capable)
    {
    }

    Msg_Ask_Capable::~Msg_Ask_Capable()
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
    }

    bool Msg_Ask_Capable::read_from_line( std::string line ) // returns false on parse error
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
      changes.clear();

      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, change_cnt;
      eis >> msg_number >> change_cnt;
      for( int i=0; i<change_cnt; ++i )
      {
	Setup_Change *change;
	if( !read_setup_change(eis,change) ) return false;
	changes.push_back( change );
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Ask_Capable::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << changes.size();
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	write_setup_change(eos,*i);
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Incapable
    // ----------------------------------------------------------------------------

    Msg_Incapable::Msg_Incapable( std::string host_nick, std::list<Setup_Change*> changes )
      : Message(msg_incapable), host_nick(host_nick), changes(changes)
    {
    }
    
    Msg_Incapable::Msg_Incapable()
      : Message(msg_incapable)
    {
    }

    Msg_Incapable::~Msg_Incapable()
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
    }

    bool Msg_Incapable::read_from_line( std::string line ) // returns false on parse error
    {
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	delete *i;
      }      
      changes.clear();

      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, change_cnt;
      eis >> msg_number >> host_nick >> change_cnt;
      for( int i=0; i<change_cnt; ++i )
      {
	Setup_Change *change;
	if( !read_setup_change(eis,change) ) return false;
	changes.push_back( change );
      }
      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Incapable::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << host_nick << changes.size();
      std::list<Setup_Change*>::iterator i;
      for( i = changes.begin(); i != changes.end(); ++i )
      {
	write_setup_change(eos,*i);
      }

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Ready
    // ----------------------------------------------------------------------------

    Msg_Ready::Msg_Ready() : Message(msg_ready) {}

    // ----------------------------------------------------------------------------
    // Msg_Start_Game
    // ----------------------------------------------------------------------------

    Msg_Start_Game::Msg_Start_Game() : Message(msg_start_game) {}

    // ----------------------------------------------------------------------------
    // Msg_Your_Turn
    // ----------------------------------------------------------------------------

    Msg_Your_Turn::Msg_Your_Turn() : Message(msg_your_turn) {}

    // ----------------------------------------------------------------------------
    // Msg_Move
    // ----------------------------------------------------------------------------

    Msg_Move::Msg_Move( Move_Sequence move )
      : Message(msg_move), move(move)
    {
    }
    
    Msg_Move::Msg_Move()
      : Message(msg_move)
    {
    }

    bool Msg_Move::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      if( !read_move_sequence(eis,move) ) return false;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Move::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      write_move_sequence(eos,move);

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Undo
    // ----------------------------------------------------------------------------

    Msg_Undo::Msg_Undo( int n )
      : Message(msg_undo), n(n)
    {
    }
    
    Msg_Undo::Msg_Undo()
      : Message(msg_undo)
    {
    }

    bool Msg_Undo::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> n;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Undo::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << n;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Ask_Undo
    // ----------------------------------------------------------------------------

    Msg_Ask_Undo::Msg_Ask_Undo( std::string host_nick, int n )
      : Message(msg_ask_undo), host_nick(host_nick), n(n)
    {
    }
    
    Msg_Ask_Undo::Msg_Ask_Undo()
      : Message(msg_ask_undo)
    {
    }

    bool Msg_Ask_Undo::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> host_nick >> n;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Ask_Undo::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << host_nick << n;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_New_Game
    // ----------------------------------------------------------------------------

    Msg_New_Game::Msg_New_Game() : Message(msg_new_game) {}

    // ----------------------------------------------------------------------------
    // Msg_Ask_New_Game
    // ----------------------------------------------------------------------------

    Msg_Ask_New_Game::Msg_Ask_New_Game( std::string host_nick )
      : Message(msg_ask_new_game), host_nick(host_nick)
    {
    }
    
    Msg_Ask_New_Game::Msg_Ask_New_Game()
      : Message(msg_ask_new_game)
    {
    }

    bool Msg_Ask_New_Game::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> host_nick;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Ask_New_Game::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << host_nick;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Chat
    // ----------------------------------------------------------------------------

    Msg_Chat::Msg_Chat( std::string host_nick, std::string message )
      : Message(msg_chat), host_nick(host_nick), message(message)
    {
    }
    
    Msg_Chat::Msg_Chat()
      : Message(msg_chat)
    {
    }

    bool Msg_Chat::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> host_nick >> message;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Chat::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << host_nick << message;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Set_Nick
    // ----------------------------------------------------------------------------

    Msg_Set_Nick::Msg_Set_Nick( std::string host_nick )
      : Message(msg_set_nick), host_nick(host_nick)
    {
    }
    
    Msg_Set_Nick::Msg_Set_Nick()
      : Message(msg_set_nick)
    {
    }

    bool Msg_Set_Nick::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number;
      eis >> msg_number;

      eis >> host_nick;

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Set_Nick::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << host_nick;

      return os.str();
    }

    // ----------------------------------------------------------------------------
    // Msg_Nick_Change
    // ----------------------------------------------------------------------------

    Msg_Nick_Change::Msg_Nick_Change( std::string old_host_nick, std::string new_host_nick,
				      std::list<int> player_ids )
      : Message(msg_nick_change), old_host_nick(old_host_nick), new_host_nick(new_host_nick),
	player_ids(player_ids)
    {
    }
    
    Msg_Nick_Change::Msg_Nick_Change()
      : Message(msg_nick_change)
    {
    }

    bool Msg_Nick_Change::read_from_line( std::string line ) // returns false on parse error
    {
      std::istringstream is(line);
      std::escape_istream eis(is);
      
      int msg_number, player_id_cnt, id;
      eis >> msg_number;

      eis >> old_host_nick >> new_host_nick >> player_id_cnt;

      player_ids.clear();
      for( int i=0; i<player_id_cnt; ++i )
      {
	eis >> id;
	player_ids.push_back(id);
      }

      if( eis.did_error_occur() )
	return false;
      return true;
    }
    
    std::string Msg_Nick_Change::write_to_line()
    {
      std::ostringstream os;
      std::escape_ostream eos(os);

      eos << old_host_nick << new_host_nick << player_ids.size();

      std::list<int>::iterator i;
      for( i = player_ids.begin(); i != player_ids.end(); ++i )
      {
	eos << *i;
      }

      return os.str();
    }




    // ----------------------------------------------------------------------------
    // Msg_Ping
    // ----------------------------------------------------------------------------

    Msg_Ping::Msg_Ping() : Message(msg_ping) {}

    // ----------------------------------------------------------------------------
    // Msg_Pong
    // ----------------------------------------------------------------------------

    Msg_Pong::Msg_Pong() : Message(msg_pong) {}

    // ----------------------------------------------------------------------------
    // Msg_Error
    // ----------------------------------------------------------------------------

    Msg_Error::Msg_Error() : Message(msg_error) {}


  }
}
