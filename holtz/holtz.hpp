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
  class Game;
  class Sequence;
  class Move;
  class Knock_Out_Move;
  class Set_Move;
  class Remove;
  class Standard_Win_Condition;
  class Standard_Ruleset;
  class No_Output;
  class Stream_Output;
  class Stream_Input;
  class AI_Input;
  class Sequence_Generator;
  class Generic_Mouse_Input;
  
  typedef enum Field_State_Type{ field_removed=-1, field_empty=0, 
				 field_white=1, field_gray=2, field_black=3 };

  const int standard_board[][7] =
    { {   -1,  0,  0,  0,  0, -1, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {    0,  0,  0,  0,  0,  0, -1 },
      {  0,  0,  0,  0,  0,  0,  0 },
      {    0,  0,  0,  0,  0,  0, -1 },
      { -1,  0,  0,  0,  0,  0, -1 },
      {   -1,  0,  0,  0,  0, -1, -1 } };
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
    typedef enum Stone_Type{ white_stone=1, gray_stone=2, black_stone=3 };
    std::map<Stone_Type, int> stone_count;

    void remove_stones();
  };

  class Player_Input
  {
  public:
    typedef enum Player_State{ finished, wait_for_event, request_undo,
			       interruption_possible };

    virtual Player_State determine_move() throw(Exception) = 0;
    virtual Sequence get_move() = 0;
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

    Player( std::string name, int id, Player_Input*, std::list<Player_Output*> = no_output,
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
    Board( const int *field_array, int width, int height );
    // field must be rectangular array!
    Board( const std::vector< std::vector<Field_State_Type> > fields );

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
    inline int get_x_size() { return field.size(); }
    inline int get_y_size() { return field[0].size(); }
  };

  class Ruleset
  {
  public:
    typedef enum type { standard_ruleset, tournament_ruleset, custom_ruleset };

    inline unsigned get_min_players() const { return min_players; }
    inline unsigned get_max_players() const { return max_players; }
  protected:
    Ruleset( Board, Win_Condition *, bool undo_possible, unsigned min_players, unsigned max_players );


    Board board;
    Stones common_stones;
    Win_Condition *win_condition;
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
    Game( Game & );
    Game &operator=( Game & );
    ~Game();

    Game_State continue_game() throw(Exception); // start or continue game
    Player *get_winner();	// returns 0 if no player won
    void reset_game();
    void reset_game( const Ruleset & );

    // init functions
    bool add_player( const Player & ); // true: enough players
    void remove_players();
    void next_player();		// next player is current player
    void prev_player();		// prev player is current player

    inline unsigned get_min_players() { return ruleset->min_players; }
    inline unsigned get_max_players() { return ruleset->max_players; }

    void copy_player_times( Game &from ); // number of players must be the same
  public:
    // public: for internal usage (friend might be used instead)
    std::list<Player> players;

    std::list<Player>::iterator current_player;

    Board board;
    Stones common_stones;
    Win_Condition *win_condition;
    bool undo_possible;
    Ruleset *ruleset;

    Game *save_game;		// shadows the game to avoid corruption by input handlers

    Ref_Counter *ref_counter;	// reference counting because of ruleset pointer
  private:
    void remove_filled_areas();
  };

  class Win_Condition
  {
  public:
    virtual bool did_player_win( Game &, Player & ) const = 0;
    virtual ~Win_Condition();
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

    virtual Move *clone() = 0;

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

    virtual Move *clone();

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

    virtual Move *clone();

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

    virtual Move *clone();

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

    virtual Move *clone();

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

    bool add_move( Game&, Move * );	// true: adding move ok
    Move *get_last_move();
    void undo_last_move( Game & );	// calls undo for last move and removes it
    void clear();

    Sequence clone();		// to store sequences, they must be cloned

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
  
  class Standard_Win_Condition : public Win_Condition
  {
  public:
    virtual bool did_player_win( Game &, Player & ) const;
  };

  class Tournament_Win_Condition : public Win_Condition
  {
  public:
    virtual bool did_player_win( Game &, Player & ) const;
  };

  class Standard_Ruleset : public Ruleset
  {
  public:
    Standard_Ruleset();
  private:
    static Standard_Win_Condition standard_win_condition;
  };

  class Tournament_Ruleset : public Ruleset
  {
  public:
    Tournament_Ruleset();
  private:
    static Tournament_Win_Condition tournament_win_condition;
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
				 hold_prefix=0, hold_white=1, hold_gray=2, hold_black=3,
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
