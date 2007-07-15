/*
 * zertz.cpp
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

#ifndef __ZERTZ_MAIN__
#define __ZERTZ_MAIN__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <iostream>

#include "util.hpp"

namespace zertz
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
  class Move_Sequence;
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

namespace zertz
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
    Stones();

    typedef enum Stone_Type{ invalid_stone=0, white_stone=1, grey_stone=2, black_stone=3 };
    std::map<Stone_Type, int> stone_count;

#ifndef __WXMSW__
    void print();
#endif

    void remove_stones();
  };

  class Common_Stones : public Stones
  {
  public:
    typedef enum Common_Stones_Type{ standard=0, tournament, custom };

    Common_Stones( Common_Stones_Type type=custom );
    Common_Stones_Type get_type() const { return type; }
  protected:
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
    typedef enum Player_State{ finished=0, wait_for_event, request_undo,
			       interruption_possible };

    virtual Player_State determine_move() throw(Exception) = 0;
    virtual Move_Sequence get_move() = 0;
    virtual long get_used_time() = 0;

    virtual ~Player_Input();
  };

  class Player_Output
  {
  public:
    virtual void report_move( const Move_Sequence & ) = 0;

    virtual ~Player_Output();
  };

  class Player
  {
  public:
    typedef enum Player_Type{ unknown=0, user, ai };
    typedef enum Help_Mode{ no_help=0, show_possible_moves, show_hint };
    typedef enum Origin{ local, remote }; // origin where player is controled

    Player( std::string name="", int id=-1, Player_Input *in=0, 
	    std::list<Player_Output*> out=no_output,
	    std::string host="", Player_Type type=unknown, Help_Mode help_mode = no_help, 
	    Origin origin = local );

    std::string name; int id;
    std::string host; 
    Player_Type type;
    Help_Mode help_mode;
    Origin origin;
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

    static Field_Pos get_middle( Field_Pos p1, Field_Pos p2 );
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
    static Field_Pos get_next( Field_Pos p1, Direction dir );

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
    typedef enum Board_Type{ s37_rings=0, s40_rings, s44_rings, s48_rings, s61_rings, custom=99 };

    Board( const int *field_array, int width, int height, Board_Type type = custom );
    // field must be rectangular array!
    Board( const std::vector< std::vector<Field_State_Type> > fields, Board_Type type = custom );
    Board();			// should only be used for structures 

    Board_Type board_type;
    inline Board_Type get_type() const { return board_type; }

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
    typedef enum Move_Type { no_move=0, knock_out_move, set_move, remove, finish_move };

    virtual Move_Type get_type() const = 0;
    virtual void do_move( Game & ) = 0;
    virtual void undo_move( Game & ) = 0;
    virtual bool check_move( Game & ) const = 0; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const = 0;
						 // true: type ok
    virtual bool may_be_first_move( Game & ) const = 0;
    virtual bool may_be_last_move( Game & ) const = 0;
    virtual std::escape_ostream &output( std::escape_ostream & ) const = 0;
    virtual bool input( std::escape_istream & ) = 0;

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
    virtual std::escape_ostream &output( std::escape_ostream & ) const;
    virtual bool input( std::escape_istream & );

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
    virtual std::escape_ostream &output( std::escape_ostream & ) const;
    virtual bool input( std::escape_istream & );

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
    virtual std::escape_ostream &output( std::escape_ostream & ) const;
    virtual bool input( std::escape_istream & );

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
    virtual std::escape_ostream &output( std::escape_ostream & ) const;
    virtual bool input( std::escape_istream & );

    virtual Move *clone() const;

    Finish_Move();
  //private:
    std::list< std::pair<Field_Pos,Stones::Stone_Type> > removed_stones;
  };

  class Move_Sequence
  {
  public:
    Move_Sequence();
    //! copies of sequences share same moves!
    Move_Sequence( const Move_Sequence &sequence );
    ~Move_Sequence();
    Move_Sequence &operator=( const Move_Sequence &sequence );

    void do_sequence( Game & ) const;
    void undo_sequence( Game & ) const;
    bool check_sequence( Game & ) const; // true: move ok
    std::escape_ostream &output( std::escape_ostream & ) const;
    bool input( std::escape_istream & );

    void add_move( Move * );		// add move unchecked
    bool add_move( Game&, Move * );	// true: adding move ok (only relative check)
    Move *get_last_move();
    void undo_last_move( Game & );	// calls undo for last move and removes it
    void clear();

    Move_Sequence clone() const;		// to store sequences, they must be cloned

    inline const std::list<Move*> &get_moves() const { return *moves; }
    inline bool is_empty() { return moves->empty(); }
  private:
    std::list<Move*> *moves;

    Ref_Counter *ref_counter;
  };

  inline std::escape_ostream &operator<<( std::escape_ostream &eos, const Move_Sequence &s )
  { s.output(eos); return eos; }
  inline std::escape_istream &operator>>( std::escape_istream &eis, Move_Sequence &s )
  { s.input(eis); return eis; }

  inline std::ostream &operator<<( std::ostream &os, const Move_Sequence &s )
  { std::escape_ostream eos(os); s.output(eos); return os; }
  inline std::istream &operator>>( std::istream &is, Move_Sequence &s )
  { std::escape_istream eis(is); s.input(eis); return is; }

  class Variant
  {
  public:
    Variant( int current_player_index, const Move_Sequence &, 
	     unsigned possible_variants, Variant *prev = 0 );
    ~Variant();

    Variant *add_variant( int current_player_index,  
			  const Move_Sequence &, unsigned possible_variants );
    bool is_prev( Variant * ) const;	// checks all previous variants
    Variant *clone( Variant *search = 0, Variant *&clone = default_clone ) const;
				// clone variant with all subvariants
				// optional: search for variant and store clone adresse of it in clone

    int current_player_index;	// player of the move sequence (0=first player,1=second player,...)
    Move_Sequence move_sequence;
    unsigned possible_variants;	// number of possible variants in the situation after this move 

    Variant *prev;
    std::list<Variant*> variants;
  private:
    friend class Variant_Tree;
    Variant();			// for root variant

    static Variant *default_clone; // just as unused default argument in clone method
  };

  class Variant_Tree
  {
  public:
    Variant_Tree();
    Variant_Tree(const Variant_Tree &);
    Variant_Tree &operator=( const Variant_Tree & );
    ~Variant_Tree();

    inline bool is_first() const { return current_variant == root; }
    bool remove_subtree( Variant * );

    inline Variant *get_current_variant() { return current_variant; }
    inline Variant *get_root_variant()    { return root; }
    inline const Variant *get_current_variant() const { return current_variant; }
    inline const Variant *get_root_variant()    const { return root; }
  public:
    friend class Game;
    void add_in_current_variant( int current_player_index, const Move_Sequence &, 
				 unsigned possible_variants );
    void move_a_variant_back();
    void clear();
  private:
    Variant *root;
    Variant *current_variant;
  };

  class Win_Condition
  {
  public:
    typedef enum Win_Condition_Type{ standard=0, tournament, generic, full_custom };

    Win_Condition( Win_Condition_Type type = full_custom );
    virtual ~Win_Condition();

    virtual bool did_player_win( Game &, Player & ) const = 0;
    virtual Win_Condition *clone() = 0;

    Win_Condition_Type get_type() const { return type; }
  protected:
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
    typedef enum Ruleset_Type { standard=0, tournament, custom };
    Ruleset( const Ruleset & );
    Ruleset();
    Ruleset &operator=( const Ruleset & );
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
    typedef enum Game_State{ finished=0, wait_for_event, next_players_turn, interruption_possible, 
			     wrong_number_of_players };

    Game( const Ruleset & );
    Game( const Game & );
    Game &operator=( const Game & );
    ~Game();

    // **********************
    // game control functions
    Game_State continue_game() throw(Exception); // start or continue game
    void stop_game();		// stop game to allow changing the position with do_move and undo_move
    int get_winner_index() { return winner_player_index; } // returns -1 if no player won
    void reset_game();		// doesn't reset players
    void reset_game( const Ruleset & );	// doesn't reset players

    // **************
    // init functions
    bool set_players( const std::vector<Player> & ); // true: right number of players
    bool add_player( const Player & ); // true: right number of players
    void remove_players();

    // **********************
    // manipulation functions
    void do_move( const Move_Sequence & );   // does move and changes current player
    bool undo_move();		// undoes move and changes current player
    //void go_to_variant( Variant * );

    // *********************
    // information functions
    inline unsigned get_min_players() { return ruleset->min_players; }
    inline unsigned get_max_players() { return ruleset->max_players; }

    int get_num_possible_moves(); // number of possible moves in current situation
    std::list<Move_Sequence> get_possible_moves(); // get possible moves in situation
    
    std::vector<Player>::iterator get_next_player( std::vector<Player>::iterator player );
    std::vector<Player>::iterator get_prev_player( std::vector<Player>::iterator player );

    // *********************
    // utility functions

    void copy_player_times( Game &from ); // number of players must be the same
  public:
    // public: for internal usage (friend might be used instead)
    std::vector<Player> players;

    std::vector<Player>::iterator current_player;
    int current_player_index;	// index of the current player
    std::vector<Player>::iterator prev_player; // the player who was current_player last time
    int prev_player_index;	// index of player that was current player before

    Ruleset *ruleset;
    Board board;
    Stones common_stones;
    Win_Condition *win_condition;
    Coordinate_Translator *coordinate_translator;
    bool undo_possible;

    Variant_Tree variant_tree;

    Ref_Counter *ref_counter;	// reference counting because of ruleset pointer

    inline Player &get_current_player() 
    { assert(current_player_index >= 0); return players[current_player_index]; }
    inline const Player &get_current_player() const
    { assert(current_player_index >= 0); return players[current_player_index]; }

    std::vector<Player>::iterator get_player( int index );
    std::vector<Player>::const_iterator get_player( int index ) const;
    int get_player_index( std::vector<Player>::iterator ) const;
  public:
    // use this functions only of you use Move::do_move before
    // use Game::do_move otherwise
    bool choose_next_player();		// next player is current player (players may be skiped)
				// call next_player after Move_Sequence::do_move
    bool choose_prev_player( int prev_prev_player_index );		
				// prev player is current player prev_prev_player can't be determined
				// automatically
  private:
    void remove_filled_areas();

    int winner_player_index;
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
    virtual std::string encode( Move_Sequence ) = 0;
    virtual Move_Sequence    decode( std::istream& ) = 0;
  };

  class Standard_Move_Translator : public Move_Translator
  {
  public:
    Standard_Move_Translator( Coordinate_Translator * );

    virtual std::string encode( Move_Sequence );
    virtual Move_Sequence    decode( std::istream& );
  private:
    Coordinate_Translator *coordinate_translator;
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
    // win_condition and coordinate translator will be deleted by Ruleset!
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
    virtual void report_move( const Move_Sequence & );
  };
  class Stream_Output : public Player_Output
  {
  public:
    Stream_Output( Game &, std::escape_ostream & );
    virtual void report_move( const Move_Sequence & );
  private:
    Game &game;
    std::escape_ostream &eos;
  };

  class Stream_Input : public Player_Input
  {
  public:
    Stream_Input( Game &, std::escape_istream & );
    virtual Player_State determine_move() throw(Exception);
    virtual Move_Sequence get_move();
  private:
    Game &game;
    std::escape_istream &eis;
    Move_Sequence sequence;
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

    Sequence_Generator( Game &, bool easy_multiple_knock = true );
    // synthesize move sequence from clicks
    Sequence_State add_click( Field_Pos pos ); 
    Sequence_State add_click_common_stone( Stones::Stone_Type stone_type ); 
    Sequence_State add_click_player_stone( int player_id, 
					   Stones::Stone_Type stone_type );
    Sequence_State undo_click(); // undo click 
    
    Move::Move_Type get_required_move_type();
    std::list<Field_Pos> get_possible_clicks();

    Field_Pos get_selected_pos();

    void reset();
    inline bool is_finished() 
    { return state == move_finished; }
    inline Move *get_move_done() // get move done by add_click (may return 0)
    { return move_done; }
    inline const Move_Sequence &get_sequence() const 
    { return sequence; }
  private:
    Move_Sequence sequence;
    Game &game;

    typedef enum Internal_State{ begin, move_from, move_dest, stone_picked, stone_set, move_finished };
    Internal_State state;

    Stones::Stone_Type picked_stone;
    Field_Pos from;
    std::stack<Field_Pos> to;	// destination positions of moves

    Move *move_done;

    bool easy_multiple_knock;	// true: don't have to click on stone again for following knock out moves
  };

  class Generic_Mouse_Input : public Player_Input
  {
  public:
    Generic_Mouse_Input( Game & );
    virtual Player_State determine_move() throw(Exception);
    virtual Move_Sequence get_move();

    virtual void init_mouse_input() = 0;
    virtual void disable_mouse_input() = 0;
  protected:
    Sequence_Generator sequence_generator;

    Game &game;
  };
}

#endif
