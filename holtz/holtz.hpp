/*
 * holtz.cpp
 * 
 * Game declaration
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

#ifndef __HOLTZ_MAIN__
#define __HOLTZ_MAIN__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <iostream>

namespace holtz
{
  class Stones;
  class Player;
  class Player_Input;
  class Player_Output;
  class Win_Condition;
  class Field_Iterator;
  class Board;
  class Move;
  class Knock_Out_Move;
  class Set_Move;
  class Remove;
  class Sequence;
  class Variant;
  class Variant_Tree;
  class Game;
  class Standard_Win_Condition;
  class Standard_Ruleset;
  class No_Output;
  class Stream_Output;
  class Stream_Input;
  class AI_Input;
  class Sequence_Generator;
  class Generic_Mouse_Input;
  class Coordinate_Translator;
  class Standard_Coordinate_Translator;
  class Move_Translator;
  
  typedef enum Field_State_Type{ field_removed=-1, field_empty=0, 
				 field_white=1, field_grey=2, field_black=3 };

  const int standard_board[][7] =
    { {   -1,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0, -1, -1 } };

  const int board_37[][7] =
    { {   -1,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0, -1, -1 } };

  const int board_40[][7] =
    { {   -1,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0, -1, -1 },
      { -1, -1,  0,  0,  0, -1, -1 } };

  const int board_44[][8] =
    { {   -1,  0,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0,  0, -1, -1 } };

  const int board_48[][8] =
    { {   -1,  0,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0,  0, -1, -1 },
      { -1, -1,  0,  0,  0,  0, -1, -1 } };

  const int board_61[][9] =
    { {   -1, -1,  0,  0,  0,  0,  0, -1, -1 },
      { -1, -1,  0,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0,  0,  0,  0 },
      { -1,  0,  0,  0,  0,  0,  0,  0,  0 },
      {   -1,  0,  0,  0,  0,  0,  0,  0, -1 },
      { -1, -1,  0,  0,  0,  0,  0,  0, -1 },
      {   -1, -1,  0,  0,  0,  0,  0, -1, -1 } };
}

namespace holtz
{
  class Exception
  {
  public:
    std::string name;
  };

  class Input_Exception : public Exception
  {
  };

  class Ref_Counter
  {
  public:
    Ref_Counter() : cnt(1) {}
    
    int cnt;
  };
  
  class Stones
  {
  public:
    typedef enum Stone_Type{ invalid_stone=0, white_stone=1, grey_stone=2, black_stone=3 };
    std::map<Stone_Type, int> stone_count;

    void remove_stones();
  };

  class Common_Stones : public Stones
  {
  public:
    typedef enum Common_Stones_Type{ standard, tournament, custom };

    Common_Stones( Common_Stones_Type );
    Common_Stones_Type type;
  };

  class Standard_Common_Stones : public Common_Stones
  {
  public:
    Standard_Common_Stones();
  };

  class Tournament_Common_Stones : public Common_Stones
  {
  public:
    Tournament_Common_Stones();
  };

  class Custom_Common_Stones : public Common_Stones
  {
  public:
    Custom_Common_Stones( int white_num, int grey_num, int black_num );
  };

  class Player_Input
  {
  public:
    typedef enum Player_State{ finished, wait_for_event, request_undo,
			       interruption_possible };

    virtual Player_State determine_move() throw(Exception) = 0;
    virtual Sequence get_move() = 0;
    virtual long get_used_time() = 0;

    virtual ~Player_Input();
  };

  class Player_Output
  {
  public:
    virtual void report_move( const Sequence & ) = 0;
    /* for future
    virtual bool is_undo_allowed() = 0;
    virtual void undo_move() = 0;
    */
    virtual ~Player_Output();
  };

  class Player
  {
  public:
    typedef enum Player_Type{ unknown, user, ai };
    typedef enum Help_Mode{ no_help, show_possible_moves, show_hint };

    Player( std::string name="", int id=-1, Player_Input *in=0, std::list<Player_Output*> out=no_output,
	    std::string host="", Player_Type type=unknown, Help_Mode help_mode = no_help );

    std::string name; int id;
    std::string host; 
    Player_Type type;
    Help_Mode help_mode;
    long total_time, average_time;
    int num_measures;		// of average time

    Stones stones;
    Player_Input *input;
    std::list<Player_Output*> outputs; 
    bool is_active;

    static std::list<Player_Output*> no_output;
  };

  struct Field_Pos
  {
    int x;
    int y;
    Field_Pos();
    Field_Pos( int x, int y );
  };
  
  bool operator==( const Field_Pos &p1, const Field_Pos &p2 );
  bool operator!=( const Field_Pos &p1, const Field_Pos &p2 );

  class Field_Iterator // obsolete?
  {
  public:
    typedef enum Direction{ invalid_direction=-1, 
			    right=0, top_right=1, top_left=2, left=3, 
			    bottom_left=4, bottom_right=5,
			    right2=6, top_right2=7, top_left2=8, left2=9, 
			    bottom_left2=10, bottom_right2=11 };

    // counting numbers turns in math. positive direction

    Field_Iterator( Board * );
    Field_Iterator( Field_Pos, Board * );
    void set_pos( Field_Pos );
    inline Field_Pos get_pos() const { return current_pos; }
    static bool is_connected( Field_Pos p1, Field_Pos p2 );
    static Direction get_direction( Field_Pos from, Field_Pos to );
    static Direction get_far_direction( Field_Pos from, Field_Pos to );

    Field_Iterator Next_Left() const;
    Field_Iterator Next_Right() const;
    Field_Iterator Next_Top_Left() const;
    Field_Iterator Next_Top_Right() const;
    Field_Iterator Next_Bottom_Left() const;
    Field_Iterator Next_Bottom_Right() const;
    Field_Iterator Next( Direction ) const;

    Field_Iterator &Left();
    Field_Iterator &Right();
    Field_Iterator &Top_Left();
    Field_Iterator &Top_Right();
    Field_Iterator &Bottom_Left();
    Field_Iterator &Bottom_Right();
    Field_Iterator &Go( Direction );

    bool is_valid_field();	// tells whether field is in range
    Field_State_Type &operator*();
  private:
    Field_Pos current_pos;
    Board *board;
  };
  
  inline bool operator==( const Field_Iterator &p1, const Field_Iterator &p2 )
  { return p1.get_pos() == p2.get_pos(); }
  inline bool operator!=( const Field_Iterator &p1, const Field_Iterator &p2 )
  { return p1.get_pos() != p2.get_pos(); }

  class Board
  {
  public:
    typedef enum Board_Type{ s37_rings, s40_rings, s44_rings, s48_rings, s61_rings, custom };

    Board( const int *field_array, int width, int height, Board_Type type = custom );
    // field must be rectangular array!
    Board( const std::vector< std::vector<Field_State_Type> > fields, Board_Type type = custom );

    Board_Type board_type;

    inline int get_stone_type_number( Field_State_Type state )
    { return state; }
    static inline bool is_stone( Field_State_Type state )
    { return state > 0; }
    static inline bool is_empty( Field_State_Type state )
    { return state == field_empty; }
    static inline bool is_removed( Field_State_Type state )
    { return state == field_removed; }

    bool is_knock_out_possible(); // is any knock out possible
    bool is_knock_out_possible( Field_Pos from );
    std::pair<bool,Field_Pos> is_knock_out_possible( Field_Pos from, Field_Pos to );
    // is knock out possible within these three fields
    static bool is_knock_out_possible( Field_Iterator p1,
				       Field_Iterator p2,
				       Field_Iterator p3 ); 
    std::list<Knock_Out_Move> get_knock_out_moves();
    std::list<Knock_Out_Move> get_knock_out_moves( Field_Pos from );

    bool is_removable( Field_Pos );
    bool is_any_field_removable();
    std::list<Field_Pos> get_empty_fields();
    std::list<Field_Pos> get_removable_fields();

    Field_Iterator first_field();

    std::vector< std::vector<Field_State_Type> > field; // field[x][y]
    inline Field_State_Type get_field( Field_Pos pos ) { return field[pos.x][pos.y]; }
    inline int get_x_size() const { return field.size(); }
    inline int get_y_size() const { return field[0].size(); }
  };

  class Move
  {
  public:
    typedef enum Move_Type { no_move, knock_out_move, set_move, remove, finish_move };

    virtual Move_Type get_type() const = 0;
    virtual void do_move( Game & ) = 0;
    virtual void undo_move( Game & ) = 0;
    virtual bool check_move( Game & ) const = 0; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const = 0;
						 // true: type ok
    virtual bool may_be_first_move( Game & ) const = 0;
    virtual bool may_be_last_move( Game & ) const = 0;
    virtual std::ostream &output( std::ostream & ) const = 0;
    virtual std::istream &input( std::istream & ) = 0;

    virtual Move *clone() const = 0;

    virtual ~Move();
  };

  class Knock_Out_Move : public Move
  {
  public:
    virtual Move_Type get_type() const;
    virtual void do_move( Game & );
    virtual void undo_move( Game & );
    virtual bool check_move( Game & ) const; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const; // true: type ok
    virtual bool may_be_first_move( Game & ) const;
    virtual bool may_be_last_move( Game & ) const;
    virtual std::ostream &output( std::ostream & ) const;
    virtual std::istream &input( std::istream & );

    virtual Move *clone() const;

    Knock_Out_Move();
    Knock_Out_Move( Field_Pos from, Field_Pos over, Field_Pos to );

  //private:
    Field_Pos from, over, to;
    Field_State_Type knocked_stone;
  };

  class Set_Move : public Move
  {
  public:
    virtual Move_Type get_type() const;
    virtual void do_move( Game & );
    virtual void undo_move( Game & );
    virtual bool check_move( Game & ) const; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const; // true: type ok
    virtual bool may_be_first_move( Game & ) const;
    virtual bool may_be_last_move( Game & ) const;
    virtual std::ostream &output( std::ostream & ) const;
    virtual std::istream &input( std::istream & );

    virtual Move *clone() const;

    Set_Move();
    Set_Move( Field_Pos pos, Stones::Stone_Type stone_type );

  //private:
    Field_Pos pos;
    Stones::Stone_Type stone_type;
    bool own_stone;		// whether stone belonged to player
  };

  class Remove : public Move
  {
  public:
    virtual Move_Type get_type() const;
    virtual void do_move( Game & );
    virtual void undo_move( Game & );
    virtual bool check_move( Game & ) const; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const; // true: type ok
    virtual bool may_be_first_move( Game & ) const;
    virtual bool may_be_last_move( Game & ) const;
    virtual std::ostream &output( std::ostream & ) const;
    virtual std::istream &input( std::istream & );

    virtual Move *clone() const;

    Remove();
    Remove( Field_Pos remove_pos );
  //private:
    Field_Pos remove_pos;
  };

  // must be added as last move
  class Finish_Move : public Move
  {
  public:
    virtual Move_Type get_type() const;
    virtual void do_move( Game & );
    virtual void undo_move( Game & );
    virtual bool check_move( Game & ) const; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const; // true: type ok
    virtual bool may_be_first_move( Game & ) const;
    virtual bool may_be_last_move( Game & ) const;
    virtual std::ostream &output( std::ostream & ) const;
    virtual std::istream &input( std::istream & );

    virtual Move *clone() const;

    Finish_Move();
  //private:
    std::list< std::pair<Field_Pos,Stones::Stone_Type> > removed_stones;
  };

  class Sequence
  {
  public:
    Sequence();
    //! copies of sequences share same moves!
    Sequence( const Sequence &sequence );
    ~Sequence();
    Sequence &operator=( const Sequence &sequence );

    void do_sequence( Game & ) const;
    void undo_sequence( Game & ) const;
    bool check_sequence( Game & ) const; // true: move ok
    std::ostream &output( std::ostream & ) const;
    std::istream &input( std::istream & );

    void add_move( Move * );		// add move unchecked
    bool add_move( Game&, Move * );	// true: adding move ok (only relative check)
    Move *get_last_move();
    void undo_last_move( Game & );	// calls undo for last move and removes it
    void clear();

    Sequence clone() const;		// to store sequences, they must be cloned

    inline const std::list<Move*> &get_moves() const { return *moves; }
    inline bool is_empty() { return moves->empty(); }
  private:
    std::list<Move*> *moves;

    Ref_Counter *ref_counter;
  };

  inline std::ostream &operator<<( std::ostream &os, const Sequence &s )
  { return s.output(os); }
  inline std::istream &operator>>( std::istream &is, Sequence &s )
  { return s.input(is); }

  class Variant
  {
  public:
    Variant( std::list<Player>::iterator current_player, const Sequence &, Variant *prev = 0 );
    ~Variant();

    Variant *add_variant( std::list<Player>::iterator current_player, const Sequence & );
    bool is_prev( Variant * );	// checks all previous variants

    std::list<Player>::iterator current_player;
    Sequence move_sequence;

    Variant *prev;
    std::list<Variant*> variants;
  private:
    friend Variant_Tree;
    Variant();			// for root variant
  };

  class Variant_Tree
  {
  public:
    Variant_Tree();
    ~Variant_Tree();

    inline bool is_first() { return current_variant == root; }
    bool remove_subtree( Variant * );

    inline Variant *get_current_variant() { return current_variant; }
    inline Variant *get_root_variant()    { return root; }
  private:
    friend Game;
    void add_in_current_variant( std::list<Player>::iterator current_player, const Sequence & );
    void move_a_variant_back();
    
    Variant *root;
    Variant *current_variant;
  };

  class Win_Condition
  {
  public:
    typedef enum Win_Condition_Type{ standard, tournament, generic, full_custom };

    Win_Condition( Win_Condition_Type type = full_custom );
    virtual ~Win_Condition();

    virtual bool did_player_win( Game &, Player & ) const = 0;
    virtual Win_Condition *clone() = 0;

    Win_Condition_Type type;	// is generic win condition with number of white/grey/black/all stones
  };

  class Coordinate_Translator
  {
  public:
    virtual ~Coordinate_Translator() {}

    virtual std::string get_field_name( Field_Pos pos ) = 0;
    virtual Field_Pos   get_field_pos ( std::string name ) = 0;

    virtual Coordinate_Translator *clone() = 0;
  };

  class Ruleset
  {
  public:
    typedef enum Ruleset_Type { standard, tournament, custom };
    Ruleset( const Ruleset & );
    Ruleset &operator=( Ruleset & );
    virtual ~Ruleset();
    virtual Ruleset *clone() const { return new Ruleset(*this); }

    inline Ruleset_Type get_type() const { return type; }
    inline unsigned get_min_players() const { return min_players; }
    inline unsigned get_max_players() const { return max_players; }
    inline Coordinate_Translator *get_coordinate_translator() const { return coordinate_translator; }
    inline void set_win_condition( Win_Condition *wc ) { delete win_condition; win_condition = wc; }
    inline void set_coordinate_translator( Coordinate_Translator *ct ) 
    { delete coordinate_translator; coordinate_translator = ct; }
  public:			// semi public 
    // win_condition and coordinate translator will be deleted by Ruleset!
    Ruleset( Ruleset_Type type, Board, Common_Stones, Win_Condition *, Coordinate_Translator *, 
	     bool undo_possible, unsigned min_players, unsigned max_players ); 

    Ruleset_Type type;

    Board board;
    Common_Stones common_stones;
    Win_Condition *win_condition;
    Coordinate_Translator *coordinate_translator;
    bool undo_possible;
    unsigned min_players, max_players;

    friend class Game;
  };

  class Game
  {
  public:
    typedef enum Game_State{ finished, wait_for_event, next_players_turn, interruption_possible, 
			     wrong_number_of_players };

    Game( const Ruleset & );
    Game( const Game & );
    Game &operator=( const Game & );
    ~Game();

    Game_State continue_game() throw(Exception); // start or continue game
    void stop_game();		// stop game to allow changing the position with do_move and undo_move
    Player *get_winner();	// returns 0 if no player won
    void reset_game();
    void reset_game( const Ruleset & );

    // init functions
    bool set_players( const std::list<Player> & ); // true: right number of players
    bool add_player( const Player & ); // true: right number of players
    void remove_players();

    void do_move( const Sequence & );   // does move and changes current player
    bool undo_move();		// undoes move and changes current player
    //void go_to_variant( Variant * );

    inline unsigned get_min_players() { return ruleset->min_players; }
    inline unsigned get_max_players() { return ruleset->max_players; }

    void copy_player_times( Game &from ); // number of players must be the same
  public:
    // public: for internal usage (friend might be used instead)
    std::list<Player> players;

    std::list<Player>::iterator current_player;
    std::list<Player>::iterator last_player;

    Ruleset *ruleset;
    Board board;
    Stones common_stones;
    Win_Condition *win_condition;
    Coordinate_Translator *coordinate_translator;
    bool undo_possible;

    Variant_Tree variant_tree;

    Game *save_game;		// shadows the game to avoid corruption by input handlers

    Ref_Counter *ref_counter;	// reference counting because of ruleset pointer

  public:
    // use this functions only of you use Move::do_move before
    // use Game::do_move otherwise
    bool next_player();		// next player is current player (players may be skiped)
				// call next_player after Sequence::do_move
    bool prev_player( std::list<Player>::iterator prev_last_player );		
				// prev player is current player prev_last_player can't be determined
				// automatically
  private:
    void remove_filled_areas();
  };


  class Standard_Coordinate_Translator : public Coordinate_Translator
  {
  public:
    Standard_Coordinate_Translator( const Board &original_board );

    virtual std::string get_field_name( Field_Pos pos );
    virtual Field_Pos   get_field_pos ( std::string name );

    virtual Coordinate_Translator *clone();
  private:
    Board orig_board;
  };

  class Move_Translator
  {
  public:
    virtual std::string encode( Sequence ) = 0;
    virtual Sequence    decode( std::istream& ) = 0;
  };

  class Standard_Move_Translator : public Move_Translator
  {
  public:
    Standard_Move_Translator( Coordinate_Translator *, Board * );

    virtual std::string encode( Sequence );
    virtual Sequence    decode( std::istream& );
  private:
    Coordinate_Translator *coordinate_translator;
    Board *board;
  };
  
  class Generic_Win_Condition : public Win_Condition
  {
  public:
    Generic_Win_Condition( int num_white, int num_grey, int num_black, int num_all );

    virtual bool did_player_win( Game &, Player & ) const;
    virtual Win_Condition *clone();

    int num_white;		// white stones to win
    int num_grey;		// grey stones to win
    int num_black;		// black stones to win
    int num_all;		// number of stones of each colour to win
  };

  class Standard_Win_Condition : public Generic_Win_Condition
  {
  public:
    virtual Win_Condition *clone() { return new Standard_Win_Condition(); }

    Standard_Win_Condition();
  };

  class Tournament_Win_Condition : public Generic_Win_Condition
  {
  public:
    virtual Win_Condition *clone() { return new Tournament_Win_Condition(); }

    Tournament_Win_Condition();
  };

  class Custom_Ruleset : public Ruleset
  {
  public:
    Custom_Ruleset( Board, Common_Stones, Win_Condition *, Coordinate_Translator *, 
		    bool undo_possible = true, 
		    unsigned min_players = 2, unsigned max_players = 4 );
  };

  class Standard_Ruleset : public Ruleset
  {
  public:
    Standard_Ruleset();
  };

  class Tournament_Ruleset : public Ruleset
  {
  public:
    Tournament_Ruleset();
  };
  
  class No_Output : public Player_Output
  {
  public:
    virtual void report_move( const Sequence & );
  };
  class Stream_Output : public Player_Output
  {
  public:
    Stream_Output( Game &, std::ostream & );
    virtual void report_move( const Sequence & );
  private:
    Game &game;
    std::ostream &os;
  };

  class Stream_Input : public Player_Input
  {
  public:
    Stream_Input( Game &, std::istream & );
    virtual Player_State determine_move() throw(Exception);
    virtual Sequence get_move();
  private:
    Game &game;
    std::istream &is;
    Sequence sequence;
  };

  // this class should help to generate move sequences from a GUI
  class Sequence_Generator
  {
  public:
    typedef enum Sequence_State{ finished=0,
				 hold_prefix=0, hold_white=1, hold_grey=2, hold_black=3,
				 another_click=4,

				 error_require_knock_out=-500, 
				 error_require_set,
				 error_require_remove,
				 error_can_t_remove,
				 error_can_t_move_here, 
				 error_can_t_set_here,
				 error_must_pick_common_stone,
				 error_wrong_player, 
				 error_impossible_yet,
				 error_must_knock_out_with_same_stone,

				 fatal_error=-1000 };

    Sequence_Generator( Game & );
    // synthesize move sequence from clicks
    Sequence_State add_click( Field_Pos pos ); 
    Sequence_State add_click_common_stone( Stones::Stone_Type stone_type ); 
    Sequence_State add_click_player_stone( int player_id, 
					   Stones::Stone_Type stone_type );
    Move *undo_click();		// undo click (returns 0) or move (returns move to be undone, delete it!)
    
    Move::Move_Type get_required_move_type();
    std::list<Field_Pos> get_possible_clicks();

    void reset();
    inline bool is_finished() 
    { return state == move_finished; }
    inline Move *get_move_done() // get move done by add_click (may return 0)
    { return move_done; }
    inline const Sequence &get_sequence() const 
    { return sequence; }
  private:
    Sequence sequence;
    Game &game;

    typedef enum Internal_State{ begin, move_from, move_dest, stone_picked, stone_set, move_finished };
    Internal_State state;

    Stones::Stone_Type picked_stone;
    Field_Pos from;
    std::stack<Field_Pos> to;	// destination positions of moves

    Move *move_done;
  };

  class Generic_Mouse_Input : public Player_Input
  {
  public:
    Generic_Mouse_Input( Game & );
    virtual Player_State determine_move() throw(Exception);
    virtual Sequence get_move();

    virtual void init_mouse_input() = 0;
    virtual void disable_mouse_input() = 0;
  protected:
    Sequence_Generator sequence_generator;

    Game &game;
  };
}

#endif
