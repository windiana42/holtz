/*
 * bgp.hpp
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

#ifndef __CHAT_PROTOCOL__
#define __CHAT_PROTOCOL__

#include "msg_net.hpp"

#include <string>

namespace holtz
{
  namespace BGP
  {
    // ============================================================================
    // ----------------------------------------------------------------------------
    // Number translations (enum) for network protocol BGP
    // ----------------------------------------------------------------------------
    // ============================================================================

    enum Boolean			
    {
      bool_false=0,
      bool_true=1
    };

    // ============================================================================
    // message types
    // ============================================================================

    enum Message_Type{		// <message_type> <parameters>
      // Version A,B,C
      msg_helo=0,		// helo <protocol> <version> <host_nick>
      msg_accept,		// accept
      msg_deny,			// deny
      msg_list_rooms=10,	// list_rooms
      msg_tell_rooms,		// tell_rooms <room_cnt> {<room_name> <host_cnt> <is_open> <phase>}*
      msg_choose_room,		// choose_room <room_name>
      msg_get_phase=20,		// get_phase
				// = get game phase (setup/playing)
      msg_tell_phase,		// tell_phase <phase>
      msg_get_game,		// get_game
				// = get game which is played (Zertz)
      msg_tell_game,		// tell_game <game>
      msg_get_setup,		// get_setup
				// = ask for game setup 
      msg_tell_setup,		// tell_setup <game_setup>
				// = tell all setup settings of the specified game
				//   (Version B,C: be aware of getting changes)
      msg_get_moves,		// get_moves
				// = ask for moves already played
      msg_tell_moves,		// tell_moves <game_setup>
				// = tell all moves already played
				//   (Version A,B,C: be aware that further moves may be reported)
      msg_request_final_setup,	// request_final_setup
				// = ask server to give a final setup at the end of 
      msg_request_move_reminder,// request_move_reminder
				// = ask server to tell me when it is my turn
      msg_setup,		// setup
				// = go to setup state (may add players)
      msg_play,			// play
				// = afterwards moves may be sent
      msg_add_player=50,	// add_player <player_settings>
      msg_accept_player,	// accept_player <id>
				// = server gives unique id for player
      msg_remove_player,	// remove_player <id>
      msg_ready,		// ready
				// = tell others that host doesn't want to change players/settings
      msg_start_game=100,	// start_game
      msg_your_turn,		// your_turn
				// = reminds simple clients that it is their turn
      msg_move,			// move <move_sequence>
      msg_undo,			// undo <n>
				// = requests to undo <n> (half) moves
      msg_ask_undo,		// ask_undo <host_nick> <n>
				// = ask user to undo
      msg_new_game,		// new_game
      msg_ask_new_game,		// ask_new_game <host_nick>
      msg_get_current_board,	// get_current_board
				// = ask for game current board (for simple clients)
      msg_tell_current_board,	// tell_current_board <board>
      msg_chat=500,		// chat <host_nick> <chat_message>
      msg_set_nick,		// set_nick <new_host_nick>
      msg_ping=900,		// ping
      msg_pong,			// pong
				// = every ping is answered with a pong
      msg_error=999		// error
				// = tell partner to go to error state
    };
  
    // ============================================================================
    // parameter types
    // ============================================================================

    // <host_nick> : string	= nick name for host (for chat/ask messages)
    // <room_name>,<chat_message> : string
    // <room_cnt>, <host_cnt>, <min_players>, <max_players>,
    // <white_stones>, <grey_stones>, <black_stones>, <all_stones>,
    // <id>, : integer
    // <is_open> : Boolean

    // ----------------------------------------------------------------------------
    // <game>: is string of
    // ----------------------------------------------------------------------------
    //   "Zertz"
    //   "Dvonn"

    // ----------------------------------------------------------------------------
    // <phase>: is one of
    // ----------------------------------------------------------------------------
    enum Game_Phase		
    {
      phase_setup=0,
      phase_playing
    };

    // ----------------------------------------------------------------------------
    // <game_setup> : consists of
    // ----------------------------------------------------------------------------
    //   <min_players> <max_players> <board_setup> <common_stones> <win_condition>

    // ----------------------------------------------------------------------------
    // <board_setup> : is one of
    // ----------------------------------------------------------------------------
    enum Board_Setup
    {
      board_37=0,		// board_37
				// = 37 ring board (standard)
      board_40,			// board_40
      board_44,			// board_44
      board_48,			// board_48
      board_61,			// board_61
      board_generic,		// board_generic <columns> {<stone_cnt>}*
				// = stones in each column are centered
      board_custom		// board_custom <columns> <rows> <custom_board_setup>
    }

    // <custom_board_setup> : <board>

    // ----------------------------------------------------------------------------
    // <board> : is array of <field_type>
    // ----------------------------------------------------------------------------
    //   { {<field_type>}*row* }*column*
    // ---
    // * store hexagonal board in 2-D array with every second column shifted up 
    //   half a row
    // * board may include preplaced stones
    // ex:
    //     <x>: stands for no field (see <field_type> for representation)
    //     <0>: stands for empty field
    //
    //   board:
    //        x   x   
    //      x   0   x 
    //        0   0
    //      x   0   0
    //        0   0
    //      x   0   x
    //
    //   representation:
    //     <x> <x> <x>  <x> <0> <0>  <0> <0> <0>  <x> <0> <0>  <x> <0> <x>  
    //        col0         col1         col2         col3         col4

    // ----------------------------------------------------------------------------
    // <field_type> : is one of
    // ----------------------------------------------------------------------------
    
    enum Field_Type
    { 
      field_no_field=-1,	// = no field or removed field at this position
      field_empty=0,		// = empty field
      field_white=1,		// = field is occupied with a white stone
      field_grey=2, 		// = field is occupied with a grey stone
      field_black=3 		// = field is occupied with a black stone
    };
    

    // ----------------------------------------------------------------------------
    // <common_stones> : consists of
    // ----------------------------------------------------------------------------
    enum Common_Stones
    {
      stones_standard=0,	// stones_standard
				// = 5 white, 7 grey, 9 black
      stones_tournament,	// stones_tournament
				// = 6 white, 8 grey, 10 black
      stones_generic		// stones_generic <white_stones> <grey_stones> <black_stones>
    }

    // ----------------------------------------------------------------------------
    // <win_condition> : is one of
    // ----------------------------------------------------------------------------
    enum Win_Condition
    {
      win_standard=0,		// win_standard
				// = 3 white, 4 grey, 5 black, 2 each
      win_tournament,		// win_tournament
				// = 4 white, 5 grey, 6 black, 3 each
      win_generic		// win_generic <white_stones> <grey_stones> <black_stones> <all_stones> 
    }

    // ----------------------------------------------------------------------------
    // <move_sequence> : is array of <move>
    // ----------------------------------------------------------------------------
    //   {<move>}* <move=finish_move>

    // ----------------------------------------------------------------------------
    // <move> : is one of 
    // ----------------------------------------------------------------------------
    //   for coordinate system see <custom_board_setup>

    enum Move_Type
    {
      knock_out_move=1,		// knock_out_move <from-row> <from-col> <over-row> <over-col> 
				//   <to-row> <to-col> 
      set_move,			// set_move <pos-row> <pos-col> <stone_type>
      remove_move,		// remove_move <pos-row> <pos-col>
      finish_move		// finish_move
    }

    // ----------------------------------------------------------------------------
    // <stone_type> : is one of
    // ----------------------------------------------------------------------------
    
    enum Stone_Type
    { 
      stone_white=1,		
      stone_grey=2, 		
      stone_black=3 		
    };
    
  }
  // ============================================================================
  // ----------------------------------------------------------------------------
  // message classes
  // ----------------------------------------------------------------------------
  // ============================================================================

  class BGP_Message
  {
  public:
    BGP_Message( Message_Type type );
    virtual ~BGP_Message() {}

    // message number at the beginning of the line indicates the message type
    static BGP_Message *read_from_line( std::string line ); // returns 0 for parse error
    virtual std::string write_to_line() { return ""; } // message does not include message type

    Message_Type get_type() { return type; }

    Message_Type type;
  };

  class Message_Text : public BGP_Message
  {
  public:
    Message_Text( std::string sender, std::string text );
    Message_Text();

    // message number (decimal,text) is included at the beginning of the line
    bool read_from_line( std::string line ); // returns false on parse error
    virtual std::string write_to_line();
  
    std::string get_sender() { return sender; }
    std::string get_text()   { return text; }
  private:
    std::string sender;
    std::string text;
  };

  class Message_Distance : public BGP_Message
  {
  public:
    Message_Distance( std::string origin, int distance );
    Message_Distance();

    // message number is included at the beginning of the line
    bool read_from_line( std::string line ); // returns false on parse error
    virtual std::string write_to_line();
  
    std::string get_origin()   { return origin; }
    int         get_distance() { return distance; }
  private:
    std::string origin;
    int distance;
  };

  class Message_Connect : public BGP_Message
  {
  public:
    Message_Connect( std::string host, int port );
    Message_Connect();

    // message number (decimal,text) is included at the beginning of the line
    bool read_from_line( std::string line ); // returns false on parse error
    virtual std::string write_to_line();
  
    std::string get_host() { return host; }
    int get_port()	 { return port; }
  private:
    std::string host;
    int port;
  };

  class Message_Tell_Server : public BGP_Message
  {
  public:
    Message_Tell_Server( int port );
    Message_Tell_Server();

    // message number (decimal,text) is included at the beginning of the line
    bool read_from_line( std::string line ); // returns false on parse error
    virtual std::string write_to_line();
  
    int get_port()	 { return port; }
  private:
    int port;
  };
}
#endif
