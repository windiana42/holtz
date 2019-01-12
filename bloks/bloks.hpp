/*
 * bloks.cpp
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

#ifndef __BLOKS_MAIN__
#define __BLOKS_MAIN__

#include <string>
#include <list>
#include <vector>
#include <map>
#include <set>
#include <stack>
#include <iostream>

#include "util.hpp"

namespace bloks
{
  class Stones;
  class Player;
  class Player_Input;
  class Player_Output;
  class Win_Condition;
  class Field_Iterator;
  class Board;
  class Move;
  class Jump_Move;
  class Set_Move;
  class Finish_Move;
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
  class Standard_Move_Translator;
  
  enum Field_State_Type{ field_removed=-1, field_empty=0, 
			 field_player1=1, field_player2=2, field_player3=3, field_player4=4, field_player_end=5 };

  const int standard_board[][20] = 
    { {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } };

  const int small_board_board[][14] =
    { {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 } };

  const int micro_board[][6] =
    { {  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0 },
      {  0,  0,  0,  0,  0,  0 } };
}

namespace bloks
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
  

  struct Field_Pos
  {
    int x;
    int y;
    Field_Pos();
    Field_Pos( int x, int y );
  };
  
  bool operator==( const Field_Pos &p1, const Field_Pos &p2 );
  bool operator!=( const Field_Pos &p1, const Field_Pos &p2 );
  bool operator<( const Field_Pos &p1, const Field_Pos &p2 );

  class Field_Iterator 
  {
  public:
    enum Direction{ invalid_direction=-1, 
		    right=0, top=1, left=2, bottom=3, 
		    right_flipped=4, top_flipped=5, left_flipped=6, bottom_flipped=7 }; // in case a stone is placed in this direction, it is flipped upside down 

    // counting numbers turns in math. positive direction

    Field_Iterator( const Board * );
    Field_Iterator( Field_Pos, const Board * );
    void set_pos( Field_Pos );
    inline Field_Pos get_pos() const { return current_pos; }
    static bool is_connected( Field_Pos p1, Field_Pos p2 );
    static Direction get_direction( Field_Pos from, Field_Pos to );
    static Direction get_far_direction( Field_Pos from, Field_Pos to );
    static Field_Pos get_next( Field_Pos p1, Direction dir );

    // get information about field pointing to
    bool is_corner() const;

    // the following functions may move out of the board area => check validity
    Field_Iterator Next_Left() const;
    Field_Iterator Next_Right() const;
    Field_Iterator Next_Top() const;
    Field_Iterator Next_Bottom() const;
    Field_Iterator Next( Direction ) const;

    Field_Iterator &Left();
    Field_Iterator &Right();
    Field_Iterator &Top();
    Field_Iterator &Bottom();
    Field_Iterator &Go( Direction );

    bool is_valid_field() const;	// tells whether field is in range
    Field_State_Type operator*() const; // don't call with invalid field
  private:
    Field_Pos current_pos;
    const Board *board;
  };
  
  inline bool operator==( const Field_Iterator &p1, const Field_Iterator &p2 )
  { return p1.get_pos() == p2.get_pos(); }
  inline bool operator!=( const Field_Iterator &p1, const Field_Iterator &p2 )
  { return p1.get_pos() != p2.get_pos(); }

  class Stone_Type 
  {
  public:
    enum Query_Type {CENTER, TOP_LEFT, BOTTOM_RIGHT}; // look for existing sub_fields
    Stone_Type() : ID(-1) {}
    Stone_Type(int ID, const int *sub_fields_array, unsigned width, unsigned height);
    int get_ID() const { return ID; }
    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > &get_sub_fields() const { return sub_fields; }
    const std::list<std::pair<int/*x*/,int/*y*/> > &get_corners() const { return corners; }
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > get_sub_fields(Field_Iterator::Direction dir) const;
    std::list<std::pair<int/*x*/,int/*y*/> > get_corners(Field_Iterator::Direction dir) const;
    std::pair<unsigned/*x*/,unsigned/*y*/> query_sub_field
    (Query_Type type, Field_Iterator::Direction dir) const
    { return rotate(sub_field_queries.find(type)->second,dir); }
    std::pair<int/*dx*/,int/*dy*/> query_sub_field_diff
    (Query_Type type1, Query_Type type2, Field_Iterator::Direction dir) const;
    unsigned get_max_x() const { return max_x; }
    unsigned get_max_y() const { return max_y; }
    unsigned get_max_x(Field_Iterator::Direction dir) const;
    unsigned get_max_y(Field_Iterator::Direction dir) const;
    std::pair<Field_Iterator::Direction, Field_Pos/*origin*/> 
    get_orientation(Field_Pos top_left, Field_Pos bottom_right, bool is_flipped) const;
    std::pair<Field_Pos /*top_left*/, Field_Pos /*bottom_right*/>
    get_click_points(Field_Iterator::Direction dir, Field_Pos origin) const;
    std::pair<unsigned/*x*/,unsigned/*y*/> rotate
    (std::pair<unsigned/*x*/,unsigned/*y*/>, Field_Iterator::Direction dir) const;
    bool get_rotation_symmetric() const { return rotation_symmetric; }
    bool get_flip_symmetric() const { return flip_symmetric; }
  private:
    int ID; // must be unique and start with 1,2,... (requirement of wxbloks.cpp)
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields; // sub fields which belog to stone (relative to 0/0)
    std::list<std::pair<int/*x*/,int/*y*/> > corners; // courner fields with only diagonally-adjacent sub_fields (relative to 0/0)
    std::map<Query_Type,std::pair<unsigned/*x*/,unsigned/*y*/> > sub_field_queries;
    unsigned max_x;
    unsigned max_y;
    bool rotation_symmetric;
    bool flip_symmetric;
  };

  std::list<Stone_Type> get_standard_stone_types();

  inline bool operator==(Stone_Type s1, Stone_Type s2) { return s1.get_ID() == s2.get_ID(); }
  inline bool operator<(Stone_Type s1, Stone_Type s2) { return s1.get_ID() < s2.get_ID(); }
  inline bool operator>(Stone_Type s1, Stone_Type s2) { return s1.get_ID() > s2.get_ID(); }
  inline bool operator<=(Stone_Type s1, Stone_Type s2) { return s1.get_ID() <= s2.get_ID(); }
  inline bool operator>=(Stone_Type s1, Stone_Type s2) { return s1.get_ID() >= s2.get_ID(); }

  class Stones
  {
  public:
    Stones();
    int get_total_stone_count() const { return total_count; }
    int get_stone_count(int stone_ID) const;
    void set_stone_count(int stone_ID, int count) 
    { if(does_include(stone_counts,stone_ID)) total_count -= stone_counts[stone_ID]; 
      stone_counts[stone_ID] = count; total_count += count; }
    void dec_stone_count(int stone_ID) 
    { assert(does_include(stone_counts,stone_ID)); --stone_counts[stone_ID]; --total_count; }
    void inc_stone_count(int stone_ID) 
    { assert(does_include(stone_counts,stone_ID)); ++stone_counts[stone_ID]; ++total_count; }

    const std::map<int/*stone_ID*/, int/*count*/>& get_stone_counts() const { return stone_counts; }

#ifndef __WXMSW__
    void print();
#endif

    void remove_stones();
  protected:
    std::map<int/*stone_ID*/, int/*count*/> stone_counts;
    int total_count;
  };

  // common stones just serves as initial stones of each player (this is different for zertz)
  class Common_Stones : public Stones
  {
  public:
    enum Common_Stones_Type{ standard=0, custom=99 };

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

  class Micro_Common_Stones : public Common_Stones
  {
  public:
    Micro_Common_Stones();
  };

  class Custom_Common_Stones : public Standard_Common_Stones // TODO: make real custom stones
  {
  public:
  };

  class Player_Input
  {
  public:
    enum Player_State{ finished=0, wait_for_event, request_undo,
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
    enum Player_Type{ unknown=0, user, ai };
    enum Help_Mode{ no_help=0, show_possible_moves, show_hint };
    enum Origin{ local, remote }; // origin where player is controled

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
    Field_State_Type field_type; // field type is set by game when player is added

    Player_Input *input;
    std::list<Player_Output*> outputs; 
    bool is_active;

    static std::list<Player_Output*> no_output;
  };

  class Board
  {
  public:
    enum Board_Type{ standard=0, small_board=1, custom=99 };
    // class Corner_Info {
    // public:
    // };
    // class Player_Info {
    // public:
    //   std::list<std::pair<Field_Pos,Corner_Info> > active_corners;
    //   std::list<std::pair<Field_Pos,Corner_Info> > inactive_corners;
    // };

    Board( const int *field_array, int width, int height,
	   const std::list<Stone_Type> &stone_types, Board_Type type = custom );
				// field_array must be rectangular array!
    Board( const std::vector< std::vector< Field_State_Type > > fields,
	   const std::list<Stone_Type> &stone_types, 
	   Board_Type type = custom );
    Board();			// should only be used for structures 

    Board_Type board_type;
    inline Board_Type get_type() const { return board_type; }

    // stone type information
    std::list<Stone_Type> stone_types; // constant after initialization
    Stone_Type invalid_stone_type;
    std::map<int/*ID*/,Stone_Type> id_stone_types; // constant after initialization
    const Stone_Type &get_stone_type(int stone_type_ID) const;
    bool is_valid_stone_type(int stone_type_ID) const;

    // field query functions
    static inline bool is_stone( Field_State_Type state )
    { return state > 0; }
    static inline bool is_empty( Field_State_Type state )
    { return state == field_empty; }
    static inline bool is_removed( Field_State_Type state )
    { return state == field_removed; }
    Field_Iterator first_field();

    // field information
    std::vector< std::vector<Field_State_Type> > field; // field[x][y]
    inline Field_State_Type get_field( Field_Pos pos ) { return field[pos.x][pos.y]; }
    inline int get_x_size() const { return field.size(); }
    inline int get_y_size() const { return field[0].size(); }

    // TODO:
    // // cache information to allow fast possible move calculation (cache get_free_corners)
    // void corner_cache_undo( const Player &player ); // this is called when a move is undone
    // void corner_cache_new_corner( const Player &player, Field_Pos pos );

    // information functions
    std::list<Field_Pos> get_free_corners( const Player &player ) const;
    std::map<Field_State_Type,int> count_fields() const;

    // debug functions
    void print();
  private:
    // TODO:
    // // cache information to allow fast possible move calculation (cache get_free_corners)
    // mutable std::map<int/*player.id*/,Player_Info> player_info;
  };

  class Move
  {
  public:
    enum Move_Type { no_move=0, set_move, finish_move };

    virtual Move_Type get_type() const = 0;
    virtual void do_move( Game & ) = 0;	         // may store extra info about the move
    virtual void undo_move( Game & ) = 0;        // mustn't destroy stored extra information
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

  class Set_Move : public Move
  {
  public:
    enum Rule_Violation { NO_VIOLATION, STONE_NOT_AVAILABLE, OUTSIDE, OVERLAP, 
			  FIRST_NO_CORNER, OWN_ADJACENT, OWN_NO_DIAGONAL };

    virtual Move_Type get_type() const;
    virtual void do_move( Game & );
    virtual void undo_move( Game & );
    virtual bool check_move( Game & ) const; // true: move ok
    virtual bool check_previous_move( Game &, Move * ) const; // true: type ok
    virtual bool may_be_first_move( Game & ) const;
    virtual bool may_be_last_move( Game & ) const;
    virtual std::escape_ostream &output( std::escape_ostream & ) const;
    virtual bool input( std::escape_istream & );

    void do_move( Game &, bool no_update_player );
    void undo_move( Game &, bool no_update_player );

    virtual Move *clone() const;

    Set_Move();
    Set_Move( Field_Pos pos, int stone_type_ID, Field_Iterator::Direction dir );

    Rule_Violation get_check_fail_reason() const { return check_fail_reason; }
  //private:
    Field_Pos pos;
    int stone_type_ID; 
    Field_Iterator::Direction dir;
    mutable Rule_Violation check_fail_reason; // can be changed by const functions
  };

  inline std::string to_string( Set_Move::Rule_Violation violation )
  { 
    switch(violation)
    {
    case Set_Move::NO_VIOLATION: return "NO_VIOLATION";
    case Set_Move::STONE_NOT_AVAILABLE: return "STONE_NOT_AVAILABLE";
    case Set_Move::OUTSIDE: return "OUTSIDE";
    case Set_Move::OVERLAP: return "OVERLAP";
    case Set_Move::FIRST_NO_CORNER: return "FIRST_NO_CORNER";
    case Set_Move::OWN_ADJACENT: return "OWN_ADJACENT";
    case Set_Move::OWN_NO_DIAGONAL: return "OWN_NO_DIAGONAL"; 
    }
    return "<unknown>";
  }

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
    //stored for undo:
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

    // this function takes care of the reserved memory
    void add_move( Move * );		// add move unchecked
    // this function takes care of the reserved memory
    bool add_move( Game&, Move * );	// true: adding move ok (only relative check)
    Move *get_last_move();
    void undo_last_move( Game & );	// calls undo for last move and removes it
    void clear();

    Move_Sequence clone() const;	// to store sequences, they must be cloned

    inline const std::list<Move*> &get_moves() const { return *moves; }
    inline bool is_empty() { return moves->empty(); }
    // for debug purposes
    const Ref_Counter* get_ref_counter() const { return ref_counter; }
  private:
    void modify_moves();	// called before <moves> is modified
    std::list<Move*> *moves;

    Ref_Counter *ref_counter;
  };

  inline std::escape_ostream &operator<<( std::escape_ostream &eos, const Move_Sequence &s )
  { s.output(eos); return eos; }
  inline std::escape_istream &operator>>( std::escape_istream &eis, Move_Sequence &s )
  { s.input(eis); return eis; }

  inline std::ostream &operator<<( std::ostream &os, const Move_Sequence &s )
  { std::escape_ostream eos(os); s.output(eos); return os; }
  inline std::ostream &operator<<( std::ostream &os, const Move &m )
  { std::escape_ostream eos(os); m.output(eos); return os; }
  inline std::istream &operator>>( std::istream &is, Move_Sequence &s )
  { std::escape_istream eis(is); s.input(eis); return is; }

  bool operator==( const Move &m1, const Move &m2 );
  bool operator<( const Move &m1, const Move &m2 );
  bool operator==( const Set_Move &m1, const Set_Move &m2 );
  bool operator<( const Set_Move &m1, const Set_Move &m2 );
  inline bool operator==( const Finish_Move &, const Finish_Move & ) { return true; }
  inline bool operator<( const Finish_Move &, const Finish_Move & ) { return false; }
  bool operator==( const Move_Sequence &s1, const Move_Sequence &s2 );
  bool operator<( const Move_Sequence &s1, const Move_Sequence &s2 );

  // with this class a variant tree could be built that stores different
  // move possibilities
  class Variant
  {
  public:
    Variant( int current_player_index, const Move_Sequence &, 
	     unsigned possible_variants, unsigned id, Variant *prev = 0 );
    ~Variant();

    Variant *add_variant( int current_player_index,  
			  const Move_Sequence &, unsigned possible_variants,
			  unsigned id );
    bool is_prev( Variant * ) const;	// checks all previous variants
    Variant *clone( Variant *search = 0, Variant *&clone = default_clone ) const;
				// clone variant with all subvariants
				// optional: search for variant and store 
				// clone adresse of it in clone

    int current_player_index;	// player of the move sequence (0=first player,1=second player,...)
    Move_Sequence move_sequence;
    unsigned possible_variants;	// number of possible variants in the situation after this move 
    unsigned id;		// id to identify variant through multiple copies of variant_tree

    Variant *prev;
    std::list<Variant*> variants;
    std::map<unsigned,Variant*> id_variant_map;
  private:
    friend class Variant_Tree;
    Variant( unsigned id );	// for root variant

    static Variant *default_clone; // just as unused default argument in clone method
  };

  // encapsulates a variant tree
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
    std::list<std::pair<Move_Sequence,int/*player index*/> > get_current_variant_moves() const;
    std::list<unsigned> get_variant_id_path( const Variant *dest_variant ) const;
    const Variant *get_variant( const std::list<unsigned> variant_id_path ) const;
    Variant *get_variant( const std::list<unsigned> variant_id_path );
  public:
    friend class Game;
    void add_in_current_variant( int current_player_index, const Move_Sequence &, 
				 unsigned possible_variants );
    void move_a_variant_back();
    void clear();
    void set_unique_id(unsigned id) { unique_id = id; }
    unsigned get_unique_id() { return unique_id; }
  private:
    Variant *root;
    Variant *current_variant;
    unsigned unique_id;
  };

  class Win_Condition
  {
  public:
    enum Win_Condition_Type{ standard=0, full_custom=99 };

    Win_Condition( Win_Condition_Type type = full_custom );
    virtual ~Win_Condition();

    virtual int/*player index*/ get_winner( Game & ) const = 0;
    virtual float/*player rank[1..n]*/ get_player_rank( Game &, Player & ) const = 0;
    //virtual bool did_player_win( Game &, Player & ) const = 0; // not used in bloks
    virtual Win_Condition *clone() = 0;

    Win_Condition_Type get_type() const { return type; }
  protected:
    Win_Condition_Type type; // is generic win condition 
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
    enum Ruleset_Type { standard=0, small_board=1, custom=99 };
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
    enum Game_State{ finished=0, finished_scores, wait_for_event, next_players_turn, interruption_possible, 
		     wrong_number_of_players };

    Game( const Ruleset & );
    Game( const Game & );
    Game &operator=( const Game & );
    ~Game();

    // **********************
    // game control functions
    Game_State continue_game() throw(Exception); // start or continue game
    void stop_game(); // stop game to allow changing the position with do_move and undo_move
    int get_winner_index() { return winner_player_index; } // returns -1 if no player won
    void reset_game();		// doesn't reset players
    void reset_game( const Ruleset & );	// doesn't reset players
    std::multimap<int/*score*/,const Player*> get_scores() const;

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
    inline bool is_out_of_order() { return false; }
    inline unsigned get_min_players() { return ruleset->min_players; }
    inline unsigned get_max_players() { return ruleset->max_players; }

    bool is_any_move_possible(); // is any move possible by anyone
    bool is_any_move_possible( const Player &player ) const; // is any move possible
    std::list<Set_Move> get_possible_moves
    ( const Player &player,
      bool only_one_flip_direction=false, bool is_flipped=false,  // is_flipped is only used for only_one_flip_direction==true
      bool just_one=false, bool just_one_per_stone=false, int just_stone_ID=-1,
      bool just_one_rotation_symmetric=true, bool random_order=false ) const;
    int get_num_possible_moves(); // number of possible moves in current situation
    //std::list<Move_Sequence> get_possible_moves(); // get possible moves in situation
    std::list<std::pair<Move_Sequence,int/*player index*/> > get_played_moves();   // get moves played since start
    
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
    std::vector<Player>::iterator get_player_by_id( int id );
    std::vector<Player>::const_iterator get_player_by_id( int id ) const;
    // for out-of-order-games:
    void set_current_player_index(int) { assert(false); }
    void undo_set_current_player_index() {}
  public:
    // use this functions only of you use Move::do_move before
    // use Game::do_move otherwise
    bool choose_next_player();		// next player is current player (players may be skiped)
				// call next_player after Move_Sequence::do_move
    bool choose_prev_player( int prev_prev_player_index );		
				// prev player is current player prev_prev_player can't be determined
				// automatically
    // AI rating uses these functions
    std::list<int/*prev_player_index*/> AI_set_current_player( Player &player );
    void AI_unset_current_player( const std::list<int> &prev_player_indices );
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
    virtual Move_Sequence decode( std::istream& ) = 0;
    virtual ~Move_Translator() {}
  };

  class Standard_Move_Translator : public Move_Translator
  {
  public:
    Standard_Move_Translator( Coordinate_Translator * );

    virtual std::string encode( Move_Sequence );
    virtual Move_Sequence decode( std::istream& );
  private:
    Coordinate_Translator *coordinate_translator;
  };
  
  class Standard_Win_Condition : public Win_Condition
  {
  public:
    virtual int/*player index*/ get_winner( Game & ) const;
    virtual float/*player rank[1..n]*/ get_player_rank( Game &, Player & ) const;
    //virtual bool did_player_win( Game &, Player & ) const { return false; /*not used in bloks*/ }

    virtual Win_Condition *clone() { return new Standard_Win_Condition(); }

    Standard_Win_Condition();
  };

  class Custom_Ruleset : public Ruleset
  {
  public:
    // win_condition and coordinate translator will be deleted by Ruleset!
    Custom_Ruleset( Board, Common_Stones, Win_Condition *, Coordinate_Translator *, 
		    bool undo_possible = true, 
		    unsigned min_players = 2, unsigned max_players = 2 );
  };

  class Standard_Ruleset : public Ruleset
  {
  public:
    Standard_Ruleset();
  };

  class Small_Board_Ruleset : public Ruleset
  {
  public:
    Small_Board_Ruleset();
  };

  class Micro_Ruleset : public Ruleset
  {
  public:
    Micro_Ruleset();
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
    typedef Set_Move::Rule_Violation Rule_Violation;
    enum Sequence_State{ finished=0, hold1=1, hold2=2,
			 another_click=3,

			 error_require_set=-500, 
			 error_can_t_set_here, 
			 error_wrong_player, 
			 error_impossible_yet,
			 error_invalid_orientation, 
			 error_rule_violation, 

			 fatal_error=-1000 };

    Sequence_Generator( Game & );
    // synthesize move sequence from clicks
    Sequence_State add_click( Field_Pos pos, bool is_flipped ); 
    Sequence_State add_click_player_stone
    ( int player_id, int stone_type_ID, int stone_index, bool is_flipped );
    Sequence_State undo_click(); // undo click 
    
    Sequence_State get_sequence_state(); // only interprete hold1/hold2/other(another_click)
    Move::Move_Type get_required_move_type();
    std::list<Field_Pos> get_possible_clicks(bool is_flipped); // call if get_sequence_state() == hold1 || hold2
    std::list<std::pair<int/*playerID*/,int/*stoneID*/> > get_possible_stone_clicks(bool is_flipped); 
				// call if get_sequence_state() == another_click

    void reset();
    inline bool is_finished() 
    { return state == move_finished; }
    inline Move *get_move_done() // get move done by add_click (may return 0)
    { return move_done; }
    inline const Move_Sequence &get_sequence() const 
    { return sequence; }
    inline void set_sequence( const Move_Sequence &seq )
    { sequence = seq; auto_undo=false; state = move_finished; }

    // getters for intermediate state during sequence generation
    int get_picked_player_ID() const; 
    int get_picked_stone_type_ID() const;
    int get_picked_stone_index() const;
    Field_Pos get_set_pos() const;
    Rule_Violation get_rule_violation() const { return rule_violation; }
  private:
    Move_Sequence sequence;
    Game &game;
    bool auto_undo;

    enum Internal_State{ begin, stone_picked, stone_pinpointed, move_finished };
    Internal_State state;

    Field_Pos set_pos;
    int picked_player_ID;
    int picked_stone_type_ID;
    int picked_stone_index;
    Rule_Violation rule_violation;

    Move *move_done;
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

  std::string DEBUG_translate_move(const Move_Sequence& sequence);
}

#endif
