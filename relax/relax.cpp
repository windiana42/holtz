/*
 * relax.cpp
 * 
 * Game implementation
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

#include "relax.hpp"
#include <assert.h>

#include "util.hpp"

#include <ctype.h>
#include <sys/types.h>
#include <algorithm>

namespace relax
{
  using namespace holtz;

  // ----------------------------------------------------------------------------
  // Stones
  // ----------------------------------------------------------------------------

  Stones::Stones()
    : current_stone(invalid_stone)
  {
  }

#ifndef __WXMSW__
  void Stones::print()
  {
  }
#endif

  // ----------------------------------------------------------------------------
  // Common_Stones
  // ----------------------------------------------------------------------------

  Common_Stones::Common_Stones( Common_Stones::Common_Stones_Type type )
    : type(type)
  {
  }

  // ----------------------------------------------------------------------------
  // Player_Input
  // ----------------------------------------------------------------------------

  Player_Input::~Player_Input()
  {
  }

  // ----------------------------------------------------------------------------
  // Player_Output
  // ----------------------------------------------------------------------------

  Player_Output::~Player_Output()
  {
  }

  // ----------------------------------------------------------------------------
  // Player
  // ----------------------------------------------------------------------------

  std::list<Player_Output*> Player::no_output;

  Player::Player( std::string name, int id, Player_Input *input, 
		  std::list<Player_Output*> outputs,
		  std::string host, Player_Type type, Help_Mode help_mode,
		  Origin origin )
    : name(name), id(id), host(host), type(type), help_mode(help_mode), 
      origin(origin), total_time(0), average_time(-1), num_measures(0), 
      input(input), outputs(outputs), is_active(false), already_done(false)
  {
  }

  // ----------------------------------------------------------------------------
  // Win_Condition
  // ----------------------------------------------------------------------------

  Win_Condition::Win_Condition( Win_Condition_Type type )
    : type(type)
  {
  }
  Win_Condition::~Win_Condition()
  {
  }

  // ----------------------------------------------------------------------------
  // Field_Pos
  // ----------------------------------------------------------------------------

  Field_Pos::Field_Pos()
    : x(-1), y(-1)
  {
  }

  Field_Pos::Field_Pos( int x, int y )
    : x(x), y(y)
  {
  }

  bool operator==( const Field_Pos &p1, const Field_Pos &p2 )
  {
    return ( p1.x == p2.x ) && ( p1.y == p2.y );
  }

  bool operator!=( const Field_Pos &p1, const Field_Pos &p2 )
  {
    return ( p1.x != p2.x ) || ( p1.y != p2.y );
  }

  Field_Pos Field_Pos::get_middle( Field_Pos p1, Field_Pos p2 )
  {
    Field_Iterator::Direction dir = Field_Iterator::get_far_direction(p1,p2);
    Field_Pos middle = Field_Iterator::get_next(p1,dir);
    if( Field_Iterator::get_next(middle,dir) != p2 )
      return Field_Pos();

    return middle;
  }

  // ----------------------------------------------------------------------------
  // Field_Iterator
  // ----------------------------------------------------------------------------

  Field_Iterator::Field_Iterator( const Board *board )
    : board(board)
  {
    current_pos.x = 0;
    current_pos.y = 0;
  }

  Field_Iterator::Field_Iterator( Field_Pos pos, const Board *board )
    : current_pos(pos), board(board)
  {
  }

  void Field_Iterator::set_pos( Field_Pos pos )
  {
    current_pos = pos;
  }

  bool Field_Iterator::is_connected( Field_Pos p1, Field_Pos p2 )
  {
    if( abs(p1.x-p2.x) > 1 ) return false;
    if( abs(p1.y-p2.y) > 1 ) return false;

    if( (p1.x-p2.x ==  0) && (p1.y-p2.y ==  0) ) return false;
    if( (p1.x-p2.x == -1) && (p1.y-p2.y !=  0) &&  (p1.y & 1) ) return false;
    if( (p1.x-p2.x ==  1) && (p1.y-p2.y !=  0) && !(p1.y & 1) ) return false;

    return true;
  }
  Field_Iterator::Direction Field_Iterator::get_direction( Field_Pos from, Field_Pos to )
  {
    if( from.y & 1 )		// if from row is odd
    {
      switch( from.y - to.y )
      {
	case  1:
	  switch( from.x - to.x )
	  {
	    case  1: return Field_Iterator::top_left;
	    case  0: return Field_Iterator::top_right;
	    default: return Field_Iterator::invalid_direction;
	  }
	  break;
	case  0:
	  switch( from.x - to.x )
	  {
	    case  1: return Field_Iterator::left; break;
	    default: return Field_Iterator::invalid_direction;
	    case -1: return Field_Iterator::right; break;
	  }
	  break;
	case -1:
	  switch( from.x - to.x )
	  {
	    case  1: return Field_Iterator::bottom_left; break;
	    case  0: return Field_Iterator::bottom_right; break;
	    default: return Field_Iterator::invalid_direction;
	  }
	  break;
	default: return Field_Iterator::invalid_direction;
      }
    }
    else			// from has even row
    {
      switch( from.y - to.y )
      {
	case  1:
	  switch( from.x - to.x )
	  {
	    default: return Field_Iterator::invalid_direction;
	    case  0: return Field_Iterator::top_left;
	    case -1: return Field_Iterator::top_right;
	  }
	  break;
	case  0:
	  switch( from.x - to.x )
	  {
	    case  1: return Field_Iterator::left; break;
	    default: return Field_Iterator::invalid_direction;
	    case -1: return Field_Iterator::right; break;
	  }
	  break;
	case -1:
	  switch( from.x - to.x )
	  {
	    default: return Field_Iterator::invalid_direction;
	    case  0: return Field_Iterator::bottom_left; break;
	    case -1: return Field_Iterator::bottom_right; break;
	  }
	  break;
	default: return Field_Iterator::invalid_direction;
      }
    }
  }
  
  Field_Iterator::Direction Field_Iterator::get_far_direction( Field_Pos from, Field_Pos to )
  {
    if( from.y & 1 )		// if from row is odd
    {
      if( from.y - to.y > 0 )
      {
	if( from.x - to.x > 0 )
	  return Field_Iterator::top_left;
	else
	  return Field_Iterator::top_right;
      }
      else if( from.y - to.y == 0 )
      {
	if( from.x - to.x > 0 )
	  return Field_Iterator::left;
	else
	  return Field_Iterator::right;
      }
      else
      {
	if( from.x - to.x > 0 )
	  return Field_Iterator::bottom_left;
	else
	  return Field_Iterator::bottom_right;
      }
    }
    else			// from has even row
    {
      if( from.y - to.y > 0 )
      {
	if( from.x - to.x >= 0 )
	  return Field_Iterator::top_left;
	else
	  return Field_Iterator::top_right;
      }
      else if( from.y - to.y == 0 )
      {
	if( from.x - to.x > 0 )
	  return Field_Iterator::left;
	else
	  return Field_Iterator::right;
      }
      else
      {
	if( from.x - to.x >= 0 )
	  return Field_Iterator::bottom_left;
	else
	  return Field_Iterator::bottom_right;
      }
    }
  }

  Field_Pos Field_Iterator::get_next( Field_Pos p1, Direction dir )
  {
    Field_Iterator it(p1, 0);
    it.Go(dir);
    return it.get_pos();
  }
  
  Field_Iterator Field_Iterator::Next_Left() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Left();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Right() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Right();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Top_Left() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Top_Left();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Top_Right() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Top_Right();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Bottom_Left() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Bottom_Left();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Bottom_Right() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Bottom_Right();
    return ret;
  }
  Field_Iterator Field_Iterator::Next( Direction dir ) const
  {
    switch( dir )
    {
      case right:		return Next_Right(); break;
      case top_right:		return Next_Top_Right(); break;
      case top_left:		return Next_Top_Left(); break;
      case left:		return Next_Left(); break;
      case bottom_left:		return Next_Bottom_Left(); break;
      case bottom_right:	return Next_Bottom_Right(); break;
      case right2:		return Next_Right(); break;
      case top_right2:		return Next_Top_Right(); break;
      case top_left2:		return Next_Top_Left(); break;
      case left2:		return Next_Left(); break;
      case bottom_left2:	return Next_Bottom_Left(); break;
      case bottom_right2:	return Next_Bottom_Right(); break;
      case invalid_direction:    assert( false );
    }
    assert( false );
    return *this;
  }
  
  Field_Iterator &Field_Iterator::Left()
  {
    --current_pos.x;
    return *this;
  }
  Field_Iterator &Field_Iterator::Right()
  {
    ++current_pos.x;
    return *this;
  }
  Field_Iterator &Field_Iterator::Top_Left()
  {
    if( current_pos.y & 1 ) // odd row number?
    {
      --current_pos.x;
    }
    else // even row number
    {
    }
    --current_pos.y;

    return *this;
  }
  Field_Iterator &Field_Iterator::Top_Right()
  {
    if( current_pos.y & 1 ) // odd row number?
    {
    }
    else // even row number
    {
      ++current_pos.x;
    }
    --current_pos.y;

    return *this;
  }
  Field_Iterator &Field_Iterator::Bottom_Left()
  {
    if( current_pos.y & 1 ) // odd row number?
    {
      --current_pos.x;
    }
    else // even row number
    {
    }
    ++current_pos.y;

    return *this;
  }
  Field_Iterator &Field_Iterator::Bottom_Right()
  {
    if( current_pos.y & 1 ) // odd row number?
    {
    }
    else // even row number
    {
      ++current_pos.x;
    }
    ++current_pos.y;

    return *this;
  }
  Field_Iterator &Field_Iterator::Go( Direction dir ) 
  {
    switch( dir )
    {
      case right:		return Right(); break;
      case top_right:		return Top_Right(); break;
      case top_left:		return Top_Left(); break;
      case left:		return Left(); break;
      case bottom_left:		return Bottom_Left(); break;
      case bottom_right:	return Bottom_Right(); break;
      case right2:		return Right(); break;
      case top_right2:		return Top_Right(); break;
      case top_left2:		return Top_Left(); break;
      case left2:		return Left(); break;
      case bottom_left2:	return Bottom_Left(); break;
      case bottom_right2:	return Bottom_Right(); break;
      case invalid_direction:    assert( false );
    }
    assert( false );
    return *this;
  }

  //! tells whether field is in range
  bool Field_Iterator::is_valid_field() const
  {
    if( board == 0 ) return false;
    if( current_pos.x < 0 ) return false;
    if( current_pos.y < 0 ) return false;
    if( current_pos.x >= board->get_x_size() ) return false;
    if( current_pos.y >= board->get_y_size() ) return false;

    return true;
  }

  Field_State_Type Field_Iterator::operator*() const
  {
    assert( is_valid_field() );

    return board->field[ current_pos.x ][ current_pos.y ];
  }

  Field_State_Type &Mutable_Field_Iterator::operator*()
  {
    assert( is_valid_field() );

    return mutable_board->field[ current_pos.x ][ current_pos.y ];
  }

  // ----------------------------------------------------------------------------
  // Board
  // ----------------------------------------------------------------------------

  Board::Board( const int *field_array, int width, int height, Board_Type board_type )
    : board_type( board_type )
  {
    field.resize( width );
    for( int x = 0; x < width; ++x )
    {
      field[x].resize( height );
      for( int y = 0; y < height; ++y )
      {
	field[x][y] = Field_State_Type( field_array[x + y*width] );
      }
    }
  }

  Board::Board( const std::vector< std::vector<Field_State_Type> > field, Board_Type board_type )
    : board_type(board_type), field(field)
  {
    // !!! checks for rectangularity
  }

  Board::Board()
  {
  }

  std::list<Field_Pos> Board::get_empty_fields()
  {
    std::list<Field_Pos> ret;
    Field_Pos pos;
	
    for( pos.x = 0; pos.x < get_x_size(); ++pos.x )
    {
      for( pos.y = 0; pos.y < get_y_size(); ++pos.y )
      {
	if( is_empty( *Field_Iterator(pos, this) ) )
	  ret.push_back(pos);
      }
    }
    return ret;
  }

  Field_Iterator Board::first_field()
  {
    Field_Pos pos( 0, 0 ); 
    return Field_Iterator( pos, this );
  }

  Field_State_Type Board::get_field( unsigned x, unsigned y )
  { 
    return field[x][y]; 
  }

  // ----------------------------------------------------------------------------
  // Ruleset
  // ----------------------------------------------------------------------------

  Ruleset::Ruleset( Ruleset_Type type, Board board, Common_Stones common_stones, 
		    Win_Condition *win_condition, Coordinate_Translator *coordinate_translator,
		    bool undo_possible, unsigned min_players, unsigned max_players ) 
    : type(type), board(board), common_stones(common_stones), win_condition(win_condition), 
      coordinate_translator(coordinate_translator),
      undo_possible(undo_possible), min_players(min_players), max_players(max_players)
  {
  }
  Ruleset::Ruleset( const Ruleset &ruleset )
    : type(ruleset.type), board(ruleset.board), common_stones(ruleset.common_stones),
      win_condition(ruleset.win_condition->clone()), 
      coordinate_translator(ruleset.coordinate_translator->clone()),
      undo_possible(ruleset.undo_possible),
      min_players(ruleset.min_players), max_players( ruleset.max_players )
  {
    nums1		  = ruleset.nums1;
    nums2		  = ruleset.nums2;
    nums3		  = ruleset.nums3;
    max_num1		  = ruleset.max_num1;
    max_num2		  = ruleset.max_num2;
    max_num3		  = ruleset.max_num3;
  }

  Ruleset::Ruleset()
    : type(custom), win_condition(0), coordinate_translator(0)
  {
  }
  Ruleset &Ruleset::operator=( const Ruleset &ruleset )
  {
    type		  = ruleset.type;
    board		  = ruleset.board;
    common_stones	  = ruleset.common_stones;
    win_condition	  = ruleset.win_condition->clone();
    coordinate_translator = ruleset.coordinate_translator->clone();
    undo_possible	  = ruleset.undo_possible;
    min_players		  = ruleset.min_players;
    max_players		  = ruleset.max_players;
    nums1		  = ruleset.nums1;
    nums2		  = ruleset.nums2;
    nums3		  = ruleset.nums3;
    max_num1		  = ruleset.max_num1;
    max_num2		  = ruleset.max_num2;
    max_num3		  = ruleset.max_num3;
    return *this;
  }

  Ruleset::~Ruleset()
  {
    delete win_condition;
    delete coordinate_translator;
  }

  Stones::Stone_Type Ruleset::get_stone( int n1, int n2, int n3 )
  {
    unsigned idx1=0,idx2=0,idx3=0;
    std::vector<int>::iterator it;
    for( it = nums1.begin(); it != nums1.end(); ++it )
      if( n1 == *it ) { idx1 = it - nums1.begin(); break; }
    for( it = nums2.begin(); it != nums2.end(); ++it )
      if( n2 == *it ) { idx2 = it - nums2.begin(); break; }
    for( it = nums3.begin(); it != nums3.end(); ++it )
      if( n3 == *it ) { idx3 = it - nums3.begin(); break; }
    /*
    assert( find(nums1.begin(),nums1.end(),n1) != nums1.end() );
    assert( find(nums2.begin(),nums2.end(),n2) != nums2.end() );
    assert( find(nums3.begin(),nums3.end(),n3) != nums3.end() );
    unsigned idx1 = find(nums1.begin(),nums1.end(),n1) - nums1.begin();
    unsigned idx2 = find(nums2.begin(),nums2.end(),n2) - nums2.begin();
    unsigned idx3 = find(nums3.begin(),nums3.end(),n3) - nums3.begin();
    */
    return Stones::Stone_Type
      ( Stones::stone_begin + idx1 + (idx2 + idx3 * nums2.size()) * nums1.size() );
  }

  boost::tuple<int,int,int> Ruleset::get_numbers( Stones::Stone_Type stone_type )
  {
    assert( stone_type >= Stones::stone_begin );
    assert( (int)stone_type < (int)(Stones::stone_begin + nums1.size()*nums2.size()*nums3.size()) );
    int idx = ((int)stone_type) - Stones::stone_begin;
    return boost::make_tuple(nums1[idx % nums1.size()], nums2[(idx / nums1.size()) % nums2.size()],
			     nums3[(idx / (nums1.size()*nums2.size())) % nums3.size()]);
  }

  boost::tuple<int,int,int> Ruleset::get_max_numbers()
  {
    return boost::make_tuple(max_num1,max_num2,max_num3);
  }

  // ----------------------------------------------------------------------------
  // Variant
  // ----------------------------------------------------------------------------
  
  Variant::Variant( int current_player_index, const Move_Sequence &move_sequence, 
		    unsigned possible_variants, unsigned id, Variant *prev )
    : current_player_index(current_player_index), move_sequence(move_sequence), 
      possible_variants(possible_variants), id(id), prev(prev)
  {
  }

  Variant::Variant( unsigned id )      // for root variant
    : possible_variants(unsigned(-1)), // don't know number of possible variants
      id(id), prev(0)
  {
  }

  Variant::~Variant()
  {
    std::list<Variant*>::iterator i;
    for( i = variants.begin(); i != variants.end(); ++i )
      delete *i;
    //variants.clear();
  }

  Variant *Variant::add_variant( int current_player_index, 
				 const Move_Sequence &move_sequence,
				 unsigned possible_variants, unsigned id )
  {
    // check whether variant already exists
    std::list<Variant*>::iterator it;
    for( it = variants.begin(); it != variants.end(); ++it )
    {
      Variant *variant = *it;
      if( variant->move_sequence == move_sequence )
      {
	return variant;
      }
    }
    Variant *variant = new Variant( current_player_index, move_sequence, possible_variants, id, 
				    this );
    variants.push_back( variant );
    id_variant_map[id] = variant;
    return variant;
  }

  bool Variant::is_prev( Variant *variant ) const
  {
    if( variant == 0 )
      return false;

    if( variant == this )
      return true;

    if( prev )
      return prev->is_prev( variant );
    else
      return false;
  }

  Variant *Variant::clone( Variant *search, Variant *&clone_dest ) const
  {
    Variant *variant_clone 
      = new Variant( current_player_index, move_sequence, possible_variants, id );
    
    std::list<Variant*>::const_iterator sub_variant;
    for( sub_variant = variants.begin(); sub_variant != variants.end(); ++sub_variant )
    {
      Variant *sub_variant_clone = (*sub_variant)->clone( search, clone_dest );
      sub_variant_clone->prev = variant_clone;
      variant_clone->variants.push_back( sub_variant_clone );
      variant_clone->id_variant_map[sub_variant_clone->id] = sub_variant_clone;
    }

    if( search )
    {
      if( this == search )	// if searched variant was found
	clone_dest = variant_clone;	// store clone of it
    }

    return variant_clone;
  }

  Variant *Variant::default_clone; // just as unused default argument in clone method

  // ----------------------------------------------------------------------------
  // Variant_Tree
  // ----------------------------------------------------------------------------

  Variant_Tree::Variant_Tree()
    : unique_id(1)
  {
    root = new Variant(unique_id++);
    current_variant = root;
  }

  Variant_Tree::Variant_Tree( const Variant_Tree &src )
    : root(0)
  {
    *this = src;		// use operator=
  }

  Variant_Tree &Variant_Tree::operator=( const Variant_Tree &src )
  {
    if( root )
      delete root;

    assert( src.current_variant->is_prev(src.root) );
    current_variant = 0;

    // clone variant tree and map current_variant to new pointers by the way
    root = src.root->clone( src.current_variant, current_variant );

    assert( current_variant != 0 );
    assert( current_variant->is_prev(root) );

    unique_id = src.unique_id;

    return *this;
  }

  Variant_Tree::~Variant_Tree()
  {
    delete root;
  }

  bool Variant_Tree::remove_subtree( Variant *variant )
  {
    if( !variant->is_prev( root ) ) // assure that variant is from this variant tree
      return false;

    if( variant == current_variant ) // that variant is not current variant
      return false;

    if( current_variant->is_prev( variant ) ) // that variant is not in current variant path
      return false;

    assert( variant->prev );

    if( does_include(variant->prev->id_variant_map,variant->id) )
      {
	variant->prev->id_variant_map.erase(variant->id);

	std::list<Variant*>::iterator i;
	for( i = variant->prev->variants.begin(); i != variant->prev->variants.end(); ++i )
	  {
	    if( *i == variant )
	      {
		variant->prev->variants.erase( i );
		delete variant;
		return true;
	      }
	  }
	assert(false);
      }
    
    return false;
  }

  std::list<std::pair<Move_Sequence,int/*player index*/> > 
  Variant_Tree::get_current_variant_moves() const
  {
    std::list<std::pair<Move_Sequence,int/*player index*/> > ret;
    const Variant *cur = get_current_variant();
    while( cur != get_root_variant() )
    {
      ret.push_front(std::make_pair(cur->move_sequence,cur->current_player_index));
      cur = cur->prev;
    }
    return ret;
  }

  std::list<unsigned> Variant_Tree::get_variant_id_path( const Variant *dest_variant ) const
  {
    std::list<unsigned> ret;
    const Variant *cur = dest_variant;
    while( cur != get_root_variant() )
    {
      ret.push_front(cur->id);
      cur = cur->prev;
    }
    return ret;
  }

  const Variant *Variant_Tree::get_variant( const std::list<unsigned> variant_id_path ) const
  {
    const Variant *cur = get_root_variant();
    for( std::list<unsigned>::const_iterator it=variant_id_path.begin(); 
	 it!=variant_id_path.end(); ++it )
      {
	unsigned id = *it;
	if( !does_include(cur->id_variant_map,id) ) return 0;
	cur = cur->id_variant_map.find(id)->second;
      }
    
    return cur;
  }

  Variant *Variant_Tree::get_variant( const std::list<unsigned> variant_id_path )
  {
    Variant *cur = get_root_variant();
    for( std::list<unsigned>::const_iterator it=variant_id_path.begin(); 
	 it!=variant_id_path.end(); ++it )
      {
	unsigned id = *it;
	if( !does_include(cur->id_variant_map,id) ) return 0;
	cur = cur->id_variant_map.find(id)->second;
      }
    
    return cur;
  }

  void Variant_Tree::add_in_current_variant( int current_player_index, 
					     const Move_Sequence &sequence, 
					     unsigned possible_variants )
  {
    current_variant 
      = current_variant->add_variant( current_player_index, sequence, possible_variants,
				      unique_id++ );
  }
  void Variant_Tree::move_a_variant_back()
  {
    assert( !is_first() );
    current_variant = current_variant->prev;
    assert( current_variant );
  }
  void Variant_Tree::clear()
  {
    delete root;
    root = new Variant(unique_id++);
    current_variant = root;
  }

  // ----------------------------------------------------------------------------
  // Game
  // ----------------------------------------------------------------------------

  Game::Game( const Ruleset &_ruleset )
    : game_phase(selecting), ruleset(_ruleset.clone()), 
      common_stones(ruleset->common_stones), 
      win_condition(ruleset->win_condition),
      coordinate_translator(ruleset->coordinate_translator),
      undo_possible(ruleset->undo_possible),
      ref_counter(new Ref_Counter()),
      last_in_order_player_index(-1),
      winner_player_index(-1)
  {
    current_player = players.begin();
    current_player_index = 0;
    prev_player = players.end();
    prev_player_index = -1;
  }

  Game::Game( const Game &game )
  {
    *this = game;		// use operator=
  }

  Game &Game::operator=( const Game &game )
  {
    // copy players
    std::vector<Player>::iterator player;
    std::vector<Player>::const_iterator player_src;
    if( players.size() == game.players.size() )
    {
      // if size is equal, use same player objects
      player_src = game.players.begin();
      for( player = players.begin(); player != players.end(); ++player, ++player_src )
      {
	assert( player_src != game.players.end() );
	*player = *player_src;
      }
    }
    else
    {
      players = game.players;
    }

    game_phase		 = game.game_phase;
    current_player_index = game.current_player_index;
    current_player	 = get_player( current_player_index );
    prev_player_index    = game.prev_player_index;
    prev_player		 = get_player( prev_player_index );

    common_stones 	  = game.common_stones;
    win_condition 	  = game.win_condition;
    coordinate_translator = game.coordinate_translator;
    undo_possible 	  = game.undo_possible;
    variant_tree	  = game.variant_tree;

    ruleset 		  = game.ruleset;
    ref_counter 	  = game.ref_counter;
    ++ref_counter->cnt;

    last_in_order_player_index = game.last_in_order_player_index;
    winner_player_index	  = game.winner_player_index;

    return *this;
  }

  Game::~Game()
  {
    if( ref_counter->cnt <= 1 )
    {
      delete ruleset;
      delete ref_counter;
    }
    else
    {
      --ref_counter->cnt;
    }
  }

  //! start or continue game
  Game::Game_State Game::continue_game() throw(Exception)
  {
    std::list<Player_Output *>::iterator output;
    Move_Sequence sequence;

    //obsolete? 
    if( (players.size() < ruleset->min_players) ||
	(players.size() > ruleset->max_players) ) 
      return wrong_number_of_players;

    // check win condition (not active for relax)
    if( prev_player != players.end() )
    {
      if( win_condition->did_player_win(*this,*prev_player) )
      {
	winner_player_index = prev_player_index;
	current_player->is_active = false;
	return finished_scores;
      }
    }

    if( !current_player->is_active ) // if no player is able to move
    {
      winner_player_index=-1;
      return finished_scores;
    }

    if( current_player->board.get_empty_fields().empty() )
      return finished_scores;

    do
    {
      bool sequence_already_given = false;
      Player_Input::Player_State state;
      if( game_phase == selecting &&
	  current_player->origin == Player::local )
      {
	state = Player_Input::finished;
	sequence = initialize_round();	// game where first player is local, does select moves
	sequence_already_given = true;
      }
      else
      {
	state = current_player->input->determine_move();
      }

      switch( state )
      {
	case Player_Input::finished:
	{
	  // get move
	  if( !sequence_already_given )
	    sequence = current_player->input->get_move();

	  // report move
	  for( output =  current_player->outputs.begin();
	       output != current_player->outputs.end();
	       ++output )
	    (*output)->report_move( sequence );

	  // calculate average time
	  if( !sequence_already_given )
	  {
	    long time = current_player->input->get_used_time();
	    current_player->total_time += time;
	    current_player->average_time 
	      = current_player->average_time * current_player->num_measures + time;
	    ++current_player->num_measures;
	    current_player->average_time /= current_player->num_measures;
	  }

	  // do move
	  do_move( sequence );

	  // check win condition
	  if( prev_player != players.end() )
	  {
	    if( win_condition->did_player_win(*this,*prev_player) )
	    {
	      winner_player_index = prev_player_index;
	      current_player->is_active = false;
	      return finished;
	    }
	  }

	  if( !current_player->is_active ) // if no player is able to move
	  {
	    winner_player_index=-1;
	    return finished_scores;
	  }

	  if( current_player->board.get_empty_fields().empty() )
	    return finished_scores;

	  return next_players_turn;
	}
	break;
	case Player_Input::wait_for_event: 
	  return wait_for_event;
	case Player_Input::interruption_possible: 
	  return interruption_possible;
	default: assert(false);
      }
    }while(true);
  }

  //! stop game to allow changing the position with do_move and undo_move
  void Game::stop_game()		
  {
  }

  void Game::reset_game()
  {
    game_phase = selecting;
    common_stones = ruleset->common_stones;
    win_condition = ruleset->win_condition;
    coordinate_translator = ruleset->coordinate_translator;
    undo_possible = ruleset->undo_possible;
    variant_tree.clear();
    winner_player_index = -1;

    // remove stones of all players
    std::vector<Player>::iterator i;
    for( i = players.begin(); i != players.end(); ++i )
    {
      i->stones = ruleset->common_stones;
      i->board = ruleset->board;
    }

    current_player = players.begin();
    current_player_index = 0;
    prev_player    = players.end();
    prev_player_index = -1;
    reset_player_already_done();
  }

  void Game::reset_game( const Ruleset &ruleset )
  {
    if( ref_counter->cnt <= 1 )
    {
      delete this->ruleset;
    }
    else
    {
      --ref_counter->cnt;
      ref_counter = new Ref_Counter();
    }

    this->ruleset = ruleset.clone();
    reset_game();
  }

  std::multimap<int/*score*/,const Player*> Game::get_scores() const
  {
    std::multimap<int/*score*/,const Player*> ret;
    std::vector<Player>::const_iterator it;
    for( it = players.begin(); it != players.end(); ++it )
    {
      const Player *player = &*it;
      int score = get_max_score(player);
      ret.insert(std::make_pair(score,player));
    }
    
    return ret;
  }

  // true: right number of players
  bool Game::set_players( const std::vector<Player> &new_players ) 
  {
    std::vector<Player>::iterator player;
    // if size is equal, use same player objects
    if( players.size() == new_players.size() )
    {
      std::vector<Player>::const_iterator player_src;

      player_src = new_players.begin();
      for( player = players.begin(); player != players.end(); ++player, ++player_src )
      {
	// rescue stones / board
	Stones player_stones = player->stones;
	Board player_board = player->board;
	// copy player
	assert( player_src != new_players.end() );
	*player = *player_src;

	player->stones = player_stones;
	player->board = player_board;
      }
    }
    else
    {
      players = new_players;

      // remove player stones
      for( player = players.begin(); player != players.end(); ++player )
      {
	player->stones = ruleset->common_stones;
	player->board = ruleset->board;
      }

      current_player       = players.begin();
      current_player_index = 0;
      prev_player	   = players.end();
      prev_player_index    = -1;
    }

    // set player active
    for( player = players.begin(); player != players.end(); ++player )
    {
      if( player == current_player )
	player->is_active = true;
      else
	player->is_active = false;
    }

    return (players.size() >= get_min_players()) && (players.size() <= get_min_players());
  }

  // true: enough players
  bool Game::add_player( const Player &player ) 
  {
    if( players.size() >= ruleset->max_players )
      return false;

    players.push_back( player );
    players.back().already_done = false;
    players.back().stones = ruleset->common_stones;
    players.back().board = ruleset->board;
    current_player = players.begin();
    current_player_index = 0;
    current_player->is_active = true;
    prev_player       = players.end(); // no player was last
    prev_player_index = -1;

    return ( (players.size() >= ruleset->min_players) &&
	     (players.size() <= ruleset->max_players) );
  }

  void Game::remove_players()
  {
    players.clear();
    current_player = players.end();
    current_player_index = -1;
    prev_player    = players.end();
    prev_player_index = -1;
  }

  Move_Sequence Game::initialize_round()
  {
    Stones::Stone_Type chosen_stone = Stones::invalid_stone;
    int num_stones = ruleset->nums1.size() * ruleset->nums2.size() * ruleset->nums3.size();
    int possible_stones=0;
    for( int i=0; i<num_stones; ++i )
      {
	Stones::Stone_Type stone_type = Stones::Stone_Type(Stones::stone_begin + i);
	possible_stones += common_stones.stone_count[stone_type];
      }
    int chosen_index = random(possible_stones);
    for( int i=0; i<num_stones; ++i )
      {
	Stones::Stone_Type stone_type = Stones::Stone_Type(Stones::stone_begin + i);
	chosen_index -= common_stones.stone_count[stone_type];
	if( chosen_index < 0 )
	  {
	    chosen_stone = stone_type;
	    break;
	  }
      }
    Move_Sequence seq;
    Move *move = new Select_Move(chosen_stone);
    seq.add_move(move);
    move = new Finish_Move();
    seq.add_move(move);
    return seq;
  }

  std::vector<Player>::iterator Game::get_player( int index )
  {
    if( index < 0 )
      return players.end();

    std::vector<Player>::iterator ret = players.begin();

    for( int i = 0; i < index; ++i )
      ++ret;

    return ret;
  }

  std::vector<Player>::const_iterator Game::get_player( int index ) const
  {
    if( index < 0 )
      return players.end();

    std::vector<Player>::const_iterator ret = players.begin();

    for( int i = 0; i < index; ++i )
      ++ret;

    return ret;
  }

  int Game::get_player_index( std::vector<Player>::iterator it ) const
  {
    if( it == players.end() )
      return -1;

    int ret = 0;
    while( it != players.begin() )
    {
      --it;
      ++ret;
    }
    return ret;
  }

  std::vector<Player>::iterator Game::get_player_by_id( int id )
  {
    std::vector<Player>::iterator ret = players.begin();
    while( ret != players.end() )
      {
	if( ret->id == id ) return ret;
	++ret;
      }

    return ret;
  }

  std::vector<Player>::const_iterator Game::get_player_by_id( int id ) const
  {
    std::vector<Player>::const_iterator ret = players.begin();
    while( ret != players.end() )
      {
	if( ret->id == id ) return ret;
	++ret;
      }

    return ret;
  }

  void Game::set_current_player_index(int player_index)
  {
    assert(is_out_of_order());
    last_in_order_player_index = current_player_index;
    bool save_active = current_player->is_active;
    current_player->is_active = false;
    current_player = get_player(player_index);
    current_player_index = player_index;
    current_player->is_active = save_active;
  }

  void Game::undo_set_current_player_index()
  {
    if( last_in_order_player_index >= 0 )
    {
      bool save_active = current_player->is_active;
      current_player->is_active = false;
      current_player = get_player(last_in_order_player_index);
      current_player_index = last_in_order_player_index;
      current_player->is_active = save_active;
      last_in_order_player_index = -1;
    }
  }

  void Game::set_selecting_phase()
  { 
    game_phase = selecting; 
    bool save_active = current_player->is_active;
    current_player->is_active = false;
    current_player = get_player(0);
    current_player_index = 0;
    current_player->is_active = save_active;
  }

  //! game dependent function
  bool Game::choose_next_player()		// next player is current player
  {
    prev_player = current_player;
    prev_player_index = current_player_index;

    // change player
    current_player->is_active = false;
    bool done;
    do{
      done = true;
      bool first = true;
      if( last_in_order_player_index >= 0 )  // jump back to last in-order player
      {
	current_player_index = last_in_order_player_index;
	current_player = get_player(current_player_index);
	last_in_order_player_index = -1;
	first = false;
      }
      switch(game_phase)
      {
      case selecting:
	current_player = players.begin();
	current_player_index = 0;
	break;
      case selected:
	current_player = players.begin();
	current_player_index = 0;
	first = false;
	game_phase = local;
	// fall through
      case local:
	while(current_player != players.end())
	{
	  if( !first && current_player->origin == Player::local ) break;
	  ++current_player;
	  ++current_player_index;
	  first = false;
	}
	if( current_player != players.end() ) break;  // 2nd break
	current_player = players.begin();
	current_player_index = 0;
	// fall through
      case remote:
	game_phase = remote;
	while(current_player != players.end())
	{
	  if( !first && current_player->origin == Player::remote ) break;
	  ++current_player;
	  ++current_player_index;
	  first = false;
	}
	if( current_player != players.end() ) break;  // 2nd break
	game_phase = selecting;
	current_player = players.begin();
	current_player_index = 0;
	reset_player_already_done();
	break;
      }
      if( game_phase != selecting && current_player->already_done ) 
	done = false;
    }while(!done);
    current_player->is_active = true;

    return true;
  }

  bool Game::choose_prev_player( int prev_prev_player_index )// prev player is current player
  {
    if( prev_player == players.end() )
      return false;

    current_player->is_active = false;
    current_player = prev_player;
    current_player_index = prev_player_index;
    current_player->is_active = true;

    prev_player	      = get_player( prev_prev_player_index );
    prev_player_index = prev_prev_player_index;
    if( game_phase == remote && current_player->origin == Player::local )
      game_phase = local;
    return true;
  }

  void Game::reset_player_already_done()
  {
    std::vector<Player>::iterator it;
    for( it=players.begin(); it!=players.end(); ++it )
    {
      Player &player = *it;
      player.already_done = false;
    }
    last_in_order_player_index = -1;
  }

  void Game::do_move( const Move_Sequence &sequence )
  {
    int player_index = current_player_index;
    if( game_phase != selecting )
      current_player->already_done = true;
    sequence.do_sequence( *this );
    choose_next_player();
    variant_tree.add_in_current_variant( player_index, sequence, get_num_possible_moves() );
  }

  bool Game::undo_move()
  {
    if( variant_tree.is_first() )
      return false;

    Move_Sequence &sequence = variant_tree.get_current_variant()->move_sequence;

    variant_tree.move_a_variant_back();
    if( !variant_tree.is_first() )
    {
      choose_prev_player( variant_tree.get_current_variant()->current_player_index );
    }
    else
      choose_prev_player( -1 );

    sequence.undo_sequence( *this );
    current_player->already_done = false;
    //!!! todo: undo game_phase change
    return true;
  }

  void Game::print()
  {
    Board &board = current_player->board;
    std::cout << "Max Score: " << get_max_score() << std::endl;
    for( int y=0; y<board.get_y_size(); ++y )
      {
	if( y % 2 == 0 ) std::cout << "    ";
	for( int x=0; x<board.get_x_size(); ++x )
	  {
	    if( board.field[x][y] == field_removed )
	      {
		std::cout << "        ";
	      }
	    else if( board.field[x][y] == field_empty )
	      {
		std::cout << "././.   ";
	      }
	    else
	      {
		boost::tuple<int,int,int> nums = ruleset->get_numbers(Stones::Stone_Type(board.field[x][y]));
		std::cout << nums.get<0>() << "/" << nums.get<1>() << "/" << nums.get<2>() << "   ";
	      }
	  }
	std::cout << std::endl;
	std::cout << std::endl;
      }
  }

  int Game::get_num_possible_moves() // number of possible moves in current situation
  {
    Board &board = current_player->board;
    return board.get_empty_fields().size();
  }

  // ******************
  // get_possible_moves

  std::list<Move_Sequence> Game::get_possible_moves() // get possible moves in situation
  {
    Move_Sequence current_sequence;
    std::list<Move_Sequence> possible_moves;

    Board &board = current_player->board;
    std::list<Field_Pos> fields = board.get_empty_fields();
    std::list<Field_Pos>::iterator it;
    for( it = fields.begin(); it != fields.end(); ++it )
      {
	Field_Pos pos = *it;
	Move_Sequence move_sequence;
	move_sequence.add_move( new Set_Move(pos, common_stones.current_stone) );
	move_sequence.add_move( new Finish_Move() );
	possible_moves.push_back( move_sequence );
      }
    return possible_moves;
  }

  // get moves played since start
  std::list<std::pair<Move_Sequence,int/*player index*/> > Game::get_played_moves() 
  {
    return variant_tree.get_current_variant_moves();
  }

  struct Speculative_Score
  {
    int num;
    unsigned count;
    unsigned missing;
    Speculative_Score(int num,unsigned count,unsigned missing)
      : num(num), count(count), missing(missing), assigned_index(0) {}
    // temporary state
    unsigned assigned_index;
  };

  std::pair<int, std::list<std::pair<int/*count*/,int/*num*/> > > 
  Game::get_max_score_detail(const Player *player, std::vector<std::map<int/*num*/, 
			     unsigned /*stones*/> > *stones_available) const
  {
    std::list<std::pair<int/*count*/,int/*num*/> > scores;
    const Board &board = player?player->board:current_player->board;
    std::map<unsigned/*dir*/,std::map<int/*num*/,std::list<Speculative_Score> > > 
      speculative_scores;
    std::map<unsigned/*dir*/,std::list<Speculative_Score> > 
      any_num_speculative_scores;
    int score = 0;
    Field_Iterator main_it(Field_Pos(0,0),&board);
    if( !main_it.is_valid_field() ) return std::make_pair(0,scores);
    // find top-left-corner
    while( *main_it == field_removed )
      {
	main_it.Right();
	if( !main_it.is_valid_field() ) return std::make_pair(0,scores);
      }
    Field_Iterator corner = main_it;
    Field_Iterator it1(&board),it2(&board);
    int cur_num=0; bool cur_num_set = false; bool valid = false; unsigned count=0, missing=0;
    // traverse board bottom-right
    while( main_it.is_valid_field() )
      {
	if( *main_it == field_removed ) break;
	// process right-row
	const unsigned right_dir = 0;
	cur_num_set = false;
	valid = true;
	count = 0; missing = 0;
	it1 = it2 = main_it;
	while( it2.is_valid_field() )
	  {
	    if( *it2 == field_removed ) break;
	    it1 = it2;
	    it2.Left();
	  }
	while( it1.is_valid_field() )
	  {
	    if( *it1 == field_removed ) break;
	    if( board.is_stone(*it1) )
	      {
		int num = ruleset->get_numbers(Stones::Stone_Type(*it1)).get<right_dir>();
		if( cur_num_set ) 
		  {
		    if( num != cur_num )
		      {
			valid = false;
			break;
		      }
		  }
		else
		  {
		    cur_num = num;
		    cur_num_set = true;
		  }
	      }
	    else
	      ++missing;
	    ++count;
	    it1.Right();
	  }
	if( valid && cur_num_set )
	  {
	    int add_score=0;
	    scores.push_back(std::make_pair(int(count),cur_num));
	    if( stones_available )
	      {
		if( (*stones_available)[right_dir][cur_num] >= missing )
		  add_score += count * cur_num;
	      }
	    else
	      add_score += count * cur_num;
	    if( missing && add_score != 0 )
	      {
		speculative_scores[right_dir][cur_num].push_back
		  ( Speculative_Score(cur_num,count,missing) );
// 		std::cout << "Dir " << right_dir << ": Speculative Score: " << cur_num*count 
// 			  << " num=" << cur_num << " count=" << count << " missing=" << missing << std::endl;
	      }
	    else
	      {
		score += add_score;
	      }
	  }
	else if( valid )
	  {
	    any_num_speculative_scores[right_dir].push_back
		  ( Speculative_Score(0,count,missing) );
// 	    std::cout << "Dir " << right_dir << ": Any Num Speculative Score: " << (count*9) << " num=" << 9
// 		      << " count=" << count << " missing=" << missing << std::endl;
	  }
	// process bottom-left-row
	const unsigned bottom_left_dir = 2;
	cur_num_set = false;
	valid = true;
	count = 0; missing = 0;
	it1 = it2 = main_it;
	while( it2.is_valid_field() )
	  {
	    if( *it2 == field_removed ) break;
	    it1 = it2;
	    it2.Top_Right();
	  }
	while( it1.is_valid_field() )
	  {
	    if( *it1 == field_removed ) break;
	    if( board.is_stone(*it1) )
	      {
		int num = ruleset->get_numbers(Stones::Stone_Type(*it1)).get<bottom_left_dir>();
		if( cur_num_set ) 
		  {
		    if( num != cur_num )
		      {
			valid = false;
			break;
		      }
		  }
		else
		  {
		    cur_num = num;
		    cur_num_set = true;
		  }
	      } else missing++;
	    ++count;
	    it1.Bottom_Left();
	  }
	if( valid && cur_num_set )
	  {
	    int add_score=0;
	    scores.push_back(std::make_pair(int(count),cur_num));
	    if( stones_available )
	      {
		if( (*stones_available)[bottom_left_dir][cur_num] >= missing )
		  add_score += count * cur_num;
	      }
	    else
	      add_score += count * cur_num;
	    if( missing && add_score != 0 )
	      {
		speculative_scores[bottom_left_dir][cur_num].push_back
		  ( Speculative_Score(cur_num,count,missing) );
// 		std::cout << "Dir " << bottom_left_dir << ": Speculative Score: " << cur_num*count 
// 			  << " num=" << cur_num << " count=" << count << " missing=" << missing << std::endl;
	      }
	    else
	      {
		score += add_score;
	      }
	  }
	else if( valid )
	  {
	    any_num_speculative_scores[bottom_left_dir].push_back
		  ( Speculative_Score(0,count,missing) );
// 	    std::cout << "Dir " << bottom_left_dir << ": Any Num Speculative Score: " << (count*9) << " num=" << 9
// 		      << " count=" << count << " missing=" << missing << std::endl;
	  }

	// iterate
	main_it.Bottom_Right();
      }
    // find left-corner
    main_it = corner;
    while( main_it.is_valid_field() )
      {
	if( *main_it == field_removed ) break;
	corner = main_it;
	// iterate
	main_it.Bottom_Left();
      }
    main_it = corner;
    // traverse board right
    while( main_it.is_valid_field() )
      {
	if( *main_it == field_removed ) break;
	// process bottom-right-row
	const unsigned bottom_right_dir = 1;
	cur_num_set = false;
	valid = true;
	count = 0; missing = 0;
	it1 = it2 = main_it;
	while( it2.is_valid_field() )
	  {
	    if( *it2 == field_removed ) break;
	    it1 = it2;
	    it2.Top_Left();
	  }
	while( it1.is_valid_field() )
	  {
	    if( *it1 == field_removed ) break;
	    if( board.is_stone(*it1) )
	      {
		int num = ruleset->get_numbers(Stones::Stone_Type(*it1)).get<bottom_right_dir>();
		if( cur_num_set ) 
		  {
		    if( num != cur_num )
		      {
			valid = false;
			break;
		      }
		  }
		else
		  {
		    cur_num = num;
		    cur_num_set = true;
		  }
	      } else missing++;
	    ++count;
	    it1.Bottom_Right();
	  }
	if( valid && cur_num_set )
	  {
	    int add_score=0;
	    scores.push_back(std::make_pair(int(count),cur_num));
	    if( stones_available )
	      {
		if( (*stones_available)[bottom_right_dir][cur_num] >= missing )
		  add_score += count * cur_num;
	      }
	    else
	      add_score += count * cur_num;
	    if( missing && add_score != 0 )
	      {
		speculative_scores[bottom_right_dir][cur_num].push_back
		  ( Speculative_Score(cur_num,count,missing) );
// 		std::cout << "Dir " << bottom_right_dir << ": Speculative Score: " << cur_num*count 
// 			  << " num=" << cur_num << " count=" << count << " missing=" << missing << std::endl;
	      }
	    else
	      {
		score += add_score;
	      }
	  }
	else if( valid )
	  {
	    any_num_speculative_scores[bottom_right_dir].push_back
		  ( Speculative_Score(0,count,missing) );
// 	    std::cout << "Dir " << bottom_right_dir << ": Any Num Speculative Score: " << (count*9) << " num=" << 9
// 		      << " count=" << count << " missing=" << missing << std::endl;
	  }

	// iterate
	main_it.Right();
      }

    // add speculative scores
    if( stones_available )
      {
// 	int save_score = score;
// 	std::vector<std::multimap<int,unsigned> > score_table;
// 	score_table.resize(3);
	for( unsigned dir=0; dir<3; ++dir )
	  {
	    int max_score = 0;
	    while( true )
	      {
		int cur_score = 0;
		bool infeasible = false;
		// check feasability and determine score
		std::vector<std::map<int/*num*/, unsigned /*stones*/> > 
		  cur_available = *stones_available;
		std::map<int/*num*/,std::list<Speculative_Score> > &map1 = speculative_scores[dir];
		std::map<int/*num*/,std::list<Speculative_Score> >::iterator it;
		std::list<Speculative_Score>::iterator list_it;
		for( it=map1.begin(); it!=map1.end(); ++it )
		  {
		    for( list_it=it->second.begin(); list_it!=it->second.end(); ++list_it )
		      {
			Speculative_Score &speculative_score = *list_it;
			if( speculative_score.assigned_index == 1 )
			  {
			    cur_score += speculative_score.num * speculative_score.count;
			    if( cur_available[dir][speculative_score.num] >= speculative_score.missing )
			      cur_available[dir][speculative_score.num] -= speculative_score.missing;
			    else
			      {
				infeasible = true;
				break;
			      }
			  }
		      }
		    if( infeasible ) break;
		  }
		if( !infeasible )
		  {
		    for( list_it=any_num_speculative_scores[dir].begin(); 
			 list_it!=any_num_speculative_scores[dir].end(); ++list_it )
		      {
			Speculative_Score &speculative_score = *list_it;
			if( speculative_score.assigned_index >= 1 && speculative_score.assigned_index <= 3 )
			  {
			    int num = ruleset->get_nums(dir)[speculative_score.assigned_index-1];
			    cur_score += num * speculative_score.count;
			    if( cur_available[dir][num] >= speculative_score.missing )
			      cur_available[dir][num] -= speculative_score.missing;
			    else
			      {
				infeasible = true;
				break;
			      }
			  }
		      }
		  }
		// check max_score
		if( !infeasible ) 
		  {
// 		    score_table[dir].insert( std::make_pair(cur_score,0/*dummy*/) );
// 		    if( score_table[dir].size() >= 10 )
// 		      score_table[dir].erase(score_table[dir].begin());
		    if( cur_score > max_score ) 
		      max_score = cur_score;
		  }
		// increment index
		bool multi_break=false;
		for( it=map1.begin(); it!=map1.end(); ++it )
		  {
		    for( list_it=it->second.begin(); list_it!=it->second.end(); ++list_it )
		      {
			Speculative_Score &speculative_score = *list_it;
			if( speculative_score.assigned_index >= 1 )
			  speculative_score.assigned_index = 0;
			else
			  {
			    ++speculative_score.assigned_index;
			    multi_break = true;
			    break;
			  }
		      }
		    if( multi_break ) break;
		  }
		if( multi_break ) continue;
		for( list_it=any_num_speculative_scores[dir].begin(); 
		     list_it!=any_num_speculative_scores[dir].end(); ++list_it )
		  {
		    Speculative_Score &speculative_score = *list_it;
		    if( speculative_score.assigned_index >= 3 )
		      speculative_score.assigned_index = 0;
		    else
		      {
			++speculative_score.assigned_index;
			multi_break = true;
			break;
		      }
		  }
		if( multi_break ) continue;
		break;
	      }
	      score += max_score;
	  }
// 	if( score > 260 )
// 	  {
// 	    std::cout << "score=" << score << " (" << (score-save_score) << "):" << std::endl;
// 	    for( unsigned dir=0; dir<3; ++dir )
// 	      {
// 		std::cout << "  dir=" << dir << ": ";
// 		std::multimap<int,unsigned>::iterator it;
// 		for( it=score_table[dir].begin(); it!=score_table[dir].end(); ++it )
// 		  {
// 		    std::cout << it->first << ",";
// 		  }
// 		std::cout << std::endl;
// 	      }
// 	  }
      }

    return std::make_pair(score, scores);
  }

  int Game::get_max_score(const Player *player, std::vector<std::map<int/*num*/, 
			  unsigned /*stones*/> > *stones_available) const
  {
    int score = get_max_score_detail(player, stones_available).first;
    //std::cout << "score=" << score << std::endl;
    return score;
  }

  std::vector<Player>::iterator Game::get_next_player( std::vector<Player>::iterator player )
  {
    ++player;
    if( player == players.end() )
      return players.begin();
    else
      return player;
  }
  std::vector<Player>::iterator Game::get_prev_player( std::vector<Player>::iterator player )
  {
    if( player == players.begin() )
      player = players.end();

    return --player;
  }

  void Game::copy_player_times( Game &from ) // number of players must be the same
  {
    assert( from.players.size() == players.size() );
    std::vector<Player>::iterator i,j;
    for( i = players.begin(), j = from.players.begin(); i != players.end(); ++i, ++j )
    {
      assert( j != from.players.end() );

      i->total_time   = j->total_time;
      i->average_time = j->average_time;
      i->num_measures = j->num_measures;
    }
  }

  // ----------------------------------------------------------------------------
  // Move
  // ----------------------------------------------------------------------------

  Move::~Move()
  {
  }

  // ----------------------------------------------------------------------------
  // Set_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Set_Move::get_type() const
  {
    return set_move;
  }
  void Set_Move::do_move( Game &game )
  {
    assert( check_move(game) );
    assert( game.current_player->stones.stone_count[ stone_type ] > 0 );
    if( game.current_player->stones.stone_count[ stone_type ] > 0 )
    {
      --game.current_player->stones.stone_count[ stone_type ];
    }
    game.current_player->board.field[pos.x][pos.y] = Field_State_Type(stone_type);
  }
  void Set_Move::undo_move( Game &game )
  {
    ++game.current_player->stones.stone_count[ stone_type ];
    game.current_player->board.field[pos.x][pos.y] = field_empty;
  }

  // true: move ok
  bool Set_Move::check_move( Game &game ) const 
  {
    if( game.game_phase == Game::selecting ) return false;
    if( stone_type != game.common_stones.current_stone ) return false;
    if( pos.x >= (int)game.current_player->board.field.size() ) return false;
    if( pos.y >= (int)game.current_player->board.field[pos.x].size() ) return false;
    if( !Board::is_empty( game.current_player->board.field[pos.x][pos.y] ) ) return false;
    if( game.current_player->stones.stone_count[ stone_type ] <= 0 ) return false;
    return true;
  }

  // true: type ok
  bool Set_Move::check_previous_move( Game &, Move *move ) const 
  {
    return move == 0;
  }
  bool Set_Move::may_be_first_move( Game & /*game*/ ) const
  {
    return true;
  }
  bool Set_Move::may_be_last_move( Game & /*game*/ ) const
  {
    return true;
  }

  std::escape_ostream &Set_Move::output( std::escape_ostream &eos ) const
  {
    eos << get_type() << pos.x << pos.y << stone_type;
    return eos;
  }

  bool Set_Move::input( std::escape_istream &eis )
  {
    int stone;
    eis >> pos.x >> pos.y >> stone;
    if( (pos.x < 0) || (pos.y < 0) )
      return false;
    if( (stone < Stones::stone_begin || stone >= Stones::stone_max_end) )
      return false;
    stone_type = Stones::Stone_Type(stone);
    return true;
  }
  
  Move *Set_Move::clone() const
  {
    return new Set_Move(*this);
  }
  
  Set_Move::Set_Move()
  {
  }

  Set_Move::Set_Move( Field_Pos pos, Stones::Stone_Type stone_type  )
    : pos(pos), stone_type(stone_type)
  {
  }

  // ----------------------------------------------------------------------------
  // Select_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Select_Move::get_type() const
  {
    return select_move;
  }
  void Select_Move::do_move( Game &game )
  {
    assert( check_move(game) );
    assert( game.common_stones.stone_count[ stone_type ] > 0 );
    prev_stone_type = game.common_stones.current_stone;
    game.common_stones.current_stone = stone_type;
    --game.common_stones.stone_count[ stone_type ];
    game.game_phase = Game::selected;
  }
  void Select_Move::undo_move( Game &game )
  {
    assert( game.game_phase != Game::selecting );
    ++game.common_stones.stone_count[ game.common_stones.current_stone ];
    game.common_stones.current_stone = prev_stone_type;
    game.game_phase = Game::selecting;
  }

  // true: move ok
  bool Select_Move::check_move( Game &game ) const 
  {
    if( game.game_phase != Game::selecting ) return false;
    if( game.common_stones.stone_count[ stone_type ] <= 0 ) return false;
    return true;
  }

  // true: type ok
  bool Select_Move::check_previous_move( Game &, Move *move ) const 
  {
    return move == 0;
  }
  bool Select_Move::may_be_first_move( Game & /*game*/ ) const
  {
    return true;
  }
  bool Select_Move::may_be_last_move( Game & /*game*/ ) const
  {
    return true;
  }

  std::escape_ostream &Select_Move::output( std::escape_ostream &eos ) const
  {
    eos << get_type() << stone_type;
    return eos;
  }

  bool Select_Move::input( std::escape_istream &eis )
  {
    int stone;
    eis >> stone;
    if( (stone < Stones::stone_begin || stone >= Stones::stone_max_end) )
      return false;
    stone_type = Stones::Stone_Type(stone);
    return true;
  }
  
  Move *Select_Move::clone() const
  {
    return new Select_Move(*this);
  }
  
  Select_Move::Select_Move()
  {
  }

  Select_Move::Select_Move( Stones::Stone_Type stone_type  )
    : stone_type(stone_type)
  {
  }

  // ----------------------------------------------------------------------------
  // Finish_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Finish_Move::get_type() const
  {
    return finish_move;
  }

  void Finish_Move::do_move( Game & /*game*/ )
  {
  }
  void Finish_Move::undo_move( Game & /*game*/ )
  {
  }

  // true: move ok
  bool Finish_Move::check_move( Game & /*game*/ ) const 
  {
    return true;
  }
  // true: type ok
  bool Finish_Move::check_previous_move ( Game &game, Move *move ) const 
  {
    if( !move )
      return false;

    return move->may_be_last_move(game);
  }
  bool Finish_Move::may_be_first_move( Game & ) const
  {
    return false;
  }
  bool Finish_Move::may_be_last_move( Game & ) const
  {
    return true;
  }

  std::escape_ostream &Finish_Move::output( std::escape_ostream &eos ) const
  {
    eos << get_type();
    return eos;
  }
  bool Finish_Move::input( std::escape_istream & /*is*/ )
  {
    return true;
  }
  
  Move *Finish_Move::clone() const
  {
    return new Finish_Move(*this);
  }

  Finish_Move::Finish_Move()
  {
  }

  // ----------------------------------------------------------------------------
  // Move_Sequence
  // ----------------------------------------------------------------------------

  void Move_Sequence::do_sequence( Game &game ) const
  {
    std::list<Move*>::iterator move;
    for( move = moves->begin(); move != moves->end(); ++move )
    {
      (*move)->do_move(game);
    }
  }

  void Move_Sequence::undo_sequence( Game &game ) const
  {
    std::list<Move*>::reverse_iterator move;
    for( move = moves->rbegin(); move != moves->rend(); ++move )
    {
      (*move)->undo_move(game);
    }
  }

  // true: move ok
  bool Move_Sequence::check_sequence( Game &game ) const
  {
    if( moves->size() == 0 ) return false;
    if( !moves->front()->may_be_first_move(game) ) return false;
    Move *prev_move = 0;

    // check all moves
    bool is_move_ok = true;
    std::list<Move*>::iterator move;
    std::list<Move*>::reverse_iterator last_done_move = moves->rend();
    for( move = moves->begin(); move != moves->end(); ++move )
    {
      if( !(*move)->check_previous_move( game, prev_move ) || 
	  !(*move)->check_move(game) ) 
      {
	is_move_ok = false;
	break;
      }
      (*move)->do_move(game);
      --last_done_move;
      prev_move = *move;
    }
    if( moves->back()->get_type() != Move::finish_move ) 
      is_move_ok = false;
    // undo all moves
    for( ; last_done_move != moves->rend(); ++last_done_move )
    {
      (*last_done_move)->undo_move(game);
    }

    return is_move_ok;
  }

  std::escape_ostream &Move_Sequence::output( std::escape_ostream &eos ) const
  {
    eos << moves->size();
    std::list<Move*>::iterator move;
    for( move = moves->begin(); move != moves->end(); ++move )
    {
      (*move)->output(eos);
    }
    return eos;
  }
  bool Move_Sequence::input( std::escape_istream &eis )
  {
    int num_moves; 
    Move::Move_Type type;
    Move *move = 0;

    clear();
    eis >> num_moves;
    for( int i = 0; i < num_moves; i++ )
    {
      int move_type;
      eis >> move_type;
      if( eis.did_error_occur() )
	return false;
      type = Move::Move_Type(move_type);
      switch( type )
      {
	case Move::set_move:		move = new Set_Move(); break;
	case Move::select_move:		move = new Select_Move(); break;
	case Move::finish_move:		move = new Finish_Move(); break;
	default: return false;
      }
      if( !move->input(eis) )
      {
	delete move; 
	return false;
      }
      moves->push_back(move);
    }
    return true;
  }

  // this function takes care of the reserved memory
  void Move_Sequence::add_move( Move *move )
  {
    modify_moves();
    moves->push_back( move );
  }

  // true: add ok
  // this function takes care of the reserved memory if add ok
  bool Move_Sequence::add_move( Game &game, Move *move )
  {
    if( !move->check_previous_move( game, get_last_move() ) )
      return false;

    modify_moves();
    moves->push_back( move );
    return true;
  }

  Move *Move_Sequence::get_last_move()
  {
    if( moves->size() == 0 )
      return 0;

    return moves->back();
  }

  //! calls undo for last move and removes it
  void Move_Sequence::undo_last_move( Game &game )
  {
    if( moves->size() )
    {
      modify_moves();
      moves->back()->undo_move(game);
      delete moves->back();
      moves->pop_back();
    }
  }

  void Move_Sequence::clear()
  {
    if( ref_counter->cnt <= 1 )
    {
      assert( ref_counter->cnt == 1);

      std::list<Move*>::iterator move;
      for( move = moves->begin(); move != moves->end(); ++move )
      {
	delete *move;
      }
      moves->clear();
    }
    else
    {
      --ref_counter->cnt;
      moves = new std::list<Move*>();
      ref_counter = new Ref_Counter();
    }
  }

  //! to store sequences, they must be cloned
  Move_Sequence Move_Sequence::clone() const
  {
    Move_Sequence ret;
    std::list<Move*>::iterator i;
    for( i = moves->begin(); i != moves->end(); ++i )
    {
      ret.moves->push_back( (*i)->clone() );
    }

    return ret;
  }

  void Move_Sequence::modify_moves()
  {
    if( ref_counter->cnt > 1 )
    {
      --ref_counter->cnt;
      moves = new std::list<Move*>(*moves);
      ref_counter = new Ref_Counter();
    }
  }

  Move_Sequence::Move_Sequence()
    : moves( new std::list<Move*>() ), ref_counter( new Ref_Counter() )
  {
  }

  Move_Sequence::Move_Sequence( const Move_Sequence &sequence )
  {
    ref_counter = sequence.ref_counter;
    ++ref_counter->cnt;
    moves = sequence.moves;
  }

  Move_Sequence &Move_Sequence::operator=( const Move_Sequence &sequence )
  {
    assert( ref_counter->cnt > 0 );

    if( --ref_counter->cnt == 0 )
    {
      std::list<Move*>::iterator move;
      for( move = moves->begin(); move != moves->end(); ++move )
      {
	delete *move;
      }
      delete moves;
      delete ref_counter;
    }
    ref_counter = sequence.ref_counter;
    ++ref_counter->cnt;
    moves = sequence.moves;

    return *this;
  }

  Move_Sequence::~Move_Sequence()
  {
    if( ref_counter->cnt <= 1 )
    {
      assert( ref_counter->cnt == 1);

      std::list<Move*>::iterator move;
      for( move = moves->begin(); move != moves->end(); ++move )
      {
	delete *move;
      }
      delete moves;
      delete ref_counter;
    }
    else
    {
      --ref_counter->cnt;
    }
  }

  bool operator==( const Move &m1, const Move &m2 )
  {
    if( (int)m1.get_type() != (int)m2.get_type() ) return false;
    switch( m1.get_type() )
    {
      case Move::no_move: break;
      case Move::set_move: 
      {
	const Set_Move *det_m1 = static_cast<const Set_Move*>(&m1);
	const Set_Move *det_m2 = static_cast<const Set_Move*>(&m2);
	return *det_m1 == *det_m2;
      }
      case Move::select_move: 
      {
	const Select_Move *det_m1 = static_cast<const Select_Move*>(&m1);
	const Select_Move *det_m2 = static_cast<const Select_Move*>(&m2);
	return *det_m1 == *det_m2;
      }
      case Move::finish_move: 
      {
	const Finish_Move *det_m1 = static_cast<const Finish_Move*>(&m1);
	const Finish_Move *det_m2 = static_cast<const Finish_Move*>(&m2);
	return *det_m1 == *det_m2;
      }
    }
    return true;
  }
  bool operator<( const Move &m1, const Move &m2 )
  {
    if( (int)m1.get_type() < (int)m2.get_type() ) return true;
    if( (int)m1.get_type() > (int)m2.get_type() ) return false;

    switch( m1.get_type() )
    {
      case Move::no_move: break;
      case Move::set_move: 
      {
	const Set_Move *det_m1 = static_cast<const Set_Move*>(&m1);
	const Set_Move *det_m2 = static_cast<const Set_Move*>(&m2);
	return *det_m1 < *det_m2;
      }
      case Move::select_move: 
      {
	const Select_Move *det_m1 = static_cast<const Select_Move*>(&m1);
	const Select_Move *det_m2 = static_cast<const Select_Move*>(&m2);
	return *det_m1 < *det_m2;
      }
      case Move::finish_move: 
      {
	const Finish_Move *det_m1 = static_cast<const Finish_Move*>(&m1);
	const Finish_Move *det_m2 = static_cast<const Finish_Move*>(&m2);
	return *det_m1 < *det_m2;
      }
    }
    return false;
  }
  bool operator==( const Set_Move &m1, const Set_Move &m2 )
  {
    if( m1.pos != m2.pos ) return false;
    if( m1.stone_type != m2.stone_type ) return false;
    return true;
  }
  bool operator<( const Set_Move &m1, const Set_Move &m2 )
  {
    if( m1.pos.x < m2.pos.x ) return true;
    if( m1.pos.x > m2.pos.x ) return false;
    if( m1.pos.y < m2.pos.y ) return true;
    if( m1.pos.y > m2.pos.y ) return false;
    if( (int)m1.stone_type < (int)m2.stone_type ) return true;
    if( (int)m1.stone_type > (int)m2.stone_type ) return false;
    return false;
  }
  bool operator==( const Select_Move &m1, const Select_Move &m2 )
  {
    if( m1.stone_type != m2.stone_type ) return false;
    return true;
  }
  bool operator<( const Select_Move &m1, const Select_Move &m2 )
  {
    if( (int)m1.stone_type < (int)m2.stone_type ) return true;
    if( (int)m1.stone_type > (int)m2.stone_type ) return false;
    return false;
  }
  bool operator==( const Move_Sequence &s1, const Move_Sequence &s2 )
  {
    std::list<Move*>::const_iterator it1, it2;
    for( it1 = s1.get_moves().begin(), it2 = s2.get_moves().begin();
	 it1 != s1.get_moves().end(); ++it1, ++it2 )
    {
      if( it2 == s2.get_moves().end() ) return false;
      if( !(**it1 == **it2) ) return false;
    }
    return true;
  }
  bool operator<( const Move_Sequence &s1, const Move_Sequence &s2 )
  {
    if( s1.get_moves().size() < s2.get_moves().size() ) return true;
    if( s1.get_moves().size() > s2.get_moves().size() ) return false;

    std::list<Move*>::const_iterator it1, it2;
    for( it1 = s1.get_moves().begin(), it2 = s2.get_moves().begin();
	 it1 != s1.get_moves().end(); ++it1, ++it2 )
    {
      assert( it2 != s2.get_moves().end() );
      if( (**it1 < **it2) ) return true;
      if( !(**it1 == **it2) ) return false; // => **it1 > **it2
    }

    return false;
  }

  // ----------------------------------------------------------------------------
  // Standard_Move_Translator
  // ----------------------------------------------------------------------------

  Standard_Move_Translator::Standard_Move_Translator( Coordinate_Translator *coordinate_translator )
    : coordinate_translator(coordinate_translator)
  {
  }

  std::string Standard_Move_Translator::encode( Move_Sequence sequence )
  {
    std::string ret;
    std::list<Move*>::const_iterator move_it;
    for( move_it = sequence.get_moves().begin(); move_it != sequence.get_moves().end(); move_it++ )
    {
      Move *move = *move_it;
      switch( move->get_type() )
      {
	case Move::no_move:
	  break;
	case Move::set_move:
	{
	  Set_Move *det_move = static_cast<Set_Move*>(move);
	  ret += coordinate_translator->get_field_name(det_move->pos);
	}
	break;
	case Move::select_move:
	{
	  Select_Move *det_move = static_cast<Select_Move*>(move);
	  ret += "s" + long_to_string(det_move->stone_type);
	}
	break;
	case Move::finish_move:
	{
	}
	break;
      }
    }

    return ret;
  }

  Move_Sequence Standard_Move_Translator::decode( std::istream &is )
  {
    Move_Sequence sequence;
    char first;
    is >> first;
    if( (first == 's') || (first == 'S') ) // is knock out move ?
    {
      int stone_type_num;
      is >> stone_type_num;
      if( stone_type_num < Stones::stone_begin || stone_type_num >= Stones::stone_max_end )
	return sequence;

      sequence.add_move( new Select_Move(Stones::Stone_Type(stone_type_num)) );
      sequence.add_move( new Finish_Move() );
    }
    else			// set move
    {
      is.unget();		// put back first character
      std::string str;
      is >> str;
      size_t i;
      // search for letters followed by digits
      for( i = 0; i < str.size(); ++i ) if( !isalpha( str[i] ) ) break;
      for( ; i < str.size(); ++i )	if( !isdigit( str[i] ) ) break;
      Field_Pos pos = coordinate_translator->get_field_pos( str.substr(0,i) );
      if( pos.x < 0 ) return Move_Sequence();

      sequence.add_move( new Set_Move( pos, Stones::invalid_stone ) );
      sequence.add_move( new Finish_Move() );
    }
    return sequence;
  }

  // ----------------------------------------------------------------------------
  // Standard_Win_Condition
  // ----------------------------------------------------------------------------

  Standard_Win_Condition::Standard_Win_Condition()
  {
    type = standard;
  }

  // ----------------------------------------------------------------------------
  // Standard_Ruleset
  // ----------------------------------------------------------------------------

  Standard_Ruleset::Standard_Ruleset()
    : Ruleset( standard,
	       Board( (const int*) standard_board, 
		      sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
		      sizeof(standard_board)    / sizeof(standard_board[0]), Board::standard ),
	       Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       false /*undo possible*/, 1, 10 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
    nums1.resize(3);
    nums1[0] = 1;
    nums1[1] = 5;
    nums1[2] = 9;
    max_num1 = 9;
    nums2.resize(3);
    nums2[0] = 2;
    nums2[1] = 6;
    nums2[2] = 7;
    max_num2 = 7;
    nums3.resize(3);
    nums3[0] = 3;
    nums3[1] = 4;
    nums3[2] = 8;
    max_num3 = 8;
    // set stone count to 1 for every stone
    for( unsigned i=0; i< nums1.size()*nums2.size()*nums3.size(); ++i )
      common_stones.stone_count[ Stones::Stone_Type(Stones::stone_begin + (int)i) ] = 1;
  }

  // ----------------------------------------------------------------------------
  // Extended_Ruleset
  // ----------------------------------------------------------------------------

  Extended_Ruleset::Extended_Ruleset()
    : Ruleset( extended,
	       Board( (const int*) board_37, 
		      sizeof(board_37[0]) / sizeof(board_37[0][0]),
		      sizeof(board_37)    / sizeof(board_37[0]), Board::extended ),
	       Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       false /*undo possible*/, 1, 10 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
    nums1.resize(3);
    nums1[0] = 1;
    nums1[1] = 5;
    nums1[2] = 9;
    max_num1 = 9;
    nums2.resize(3);
    nums2[0] = 2;
    nums2[1] = 6;
    nums2[2] = 7;
    max_num2 = 7;
    nums3.resize(3);
    nums3[0] = 3;
    nums3[1] = 4;
    nums3[2] = 8;
    max_num3 = 8;
    // set stone count to 1 for every stone
    for( unsigned i=0; i< nums1.size()*nums2.size()*nums3.size(); ++i )
      common_stones.stone_count[ Stones::Stone_Type(Stones::stone_begin + (int)i) ] = 2;
  }

  // ----------------------------------------------------------------------------
  // Custom_Ruleset
  // ----------------------------------------------------------------------------

  Custom_Ruleset::Custom_Ruleset( Board board, Common_Stones common_stones, 
				  Win_Condition *win_condition, 
				  Coordinate_Translator *coordinate_translator, 
				  bool undo_possible, unsigned min_players, unsigned max_players )
    : Ruleset( custom, board, common_stones, win_condition, coordinate_translator, undo_possible, 
	       min_players, max_players )
  {
  }

  // ----------------------------------------------------------------------------
  // No_Output
  // ----------------------------------------------------------------------------

  void No_Output::report_move( const Move_Sequence & )
  {
  }

  // ----------------------------------------------------------------------------
  // Stream_Output
  // ----------------------------------------------------------------------------

  Stream_Output::Stream_Output( Game &game, std::escape_ostream &eos )
    : game(game), eos(eos)
  {
  }

  void Stream_Output::report_move( const Move_Sequence &sequence )
  {
    eos << "player " << game.current_player->id;
    eos << sequence;
  }

  // ----------------------------------------------------------------------------
  // Stream_Input
  // ----------------------------------------------------------------------------

  Stream_Input::Stream_Input( Game &game, std::escape_istream &eis )
    : game(game), eis(eis)
  {
  }

  Player_Input::Player_State Stream_Input::determine_move() throw(Exception)
  {
    sequence.clear();
    
    std::string str;
    int num;
    eis >> str;
    if( str != "player" ) throw Input_Exception();
    eis >> num;
    if( num != game.current_player->id ) throw Input_Exception();
    
    eis >> sequence;
    if( !sequence.check_sequence(game) ) throw Input_Exception();
    sequence.do_sequence(game);
    
    return finished;
  }

  Move_Sequence Stream_Input::get_move()
  {
    return sequence;
  }

  // ----------------------------------------------------------------------------
  // Sequence_Generator
  // ----------------------------------------------------------------------------

  Sequence_Generator::Sequence_Generator( Game &game, bool easy_multiple_knock )
    : game(game), auto_undo(true), state(begin), easy_multiple_knock(easy_multiple_knock)
  {
  }

  // synthesize move sequence from clicks
  Sequence_Generator::Sequence_State Sequence_Generator::add_click
  ( int player_id, Field_Pos pos )
  {
    if( game.get_current_player().id != player_id ) 
      return error_wrong_player;
    move_done = 0;
    Field_Iterator p1( pos, &game.current_player->board );
    switch( state )
    {
      case begin:
	if( !Board::is_empty(*p1) )
	  {
	    return error_can_t_set_here;
	  }
	else
	  {
	    Move *move = new Set_Move( pos, game.common_stones.current_stone );
	    if( !move->check_move(game) )
	      {
		delete move;
		state = begin;
		return fatal_error;				 // why isn't this possible?
	      }
	    if( !sequence.add_move(game,move) )		 // check whether move may be added yet
	      {
		delete move;
		return fatal_error;
	      }
	    move->do_move(game);
	    move_done = move;
	    // add finish move
	    move = new Finish_Move();
	    bool ret = sequence.add_move(game,move);
	    assert( ret );
	    move->do_move(game);
	  
	    state = move_finished;
	    return finished;
	  }
	break;
      case move_finished:
	break;
    }
    /*
    switch( state )
    {
      case begin:
	if( Board::is_stone(*p1) )			 // clicked on stone?
	{
	  / * user will recognize his fault himself...
	     if( !game.board.is_knock_out_possible(pos) )
	     return error_...;
	  * /
	  from = pos;
	  picked_stone = Stones::Stone_Type(*p1);
	  state = move_from;
	  return Sequence_State(hold_prefix + *p1);
	}
	else
	{
	  if( game.board.is_knock_out_possible() )
	    return error_require_knock_out;
	  else
	    return error_require_set;
	}
	break;
      case move_from:
	if( Board::is_empty(*p1) )			 // clicked on stone?
	{
	  std::pair<bool,Field_Pos> ret = game.board.is_knock_out_possible(from,pos);
	  if( !ret.first )
	  {
	    return error_can_t_move_here;
	  }
	  const Field_Pos &over = ret.second;

	  to.push(pos);

	  Move *move = new Knock_Out_Move(from,over,pos);
	  if( !move->check_move(game) )
	  {
	    delete move;
	    state = begin;
	    return fatal_error;				 // why isn't this possible?
	  }

	  if( !sequence.add_move(game,move) )		 // check whether move may be added yet
	  {
	    delete move;
	    return fatal_error;
	  }
	  move->do_move(game);
	  move_done = move;

	  if( game.board.is_knock_out_possible(pos) )	 // another knock out possible from position?
	  {
	    if( easy_multiple_knock )
	    {
	      from = to.top();
	      // picked_stone is still the same
	      state = move_from;
	      return Sequence_State(hold_prefix + picked_stone);
	    }
	    else
	      state = move_dest;

	    return another_click;
	  }
	  else
	  {
	    // add finish move
	    move = new Finish_Move();
	    bool ret = sequence.add_move(game,move);
	    assert( ret );
	    move->do_move(game);
	  
	    state = move_finished;
	    return finished;
	  }
	}
	else
	  return error_can_t_move_here;
	break;
      case move_dest:
	if( pos != to.top() )
	  return error_must_knock_out_with_same_stone;
	else
	{
	  assert( Board::is_stone(*p1) );			 // clicked on stone!

	  from = pos;
	  picked_stone = Stones::Stone_Type(*p1);
	  state = move_from;
	  return Sequence_State(hold_prefix + *p1);
	}
	break;
      case stone_picked:
	if( Board::is_empty(*p1) )			 // clicked on stone?
	{
	  Move *move = new Set_Move( pos, picked_stone );
	  if( !move->check_move(game) )
	  {
	    delete move;
	    state = begin;
	    return fatal_error;				 // why isn't this possible?
	  }

	  if( !sequence.add_move(game,move) )		 // check whether move may be added yet
	  {
	    delete move;
	    return fatal_error;
	  }
	  move->do_move(game);
	  move_done = move;

	  if( game.board.is_any_field_removable() )
	  {
	    state = stone_set;
	    return another_click;
	  }
	  else
	  {
	    // add finish move
	    move = new Finish_Move();
	    bool ret = sequence.add_move(game,move);
	    assert( ret );
	    move->do_move(game);
	  
	    state = move_finished;
	    return finished;
	  }
	}
	else
	  return error_can_t_set_here;
	break;
      case stone_set:
	if( Board::is_empty(*p1) )			 // clicked on stone?
	{
	  if( !game.board.is_removable(pos) )
	    return error_can_t_remove;

	  Move *move = new Remove( pos );
	  if( !move->check_move(game) )
	  {
	    delete move;
	    state = begin;
	    return fatal_error;				 // why isn't this possible?
	  }

	  if( !sequence.add_move(game,move) )		 // check whether move may be added yet
	  {
	    delete move;
	    return fatal_error;
	  }
	  move->do_move(game);
	  move_done = move;

	  // add finish move
	  move = new Finish_Move();
	  bool ret = sequence.add_move(game,move);
	  assert( ret );
	  move->do_move(game);
	
	  state = move_finished;
	  return finished;
	}
	else
	  return error_require_remove;
	break;
      case move_finished:
	return fatal_error;
    }
    */
    assert( false );
    return fatal_error;
  }

  Sequence_Generator::Sequence_State Sequence_Generator::undo_click()
  {
    /*
    switch( state )
    {
      case begin:
	// nothing to be done (already at beginning of sequence)
	break;
      case move_from:
	// undo partial move (undo hold)
	if( to.size() == 0 )
	  state = begin;
	else
	{
	  if( easy_multiple_knock )
	  {
	    to.pop();						 // reload to position from last move

	    sequence.undo_last_move(game);

	    if( to.size() == 0 )
	      state = begin;
	    else
	    {
	      from = to.top();
	      return Sequence_State(hold_prefix + picked_stone);
	    }
	  }
	  else
	    state = move_dest;
	}
	break;
      case move_dest:
	assert( to.size() != 0 );
	to.pop();						 // reload to position from last move

	sequence.undo_last_move(game);

	if( to.size() == 0 )
	  state = begin;
	else
	{
	  from = to.top();
	  return Sequence_State(hold_prefix + picked_stone);
	}
	break;
      case stone_picked:
	state = begin;
	break;
      case stone_set:
	sequence.undo_last_move(game);
	state = begin;
      
	break;
      default:
	assert(false);
    }
    */
    return another_click;
  }

  Move::Move_Type Sequence_Generator::get_required_move_type()
  {
    switch( state )
    {
      case begin:
	return Move::set_move;
      case move_finished:
	break;
    }
    return Move::no_move;
  }

  std::list<Field_Pos> Sequence_Generator::get_possible_clicks()
  {
    std::list<Field_Pos> ret;
    return ret;
    /*
    switch( get_required_move_type() )
    {
      case Move::knock_out_move:
      {
	switch( state)
	{
	  case begin:
	  {
	    std::list<Knock_Out_Move> moves = game.board.get_knock_out_moves();
	    std::list<Knock_Out_Move>::iterator move;
	    for( move = moves.begin(); move != moves.end(); ++move )
	    {
	      ret.push_back( move->from );
	    }
	  }
	  break;
	  case move_from:
	  {
	    std::list<Knock_Out_Move> moves = game.board.get_knock_out_moves( from );
	    std::list<Knock_Out_Move>::iterator move;
	    for( move = moves.begin(); move != moves.end(); ++move )
	    {
	      ret.push_back( move->to );
	    }
	  }
	  break;
	  case move_dest:
	  {
	    ret.push_back( to.top() ); // only another knock out with this stone is possible
	  }
	  break;
	  default: assert( false );
	}
      }
      break;
      case Move::set_move:
      {
	ret = game.board.get_empty_fields();
      }
      break;
      case Move::remove:
      {
	ret = game.board.get_removable_fields();
      }
      break;
      case Move::no_move:
      case Move::finish_move:
	break;
    }
    return ret;
    */
  }

  Field_Pos Sequence_Generator::get_selected_pos()
  {
    /*
    switch( state )
    {
      case move_from:
	return from;
      case begin:
      case move_dest:
      case stone_picked:	// I know only selected stone type
      case stone_set:
      case move_finished:
	return Field_Pos();
    }
    */
    assert(false);
    return Field_Pos();
  }

  void Sequence_Generator::reset()
  {
    if( auto_undo )
      sequence.undo_sequence(game);
    sequence.clear();
    while( to.size() > 0 ) 
      to.pop();
    state = begin;
    auto_undo = true;
  }

  // ----------------------------------------------------------------------------
  // Generic_Mouse_Input
  // ----------------------------------------------------------------------------

  Generic_Mouse_Input::Generic_Mouse_Input( Game &game )
    : sequence_generator(game), game(game)
  {
  }

  Player_Input::Player_State Generic_Mouse_Input::determine_move() throw(Exception)
  {
    if( sequence_generator.is_finished() )
      return finished;

    init_mouse_input();
    return wait_for_event;
  }

  Move_Sequence Generic_Mouse_Input::get_move()
  {
    Move_Sequence sequence = sequence_generator.get_sequence();
    disable_mouse_input();
    sequence_generator.reset();
    return sequence;
  }

  // ----------------------------------------------------------------------------
  // Standard_Coordinate_Translator
  // ----------------------------------------------------------------------------

  Standard_Coordinate_Translator::Standard_Coordinate_Translator( const Board &board )
    : orig_board(board)
  {    
  }
  std::string Standard_Coordinate_Translator::get_field_name( Field_Pos pos )
  {
    std::string name;
    int first_x;
    for( first_x = 0; first_x < orig_board.get_x_size(); ++first_x )
      if( !Board::is_removed(orig_board.field[first_x][pos.y]) )
	break;

    name += 'a' + pos.y;
    name += holtz::long_to_string( pos.x - first_x + 1 );
    return name;
  }

  Field_Pos Standard_Coordinate_Translator::get_field_pos ( std::string name )
  {
    Field_Pos pos;

    if( name.size() > 1 )
    {
      if( (name[0] >= 'a') && (name[0] <= 'z') )
      {
	std::string num_string = name.substr(1);
	std::pair<long,unsigned/*digits*/> num = holtz::string_to_long( num_string );
	if( num.second == num_string.size() )
	{
	  if( num.first > 0 )
	  {
	    pos.x = num.first - 1;
	    pos.y = name[0] - 'a';

	    int first_x;
	    for( first_x = 0; first_x < orig_board.get_x_size(); ++first_x )
	      if( !Board::is_removed(orig_board.field[first_x][pos.y]) )
		break;
	    pos.x += first_x;
	  }
	}
      }
    }
    return pos;
  }

  Coordinate_Translator *Standard_Coordinate_Translator::clone()
  {
    return new Standard_Coordinate_Translator( orig_board );
  }

  void recursive_find_optimal_solution( Game &game, std::list<Stones::Stone_Type> &remaining_stones,
					Game &best_game, int &best_score, int &cnt )
  {
    std::list<Field_Pos> positions = game.current_player->board.get_empty_fields();
    if( remaining_stones.size() == 0 ) 
      {
	// check score
	int score = game.get_max_score();
	if( score > best_score )
	  {
	    best_game = game;
	    best_score = score;
	    std::cout << "New best score: " << score << std::endl;
	  }
	return;
      }
    cnt++;
    if( cnt % 1000000 == 0 ) std::cout << cnt << std::endl;
    
    Stones::Stone_Type current_stone = remaining_stones.back();
    remaining_stones.pop_back();
    std::list<Field_Pos>::iterator it;
    for( it=positions.begin(); it!=positions.end(); ++it )
      {
	Field_Pos pos = *it;
	Move_Sequence move_sequence;
	move_sequence.add_move(new Set_Move(pos,current_stone));
	move_sequence.add_move(new Finish_Move());
	move_sequence.do_sequence(game);
	recursive_find_optimal_solution( game, remaining_stones, best_game, best_score, cnt );
	move_sequence.undo_sequence(game);
      }
    remaining_stones.push_back(current_stone);
  }

  void recursive_find_optimal_solution2( Game &game, std::list<Stones::Stone_Type> &remaining_stones,
					Game &best_game, int &best_score, int &cnt )
  {
    std::list<Field_Pos> positions = game.current_player->board.get_empty_fields();
    if( remaining_stones.size() == 0 ) 
      {
	// check score
	int score = game.get_max_score();
	if( score > best_score )
	  {
	    best_game = game;
	    best_score = score;
	    std::cout << "New best score: " << score << std::endl;
	  }
	return;
      }
    cnt++;
    if( cnt % 1000000 == 0 ) std::cout << cnt << std::endl;
    
    Stones::Stone_Type current_stone = remaining_stones.back();
    remaining_stones.pop_back();
    std::list<Field_Pos>::iterator it;
    for( it=positions.begin(); it!=positions.end(); ++it )
      {
	Field_Pos pos = *it;
	game.current_player->board.field[pos.x][pos.y] = Field_State_Type(current_stone);
	recursive_find_optimal_solution2( game, remaining_stones, best_game, best_score, cnt );
	game.current_player->board.field[pos.x][pos.y] = field_empty;
      }
    remaining_stones.push_back(current_stone);
  }

  class Predicate
  {
  public:
    Predicate( Ruleset *ruleset ) : ruleset(ruleset) {}

    bool operator() (Stones::Stone_Type lhs, Stones::Stone_Type rhs) {
      boost::tuple<int,int,int> lhs_nums = ruleset->get_numbers(lhs);
      boost::tuple<int,int,int> rhs_nums = ruleset->get_numbers(rhs);
//       if( (lhs_nums.get<0>() != 9 || lhs_nums.get<1>() != 7 || lhs_nums.get<2>() != 8) &&
// 	  (rhs_nums.get<0>() != 9 || rhs_nums.get<1>() != 7 || rhs_nums.get<2>() != 8) )
// 	return rand() % 2;	// random sorting
      bool res = max(lhs_nums.get<0>(),lhs_nums.get<1>(),lhs_nums.get<2>()) <
	max(rhs_nums.get<0>(),rhs_nums.get<1>(),rhs_nums.get<2>());
//       std::cout << "Predicate: " << res << "==" 
// 		<< lhs_nums.get<0>() << "/" << lhs_nums.get<1>() << "/" << lhs_nums.get<2>() << " < " 
// 		<< rhs_nums.get<0>() << "/" << rhs_nums.get<1>() << "/" << rhs_nums.get<2>() 
// 		<< std::endl;
      return res;
    }
  private:
    Ruleset *ruleset;
  };

  struct Recursion_State
  {
    Game game;
    std::list<Stones::Stone_Type> remaining_stones;
    Game best_game; int best_score; long cnt; long bound_cnt;
    std::list<Field_Pos> empty_fields;
    std::vector<std::map<int/*num*/, unsigned /*stones*/> > stones_available;
    Recursion_State(Game game, std::list<Stones::Stone_Type> remaining_stones)
      : game(game), remaining_stones(remaining_stones), 
	best_game(game), best_score(-1), cnt(0), bound_cnt(0)
    {
      // sort remaining stones from low to high numbers
      this->remaining_stones.sort(Predicate(game.ruleset));
      // get empty fields
      empty_fields = game.current_player->board.get_empty_fields();
      // determine available stone counts
      stones_available.resize(3);
      std::list<Stones::Stone_Type>::iterator it;
      for(it = this->remaining_stones.begin(); it != this->remaining_stones.end(); ++it )
	{
	  Stones::Stone_Type stone_type = *it;
	  boost::tuple<int,int,int> nums = game.ruleset->get_numbers(stone_type);
	  std::cout << nums.get<0>() << "/" << nums.get<1>() << "/" << nums.get<2>() << std::endl;
	  if( does_include( stones_available[0], nums.get<0>() ) )
	    ++stones_available[0][nums.get<0>()];
	  else
	    stones_available[0][nums.get<0>()] = 1;
	  if( does_include( stones_available[1], nums.get<1>() ) )
	    ++stones_available[1][nums.get<1>()];
	  else
	    stones_available[1][nums.get<1>()] = 1;
	  if( does_include( stones_available[2], nums.get<2>() ) )
	    ++stones_available[2][nums.get<2>()];
	  else
	    stones_available[2][nums.get<2>()] = 1;
	}
    }
    void print()
    {
      std::cout << "Debug Print:" << std::endl;
      std::list<Stones::Stone_Type>::iterator it;
      for(it = remaining_stones.begin(); it != remaining_stones.end(); ++it )
	{
	  Stones::Stone_Type stone_type = *it;
	  boost::tuple<int,int,int> nums = game.ruleset->get_numbers(stone_type);
	  std::cout << "  " << nums.get<0>() << "/" << nums.get<1>() << "/" << nums.get<2>() << std::endl;
	}
    }
  };

  void recursive_find_optimal_solution3( Recursion_State &state, int level )
  {
    if( state.remaining_stones.size() == 0 || state.empty_fields.size() == 0 ) 
      {
	// check score
	int score = state.game.get_max_score();
	if( score > state.best_score )
	  {
	    state.best_game = state.game;
	    state.best_score = score;
	    std::cout << "New best score: " << score << std::endl;
	    state.best_game.print();
	  }
	return;
      }
    int max_score = state.game.get_max_score(0,&state.stones_available);
    if( max_score <= state.best_score )
      {
	++state.bound_cnt;
	if( state.bound_cnt % 10000000 == 0 ) std::cout << "bounds:" << state.bound_cnt << std::endl;
	return;						     // bound
      }

    state.cnt++;
    if( state.cnt % 10000000 == 0 ) std::cout << "branches:" << state.cnt << std::endl;
    
    Stones::Stone_Type current_stone = state.remaining_stones.back();
    boost::tuple<int,int,int> nums = state.game.ruleset->get_numbers(current_stone);
    if( level < 2 )
      {
	for( int i=0; i<level; ++i ) std::cout << "  ";
	std::cout << "Level " << level << " tries: " 
		  << nums.get<0>() << "/" << nums.get<1>() << "/" << nums.get<2>() << std::endl;
      }
    state.remaining_stones.pop_back();
    assert(state.stones_available[0][nums.get<0>()] > 0);
    assert(state.stones_available[1][nums.get<1>()] > 0);
    assert(state.stones_available[2][nums.get<2>()] > 0);
    --state.stones_available[0][nums.get<0>()];
    --state.stones_available[1][nums.get<1>()];
    --state.stones_available[2][nums.get<2>()];
    int level_cnt = 0;
    unsigned num_fields = state.empty_fields.size(); 
    for( unsigned i=0; i<num_fields; ++i )
      {
	unsigned save_bound_cnt = state.bound_cnt;
	unsigned save_cnt = state.cnt;
	Field_Pos pos = state.empty_fields.front();
	state.empty_fields.pop_front();
	state.game.current_player->board.field[pos.x][pos.y] = Field_State_Type(current_stone);
	recursive_find_optimal_solution3( state, level+1 );
	//int sub_max_score = state.game.get_max_score(0,&state.stones_available);  // debug code
	state.game.current_player->board.field[pos.x][pos.y] = field_empty;
	state.empty_fields.push_back(pos);
	level_cnt++;
	if( level < 2 )
	  {
	    for( int i=0; i<level; ++i ) std::cout << "  ";
	    std::cout << "Level " << level << ": " << level_cnt << "/" << num_fields 
		      << " bounds: " << (state.bound_cnt - save_bound_cnt) 
		      << " cnt: " << (state.cnt - save_cnt) 
		      << " x: " << pos.x << " y: " << pos.y
	      //		      << " max_score: " << sub_max_score
		      << std::endl;
	  }
      }
    state.remaining_stones.push_back(current_stone);
    ++state.stones_available[0][nums.get<0>()];
    ++state.stones_available[1][nums.get<1>()];
    ++state.stones_available[2][nums.get<2>()];
  }

  void recursive_find_optimal_solution4( Recursion_State &state, int level )
  {
    if( state.remaining_stones.size() == 0 || state.empty_fields.size() == 0 ) 
      {
	// check score
	int score = state.game.get_max_score();
	if( score > state.best_score )
	  {
	    state.best_game = state.game;
	    state.best_score = score;
	    std::cout << "New best score: " << score << std::endl;
	    state.best_game.print();
	  }
	return;
      }

    state.cnt++;
    if( state.cnt % 10000000 == 0 ) std::cout << "branches:" << state.cnt << std::endl;
    
    Stones::Stone_Type current_stone = state.remaining_stones.back();
    boost::tuple<int,int,int> nums = state.game.ruleset->get_numbers(current_stone);
    if( level < 2 )
      {
	for( int i=0; i<level; ++i ) std::cout << "  ";
	std::cout << "Level " << level << " tries: " 
		  << nums.get<0>() << "/" << nums.get<1>() << "/" << nums.get<2>() << std::endl;
      }
    state.remaining_stones.pop_back();
    assert(state.stones_available[0][nums.get<0>()] > 0);
    assert(state.stones_available[1][nums.get<1>()] > 0);
    assert(state.stones_available[2][nums.get<2>()] > 0);
    --state.stones_available[0][nums.get<0>()];
    --state.stones_available[1][nums.get<1>()];
    --state.stones_available[2][nums.get<2>()];

    // scan fields
    std::multimap<int/*max_score*/,Field_Pos> positions;    
    std::list<Field_Pos>::iterator it;
    for( it=state.empty_fields.begin(); it!=state.empty_fields.end(); ++it )
      {
	Field_Pos pos = *it;
	state.game.current_player->board.field[pos.x][pos.y] = Field_State_Type(current_stone);
	int sub_max_score = state.game.get_max_score(0,&state.stones_available);
	if( sub_max_score > state.best_score )
	  positions.insert( std::make_pair(sub_max_score,pos) );
	else
	  ++state.bound_cnt;
	state.game.current_player->board.field[pos.x][pos.y] = field_empty;
      }
    
    // traverse fields
    int level_cnt = 0;
    unsigned num_fields = positions.size();
    std::multimap<int/*max_score*/,Field_Pos>::reverse_iterator prio_it;
    for( prio_it = positions.rbegin(); prio_it != positions.rend(); ++prio_it )
      {
	unsigned save_bound_cnt = state.bound_cnt;
	unsigned save_cnt = state.cnt;
	Field_Pos pos = prio_it->second;
	state.empty_fields.erase(find(state.empty_fields.begin(),state.empty_fields.end(),pos));
	state.game.current_player->board.field[pos.x][pos.y] = Field_State_Type(current_stone);
	recursive_find_optimal_solution4( state, level+1 );
	state.game.current_player->board.field[pos.x][pos.y] = field_empty;
	state.empty_fields.push_back(pos);
	level_cnt++;
	if( level < 2 )
	  {
	    for( int i=0; i<level; ++i ) std::cout << "  ";
	    std::cout << "Level " << level << ": " << level_cnt << "/" << num_fields 
		      << " bounds: " << (state.bound_cnt - save_bound_cnt) 
		      << " cnt: " << (state.cnt - save_cnt) 
		      << std::endl;
	  }
      }
    state.remaining_stones.push_back(current_stone);
    ++state.stones_available[0][nums.get<0>()];
    ++state.stones_available[1][nums.get<1>()];
    ++state.stones_available[2][nums.get<2>()];
  }

  Game find_optimal_solution( Game game, Stones stones )
  {
    game.common_stones = stones;
    
    // fill remaining stones
    std::list<Stones::Stone_Type> remaining_stones;
    for( int idx = Stones::stone_begin; idx < Stones::stone_max_end; ++idx )
      {
	Stones::Stone_Type stone_type = Stones::Stone_Type(idx);
	if( !does_include(stones.stone_count,stone_type) ) continue;
	for( int i=0; i < stones.stone_count[stone_type]; ++i )
	  remaining_stones.push_back(stone_type);
      }

    Recursion_State state(game, remaining_stones);

    // enter recursion
    recursive_find_optimal_solution4( state, 0/*level*/ );
    std::cout << "Best score: " << state.best_score
	      << " bounds: " << state.bound_cnt
	      << " cnt: " << state.cnt
	      << std::endl;
    return state.best_game;
  }
}

void debug_print(relax::Player &player)
{ 
  std::cout << "id=" << player.id << "name=" << player.name << ",host=" << player.host << std::endl; 
}
