/*
 * zertz.cpp
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

#include "zertz.hpp"
#include <assert.h>

#include "util.hpp"

#include <ctype.h>
#include <sys/types.h>

namespace zertz
{
  using namespace holtz;

  // ----------------------------------------------------------------------------
  // Stones
  // ----------------------------------------------------------------------------

  Stones::Stones()
  {
    stone_count[white_stone] = 0;
    stone_count[grey_stone ] = 0;
    stone_count[black_stone] = 0;
  }

#ifndef __WXMSW__
  void Stones::print()
  {
    std::cout << std::endl;
    std::cout << stone_count[white_stone] << " " << stone_count[grey_stone] << " " 
	      << stone_count[black_stone] << std::endl;
  }
#endif

  void Stones::remove_stones()
  {
    stone_count.clear();
    stone_count[white_stone] = 0;
    stone_count[grey_stone ] = 0;
    stone_count[black_stone] = 0;
  }

  // ----------------------------------------------------------------------------
  // Common_Stones
  // ----------------------------------------------------------------------------

  Common_Stones::Common_Stones( Common_Stones::Common_Stones_Type type )
    : type(type)
  {
  }

  // ----------------------------------------------------------------------------
  // Basic_Common_Stones
  // ----------------------------------------------------------------------------

  Basic_Common_Stones::Basic_Common_Stones()
    : Common_Stones( basic )
  {
    stone_count[ Stones::white_stone ] = 5;
    stone_count[ Stones::grey_stone ]  = 7;
    stone_count[ Stones::black_stone ] = 9;
  }

  // ----------------------------------------------------------------------------
  // Standard_Common_Stones
  // ----------------------------------------------------------------------------

  Standard_Common_Stones::Standard_Common_Stones()
    : Common_Stones( standard )
  {
    stone_count[ Stones::white_stone ] =  6;
    stone_count[ Stones::grey_stone ]  =  8;
    stone_count[ Stones::black_stone ] = 10;
  }

  // ----------------------------------------------------------------------------
  // Custom_Common_Stones
  // ----------------------------------------------------------------------------

  Custom_Common_Stones::Custom_Common_Stones( int white_num, int grey_num, int black_num )
    : Common_Stones( custom )
  {
    stone_count[ Stones::white_stone ] = white_num;
    stone_count[ Stones::grey_stone ]  = grey_num;
    stone_count[ Stones::black_stone ] = black_num;
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
      input(input), outputs(outputs), is_active(false)
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

  Field_Iterator::Field_Iterator( Board *board )
    : board(board)
  {
    current_pos.x = 0;
    current_pos.y = 0;
  }

  Field_Iterator::Field_Iterator( Field_Pos pos, Board *board )
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
  bool Field_Iterator::is_valid_field()
  {
    if( board == 0 ) return false;
    if( current_pos.x < 0 ) return false;
    if( current_pos.y < 0 ) return false;
    if( current_pos.x >= board->get_x_size() ) return false;
    if( current_pos.y >= board->get_y_size() ) return false;

    return true;
  }

  Field_State_Type &Field_Iterator::operator*()
  {
    assert( is_valid_field() );

    return board->field[ current_pos.x ][ current_pos.y ];
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

  bool Board::is_knock_out_possible( Field_Iterator p1,
				     Field_Iterator p2,
				     Field_Iterator p3 )
  {
    if( Board::is_stone(*p2) )	// middle must be stone
    {
      if(( Board::is_empty(*p1) && Board::is_stone(*p3) ) ||
	 ( Board::is_empty(*p3) && Board::is_stone(*p1) ))
      {
	// check if p1,p2,p3 are three fields in a row
	Field_Iterator::Direction dir = 
	  Field_Iterator::get_far_direction( p1.get_pos(), p2.get_pos() );
	if( p1.Next(dir) != p2 )
	  return false;
	if( p2.Next(dir) != p3 )
	  return false;

	return true;	// move possible
      }
    }
    return false;
  }

  bool Board::is_knock_out_possible()
  {
    Field_Pos pos;
    Field_Iterator p1(this), p2(this), p3(this);
	
    for( pos.x = 0; pos.x < get_x_size(); ++pos.x )
    {
      for( pos.y = 0; pos.y < get_y_size(); ++pos.y )
      {
	p1.set_pos( pos );
	// ******************************************
	// search each pair of three fields in a row:

	// search: horizontal
	p2 = p1.Next_Right();
	p3 = p2.Next_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( is_knock_out_possible(p1,p2,p3) )
	    return true;
	}

	// search: bottom left
	p2 = p1.Next_Bottom_Left();
	p3 = p2.Next_Bottom_Left();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( is_knock_out_possible(p1,p2,p3) )
	    return true;
	}

	// search: bottom right
	p2 = p1.Next_Bottom_Right();
	p3 = p2.Next_Bottom_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( is_knock_out_possible(p1,p2,p3) )
	    return true;
	}
      }
    }
    return false;
  }
  bool Board::is_knock_out_possible( Field_Pos pos )
  {
    Field_Iterator p1( pos, this ), p2(this), p3(this);

    // search in any direction
    int dir;
    for( dir =  Field_Iterator::right;
	 dir <= Field_Iterator::bottom_right; ++dir )
    {
      p2 = p1.Next( Field_Iterator::Direction(dir) );
      p3 = p2.Next( Field_Iterator::Direction(dir) );
      if( p3.is_valid_field() )
      {
	assert( p2.is_valid_field() );
	if( is_stone(*p2) )	// middle must be stone
	{
	  if( is_empty(*p3) && is_stone(*p1) )
	    return true;	// move possible
	}
      }
    }
    return false;
  }

  std::pair<bool,Field_Pos> Board::is_knock_out_possible( Field_Pos from, Field_Pos to )
  {
    Field_Iterator p1(from,this), p2(this), p3(to,this);
    Field_Iterator::Direction dir = Field_Iterator::get_far_direction(from,to);

    p2 = p1.Next(dir);

    return std::pair<bool,Field_Pos>( is_knock_out_possible(p1,p2,p3), p2.get_pos() );
  }

  std::list<Knock_Out_Move> Board::get_knock_out_moves()
  {
    std::list<Knock_Out_Move> ret;

    Field_Pos pos;
    Field_Iterator p1(this), p2(this), p3(this);
	
    for( pos.x = 0; pos.x < get_x_size(); ++pos.x )
    {
      for( pos.y = 0; pos.y < get_y_size(); ++pos.y )
      {
	p1.set_pos( pos );
	// ******************************************
	// search each pair of three fields in a row:

	// search: horizontal
	p2 = p1.Next_Right();
	p3 = p2.Next_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      ret.push_back( Knock_Out_Move( p1.get_pos(), p2.get_pos(), p3.get_pos() ) );
	    else
	      ret.push_back( Knock_Out_Move( p3.get_pos(), p2.get_pos(), p1.get_pos() ) );
	  }
	}

	// search: bottom left
	p2 = p1.Next_Bottom_Left();
	p3 = p2.Next_Bottom_Left();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      ret.push_back( Knock_Out_Move( p1.get_pos(), p2.get_pos(), p3.get_pos() ) );
	    else
	      ret.push_back( Knock_Out_Move( p3.get_pos(), p2.get_pos(), p1.get_pos() ) );
	  }
	}

	// search: bottom right
	p2 = p1.Next_Bottom_Right();
	p3 = p2.Next_Bottom_Right();
	if( p3.is_valid_field() )
	{
	  assert( p2.is_valid_field() );
	  if( Board::is_knock_out_possible(p1,p2,p3) )
	  {
	    if( Board::is_stone( *p1 ) )
	      ret.push_back( Knock_Out_Move( p1.get_pos(), p2.get_pos(), p3.get_pos() ) );
	    else
	      ret.push_back( Knock_Out_Move( p3.get_pos(), p2.get_pos(), p1.get_pos() ) );
	  }
	}
      }
    }
    return ret;
  }

  std::list<Knock_Out_Move> Board::get_knock_out_moves( Field_Pos p1 )
  {
    std::list<Knock_Out_Move> ret;
    Field_Iterator from(p1, this), over(this), to(this);
    // search for knock out possibility in any direction
    int dir;
    for( dir =  Field_Iterator::right;
	 dir <= Field_Iterator::bottom_right; ++dir )
    {
      over = from.Next( Field_Iterator::Direction(dir) );
      to   = over.Next( Field_Iterator::Direction(dir) );
      if( to.is_valid_field() )
      {
	assert( over.is_valid_field() );
	if( Board::is_stone(*over) )	// middle must be stone
	{
	  if( Board::is_empty(*to) && Board::is_stone(*from) )
	  {
	    ret.push_back( Knock_Out_Move( from.get_pos(), over.get_pos(), to.get_pos() ) );
	  }
	}
      }
    }
    return ret;
  }

  bool Board::is_removable( Field_Pos pos )
  {
    Field_Iterator p1( pos, this ), next(this);
    bool is_last_removed;

    assert( p1.is_valid_field() );
    if( !is_empty(*p1) ) return false;

    // check whether predecessor of direction right is removed
    next = p1.Next_Bottom_Right();
    if( !next.is_valid_field() || is_removed(*next) )
      is_last_removed = true;
    else
      is_last_removed = false;

    // check whether any two adjacent fields are removed
    // search in any direction
    for( int dir = Field_Iterator::right;
	 dir <= Field_Iterator::bottom_right; ++dir )
    {
      next = p1.Next( Field_Iterator::Direction(dir) );
      if( !next.is_valid_field() || is_removed(*next) )
      {
	if( is_last_removed )
	  return true;
	else
	  is_last_removed = true;
      }
      else
	is_last_removed = false;
    }
    return false;
  }

  bool Board::is_any_field_removable()
  {
    Field_Pos pos;
	
    for( pos.x = 0; pos.x < get_x_size(); ++pos.x )
    {
      for( pos.y = 0; pos.y < get_y_size(); ++pos.y )
      {
	if( is_removable( pos ) )
	  return true;
      }
    }
    return false;
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
  std::list<Field_Pos> Board::get_removable_fields()
  {
    std::list<Field_Pos> ret;
    Field_Pos pos;
	
    for( pos.x = 0; pos.x < get_x_size(); ++pos.x )
    {
      for( pos.y = 0; pos.y < get_y_size(); ++pos.y )
      {
	if( is_removable( pos ) )
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
    return *this;
  }

  Ruleset::~Ruleset()
  {
    delete win_condition;
    delete coordinate_translator;
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

  std::list<std::pair<Move_Sequence,int/*player index*/> > Variant_Tree::get_current_variant_moves() const
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
    : ruleset(_ruleset.clone()), 
      board(ruleset->board), 
      common_stones(ruleset->common_stones), 
      win_condition(ruleset->win_condition),
      coordinate_translator(ruleset->coordinate_translator),
      undo_possible(ruleset->undo_possible),
      ref_counter(new Ref_Counter()),
      winner_player_index(-1)
  {
    current_player = players.begin();
    current_player_index = 0;
    prev_player = players.end();
    prev_player_index = -1;
  }

  Game::Game( const Game &game )
    : board(game.board)
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

    current_player_index = game.current_player_index;
    current_player	 = get_player( current_player_index );
    prev_player_index    = game.prev_player_index;
    prev_player		 = get_player( prev_player_index );

    board		  = game.board;
    common_stones 	  = game.common_stones;
    win_condition 	  = game.win_condition;
    coordinate_translator = game.coordinate_translator;
    undo_possible 	  = game.undo_possible;
    variant_tree	  = game.variant_tree;

    ruleset 		  = game.ruleset;
    ref_counter 	  = game.ref_counter;
    ++ref_counter->cnt;

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
      return finished;
    }

    do
    {
      Player_Input::Player_State state = current_player->input->determine_move();
      switch( state )
      {
	case Player_Input::finished:
	{
	  // get move
	  sequence = current_player->input->get_move();

	  // report move
	  for( output =  current_player->outputs.begin();
	       output != current_player->outputs.end();
	       ++output )
	    (*output)->report_move( sequence );

	  // calculate average time
	  long time = current_player->input->get_used_time();
	  current_player->total_time += time;
	  current_player->average_time 
	    = current_player->average_time * current_player->num_measures + time;
	  ++current_player->num_measures;
	  current_player->average_time /= current_player->num_measures;

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
	    return finished;
	  }

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
    board = ruleset->board;
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
      i->stones.remove_stones(); 
    }

    current_player = players.begin();
    current_player_index = 0;
    prev_player    = players.end();
    prev_player_index = -1;
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
	// rescue captured stones
	Stones player_stones = player->stones;
	// copy player
	assert( player_src != new_players.end() );
	*player = *player_src;

	player->stones = player_stones;
      }
    }
    else
    {
      players = new_players;

      // remove player stones
      for( player = players.begin(); player != players.end(); ++player )
      {
	player->stones = Stones();
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

  //! game dependent function
  bool Game::choose_next_player()		// next player is current player
  {
    prev_player = current_player;
    prev_player_index = current_player_index;

    // check if still stones are there
    bool common_stones_avail;
    if( common_stones.stone_count[ Stones::white_stone ] +
	common_stones.stone_count[ Stones::grey_stone ] +
	common_stones.stone_count[ Stones::black_stone ] == 0 )
      common_stones_avail = false;
    else
      common_stones_avail = true;

    // change player
    if( players.size() )
    {
      current_player->is_active = false;
      bool next_player; unsigned players_skipped = 0;
      do
      {
	++current_player; 
	++current_player_index;
	if( current_player == players.end() )		 // if last player
	{
	  current_player = players.begin();		 // cycle to first player
	  current_player_index = 0;
	}
	next_player = false;
	if( !common_stones_avail )
	{
	  // assert that player has stones if no knock out move is possible
	  if( !board.is_knock_out_possible() )
	  {
	    if( current_player->stones.stone_count[ Stones::white_stone ] +
		current_player->stones.stone_count[ Stones::grey_stone  ] +
		current_player->stones.stone_count[ Stones::black_stone ] == 0 )
	    {
	      next_player = true; // player can't move
	      ++players_skipped;
	      if( players_skipped >= players.size() )
		return false; // no player may move
	    }
	  }
	}
      }while( next_player );
      current_player->is_active = true;
    }
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
    return true;
  }

  void Game::do_move( const Move_Sequence &sequence )
  {
    int player_index = current_player_index;
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
    return true;
  }

  // **********************
  // get_num_possible_moves

  // counts knock out moves beginning with from->over->to
  int get_num_knock_out_moves( Game &game, 
			       Move_Sequence &current_move_sequence, 
			       Field_Iterator from, Field_Iterator over, Field_Iterator to )
  {
    int num_possible_moves = 0;

    Move *move = new Knock_Out_Move( from.get_pos(), over.get_pos(), to.get_pos() );
    if( move->check_move( game ) )
    {
      bool ok = current_move_sequence.add_move( game, move );
      if( ok )
      {
	move->do_move( game );
	from = to;			// destination field is start field for further moves

	bool knock_out = false;
	// search in any direction
	int dir;
	for( dir =  Field_Iterator::right;
	     dir <= Field_Iterator::bottom_right; ++dir )
	{
	  over = from.Next( Field_Iterator::Direction(dir) );
	  to   = over.Next( Field_Iterator::Direction(dir) );
	  if( to.is_valid_field() )
	  {
	    assert( over.is_valid_field() );
	    if( Board::is_stone(*over) )	// middle must be stone
	    {
	      if( Board::is_empty(*to) && Board::is_stone(*from) )
	      {
		knock_out = true;
		get_num_knock_out_moves( game, current_move_sequence, from, over, to );
				// recursive move adding
	      }
	    }
	  }
	}
	if( !knock_out )		// if no further knock out possible
	{
	  // add finish move
	  move = new Finish_Move();
	  if( move->check_move( game ) )
	  {
	    ok = current_move_sequence.add_move(game,move);
	    if( ok )
	    {
	      move->do_move(game);
      
	      num_possible_moves++;
	      
	      current_move_sequence.undo_last_move( game );
	    }
	  }
	}
	current_move_sequence.undo_last_move( game );
      }
    }
    return num_possible_moves;
  }

  // get number of set-remove moves beginning with a special set move
  int get_num_set_moves( Game &game, Field_Iterator to, Stones::Stone_Type type )
  {
    int num_possible_moves = 0;
    Move_Sequence sequence;

    Move *move = new Set_Move( to.get_pos(), type );
    if( move->check_move( game ) )
    {
      bool ok = sequence.add_move( game, move );
      if( ok )
      {
	move->do_move( game );

	bool any_removable = false;
	Field_Pos pos;
	for( pos.y=0; pos.y < game.board.get_y_size(); ++pos.y )
	  for( pos.x=0; pos.x < game.board.get_x_size(); ++pos.x )
	  {
	    if( game.board.is_removable(pos) )
	    {
	      any_removable = true;

	      move = new Remove( pos );
	      if( move->check_move( game ) )
	      {
		ok = sequence.add_move( game, move );
		if( ok )
		{
		  move->do_move( game );
	
		  move = new Finish_Move();
		  if( move->check_move( game ) )
		  {
		    ok = sequence.add_move( game, move );
		    if( ok )
		    {
		      move->do_move( game );
	
		      num_possible_moves++;

		      sequence.undo_last_move( game );
		    }
		  }
		  sequence.undo_last_move( game );
		}
	      }
	    }
	  }
    
	if( !any_removable )	// is it already a valid set-move as there is no field removable?
	{
	  Move *move = new Finish_Move();
	  if( move->check_move( game ) )
	  {
	    bool ok = sequence.add_move( game, move );
	    if( ok )
	    {
	      move->do_move( game );

	      num_possible_moves++;

	      sequence.undo_last_move( game );
	    }
	  }
	}

	sequence.undo_last_move( game );
      }
    }
    return num_possible_moves;
  }

  int Game::get_num_possible_moves() // number of possible moves in current situation
  {
    int num_possible_moves = 0;
    Move_Sequence current_sequence;
    std::list<Move_Sequence> possible_moves;
    Game save_game = *this;
    
    if( save_game.board.is_knock_out_possible() )
    {
      Field_Pos pos;
      Field_Iterator p1(&save_game.board), p2(&save_game.board), p3(&save_game.board);

      for( pos.y=0; pos.y < save_game.board.get_y_size(); ++pos.y )
	for( pos.x=0; pos.x < save_game.board.get_x_size(); ++pos.x )
	{
	  p1.set_pos( pos );
	  // ******************************************
	  // search each pair of three fields in a row:
	  
	  // search: horizontal
	  p2 = p1.Next_Right();
	  p3 = p2.Next_Right();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p1, p2, p3 );
	      else
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }

	  // search: bottom left
	  p2 = p1.Next_Bottom_Left();
	  p3 = p2.Next_Bottom_Left();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p1, p2, p3 );
	      else
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }
	  
	  // search: bottom right
	  p2 = p1.Next_Bottom_Right();
	  p3 = p2.Next_Bottom_Right();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p1, p2, p3 );
	      else
		num_possible_moves += get_num_knock_out_moves( save_game, current_sequence, 
							       p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }
	}
    }
    else
    {
      Field_Pos pos;
      Field_Iterator p1(&save_game.board);
     
      for( pos.y=0; pos.y < save_game.board.get_y_size(); ++pos.y )
	for( pos.x=0; pos.x < save_game.board.get_x_size(); ++pos.x )
	{
	  p1.set_pos( pos );
	  if( *p1 == field_empty )
	  {
	    bool any = false;
	    for( int type = Stones::white_stone; type <= Stones::black_stone; ++type )
	    {
	      if( save_game.common_stones.stone_count[ Stones::Stone_Type(type) ] > 0 )
	      {
		num_possible_moves += get_num_set_moves( save_game, p1, Stones::Stone_Type(type) );
		any = true;
	      }
	    }
	    if( !any )
	    {
	      for( int type = Stones::white_stone; type <= Stones::black_stone; ++type )
	      {
		if( save_game.current_player->stones.stone_count[ Stones::Stone_Type(type) ] > 0 )
		  num_possible_moves += get_num_set_moves( save_game, p1, Stones::Stone_Type(type) );
	      }
	    }
	  }
	}
    }
    return num_possible_moves;
  }

  // ******************
  // get_possible_moves

  // fills possible_moves list with knock out moves beginning with from->over->to
  void get_knock_out_moves( Game &game, std::list<Move_Sequence> &possible_moves,
			    Move_Sequence &current_move_sequence, 
			    Field_Iterator from, Field_Iterator over, Field_Iterator to )
  {
    Move *move = new Knock_Out_Move( from.get_pos(), over.get_pos(), to.get_pos() );
    if( move->check_move( game ) )
    {
      bool ok = current_move_sequence.add_move( game, move );
      if( ok )
      {
	move->do_move( game );
	from = to;			// destination field is start field for further moves

	bool knock_out = false;
	// search in any direction
	int dir;
	for( dir =  Field_Iterator::right;
	     dir <= Field_Iterator::bottom_right; ++dir )
	{
	  over = from.Next( Field_Iterator::Direction(dir) );
	  to   = over.Next( Field_Iterator::Direction(dir) );
	  if( to.is_valid_field() )
	  {
	    assert( over.is_valid_field() );
	    if( Board::is_stone(*over) )	// middle must be stone
	    {
	      if( Board::is_empty(*to) && Board::is_stone(*from) )
	      {
		knock_out = true;
		get_knock_out_moves( game, possible_moves, current_move_sequence, from, over, to );
				// recursive move adding
	      }
	    }
	  }
	}
	if( !knock_out )		// if no further knock out possible
	{
	  // add finish move
	  move = new Finish_Move();
	  if( move->check_move( game ) )
	  {
	    ok = current_move_sequence.add_move(game,move);
	    if( ok )
	    {
	      move->do_move(game);
      
	      possible_moves.push_back( current_move_sequence.clone() );
	      
	      current_move_sequence.undo_last_move( game );
	    }
	  }
	}
	current_move_sequence.undo_last_move( game );
      }
    }
  }

  // fills possible_moves list with set-remove moves beginning with a special set move
  void get_set_moves( Game &game, std::list<Move_Sequence> &possible_moves, 
		      Field_Iterator to, Stones::Stone_Type type )
  {
    Move_Sequence sequence;

    Move *move = new Set_Move( to.get_pos(), type );
    if( move->check_move( game ) )
    {
      bool ok = sequence.add_move( game, move );
      if( ok )
      {
	move->do_move( game );

	bool any_removable = false;
	Field_Pos pos;
	for( pos.y=0; pos.y < game.board.get_y_size(); ++pos.y )
	  for( pos.x=0; pos.x < game.board.get_x_size(); ++pos.x )
	  {
	    if( game.board.is_removable(pos) )
	    {
	      any_removable = true;

	      move = new Remove( pos );
	      if( move->check_move( game ) )
	      {
		ok = sequence.add_move( game, move );
		if( ok )
		{
		  move->do_move( game );
	
		  move = new Finish_Move();
		  if( move->check_move( game ) )
		  {
		    ok = sequence.add_move( game, move );
		    if( ok )
		    {
		      move->do_move( game );
	
		      possible_moves.push_back( sequence.clone() );

		      sequence.undo_last_move( game );
		    }
		  }
		  sequence.undo_last_move( game );
		}
	      }
	    }
	  }
    
	if( !any_removable )	// is it already a valid set-move as there is no field removable?
	{
	  Move *move = new Finish_Move();
	  if( move->check_move( game ) )
	  {
	    bool ok = sequence.add_move( game, move );
	    if( ok )
	    {
	      move->do_move( game );

	      possible_moves.push_back( sequence.clone() );

	      sequence.undo_last_move( game );
	    }
	  }
	}

	sequence.undo_last_move( game );
      }
    }
  }

  std::list<Move_Sequence> Game::get_possible_moves() // get possible moves in situation
  {
    Move_Sequence current_sequence;
    std::list<Move_Sequence> possible_moves;
    Game save_game = *this;
    
    if( save_game.board.is_knock_out_possible() )
    {
      Field_Pos pos;
      Field_Iterator p1(&save_game.board), p2(&save_game.board), p3(&save_game.board);

      for( pos.y=0; pos.y < save_game.board.get_y_size(); ++pos.y )
	for( pos.x=0; pos.x < save_game.board.get_x_size(); ++pos.x )
	{
	  p1.set_pos( pos );
	  // ******************************************
	  // search each pair of three fields in a row:
	  
	  // search: horizontal
	  p2 = p1.Next_Right();
	  p3 = p2.Next_Right();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		get_knock_out_moves( save_game, possible_moves, current_sequence, p1, p2, p3 );
	      else
		get_knock_out_moves( save_game, possible_moves, current_sequence, p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }

	  // search: bottom left
	  p2 = p1.Next_Bottom_Left();
	  p3 = p2.Next_Bottom_Left();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		get_knock_out_moves( save_game, possible_moves, current_sequence, p1, p2, p3 );
	      else
		get_knock_out_moves( save_game, possible_moves, current_sequence, p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }
	  
	  // search: bottom right
	  p2 = p1.Next_Bottom_Right();
	  p3 = p2.Next_Bottom_Right();
	  if( p3.is_valid_field() )
	  {
	    assert( p2.is_valid_field() );
	    if( Board::is_knock_out_possible(p1,p2,p3) )
	    {
	      if( Board::is_stone( *p1 ) )
		get_knock_out_moves( save_game, possible_moves, current_sequence, p1, p2, p3 );
	      else
		get_knock_out_moves( save_game, possible_moves, current_sequence, p3, p2, p1 );
	      assert( current_sequence.is_empty() );
	    }
	  }
	}
    }
    else
    {
      Field_Pos pos;
      Field_Iterator p1(&save_game.board);
     
      for( pos.y=0; pos.y < save_game.board.get_y_size(); ++pos.y )
	for( pos.x=0; pos.x < save_game.board.get_x_size(); ++pos.x )
	{
	  p1.set_pos( pos );
	  if( *p1 == field_empty )
	  {
	    bool any = false;
	    for( int type = Stones::white_stone; type <= Stones::black_stone; ++type )
	    {
	      if( save_game.common_stones.stone_count[ Stones::Stone_Type(type) ] > 0 )
	      {
		get_set_moves( save_game, possible_moves, p1, Stones::Stone_Type(type) );
		any = true;
	      }
	    }
	    if( !any )
	    {
	      for( int type = Stones::white_stone; type <= Stones::black_stone; ++type )
	      {
		if( save_game.current_player->stones.stone_count[ Stones::Stone_Type(type) ] > 0 )
		  get_set_moves( save_game, possible_moves, p1, Stones::Stone_Type(type) );
	      }
	    }
	  }
	}
    }

    return possible_moves;
  }

  // get moves played since start
  std::list<std::pair<Move_Sequence,int/*player index*/> > Game::get_played_moves() 
  {
    return variant_tree.get_current_variant_moves();
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
  // Knock_Out_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Knock_Out_Move::get_type() const
  {
    return knock_out_move;
  }

  void Knock_Out_Move::do_move( Game &game )
  {
    assert( check_move(game) );
    Field_State_Type stone = game.board.field[from.x][from.y];
    knocked_stone	   = game.board.field[over.x][over.y];
    assert( Board::is_stone( knocked_stone ) );
    assert( game.current_player->stones.stone_count[ Stones::Stone_Type(knocked_stone) ] >= 0 );
    ++game.current_player->stones.stone_count[ Stones::Stone_Type(knocked_stone) ];

    game.board.field[from.x][from.y] = field_empty;
    game.board.field[over.x][over.y] = field_empty;
    game.board.field[  to.x][  to.y] = stone;    
  }
  void Knock_Out_Move::undo_move( Game &game )
  {
    Field_State_Type stone = game.board.field[to.x][to.y];
    assert( game.current_player->stones.stone_count[ Stones::Stone_Type(knocked_stone) ] > 0 );
    --game.current_player->stones.stone_count[ Stones::Stone_Type(knocked_stone) ];
    game.board.field[from.x][from.y] = stone;
    game.board.field[over.x][over.y] = knocked_stone;
    game.board.field[  to.x][  to.y] = field_empty;    
  }

  // true: move ok
  bool Knock_Out_Move::check_move( Game &game ) const 
  {
    if( !Board::is_stone( game.board.field[from.x][from.y] ) ) return false;
    if( !Board::is_stone( game.board.field[over.x][over.y] ) ) return false;
    if( !Board::is_empty( game.board.field[  to.x][  to.y] ) ) return false;

    if( !Field_Iterator::is_connected(from,over) ) return false;
    if( !Field_Iterator::is_connected(over,to) ) return false;
    if( Field_Iterator::get_direction(from,over) !=
	Field_Iterator::get_direction(over,to) ) return false;

    return true;
  }
  // true: type ok
  bool Knock_Out_Move::check_previous_move( Game &, Move *move ) const 
  {
    if( !move )
      return true;

    if( move->get_type() == knock_out_move )
    {
      Knock_Out_Move *knock_move = dynamic_cast<Knock_Out_Move *>(move);
      if( knock_move->to == from )
	return true;
    }

    return false;
  }
  bool Knock_Out_Move::may_be_first_move( Game & ) const
  {
    return true;
  }
  bool Knock_Out_Move::may_be_last_move( Game &game ) const
  {
    return !game.board.is_knock_out_possible(to);
  }

  std::escape_ostream &Knock_Out_Move::output( std::escape_ostream &eos ) const
  {
    eos << get_type() << from.x << from.y << over.x << over.y << to.x << to.y;
    return eos;
  }
  bool Knock_Out_Move::input( std::escape_istream &eis )
  {
    eis >> from.x >> from.y >> over.x >> over.y >> to.x >> to.y;
    if( (from.x < 0) || (from.y < 0) || (over.x < 0) || (over.y < 0) || (to.x < 0) || (to.y < 0) )
      return false;
    return true;
  }

  Move *Knock_Out_Move::clone() const
  {
    return new Knock_Out_Move(*this);
  }
  
  Knock_Out_Move::Knock_Out_Move()
  {
  }
  Knock_Out_Move::Knock_Out_Move( Field_Pos from, Field_Pos over, 
				  Field_Pos to )
    : from(from), over(over), to(to)
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
    if( game.common_stones.stone_count[ stone_type ] > 0 )
    {
      --game.common_stones.stone_count[ stone_type ];
      own_stone = false;
    }
    else
    {
      assert( game.current_player->stones.stone_count[ stone_type ] > 0 );
      --game.current_player->stones.stone_count[ stone_type ];
      own_stone = true;
    }
    game.board.field[pos.x][pos.y] = Field_State_Type(stone_type);
  }
  void Set_Move::undo_move( Game &game )
  {
    if( own_stone )
    {
      ++game.current_player->stones.stone_count[ stone_type ];
    }
    else
    {
      ++game.common_stones.stone_count[ stone_type ];
    }
    game.board.field[pos.x][pos.y] = field_empty;
  }
  // true: move ok
  bool Set_Move::check_move( Game &game ) const 
  {
    if( !Board::is_empty( game.board.field[pos.x][pos.y] ) ) return false;
    if( game.common_stones.stone_count[ stone_type ] <= 0 )
    {
      // if there are other common stones
      if( game.common_stones.stone_count[ Stones::white_stone ] +
	  game.common_stones.stone_count[ Stones::grey_stone ]  +
	  game.common_stones.stone_count[ Stones::black_stone ] > 0 )
	return false;
      else						 // there are no common stones
	if( game.current_player->stones.stone_count[ stone_type ] <= 0 )
	  return false;					 // return false if player doesn't have that stone
    }

    return true;
  }
  // true: type ok
  bool Set_Move::check_previous_move( Game &, Move *move ) const 
  {
    return move == 0;
  }
  bool Set_Move::may_be_first_move( Game &game ) const
  {
    return !game.board.is_knock_out_possible();
  }
  bool Set_Move::may_be_last_move( Game &game ) const
  {
    return !game.board.is_any_field_removable();
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
    stone_type = Stones::Stone_Type(stone);
    switch(stone_type)
    {
      case Stones::white_stone:
      case Stones::grey_stone:
      case Stones::black_stone:
	return true;
      case Stones::invalid_stone:
	return false;
    }
    return false;
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
  // Remove
  // ----------------------------------------------------------------------------

  Move::Move_Type Remove::get_type() const
  {
    return remove;
  }

  void Remove::do_move( Game &game )
  {
    assert( check_move(game) );
    game.board.field[remove_pos.x][remove_pos.y] = field_removed;
  }
  void Remove::undo_move( Game &game )
  {
    game.board.field[remove_pos.x][remove_pos.y] = field_empty;
  }
  // true: move ok
  bool Remove::check_move( Game &game ) const 
  {
    if( !Board::is_empty( game.board.field[remove_pos.x][remove_pos.y] ) ) return false;

    return game.board.is_removable(remove_pos);
  }
  // true: type ok
  bool Remove::check_previous_move( Game &, Move *move ) const 
  {
    if( !move )
      return false;

    return move->get_type() == set_move;
  }
  bool Remove::may_be_first_move( Game & ) const
  {
    return false;
  }
  bool Remove::may_be_last_move( Game & ) const
  {
    return true;
  }

  std::escape_ostream &Remove::output( std::escape_ostream &eos ) const
  {
    eos << get_type() << remove_pos.x << remove_pos.y;
    return eos;
  }
  bool Remove::input( std::escape_istream &eis )
  {
    eis >> remove_pos.x >> remove_pos.y;
    if( (remove_pos.x < 0) || (remove_pos.y < 0) )
      return false;
    return true;
  }
  
  Move *Remove::clone() const
  {
    return new Remove(*this);
  }

  Remove::Remove()
  {
  }

  Remove::Remove( Field_Pos remove_pos )
    : remove_pos(remove_pos)
  {
  }

  // ----------------------------------------------------------------------------
  // Finish_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Finish_Move::get_type() const
  {
    return finish_move;
  }

  // structure element to determine whether any area is totally filled
  class Area						 
  {
  public:
    Area()
      : main_area(0), filled(true)
    {
    }
    inline void set_main_area( Area *area )
    {
      get_main_area()->main_area = area;
    }
    Area *get_main_area()
    {
      Area *area = this;
      while( area->main_area != 0 )			 // only the real main area has no chief
      {
	area = area->main_area;
      }
      return area;
    }
    void connect_area( Area *area )
    {
      Area *m1 = get_main_area();
      Area *m2 = area->get_main_area();
      if( m1 != m2 )					 // area areas not already connected?
      {
	if( !m2->is_filled() )
	{
	  m1->set_unfilled();
	}
	m2->set_main_area(m1);
      }
    }
    inline void set_unfilled() 
    { 
      get_main_area()->filled = false; 
    }
    inline bool is_filled()
    {
      return get_main_area()->filled;
    }
  private:
    Area *main_area;					 // connected area which stores all relevant data
    bool filled;					 // whether this area is totally filled
  };

  void Finish_Move::do_move( Game &game )
  {
    removed_stones.clear();

    // **********************************
    // search for filled isolated islands

    // tells for all areas whether they are totally filled
    std::list<Area> areas; 
    Area *current_area;

    // stores for each field to which area set it belongs
    std::vector< std::vector<Area *> > field_area(game.board.get_x_size());

    Field_Pos pos;
    Field_Iterator p1( &game.board ), p2( &game.board );
    int dir;
	
    // resize field_area
    for( pos.x = 0; pos.x < game.board.get_x_size(); ++pos.x )
    {
      field_area[pos.x].resize(game.board.get_y_size());
    }

    for( pos.y = 0; pos.y < game.board.get_y_size(); ++pos.y )
    {
      for( pos.x = 0; pos.x < game.board.get_x_size(); ++pos.x )
      {
	p1.set_pos( pos );

	assert( p1.is_valid_field() );

	current_area = 0;
	if( !Board::is_removed(*p1) )			 // removed fields don't belong to area
	{
	  for( dir = Field_Iterator::top_right;
	       dir <= Field_Iterator::left; ++dir )
	  {
	    p2 = p1.Next( Field_Iterator::Direction(dir) );
	    if( p2.is_valid_field() )
	    {
	      Area *p2_area = field_area[p2.get_pos().x][p2.get_pos().y];
	      if( p2_area != 0 )
	      {
		if( current_area == 0 )			 // no area yet
		{
		  current_area = p2_area;
		}
		else
		{
		  if( current_area != p2_area )		 // different areas?
		  {
		    p2_area->connect_area( current_area ); // connect them => they are the same
		  }
		}
	      }
	    }
	  }

	  if( current_area == 0 )			 // no adjacent area?
	  {
	    areas.push_back( Area() );
	    current_area = &areas.back();
	  }

	  if( Board::is_empty(*p1) )			 // if field is empty
	    current_area->set_unfilled();		 // area of this field can't be filled
	}
	
	field_area[pos.x][pos.y] = current_area;	  
      }
    }    

    // **********************************
    // check for filled isolated islands

    std::list<Area>::iterator area;
    bool any_filled_areas = false;
    for( area = areas.begin(); area != areas.end(); ++area )
    {
      if( area->is_filled() )
      {
	any_filled_areas = true;
	break;
      }
    }

    // **************************************
    // collect all stones from filled areas

    if( any_filled_areas )
    {
      for( pos.x = 0; pos.x < game.board.get_x_size(); ++pos.x )
      {
	for( pos.y = 0; pos.y < game.board.get_y_size(); ++pos.y )
	{
	  Area *area = field_area[pos.x][pos.y];
	  if( area )
	  {
	    if( area->is_filled() )
	    {
	      p1.set_pos(pos);
	      assert( Board::is_stone(*p1) );
	      Stones::Stone_Type stone_type = Stones::Stone_Type(*p1);
	      
	      ++game.current_player->stones.stone_count[ stone_type ];
	      
	      removed_stones.push_back( std::pair<Field_Pos,Stones::Stone_Type>(pos,stone_type) );
	      
	      *p1 = field_removed;
	    }
	  }
	}
      }
    }
  }
  void Finish_Move::undo_move( Game &game )
  {
    Field_Iterator p1( &game.board );

    // restore stones removed because of filled islands
    std::list< std::pair<Field_Pos,Stones::Stone_Type> >::iterator i;
    for( i = removed_stones.begin(); i != removed_stones.end(); ++i )
    {
      Stones::Stone_Type &stone_type = i->second;

      p1.set_pos(i->first);
      *p1 = Field_State_Type(stone_type);

      --game.current_player->stones.stone_count[ stone_type ];
      assert( game.current_player->stones.stone_count[ stone_type ] >= 0 );
    }
  }
  // true: move ok
  bool Finish_Move::check_move( Game & ) const 
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
  bool Finish_Move::input( std::escape_istream & )
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
	case Move::knock_out_move:	move = new Knock_Out_Move(); break;
	case Move::set_move:		move = new Set_Move(); break;
	case Move::remove:		move = new Remove(); break;
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
      case Move::knock_out_move: 
      {
	const Knock_Out_Move *det_m1 = static_cast<const Knock_Out_Move*>(&m1);
	const Knock_Out_Move *det_m2 = static_cast<const Knock_Out_Move*>(&m2);
	return *det_m1 == *det_m2;
      }
      case Move::set_move: 
      {
	const Set_Move *det_m1 = static_cast<const Set_Move*>(&m1);
	const Set_Move *det_m2 = static_cast<const Set_Move*>(&m2);
	return *det_m1 == *det_m2;
      }
      case Move::remove: 
      {
	const Remove *det_m1 = static_cast<const Remove*>(&m1);
	const Remove *det_m2 = static_cast<const Remove*>(&m2);
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
      case Move::knock_out_move: 
      {
	const Knock_Out_Move *det_m1 = static_cast<const Knock_Out_Move*>(&m1);
	const Knock_Out_Move *det_m2 = static_cast<const Knock_Out_Move*>(&m2);
	return *det_m1 < *det_m2;
      }
      case Move::set_move: 
      {
	const Set_Move *det_m1 = static_cast<const Set_Move*>(&m1);
	const Set_Move *det_m2 = static_cast<const Set_Move*>(&m2);
	return *det_m1 < *det_m2;
      }
      case Move::remove: 
      {
	const Remove *det_m1 = static_cast<const Remove*>(&m1);
	const Remove *det_m2 = static_cast<const Remove*>(&m2);
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
  bool operator==( const Knock_Out_Move &m1, const Knock_Out_Move &m2 )
  {
    if( m1.from != m2.from ) return false;
    if( m1.to != m2.to ) return false;
    return true;
  }
  bool operator<( const Knock_Out_Move &m1, const Knock_Out_Move &m2 )
  {
    if( m1.from.x < m2.from.x ) return true;
    if( m1.from.x > m2.from.x ) return false;
    if( m1.from.y < m2.from.y ) return true;
    if( m1.from.y > m2.from.y ) return false;
    if( m1.to.x < m2.to.x ) return true;
    if( m1.to.x > m2.to.x ) return false;
    if( m1.to.y < m2.to.y ) return true;
    if( m1.to.y > m2.to.y ) return false;
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
  bool operator==( const Remove &m1, const Remove &m2 )
  {
    if( m1.remove_pos != m2.remove_pos ) return false;
    return true;
  }
  bool operator<( const Remove &m1, const Remove &m2 )
  {
    if( m1.remove_pos.x < m2.remove_pos.x ) return true;
    if( m1.remove_pos.x > m2.remove_pos.x ) return false;
    if( m1.remove_pos.y < m2.remove_pos.y ) return true;
    if( m1.remove_pos.y > m2.remove_pos.y ) return false;
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
    bool first_knock_out = true;
    for( move_it = sequence.get_moves().begin(); move_it != sequence.get_moves().end(); move_it++ )
    {
      Move *move = *move_it;
      switch( move->get_type() )
      {
	case Move::no_move:
	  break;
	case Move::knock_out_move:
	{
	  Knock_Out_Move *det_move = static_cast<Knock_Out_Move*>(move);
	  if( first_knock_out )
	  {
	    first_knock_out = false;
	    ret += "x ";
	    ret += coordinate_translator->get_field_name(det_move->from);
	  }
	  switch( det_move->knocked_stone )
	  {
	    case field_removed:
	    case field_empty:
	      break;
	    case field_white: ret += 'W'; break;
	    case field_grey:  ret += 'G'; break;
	    case field_black: ret += 'B'; break;
	  }
	  ret += coordinate_translator->get_field_name(det_move->to);
	}
	break;
	case Move::set_move:
	{
	  Set_Move *det_move = static_cast<Set_Move*>(move);
	  switch( det_move->stone_type )
	  {
	    case Stones::invalid_stone: break;
	    case Stones::white_stone:   ret += 'W'; break;
	    case Stones::grey_stone:    ret += 'G'; break;
	    case Stones::black_stone:   ret += 'B'; break;
	  }
	  ret += coordinate_translator->get_field_name(det_move->pos);
	}
	break;
	case Move::remove:
	{
	  Remove *det_move = static_cast<Remove*>(move);
	  ret += ',';
	  ret += coordinate_translator->get_field_name(det_move->remove_pos);
	}
	break;
	case Move::finish_move:
	{
	  Finish_Move *det_move = static_cast<Finish_Move*>(move);
	  if( !det_move->removed_stones.empty() )
	    ret += " x ";
	  std::list< std::pair<Field_Pos,Stones::Stone_Type> >::iterator removed_it;
	  for( removed_it = det_move->removed_stones.begin();
	       removed_it != det_move->removed_stones.end(); ++removed_it )
	  {
	    Field_Pos pos = removed_it->first;
	    Stones::Stone_Type stone_type = removed_it->second;
	    switch( stone_type )
	    {
	      case Stones::invalid_stone: break;
	      case Stones::white_stone:   ret += 'W'; break;
	      case Stones::grey_stone:    ret += 'G'; break;
	      case Stones::black_stone:   ret += 'B'; break;
	    }
	    ret += coordinate_translator->get_field_name(pos);
	  }
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
    if( (first == 'x') || (first == 'X') ) // is knock out move ?
    {
      std::string str;
      is >> str;
      size_t i;
      // search for letters followed by digits
      for( i = 0; i < str.size(); ++i ) if( !isalpha( str[i] ) ) break;
      for( ; i < str.size(); ++i )	if( !isdigit( str[i] ) ) break;
      Field_Pos from = coordinate_translator->get_field_pos( str.substr(0,i) );
      if( from.x < 0 ) return Move_Sequence();
      do
      {
	Stones::Stone_Type stone; // stone won't be checked
	switch( str[i] )
	{
	  case 'w':
	  case 'W': stone = Stones::white_stone; break;
	  case 'g':
	  case 'G': stone = Stones::grey_stone; break;
	  case 'b':
	  case 'B': stone = Stones::black_stone; break;
	  default: return Move_Sequence();
	}
	++i;
	size_t start = i;
	// search for letters followed by digits
	for( ; i < str.size(); ++i ) if( !isalpha( str[i] ) ) break;
	for( ; i < str.size(); ++i ) if( !isdigit( str[i] ) ) break;
	Field_Pos to = coordinate_translator->get_field_pos( str.substr(start, i - start) );
	if( to.x < 0 ) return Move_Sequence();

	Field_Pos middle = Field_Pos::get_middle(from, to);

	Knock_Out_Move *move = new Knock_Out_Move(from, middle, to);
	sequence.add_move( move );

	from = to;
      }while( i < str.size() );
      sequence.add_move( new Finish_Move() );
    }
    else			// set move
    {
      Stones::Stone_Type stone;
      switch( first )		// first character already read
      {
	case 'w':
	case 'W': stone = Stones::white_stone; break;
	case 'g':
	case 'G': stone = Stones::grey_stone; break;
	case 'b':
	case 'B': stone = Stones::black_stone; break;
	default: return Move_Sequence();
      }

      std::string str;
      is >> str;
      size_t i;
      // search for letters followed by digits
      for( i = 0; i < str.size(); ++i ) if( !isalpha( str[i] ) ) break;
      for( ; i < str.size(); ++i )	if( !isdigit( str[i] ) ) break;
      Field_Pos pos = coordinate_translator->get_field_pos( str.substr(0,i) );
      if( pos.x < 0 ) return Move_Sequence();
      
      sequence.add_move( new Set_Move( pos, stone ) );

      if( str[i] != ',' ) return Move_Sequence();
      ++i;

      size_t start = i;
      // search for letters followed by digits
      for( ; i < str.size(); ++i ) if( !isalpha( str[i] ) ) break;
      for( ; i < str.size(); ++i ) if( !isdigit( str[i] ) ) break;
      if( i < str.size() ) return Move_Sequence();
      pos = coordinate_translator->get_field_pos( str.substr(start,i - start) );
      if( pos.x < 0 ) return Move_Sequence();

      sequence.add_move( new Remove(pos) );
      sequence.add_move( new Finish_Move() );

      // read probable removed stones
      char next;
      is >> next;
      if( next != 'x' )
	is.unget();
      else
      {
	is >> str;		// read removed fields
	// and forget
      }
    }
    return sequence;
  }

  // ----------------------------------------------------------------------------
  // Generic_Win_Condition
  // ----------------------------------------------------------------------------

  Generic_Win_Condition::Generic_Win_Condition( int num_white, int num_grey, 
						int num_black, int num_all )
    : Win_Condition( generic ), num_white(num_white), num_grey(num_grey), num_black(num_black),
      num_all(num_all)
  {
  }

  bool Generic_Win_Condition::did_player_win( Game &game, Player &player )
    const
  {
    if( player.stones.stone_count[Stones::white_stone] >= num_white )
      return true;
    if( player.stones.stone_count[Stones::grey_stone]  >= num_grey )
      return true;
    if( player.stones.stone_count[Stones::black_stone] >= num_black )
      return true;
    if(( player.stones.stone_count[Stones::white_stone] >= num_all ) &&
       ( player.stones.stone_count[Stones::grey_stone]  >= num_all ) &&
       ( player.stones.stone_count[Stones::black_stone] >= num_all ))
      return true;

    bool any_field_left = false;
    Field_Pos pos;
    for( pos.x = 0; pos.x < game.board.get_x_size(); ++pos.x )
      for( pos.y = 0; pos.y < game.board.get_y_size(); ++pos.y )
      {
	if( game.board.field[pos.x][pos.y] != field_removed )
	{
	  any_field_left = true;
	  break;
	}
      }

    if( !any_field_left )	// current player wins, if no field left
      return true;

    return false;
  }  

  Win_Condition *Generic_Win_Condition::clone()
  {
    return new Generic_Win_Condition( num_white, num_grey, num_black, num_all );
  }

  // ----------------------------------------------------------------------------
  // Basic_Win_Condition
  // ----------------------------------------------------------------------------

  Basic_Win_Condition::Basic_Win_Condition()
    : Generic_Win_Condition(3,4,5,2)
  {
    type = basic;
  }

  // ----------------------------------------------------------------------------
  // Standard_Win_Condition
  // ----------------------------------------------------------------------------

  Standard_Win_Condition::Standard_Win_Condition()
    : Generic_Win_Condition(4,5,6,3)
  {
    type = standard;
  }

  // ----------------------------------------------------------------------------
  // Basic_Ruleset
  // ----------------------------------------------------------------------------

  Basic_Ruleset::Basic_Ruleset()
    : Ruleset( basic,
	       Board( (const int*) standard_board, 
		      sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
		      sizeof(standard_board)    / sizeof(standard_board[0]), Board::s37_rings ),
	       Basic_Common_Stones(),
	       new Basic_Win_Condition(), 0 /*coordinate translator init below */,
	       true /*undo possible*/, 2, 2 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
  }

  // ----------------------------------------------------------------------------
  // Standard_Ruleset
  // ----------------------------------------------------------------------------

  Standard_Ruleset::Standard_Ruleset()
    : Ruleset( standard,
	       Board( (const int*) standard_board, 
		      sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
		      sizeof(standard_board)    / sizeof(standard_board[0]), Board::s37_rings ),
	       Standard_Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       true /*undo possible*/, 2, 2 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
  }

  // ----------------------------------------------------------------------------
  // Tournament_Ruleset
  // ----------------------------------------------------------------------------

  Tournament_Ruleset::Tournament_Ruleset()
    : Ruleset( tournament,
	       Board( (const int*) board_48, 
		      sizeof(board_48[0]) / sizeof(board_48[0][0]),
		      sizeof(board_48)    / sizeof(board_48[0]), Board::s48_rings ),
	       Standard_Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       false /*no undo possible*/, 2, 2 )  
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
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
  ( Field_Pos pos )
  {
    move_done = 0;
    Field_Iterator p1( pos, &game.board );

    switch( state )
    {
      case begin:
	if( Board::is_stone(*p1) )			 // clicked on stone?
	{
	  /* user will recognize his fault himself...
	     if( !game.board.is_knock_out_possible(pos) )
	     return error_...;
	  */
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
    assert( false );
    return fatal_error;
  }

  Sequence_Generator::Sequence_State Sequence_Generator::add_click_common_stone
  ( Stones::Stone_Type stone_type )
  {
    move_done = 0;
    if( state != begin )
      return error_impossible_yet;

    if( game.board.is_knock_out_possible() )
      return error_require_knock_out;

    if( game.common_stones.stone_count[stone_type] <= 0 )
      return fatal_error;				 // clicked on stone which is not there?!?

    picked_stone = stone_type;
    state = stone_picked;

    return Sequence_State(hold_prefix + stone_type);
  }

  Sequence_Generator::Sequence_State Sequence_Generator::add_click_player_stone
  ( int player_id, Stones::Stone_Type stone_type )
  {
    move_done = 0;
    if( state != begin )
      return error_impossible_yet;

    if( player_id != game.current_player->id )
      return error_wrong_player;

    if( game.current_player->stones.stone_count[stone_type] <= 0 ) 
      return fatal_error;		 // clicked on stone which is not there?!?

    if( game.board.is_knock_out_possible() )
      return error_require_knock_out;

    if( game.common_stones.stone_count[Stones::white_stone] +
	game.common_stones.stone_count[Stones::grey_stone]  +
	game.common_stones.stone_count[Stones::black_stone] > 0 )
      return error_must_pick_common_stone;

    picked_stone = stone_type;
    state = stone_picked;

    return Sequence_State(hold_prefix + stone_type);
  }

  Sequence_Generator::Sequence_State Sequence_Generator::undo_click()
  {
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
    return another_click;
  }

  Move::Move_Type Sequence_Generator::get_required_move_type()
  {
    switch( state )
    {
      case begin:
      case move_from:
      {
	if( game.board.is_knock_out_possible() )
	  return Move::knock_out_move;
	else
	  return Move::set_move;
      }
      break;
      case move_dest:
      {
	return Move::knock_out_move;
      }
      break;
      case stone_picked:
      {
	return Move::set_move;
      }
      break;
      case stone_set:
      {
	return Move::remove;
      }
      break;
      case move_finished:
	break;
    }
    return Move::no_move;
  }

  std::list<Field_Pos> Sequence_Generator::get_possible_clicks()
  {
    std::list<Field_Pos> ret;
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
  }

  Field_Pos Sequence_Generator::get_selected_pos()
  {
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
}

