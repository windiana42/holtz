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


#if (defined(VERSION_ZERTZ) && defined(VERSION_DVONN))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_DVONN defined"
#endif
#if (defined(VERSION_ZERTZ) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_BLOKS defined"
#endif
#if (defined(VERSION_ZERTZ) && defined(VERSION_RELAX))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_RELAX defined"
#endif
#if (defined(VERSION_RELAX) && defined(VERSION_DVONN))
#  error "something went wrong in include sequence: VERSION_RELAX and VERSION_DVONN defined"
#endif
#if (defined(VERSION_RELAX) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_RELAX and VERSION_BLOKS defined"
#endif
#if (defined(VERSION_DVONN) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_DVONN and VERSION_BLOKS defined"
#endif

#if (defined(VERSION_ZERTZ) && !defined(__ZERTZ_BGP__)) || \
  (defined(VERSION_DVONN) && !defined(__DVONN_BGP__)) || \
  (defined(VERSION_BLOKS) && !defined(__BLOKS_BGP__)) || \
  (defined(VERSION_RELAX) && !defined(__RELAX_BGP__))

#if defined(VERSION_ZERTZ)
#  define __ZERTZ_BGP__
//#  warning "using zertz..."
#elif defined(VERSION_DVONN)
#  define __DVONN_BGP__
//#  warning "using dvonn..."
#elif defined(VERSION_BLOKS)
#  define __BLOKS_BGP__
//#  warning "using bloks..."
#elif defined(VERSION_RELAX)
#  define __RELAX_BGP__
//#  warning "using relax..."
#else
#  error "Please define either VERSION_ZERTZ or VERSION_DVONN or VERSION_BLOKS or VERSION_RELAX"
#endif

#if defined(VERSION_ZERTZ)
#  undef VERSION_ZERTZ
#  include "msg_net.hpp"
#  include "zertz/zertz.hpp"
#  define VERSION_ZERTZ
#elif defined(VERSION_DVONN)
#  undef VERSION_DVONN
#  include "msg_net.hpp"
#  include "dvonn/dvonn.hpp"
#  define VERSION_DVONN
#elif defined(VERSION_BLOKS)
#  undef VERSION_BLOKS
#  include "msg_net.hpp"
#  include "bloks/bloks.hpp"
#  define VERSION_BLOKS
#elif defined(VERSION_RELAX)
#  undef VERSION_RELAX
#  include "msg_net.hpp"
#  include "relax/relax.hpp"
#  define VERSION_RELAX
#endif

#include <string>

#if defined(VERSION_ZERTZ)
namespace zertz
#elif defined(VERSION_DVONN)
namespace dvonn
#elif defined(VERSION_BLOKS)
namespace bloks
#elif defined(VERSION_RELAX)
namespace relax
#endif
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
      // Subversion A,B,C
      msg_helo=0,		// helo <protocol> <host_nick>
      msg_list_protocols,	// list_protocols <protocol_cnt> {<protocol>}*
      msg_disconnect,		// disconnect
      msg_accept,		// accept
      msg_deny,			// deny
      msg_defer,		// defer <id>
				// = reject request at the moment, it may be accepted later
      msg_try_again,		// try_again <id>
				// = requests that were defered with <id> may be tried again
      msg_get_rooms=10,		// get_rooms
      msg_tell_rooms,		// tell_rooms <room_cnt> { <room> }*
      msg_choose_room,		// choose_room <room_name>
      msg_get_phase=20,		// get_phase
				// = get game phase (setup/playing)
      msg_tell_phase,		// tell_phase <phase>
      msg_get_game,		// get_game
				// = get game which is played (Zertz)
      msg_tell_game,		// tell_game <game>
      msg_get_setup,		// get_setup
				// = ask for game setup 
      msg_tell_setup,		// tell_setup <setup>
				// = tell all setup settings of the specified game
      msg_get_moves,		// get_moves
				// = ask for moves already played
      msg_tell_moves,		// tell_moves <moves>
				// = tell all moves already played
      msg_get_situation,	// get_situation
				// = ask for current board and player state of the game
      msg_tell_situation,	// tell_situation <situation>
				// = tell the current situation 
      msg_request_final_setup,	// request_final_setup
				// = ask server to give a final setup at the end of 
      msg_request_move_reminder,// request_move_reminder
				// = ask server to tell me when it is my turn
      msg_take_player,		// take_player <id>
				// = request to take over abandoned player <id>
      // additional commands for only a few games
      msg_tell_player_setup,	// tell_player_setup <setup>
				// = tell all setup settings of the specified game with player indices
      msg_tell_player_moves,	// tell_player_moves <moves>
				// = tell all moves already played with player indices

      msg_setup=40,		// setup
				// = go to setup state (may add players)

      // Subversion B,C:
      msg_get_setup_and_changes,// get_setup_and_changes
				// = ask for game setup and allow master to send changes afterwards

      // Subversion A,B,C:
      msg_get_moves_and_play,	// get_moves_and_play
				// = ask for moves already played and be aware of getting further 
				//   moves
      msg_get_situation_and_play, // get_situation_and_play
				// = ask for current situation of the game and be aware that moves 
				//   may be sent from now on

      msg_add_player=50,	// add_player <player_setting>
      msg_accept_player,	// accept_player <id>
				// = server gives unique id for player
      msg_remove_player,	// remove_player <id>
				// = ask server to remove player
      msg_setup_change=60,	// setup_change <setup_changes>
				// = request setup change (<change_number> helps to serialize 
				//   changes) simultaneous changes with same <n> leads to auto 
				//   increment of the <n> told by the slave

      // Subversion C:
      msg_ask_setup_change,	// ask_setup_change <setup_changes>
				// = master may ask whether to accept one setup change to all slaves

      // Subversion A,B,C:
      msg_ask_capable,		// ask_capable <setup_changes>
				// = master may ask all slaves that weren't involved in the decision 
				//   whether they are capable to play with a certain setup
      msg_incapable,		// incapable <host_nick> <setup_changes>
				// = report the setup changes the slave isn't capable of
      msg_ready=90,		// ready
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
      msg_chat=500,		// chat <host_nick> <chat_message>
      msg_set_nick,		// set_nick <new_host_nick>
      msg_nick_change,		// nick_change <old_host_nick> <new_host_nick> <cnt> {<player_id>}*
      msg_ping=900,		// ping
      msg_pong,			// pong
				// = every ping is answered with a pong
      msg_error=999		// error
				// = tell partner to go to error state
    };

    std::string to_string(Message_Type type);
  
    // ============================================================================
    // parameter types
    // ============================================================================

    // <host_nick> : string	= nick name for host (for chat/ask messages)
    // <room_name>,<chat_message> : string
    // <room_cnt>, <host_cnt>, <min_players>, <max_players>,
    // <white_stones>, <grey_stones>, <black_stones>, <all_stones>,
    // <id>,<n> : integer
    // <is_open> : Boolean

    // ----------------------------------------------------------------------------
    // <protocol>: consists of
    // ----------------------------------------------------------------------------
    //   <protocol_name> <version_numer> <subversion_letter>
    struct Protocol
    {
      std::string name;		// protocol name ("BGP")
      float number;		// fixed point format ####.##
      char letter;		// subversion letter 'a'-'d'

      Protocol(){}
      Protocol(std::string name, float number, char letter)
	: name(name), number(number), letter(letter) {}
    };

    // ----------------------------------------------------------------------------
    // <protocol_name>: is string of
    // ----------------------------------------------------------------------------
    //   "BGP"

    // ----------------------------------------------------------------------------
    // <version_number>: fixed point format: ####.##
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    // <subversion_letter>: is char of
    // ----------------------------------------------------------------------------
    //   'a' = in any state it is clear which side has to send the next message
    //   'b' = slave can ask for setup changes
    //   'c' = like 'b' and master may pass one setup change question to all slaves
    //   'd' = like 'c' and undo or new game requests may be sent in any state

    // ----------------------------------------------------------------------------
    // <phase>: is one of
    // ----------------------------------------------------------------------------
    enum Phase_Type
    {
      phase_setup=0,
      phase_playing
    };

    // ----------------------------------------------------------------------------
    // <room>: consists of
    // ----------------------------------------------------------------------------
    //   <room_name> <host_cnt> <is_open> <phase>
    struct Room
    {
      std::string name;
      int host_cnt;
      bool is_open;
      Phase_Type phase;

      Room() {}
      Room(std::string name, int host_cnt, bool is_open, Phase_Type phase)
	: name(name), host_cnt(host_cnt), is_open(is_open), phase(phase) {}
    };

    // ----------------------------------------------------------------------------
    // <game>: is string of
    // ----------------------------------------------------------------------------
    //   "/GIPFPROJECT/ZERTZ"
    //   "/GIPFPROJECT/DVONN" 
    //   "/GIPFPROJECT/BLOKS" 
    //     ...

    // ----------------------------------------------------------------------------
    // <setup> : consists of
    // ----------------------------------------------------------------------------
    //   <ruleset> <moves> <initial_players> <current_players>
    // ----------------------------------------------------------------------------
    // * initial players store default names and pre gained stones
    // * current players store already registered players
    struct Setup
    {
      Ruleset ruleset;
      std::list<std::pair<Move_Sequence,int/*player index*/> > init_moves; 
				// moves executed before players start
      std::list<Player> initial_players; // players with given stones and default names
      std::list<Player> current_players; // currently registered players
    };

    // ----------------------------------------------------------------------------
    // <ruleset> : is one of
    // ----------------------------------------------------------------------------
    enum Ruleset_Type
    {
#if defined(VERSION_ZERTZ)
      ruleset_basic=0,		// = basic rules
      ruleset_standard,		// = standard rules
      ruleset_tournament,	// = tournament rules
#elif defined(VERSION_BLOKS)
      ruleset_standard=0,	// = standard rules
      ruleset_small_board,	// = small board rules
#else
      ruleset_standard=0,	// = standard rules
#endif
      ruleset_custom=99		// ruleset_custom <start_board> <common_stones> <win_condition>
				//   <min_players> <max_players>
    };

    // ----------------------------------------------------------------------------
    // <start_board> : is one of
    // ----------------------------------------------------------------------------
#if defined(VERSION_ZERTZ)
    enum Start_Board_Type
    {
      board_37_rings=0,		// board_37_rings
				// = 37 ring board (standard)
      board_40_rings,		// board_40_rings
      board_44_rings,		// board_44_rings
      board_48_rings,		// board_48_rings
      board_61_rings,		// board_61_rings
      board_custom=99		// board_custom <board>
    };
#elif defined(VERSION_DVONN)
    enum Start_Board_Type
    {
      board_49_rings=0,		// board_49_rings
				// = 49 ring board (standard)
      board_custom=99		// board_custom <board>
    };
#elif defined(VERSION_BLOKS)
    enum Start_Board_Type
    {
      board_standard=0,		// board_standard
      board_small_board,	// board_small_board
      board_custom=99		// board_custom <board>
    };
#elif defined(VERSION_RELAX)
    enum Start_Board_Type
    {
      board_standard=0,		// board_standard
      board_custom=99		// board_custom <board>
    };
#endif    


    // ----------------------------------------------------------------------------
    // <board_state> : is one of
    // ----------------------------------------------------------------------------
#if defined(VERSION_ZERTZ)
    enum Board_State_Type
    {
      board_state_default=0,
    };
#elif defined(VERSION_DVONN)
    enum Board_State_Type
    {
      board_state_set_moves=0,
      board_state_jump_moves,
    };
#elif defined(VERSION_BLOKS)
    enum Board_State_Type
    {
      board_state_default=0,
    };
#elif defined(VERSION_RELAX)
    enum Board_State_Type
    {
      board_state_default=0,
    };
#endif    

    // ----------------------------------------------------------------------------
    // <board> : is array of <field_type>
    // ----------------------------------------------------------------------------
    //   <board_state> <rows> <columns> { {<field_type>}*(row) }*(column)
    // ----------------------------------------------------------------------------
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
#if defined(VERSION_ZERTZ)
    enum Field_Type
    { 
      field_no_field=-1,	// = no field or removed field at this position
      field_empty=0,		// = empty field
      field_white=1,		// = field is occupied with a white stone
      field_grey=2, 		// = field is occupied with a grey stone
      field_black=3 		// = field is occupied with a black stone
    };
#elif defined(VERSION_DVONN)
    enum Field_Type
    { 
      field_no_field=-1,	// = no field or removed field at this position
      field_empty=0,		// = empty field
      field_red=1, 		// = field is occupied with a red stone
      field_white=2,		// = field is occupied with a white stone
      field_black=3 		// = field is occupied with a black stone
    };
#elif defined(VERSION_BLOKS)
    enum Field_Type
    { 
      field_no_field=-1,	// = no field or removed field at this position
      field_empty=0,		// = empty field
      field_player1=1, 		// = field is occupied with a stone of player1
      field_player2=2, 		// = field is occupied with a stone of player2
      field_player3=3, 		// = field is occupied with a stone of player3
      field_player4=4, 		// = field is occupied with a stone of player4
    };
#endif
    
    // ----------------------------------------------------------------------------
    // <common_stones> : is one of
    // ----------------------------------------------------------------------------
    enum Common_Stones_Type
    {
#if defined(VERSION_ZERTZ)
      stones_basic=0,		// stones_basic
				// = 5 white, 7 grey, 9 black
      stones_standard,		// stones_standard
				// = 6 white, 8 grey, 10 black
#else
      stones_standard=0,	// stones_standard
#endif
      stones_custom=99		// stones_custom <white_stones> <grey_stones> <black_stones>
    };

    // ----------------------------------------------------------------------------
    // <win_condition> : is one of
    // ----------------------------------------------------------------------------
    enum Win_Condition_Type
    {
#if defined(VERSION_ZERTZ)
      win_basic=0,		// win_basic
				// = 3 white, 4 grey, 5 black, 2 each
      win_standard,		// win_standard
				// = 4 white, 5 grey, 6 black, 3 each
      win_generic		// win_generic <white_stones> <grey_stones> <black_stones>
				//   <all_stones> 
#else
      win_standard=0		// win_standard
#endif
    };

    // ----------------------------------------------------------------------------
    // <initial_players> : <players>
    // ----------------------------------------------------------------------------

    // ----------------------------------------------------------------------------
    // <players> : consists of
    // ----------------------------------------------------------------------------
    //   <player_cnt> { <player_id> <player_name> <white_stones> <grey_stones> <black_stones> }*

    // ----------------------------------------------------------------------------
    // <current_players> : <player_settings>
    // ----------------------------------------------------------------------------
    // <player_id>s of initial players and current players may differ

    // ----------------------------------------------------------------------------
    // <player_settings> : is array of <player_setting>
    // ----------------------------------------------------------------------------
    //   <player_cnt> { <player_id> <player_setting> }*

    // ----------------------------------------------------------------------------
    // <player_setting> : consists of
    // ----------------------------------------------------------------------------
    //   <player_name> <host_nick> <player_type> <help_mode>

    // ----------------------------------------------------------------------------
    // <player_type> : is one of
    // ----------------------------------------------------------------------------
    enum Player_Type
    {
      player_unknown=0,		// = unknown player type
      player_user,		// = player is a human user
      player_ai			// = artificial intelligence
    };

    // ----------------------------------------------------------------------------
    // <help_mode> : is one of
    // ----------------------------------------------------------------------------
    enum Help_Mode_Type
    {
      help_none=0,		// = no help is displayed
      help_show_possible_moves,	// = possible moves are highlighted
      help_ai_hints		// = ai gives hints or shows his current thoughts
    };

    // ----------------------------------------------------------------------------
    // <situation> : consists of
    // ----------------------------------------------------------------------------
    //   <board> <players>
    struct Situation
    {
      Board board;
      std::list<Player> players;
    };

    // ----------------------------------------------------------------------------
    // <setup_changes> : is array of <setup_change>
    // ----------------------------------------------------------------------------
    //   <n> {<setup_change>}*

    // ----------------------------------------------------------------------------
    // <setup_change> : is one of
    // ----------------------------------------------------------------------------
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

    class Setup_Change
    {
    public:
      Setup_Change(Setup_Change_Type type);
      Setup_Change_Type get_type() { return type; }
      virtual ~Setup_Change() {}
    private:
      Setup_Change_Type type;
    };

    class Setup_Change_Board : public Setup_Change
    {
    public:
      Setup_Change_Board();

      Board board;
    };

    class Setup_Change_Common_Stones : public Setup_Change
    {
    public:
      Setup_Change_Common_Stones();

      Common_Stones common_stones;
    };

    // ----------------------------------------------------------------------------
    // <moves> : is array of <move_sequence>
    // ----------------------------------------------------------------------------
    //   <n> {<move_sequence>}*

    // ----------------------------------------------------------------------------
    // <move_sequence> : is array of <move>
    // ----------------------------------------------------------------------------
    //   <n> {<move>}* 
    
    // ----------------------------------------------------------------------------
    // <move> : is one of 
    // ----------------------------------------------------------------------------
    // * for coordinate system see <custom_board_setup>

    enum Move_Type
    {
      knock_out_move=1,		// knock_out_move <from-row> <from-col> <over-row> <over-col> 
				//   <to-row> <to-col> 
      set_move,			// set_move <pos-row> <pos-col> <stone_type>
      remove_move,		// remove_move <pos-row> <pos-col>
      finish_move		// finish_move
    };

    // ----------------------------------------------------------------------------
    // <stone_type> : is one of
    // ----------------------------------------------------------------------------
    
    enum Stone_Type
    { 
      stone_white=1,		
      stone_grey=2, 		
      stone_black=3 		
    };
    
    // ============================================================================
    // ----------------------------------------------------------------------------
    // message classes
    // ----------------------------------------------------------------------------
    // ============================================================================

    class Message
    {
    public:
      Message( Message_Type type );
      virtual ~Message() {}

      // message number at the beginning of the line indicates the message type
      static Message *read_from_line( std::string line ); // returns 0 for parse error
      virtual std::string write_to_line() { return ""; } // message does not include message type
      virtual void print( std::ostream &os ) 
      { os << to_string(get_type()) << ' ' << write_to_line() << '\n'; }

      Message_Type get_type() { return type; }
    protected:
      Message_Type type;
    };

    class Msg_Helo : public Message
    {
    public:

      Msg_Helo( Protocol, std::string host_nick );
      Msg_Helo();

      // message number (decimal,text) is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Protocol    get_protocol()     { return protocol; }
      std::string get_host_nick()    { return host_nick; }
    private:
      Protocol protocol;
      std::string host_nick;
    };

    class Msg_List_Protocols : public Message
    {
    public:
      Msg_List_Protocols( std::list<Protocol> protocols );
      Msg_List_Protocols();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<Protocol> get_protocols() { return protocols; }
    private:
      std::list<Protocol> protocols;
    };

    class Msg_Disconnect : public Message
    {
    public:
      Msg_Disconnect();
    };

    class Msg_Accept : public Message
    {
    public:
      Msg_Accept();
    };

    class Msg_Deny : public Message
    {
    public:
      Msg_Deny();
    };

    class Msg_Defer : public Message
    {
    public:
      Msg_Defer( int id );
      Msg_Defer();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_id()   { return id; }
    private:
      int id;
    };

    class Msg_Try_Again : public Message
    {
    public:
      Msg_Try_Again( int id );
      Msg_Try_Again();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_id()   { return id; }
    private:
      int id;
    };

    class Msg_Get_Rooms : public Message
    {
    public:
      Msg_Get_Rooms();
    };

    class Msg_Tell_Rooms : public Message
    {
    public:
      Msg_Tell_Rooms( std::list<Room> rooms );
      Msg_Tell_Rooms();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<Room> get_rooms()   { return rooms; }
    private:
      std::list<Room> rooms;
    };

    class Msg_Choose_Room : public Message
    {
    public:
      Msg_Choose_Room( Room room );
      Msg_Choose_Room();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Room get_room() { return room; }
    private:
      Room room;
    };

    class Msg_Get_Phase : public Message
    {
    public:
      Msg_Get_Phase();
    };

    class Msg_Tell_Phase : public Message
    {
    public:
      Msg_Tell_Phase( Phase_Type );
      Msg_Tell_Phase();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Phase_Type get_phase()   { return phase; }
    private:
      Phase_Type phase;
    };

    class Msg_Get_Game : public Message
    {
    public:
      Msg_Get_Game();
    };

    class Msg_Tell_Game : public Message
    {
    public:
      Msg_Tell_Game( std::string game );
      Msg_Tell_Game();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::string get_game()   { return game; }
    private:
      std::string game;
    };

    class Msg_Get_Setup : public Message
    {
    public:
      Msg_Get_Setup();
    };

    class Msg_Tell_Setup : public Message
    {
    public:
      Msg_Tell_Setup( Setup, bool exchange_player_indices=false );
      Msg_Tell_Setup( bool exchange_player_indices=false );

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Setup get_setup()   { return setup; }
    private:
      Setup setup;
      bool exchange_player_indices;
    };

    class Msg_Tell_Player_Setup : public Msg_Tell_Setup
    {
    public:
      Msg_Tell_Player_Setup( Setup s ) : Msg_Tell_Setup(s,true) { type = msg_tell_player_setup; }
      Msg_Tell_Player_Setup() : Msg_Tell_Setup(true) { type = msg_tell_player_setup; }
    };

    class Msg_Get_Moves : public Message
    {
    public:
      Msg_Get_Moves();
    };

    class Msg_Tell_Moves : public Message
    {
    public:
      Msg_Tell_Moves( std::list<std::pair<Move_Sequence,int/*player index*/> > moves, 
		      bool exchange_player_indices=false );
      Msg_Tell_Moves( bool exchange_player_indices=false );

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<std::pair<Move_Sequence,int/*player index*/> > get_moves()   { return moves; }
    private:
      std::list<std::pair<Move_Sequence,int/*player index*/> > moves;
      bool exchange_player_indices;
    };

    class Msg_Tell_Player_Moves : public Msg_Tell_Moves
    {
    public:
      Msg_Tell_Player_Moves( std::list<std::pair<Move_Sequence,int/*player index*/> > m ) 
	: Msg_Tell_Moves(m,true) { type = msg_tell_player_moves; }
      Msg_Tell_Player_Moves() : Msg_Tell_Moves(true) { type = msg_tell_player_moves; }
    };

    class Msg_Get_Situation : public Message
    {
    public:
      Msg_Get_Situation();
    };

    class Msg_Tell_Situation : public Message
    {
    public:
      Msg_Tell_Situation( const Board &board, 
			  const std::list<Player> &players );
      Msg_Tell_Situation();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Board		       get_board()     { return board; }
      const std::list<Player> &get_players()   { return players; }
    private:
      Board board;
      std::list<Player> players;
    };

    class Msg_Request_Final_Setup : public Message
    {
    public:
      Msg_Request_Final_Setup();
    };

    class Msg_Request_Move_Reminder : public Message
    {
    public:
      Msg_Request_Move_Reminder();
    };

    class Msg_Take_Player : public Message
    {
    public:
      Msg_Take_Player( int id );
      Msg_Take_Player();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_id()   { return id; }
    private:
      int id;
    };

    class Msg_Setup : public Message
    {
    public:
      Msg_Setup();
    };

    class Msg_Get_Setup_And_Changes : public Message
    {
    public:
      Msg_Get_Setup_And_Changes();
    };

    class Msg_Get_Moves_And_Play : public Message
    {
    public:
      Msg_Get_Moves_And_Play();
    };

    class Msg_Get_Situation_And_Play : public Message
    {
    public:
      Msg_Get_Situation_And_Play();
    };

    class Msg_Add_Player : public Message
    {
    public:
      Msg_Add_Player( Player );
      Msg_Add_Player();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Player get_player()   { return player; }
    private:
      Player player;
    };

    class Msg_Accept_Player : public Message
    {
    public:
      Msg_Accept_Player( int id );
      Msg_Accept_Player();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_id()   { return id; }
    private:
      int id;
    };

    class Msg_Remove_Player : public Message
    {
    public:
      Msg_Remove_Player( int id );
      Msg_Remove_Player();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_id()   { return id; }
    private:
      int id;
    };

    class Msg_Setup_Change : public Message
    {
    public:
      // the setup changes will be deleted by this object (if not obtained)
      Msg_Setup_Change( std::list<Setup_Change*> changes );
      Msg_Setup_Change();
      ~Msg_Setup_Change();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<Setup_Change*>	get_changes()		{ return changes; }
      std::list<Setup_Change*>	obtain_changes()	{ return changes; changes.clear(); }
    private:
      std::list<Setup_Change*> changes;

      // don't copy objects of this type
      Msg_Setup_Change(const Msg_Setup_Change&);
      void operator=(const Msg_Setup_Change&);
    };

    class Msg_Ask_Setup_Change : public Message
    {
    public:
      // the setup changes will be deleted by this object (if not obtained)
      Msg_Ask_Setup_Change( std::list<Setup_Change*> changes );
      Msg_Ask_Setup_Change();
      ~Msg_Ask_Setup_Change();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<Setup_Change*>	get_changes()		{ return changes; }
      std::list<Setup_Change*>	obtain_changes()	{ return changes; changes.clear(); }
    private:
      std::list<Setup_Change*> changes;

      // don't copy objects of this type
      Msg_Ask_Setup_Change(const Msg_Ask_Setup_Change&);
      void operator=(const Msg_Ask_Setup_Change&);
    };

    class Msg_Ask_Capable : public Message
    {
    public:
      // the setup changes will be deleted by this object (if not obtained)
      Msg_Ask_Capable( std::list<Setup_Change*> changes );
      Msg_Ask_Capable();
      ~Msg_Ask_Capable();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      std::list<Setup_Change*>	get_changes()		{ return changes; }
      std::list<Setup_Change*>	obtain_changes()	{ return changes; changes.clear(); }
    private:
      std::list<Setup_Change*> changes;

      // don't copy objects of this type
      Msg_Ask_Capable(const Msg_Ask_Capable&);
      void operator=(const Msg_Ask_Capable&);
    };

    class Msg_Incapable : public Message
    {
    public:
      // the setup changes will be deleted by this object (if not obtained)
      Msg_Incapable( std::string host_nick, std::list<Setup_Change*> changes );
      Msg_Incapable();
      ~Msg_Incapable();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string		get_host_nick()		{ return host_nick; }
      std::list<Setup_Change*>	get_changes()		{ return changes; }
      std::list<Setup_Change*>	obtain_changes()	{ return changes; changes.clear(); }
    private:
      std::string host_nick;
      std::list<Setup_Change*> changes;

      // don't copy objects of this type
      Msg_Incapable(const Msg_Incapable&);
      void operator=(const Msg_Incapable&);
    };

    class Msg_Ready : public Message
    {
    public:
      Msg_Ready();
    };

    class Msg_Start_Game : public Message
    {
    public:
      Msg_Start_Game();
    };

    class Msg_Your_Turn : public Message
    {
    public:
      Msg_Your_Turn();
    };

    class Msg_Move : public Message
    {
    public:
      Msg_Move( int player_id, Move_Sequence move );
      Msg_Move();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      Move_Sequence get_move()   { return move; }
      int get_player_id() { return player_id; }
    private:
      int player_id;
      Move_Sequence move;
    };

    class Msg_Undo : public Message
    {
    public:
      Msg_Undo( int n );
      Msg_Undo();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();
  
      int get_n()   { return n; }
    private:
      int n;
    };

    class Msg_Ask_Undo : public Message
    {
    public:
      Msg_Ask_Undo( std::string host_nick, int n );
      Msg_Ask_Undo();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string		get_host_nick()		{ return host_nick; }
      int			get_n()			{ return n; }
    private:
      std::string host_nick;
      int n;
    };

    class Msg_New_Game : public Message
    {
    public:
      Msg_New_Game();
    };

    class Msg_Ask_New_Game : public Message
    {
    public:
      Msg_Ask_New_Game( std::string host_nick );
      Msg_Ask_New_Game();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string get_host_nick()		{ return host_nick; }
    private:
      std::string host_nick;
    };

    class Msg_Chat : public Message
    {
    public:
      Msg_Chat( std::string host_nick, std::string message );
      Msg_Chat();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string		get_host_nick()		{ return host_nick; }
      std::string		get_message()		{ return message; }
    private:
      std::string host_nick;
      std::string message;
    };

    class Msg_Set_Nick : public Message
    {
    public:
      Msg_Set_Nick( std::string host_nick );
      Msg_Set_Nick();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string		get_host_nick()		{ return host_nick; }
    private:
      std::string host_nick;
    };

    class Msg_Nick_Change : public Message
    {
    public:
      Msg_Nick_Change( std::string old_host_nick, std::string new_host_nick,
		       std::list<int> player_ids );
      Msg_Nick_Change();

      // message number is included at the beginning of the line
      bool read_from_line( std::string line ); // returns false on parse error
      virtual std::string write_to_line();

      std::string		get_old_host_nick()	{ return old_host_nick; }
      std::string		get_new_host_nick()	{ return new_host_nick; }
      std::list<int>		get_player_ids()	{ return player_ids; }
    private:
      std::string old_host_nick;
      std::string new_host_nick;
      std::list<int> player_ids;
    };

    class Msg_Ping : public Message
    {
    public:
      Msg_Ping();
    };

    class Msg_Pong : public Message
    {
    public:
      Msg_Pong();
    };

    class Msg_Error : public Message
    {
    public:
      Msg_Error();
    };
  }
}
#endif
