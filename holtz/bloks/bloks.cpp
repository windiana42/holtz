/*
 * bloks.cpp
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

#include "bloks.hpp"
#include <assert.h>
#include <iomanip>

#include "util.hpp"

#include <ctype.h>
#include <sys/types.h>

namespace bloks
{
  using namespace holtz;

  // ----------------------------------------------------------------------------
  // Stone_Type
  // -----------------------------------------------------------

  Stone_Type::Stone_Type(int ID, const int *sub_fields_array, unsigned width, unsigned height)
    : ID(ID), max_x(0), max_y(0), rotation_symmetric(false), flip_symmetric(false)
  {
    // initialize corners array
    std::vector<std::vector<bool> > corners_array;//[y+1][x+1]
    corners_array.resize(height+2);
    for(unsigned y=0;y<height+2;++y) 
    {
      corners_array[y].resize(width+2);
      for(unsigned x=0;x<width+2;++x) 
      {
	corners_array[y][x] = false;
      }
    }
    // process sub_fields array
    for(unsigned y=0;y<height;++y) 
    {
      for(unsigned x=0;x<width;++x) 
      {
	if( sub_fields_array[x+y*width] > 0 )
	{
	  sub_fields.push_back(std::pair<unsigned,unsigned>(x,y));
	  if(x > max_x) max_x = x;
	  if(y > max_y) max_y = y;
	  // mark potential corners
	  for(int i=-1;i<=1; i+=2) 
	  {
	    for(int j=-1;j<=1; j+=2) 
	    {
	      // x2/y2 is diagonally adjacent to x/y
	      int y2 = int(y)+i;
	      int x2 = int(x)+j;
	      corners_array[y2+1][x2+1]=true;
	    }
	  }
	}
      }
    }
    // cross out invalid corners
    for(unsigned y=0;y<height;++y) 
    {
      for(unsigned x=0;x<width;++x) 
      {
	if( sub_fields_array[x+y*width] > 0 )
	{
	  for(int i=-1;i<=1; ++i) 
	  {
	    for(int j=-1;j<=1; j+=2) 
	    {
	      // x2/y2 is adjacent to x/y
	      int y2 = int(y);
	      int x2 = int(x);
	      if( j < 0 ) y2 += i;
	      else x2 += i;
	      corners_array[y2+1][x2+1]=false;
	    }
	  }
	}
      }
    }
    // store corners as list
    for(unsigned y=0;y<height+2;++y) 
    {
      for(unsigned x=0;x<width+2;++x) 
      {
	if(corners_array[y][x]) {
	  corners.push_back(std::pair<int/*x*/,int/*y*/>(int(x)-1,int(y)-1));
	}
	// // debug output:
	// bool field=false;
	// if( x > 0 && y > 0 && x < width+1 && y < height+1 )
	// {
	//   if( sub_fields_array[x-1+(y-1)*width] > 0 )
	//   {
	//     field = true;
	//   }
	// }
	// if(field)
	//   std::cout << "X";
	// else if(corners_array[y][x])
	//   std::cout << "O";
	// else
	//   std::cout << " ";
      }
      //std::cout << std::endl;
    }
    // determine rotation symmetric 
    if( max_x == max_y ) 
    {
      rotation_symmetric = true; // starting assumption
      for(unsigned y=0;y<=max_y;++y) 
      {
	for(unsigned x=0;x<=max_x;++x) 
	{
	  if( sub_fields_array[x+y*width] != sub_fields_array[(max_y-y)+x*width] )
	  {
	    rotation_symmetric = false;
	    break;
	  }
	}
	if(!rotation_symmetric) break;
      }
    }
    else
    {
      rotation_symmetric = false;
    }
    // determine flip symmetric
    flip_symmetric = false; //starting assumption
    for( int dir_int = Field_Iterator::right_flipped;
	 dir_int <= Field_Iterator::bottom_flipped; ++dir_int )
    {
      Field_Iterator::Direction dir = Field_Iterator::Direction(dir_int);

      if( (dir != Field_Iterator::top_flipped && dir != Field_Iterator::bottom_flipped) || (max_x == max_y) )
      {
	bool flip_symmetric_rotation_x = true; // starting assumption
	for(unsigned y=0;y<=max_y;++y) 
	{
	  for(unsigned x=0;x<=max_x;++x) 
	  {
	    std::pair<unsigned/*x*/,unsigned/*y*/> rot_pos = rotate(std::pair<unsigned/*x*/,unsigned/*y*/>(x,y), dir);
	    if( sub_fields_array[x+y*width] != sub_fields_array[rot_pos.first+rot_pos.second*width] )
	    {
	      flip_symmetric_rotation_x = false;
	      break;
	    }
	  }
	  if(!flip_symmetric_rotation_x) break;
	}
	if( flip_symmetric_rotation_x ) 
	{
	  flip_symmetric = true;
	  break;
	}
      }
    }
    // precalculate query positions
    int center_utility = 10000000;
    int top_left_utility = 10000000;
    int bottom_right_utility = 10000000;
    unsigned center_match_x = 0;
    unsigned center_match_y = 0;
    unsigned top_left_match_x = 0;
    unsigned top_left_match_y = 0;
    unsigned bottom_right_match_x = 0;
    unsigned bottom_right_match_y = 0;
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned rx = it->first;
      unsigned ry = it->second;
      int pos_utility;
      // center
      pos_utility = fabs(int(rx) - (max_x+1)/2.) + fabs(int(ry) - (max_y+1)/2.);
      if( pos_utility < center_utility )
      {
	center_utility = pos_utility;
	center_match_x = rx;
	center_match_y = ry;
      }
      // top_left
      pos_utility = ry * 10 + rx*11; // left slightly dominates top
      if( pos_utility < top_left_utility )
      {
	top_left_utility = pos_utility;
	top_left_match_x = rx;
	top_left_match_y = ry;
      }
      // bottom_right
      pos_utility = (max_y-ry) * 10 + (max_x-rx) * 11; // right slightly dominates bottom
      if( pos_utility < bottom_right_utility )
      {
	bottom_right_utility = pos_utility;
	bottom_right_match_x = rx;
	bottom_right_match_y = ry;
      }
    }
    sub_field_queries[CENTER] = std::make_pair(center_match_x,center_match_y);
    sub_field_queries[TOP_LEFT] = std::make_pair(top_left_match_x,top_left_match_y);
    sub_field_queries[BOTTOM_RIGHT] = std::make_pair(bottom_right_match_x,bottom_right_match_y);
  }

  std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > 
  Stone_Type::get_sub_fields(Field_Iterator::Direction dir) const
  {
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > ret;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned raw_x=it->first;
      unsigned raw_y=it->second;
      assert(raw_x <= get_max_x());
      assert(raw_y <= get_max_y());
      unsigned x=0;
      unsigned y=0;
      switch(dir) 
      {
      case Field_Iterator::right:		
	x = raw_x; 
	y = raw_y;
	break;
      case Field_Iterator::right_flipped:		
	// invert y-axis of original coordinates
	x = raw_x; 
	y = get_max_y()-raw_y;
	break;
      case Field_Iterator::top:			
	x = raw_y; 
	y = get_max_x()-raw_x;
	break;
      case Field_Iterator::top_flipped:		
	// invert y-axis of original coordinates
	x = get_max_y()-raw_y; 
	y = get_max_x()-raw_x;
	break;
      case Field_Iterator::left:		
	x = get_max_x()-raw_x;
	y = get_max_y()-raw_y;
	break;
      case Field_Iterator::left_flipped:		
	// invert y-axis of original coordinates
	x = get_max_x()-raw_x;
	y = raw_y;
	break;
      case Field_Iterator::bottom:		
	x = get_max_y()-raw_y;
	y = raw_x;
	break;
      case Field_Iterator::bottom_flipped:
	// invert y-axis of original coordinates
	x = raw_y;
	y = raw_x;
	break;
      case Field_Iterator::invalid_direction:    assert( false );
      }
      ret.push_back(std::pair<unsigned,unsigned>(x,y));
    }
    return ret;
  }

  std::list<std::pair<int/*x*/,int/*y*/> > 
  Stone_Type::get_corners(Field_Iterator::Direction dir) const
  {
    std::list<std::pair<int/*x*/,int/*y*/> >::const_iterator it;
    std::list<std::pair<int/*x*/,int/*y*/> > ret;
    for( it=corners.begin(); it!=corners.end(); ++it )
    {
      int raw_x=it->first;
      int raw_y=it->second;
      assert(raw_x <= int(get_max_x()+1));
      assert(raw_x >= -1 );
      assert(raw_y <= int(get_max_y()+1));
      assert(raw_y >= -1 );
      int x=0;
      int y=0;
      switch(dir) 
      {
      case Field_Iterator::right:		
	x = raw_x; 
	y = raw_y;
	break;
      case Field_Iterator::right_flipped:		
	// invert y-axis of original coordinates
	x = raw_x; 
	y = int(get_max_y())-raw_y;
	break;
      case Field_Iterator::top:			
	x = raw_y; 
	y = int(get_max_x())-raw_x;
	break;
      case Field_Iterator::top_flipped:		
	// invert y-axis of original coordinates
	x = int(get_max_y())-raw_y; 
	y = int(get_max_x())-raw_x;
	break;
      case Field_Iterator::left:		
	x = int(get_max_x())-raw_x;
	y = int(get_max_y())-raw_y;
	break;
      case Field_Iterator::left_flipped:		
	// invert y-axis of original coordinates
	x = int(get_max_x())-raw_x;
	y = raw_y;
	break;
      case Field_Iterator::bottom:		
	x = int(get_max_y())-raw_y;
	y = raw_x;
	break;
      case Field_Iterator::bottom_flipped:
	// invert y-axis of original coordinates
	x = raw_y;
	y = raw_x;
	break;
      case Field_Iterator::invalid_direction:    assert( false );
      }
      ret.push_back(std::pair<int,int>(x,y));
    }
    return ret;
  }

  std::pair<int/*dx*/,int/*dy*/> Stone_Type::query_sub_field_diff
  (Query_Type type1, Query_Type type2, Field_Iterator::Direction dir) const
  {
    std::pair<unsigned/*x*/,unsigned/*y*/> q1 = query_sub_field(type1,dir);
    std::pair<unsigned/*x*/,unsigned/*y*/> q2 = query_sub_field(type2,dir);
    return std::make_pair(q2.first-int(q1.first),q2.second-int(q1.second));
  }

  unsigned Stone_Type::get_max_x(Field_Iterator::Direction dir) const
  {
    switch(dir)
    {
      case Field_Iterator::top:			
      case Field_Iterator::top_flipped:		
      case Field_Iterator::bottom:		
      case Field_Iterator::bottom_flipped:
	return max_y;
      case Field_Iterator::right:		
      case Field_Iterator::right_flipped:		
      case Field_Iterator::left:		
      case Field_Iterator::left_flipped:		
      case Field_Iterator::invalid_direction:
	break;
    }
    return max_x;
  }
  unsigned Stone_Type::get_max_y(Field_Iterator::Direction dir) const
  {
    switch(dir)
    {
      case Field_Iterator::top:			
      case Field_Iterator::top_flipped:		
      case Field_Iterator::bottom:		
      case Field_Iterator::bottom_flipped:
	return max_x;
      case Field_Iterator::right:		
      case Field_Iterator::right_flipped:		
      case Field_Iterator::left:		
      case Field_Iterator::left_flipped:		
      case Field_Iterator::invalid_direction:
	break;
    }
    return max_y;
  }

  std::pair<Field_Iterator::Direction, Field_Pos/*origin*/> 
  Stone_Type::get_orientation(Field_Pos top_left, Field_Pos bottom_right, bool is_flipped) const
  {
    std::pair<Field_Iterator::Direction, Field_Pos/*origin*/> ret(Field_Iterator::invalid_direction, Field_Pos());
    // check possibility of top_left/bottom_right match
    std::pair<unsigned/*x*/,unsigned/*y*/> sub_top_left = query_sub_field(TOP_LEFT, is_flipped?Field_Iterator::right_flipped : Field_Iterator::right);
    std::pair<unsigned/*x*/,unsigned/*y*/> sub_bottom_right = query_sub_field(BOTTOM_RIGHT, is_flipped?Field_Iterator::right_flipped : Field_Iterator::right);
    int dx = sub_bottom_right.first - int(sub_top_left.first);
    int dy = sub_bottom_right.second - int(sub_top_left.second);
    int match_dx = bottom_right.x - int(top_left.x);
    int match_dy = bottom_right.y - int(top_left.y);
    if( (abs(dx) != abs(match_dx) || abs(dy) != abs(match_dy)) && (abs(dx) != abs(match_dy) || abs(dy) != abs(match_dx)) ) 
    {
      // std::cout << "no dx/dy match " << (dx==match_dx) << "/" << (dy==match_dy) << "/" 
      // 		<< (dx==match_dy) << "/" << (dy==match_dx) 
      // 		<< " " << dx << " " << dy << " " << match_dx << " " << match_dy
      // 		<< std::endl;
      return ret;
    }

    // search all directions whether top_left and bottom_right have a match
    if( dx == match_dx && dy == match_dy )
    {
      Field_Pos origin(top_left.x-sub_top_left.first,top_left.y-sub_top_left.second);
      return std::make_pair(is_flipped?Field_Iterator::right_flipped : Field_Iterator::right,origin);
    }
    if( dx == -match_dy && dy == match_dx )
    {
      Field_Pos origin(top_left.x-sub_top_left.second,top_left.y-(max_x-sub_top_left.first));
      return std::make_pair(is_flipped?Field_Iterator::top_flipped : Field_Iterator::top,origin);
    }
    if( dx == -match_dx && dy == -match_dy )
    {
      Field_Pos origin(top_left.x-(max_x-sub_top_left.first),top_left.y-(max_y-sub_top_left.second));
      return std::make_pair(is_flipped?Field_Iterator::left_flipped : Field_Iterator::left,origin);
    }
    if( dx == match_dy && dy == -match_dx )
    {
      Field_Pos origin(top_left.x-(max_y-sub_top_left.second),top_left.y-sub_top_left.first);
      return std::make_pair(is_flipped?Field_Iterator::bottom_flipped : Field_Iterator::bottom,origin);
    }
    std::cout << "no orientation match found" << std::endl;
    return ret;
  }


  std::pair<Field_Pos /*top_left*/, Field_Pos /*bottom_right*/>
  Stone_Type::get_click_points(Field_Iterator::Direction dir, Field_Pos origin) const
  {
    std::pair<Field_Pos /*top_left*/, Field_Pos /*bottom_right*/> ret;

    if( dir == Field_Iterator::invalid_direction )
    {
      std::cout << "no orientation match found" << std::endl;
      return ret;
    }

    // resolve top_left/bottom_right match
    std::pair<unsigned/*x*/,unsigned/*y*/> sub_top_left = query_sub_field(TOP_LEFT, dir);
    std::pair<unsigned/*x*/,unsigned/*y*/> sub_bottom_right = query_sub_field(BOTTOM_RIGHT, dir);
    Field_Pos top_left(origin.x+sub_top_left.first,origin.y+sub_top_left.second);
    Field_Pos bottom_right(origin.x+sub_bottom_right.first,origin.y+sub_bottom_right.second);
    return std::make_pair(top_left,bottom_right);
  }

  std::pair<unsigned/*x*/,unsigned/*y*/> Stone_Type::rotate
  (std::pair<unsigned/*x*/,unsigned/*y*/> rel_pos, Field_Iterator::Direction dir) const // rotate or flip-rotate based on dir
  {
    switch(dir)
    {
    case Field_Iterator::invalid_direction:
    case Field_Iterator::right:
      return rel_pos;
    case Field_Iterator::right_flipped:
      // invert y-axis of original coordinates
      return std::make_pair(rel_pos.first,max_y-rel_pos.second);
    case Field_Iterator::top:
      return std::make_pair(rel_pos.second,max_x-rel_pos.first);
    case Field_Iterator::top_flipped:
      // invert y-axis of original coordinates
      return std::make_pair(max_y-rel_pos.second,max_x-rel_pos.first);
    case Field_Iterator::left:
      return std::make_pair(max_x-rel_pos.first,max_y-rel_pos.second);
    case Field_Iterator::left_flipped:
      // invert y-axis of original coordinates
      return std::make_pair(max_x-rel_pos.first,rel_pos.second);
    case Field_Iterator::bottom:
      return std::make_pair(max_y-rel_pos.second,rel_pos.first);
    case Field_Iterator::bottom_flipped:
      // invert y-axis of original coordinates
      return std::make_pair(rel_pos.second,rel_pos.first);
    }
    return rel_pos;
  }

  std::list<Stone_Type> get_standard_stone_types()
  {
    std::list<Stone_Type> ret;
    int ID=1;
    {
      const int sub_fields_array[][1] =
	{ {  1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      //std::cout << "get_standard_stone_types: " << ID << ": " << stone.get_sub_fields().size() << std::endl;
      ++ID;
    } 
    {
      const int sub_fields_array[][2] =
	{ {  1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      //std::cout << "get_standard_stone_types: " << ID << ": " << stone.get_sub_fields().size() << std::endl;
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      //std::cout << "get_standard_stone_types: " << ID << ": " << stone.get_sub_fields().size() << std::endl;
      ++ID;
    } 
    {
      const int sub_fields_array[][2] =
	{ {  1, 0 },
	  {  1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][4] =
	{ {  1,  1,  1,  1} };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 1, 1 },
	  {  0, 1, 0 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][2] =
	{ {  1, 1 },
	  {  1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    }
    {
      const int sub_fields_array[][3] =
	{ {  1, 1, 0 },
	  {  0, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][5] =
	{ {  1,  1,  1,  1, 1} };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][4] =
	{ {  1, 0, 0, 0 },
	  {  1, 1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][4] =
	{ {  1, 1, 0, 0 },
	  {  0, 1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 1, 0 },
	  {  1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 1 },
	  {  1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][4] =
	{ {  1, 1, 1, 1 },
	  {  0, 0, 1, 0 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 1, 1 },
	  {  1, 0, 0 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 0, 0 },
	  {  1, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 1, 0 },
	  {  0, 1, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 1, 1 },
	  {  0, 0, 1 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  1, 0, 0 },
	  {  1, 1, 1 },
	  {  0, 1, 0 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    {
      const int sub_fields_array[][3] =
	{ {  0, 1, 0 },
	  {  1, 1, 1 },
	  {  0, 1, 0 } };
      std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields;
      Stone_Type stone(ID,(const int*) sub_fields_array, 
		       sizeof(sub_fields_array[0]) / sizeof(sub_fields_array[0][0]),
		       sizeof(sub_fields_array)    / sizeof(sub_fields_array[0]));
      ret.push_back(stone);
      ++ID;
    } 
    return ret;
  }

  // ----------------------------------------------------------------------------
  // Stones
  // ----------------------------------------------------------------------------

  Stones::Stones()
  {
  }

  int Stones::get_stone_count(int stone_ID) const
  {
    if( does_include(stone_counts,stone_ID) )
      return stone_counts.find(stone_ID)->second;
    else
      return 0;      
  }

#ifndef __WXMSW__
  void Stones::print()
  {
    std::cout << std::endl;
    std::map<int/*stone_ID*/, int/*count*/>::iterator it;
    for( it=stone_counts.begin(); it!=stone_counts.end(); ++it )
      {
	int stone_ID = it->first;
	int count = it->second;
	if(count > 0 ) {
	  std::cout << count << " stones of type " << stone_ID << std::endl;
	}
      }
  }
#endif

  void Stones::remove_stones()
  {
    stone_counts.clear();
    total_count = 0;
  }

  // ----------------------------------------------------------------------------
  // Common_Stones
  // ----------------------------------------------------------------------------

  Common_Stones::Common_Stones( Common_Stones::Common_Stones_Type type )
    : type(type)
  {
  }

  // ----------------------------------------------------------------------------
  // Standard_Common_Stones
  // ----------------------------------------------------------------------------

  Standard_Common_Stones::Standard_Common_Stones()
    : Common_Stones( standard )
  {
    std::list<Stone_Type> types = get_standard_stone_types();
    std::list<Stone_Type>::iterator it;
    for( it=types.begin(); it!=types.end(); ++it )
      {
	const Stone_Type &stone_type = *it;
	stone_counts[ stone_type.get_ID() ] = 1;
      }
  }

  // ----------------------------------------------------------------------------
  // Micro_Common_Stones
  // ----------------------------------------------------------------------------

  Micro_Common_Stones::Micro_Common_Stones()
    : Common_Stones( custom )
  {
    stone_counts[ 1 ] = 2;//10;
    stone_counts[ 4 ] = 2;//10;
  }

  // ----------------------------------------------------------------------------
  // Custom_Common_Stones
  // ----------------------------------------------------------------------------

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

  bool operator<( const Field_Pos &p1, const Field_Pos &p2 )
  {
    if( p1.x < p2.x ) return true;
    if( p1.x > p2.x ) return false;
    if( p1.y < p2.y ) return true;
    if( p1.y > p2.y ) return false;
    return false;
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
    if( abs(p1.y-p2.y) + abs(p1.x-p2.x) > 1 ) return false;
    return true;
  }
  Field_Iterator::Direction Field_Iterator::get_direction( Field_Pos from, Field_Pos to )
  {
    switch( from.y - to.y )
    {
      case  1:
	switch( from.x - to.x )
	  {
	  case  0: return Field_Iterator::top;
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
	  case  0: return Field_Iterator::bottom; break;
	  default: return Field_Iterator::invalid_direction;
	  }
	break;
      default: return Field_Iterator::invalid_direction;
    }
  }
  
  Field_Iterator::Direction Field_Iterator::get_far_direction( Field_Pos from, Field_Pos to )
  {
    if( from.y - to.y > 0 )
    {
      return Field_Iterator::top;
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
      return Field_Iterator::bottom;
    }
  }

  Field_Pos Field_Iterator::get_next( Field_Pos p1, Direction dir )
  {
    Field_Iterator it(p1, 0);
    it.Go(dir);
    return it.get_pos();
  }
  
  bool Field_Iterator::is_corner() const
  {
    return current_pos==Field_Pos(0,0) 
      || current_pos==Field_Pos(board->get_x_size()-1,0)
      || current_pos==Field_Pos(0,board->get_y_size()-1) 
      || current_pos==Field_Pos(board->get_x_size()-1,board->get_y_size()-1);
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
  Field_Iterator Field_Iterator::Next_Top() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Top();
    return ret;
  }
  Field_Iterator Field_Iterator::Next_Bottom() const
  {
    Field_Iterator ret( current_pos, board );
    ret.Bottom();
    return ret;
  }
  Field_Iterator Field_Iterator::Next( Direction dir ) const
  {
    switch( dir )
    {
      case right:		return Next_Right(); break;
      case top:			return Next_Top(); break;
      case left:		return Next_Left(); break;
      case bottom:		return Next_Bottom(); break;
      case right_flipped:	return Next_Right(); break;	// flipped iterator movements make no sense but are supported...
      case top_flipped:		return Next_Top(); break;	// flipped iterator movements make no sense but are supported...
      case left_flipped:	return Next_Left(); break;	// flipped iterator movements make no sense but are supported...
      case bottom_flipped:	return Next_Bottom(); break;	// flipped iterator movements make no sense but are supported...
      case invalid_direction:   assert( false );
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
  Field_Iterator &Field_Iterator::Top()
  {
    --current_pos.y;
    return *this;
  }
  Field_Iterator &Field_Iterator::Bottom()
  {
    ++current_pos.y;
    return *this;
  }
  Field_Iterator &Field_Iterator::Go( Direction dir ) 
  {
    switch( dir )
    {
      case right:		return Right(); break;
      case top:			return Top(); break;
      case left:		return Left(); break;
      case bottom:		return Bottom(); break;
      case right_flipped:		return Right(); break;
      case top_flipped:		return Top(); break;
      case left_flipped:		return Left(); break;
      case bottom_flipped:		return Bottom(); break;
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

  // ----------------------------------------------------------------------------
  // Board
  // ----------------------------------------------------------------------------

  Board::Board( const int *field_array, int width, int height,
		const std::list<Stone_Type> &stone_types, Board_Type board_type )
    : board_type( board_type ), stone_types(stone_types)
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
    std::list<Stone_Type>::const_iterator it;
    for(it=stone_types.begin(); it!=stone_types.end(); ++it)
    {
      const Stone_Type &stone = *it;
      id_stone_types[stone.get_ID()] = stone;
    }
  }

  Board::Board( const std::vector< std::vector< Field_State_Type> > field,
		const std::list<Stone_Type> &stone_types, 
		Board_Type board_type )
    : board_type(board_type), stone_types(stone_types), field(field)
  {
    // !!! checks for rectangularity
    std::list<Stone_Type>::const_iterator it;
    for(it=stone_types.begin(); it!=stone_types.end(); ++it)
    {
      const Stone_Type &stone = *it;
      id_stone_types[stone.get_ID()] = stone;
    }
  }

  Board::Board()
  {
  }

  const Stone_Type &Board::get_stone_type(int stone_type_ID) const {
    std::map<int,Stone_Type>::const_iterator it;
    it = id_stone_types.find(stone_type_ID); 
    if( it == id_stone_types.end() ) 
      return invalid_stone_type;
    else
      return it->second;
  }

  bool Board::is_valid_stone_type(int stone_type_ID) const {
    std::map<int,Stone_Type>::const_iterator it;
    it = id_stone_types.find(stone_type_ID); 
    if( it == id_stone_types.end() ) 
      return false;
    else
      return true;
  }

  std::list<Field_Pos> Board::get_free_corners( const Player &player ) const
  {
    std::list<Field_Pos> ret;
    //** determine whether player has already fields on the board
    std::map<Field_State_Type,int> counts = count_fields();
    if( !does_include(counts,player.field_type) || counts[player.field_type] == 0 )
    {
      // find free corner fields of the board
      for( int x=0; x<=1; ++x )
	for( int y=0; y<=1; ++y )
	{
	  Field_Pos corner(x*(get_x_size()-1),y*(get_y_size()-1));
	  Field_Iterator p(corner,this);
	  if( p.is_valid_field() ) 
	  {
	    if( *p == field_empty )
	    {
	      ret.push_back(corner);
	    }
	  }
	}
      return ret;
    }
    //** determine free fields diagonally connected to player
    if(get_x_size() < 0 || get_y_size() < 0 ) return ret; // safety precaution due to raw array access
    bool* corner_field = new bool[get_x_size()*get_y_size()]; // corner_field[x+y*get_x_size()]
    int idx,x,y;
    for( idx=0; idx<get_x_size()*get_y_size(); ++idx )
      corner_field[idx] = false;
    // determine candidates
    for( y=0; y<get_y_size(); ++y )
    {
      for( x=0; x<get_x_size(); ++x )
      {
	if( field[x][y] == player.field_type )
	{
	  for( int i=-1; i<=+1; i+=2 )
	  {
	    for( int j=-1; j<=+1; j+=2 )
	    {
	      Field_Pos corner_pos(x+i, y+j);
	      Field_Iterator p(corner_pos,this);
	      if( p.is_valid_field() ) 
	      {
		if( *p == field_empty )
		{
		  corner_field[corner_pos.x+corner_pos.y*get_x_size()] = true; // potential candidate
		}
	      }
	    }
	  }
	}
      }
    }
    // cross out candidates due to adjacent field of same player
    for( y=0; y<get_y_size(); ++y )
    {
      for( x=0; x<get_x_size(); ++x )
      {
	if( field[x][y] == player.field_type )
	{
	  for( int i=-1; i<=+1; i+=2 )
	  {
	    for( int j=-1; j<=+1; j+=2 )
	    {
	      Field_Pos no_corner_pos;
	      if( j < 0 ) no_corner_pos = Field_Pos(x+i, y);
	      else no_corner_pos = Field_Pos(x, y+i);
	      Field_Iterator p(no_corner_pos,this);
	      if( p.is_valid_field() ) 
	      {
		corner_field[no_corner_pos.x+no_corner_pos.y*get_x_size()] = false;
	      } 
	    }
	  }
	}
      }
    }
    for( y=0; y<get_y_size(); ++y )
    {
      for( x=0; x<get_x_size(); ++x )
      {
	if( corner_field[x+y*get_x_size()] ) 
	{
	  ret.push_back(Field_Pos(x,y));
	}
      }
    }
    delete[] corner_field;
    return ret;
  }

  std::map<Field_State_Type,int> Board::count_fields() const
  {
    std::map<Field_State_Type,int> counts;
    for( int x=0; x<get_x_size(); ++x )
    {
      for( int y=0; y<get_y_size(); ++y )
      {
	Field_State_Type field_type = field[x][y];
	if( !does_include(counts,field_type) )
	  counts[field_type] = 0;
	++counts[field_type];
      }
    }
    return counts;
  }

  Field_Iterator Board::first_field()
  {
    Field_Pos pos( 0, 0 ); 
    return Field_Iterator( pos, this );
  }

  void Board::print()
  {
    for( int y=0; y<get_y_size(); ++y )
      {
	if( y%2 == 0 ) 
	  std::cout << "   ";	// indent
	for( int x=0; x<get_x_size(); ++x )
	  {
	    Field_State_Type top = field.at(x).at(y);
	    switch( top )
	      {
	      case field_removed:
	      case field_empty:
		std::cout << "    ; ";
		break;
	      case field_player1:
		std::cout << std::setw(2) << "P1; ";
		break;
	      case field_player2:
		std::cout << std::setw(2) << "P2; ";
		break;
	      case field_player3:
		std::cout << std::setw(2) << "P3; ";
		break;
	      case field_player4:
		std::cout << std::setw(2) << "P4; ";
		break;
	      default:
		std::cout << std::setw(2) << "<invalid>; ";
		break;
	      }
	  }
	std::cout << std::endl;
      }
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
    // initialize player stones
    std::vector<Player>::iterator it;
    for(it=players.begin(); it!=players.end(); ++it)
    {
      Player &player = *it;
      player.stones = common_stones;
    }
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

    // number of players in game may be more than physical players active since
    // one physical player may control several players within the game
    if( (players.size() < ruleset->min_players) ||
	(players.size() > ruleset->max_players) ) 
      return wrong_number_of_players;

    // check win condition
    // attention: current_player->is_active may not be correct after set_players()/add_player() call
    winner_player_index=win_condition->get_winner(*this);
    if( winner_player_index >= 0 ) // if any player already won
    {
      return finished_scores;
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
	  if( !current_player->is_active ) // if no player is able to move
	  {
	    winner_player_index=win_condition->get_winner(*this);
	    return finished_scores;
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

  std::multimap<int/*score*/,const Player*> Game::get_scores() const
  {
    std::multimap<int/*score*/,const Player*> ret;
    std::map<Field_State_Type,int> field_counts = board.count_fields();
    std::vector<Player>::const_iterator player;
    for( player = players.begin(); player != players.end(); ++player )
    {
      const Player *player_ptr = &*player;
      int score = 0;
      if( does_include(field_counts,player->field_type) )
      {
	score = field_counts[player->field_type];
      }
      ret.insert(std::make_pair(score,player_ptr));
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

      int cnt=0;
      player_src = new_players.begin();
      for( player = players.begin(); player != players.end(); ++player, ++player_src )
      {
	// copy player
	assert( player_src != new_players.end() );
	*player = *player_src;
	player->field_type = Field_State_Type(field_player1 + cnt);
	player->stones = common_stones;
	cnt++;
      }
    }
    else
    {
      players = new_players;

      current_player       = players.begin();
      current_player_index = 0;
      prev_player	   = players.end();
      prev_player_index    = -1;
      int cnt=0;
      for( player = players.begin(); player != players.end(); ++player )
      {
	player->field_type = Field_State_Type(field_player1 + cnt);
	player->stones = common_stones;
	cnt++;
      }
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
    players.back().field_type = Field_State_Type(field_player1+players.size()-1);
    players.back().stones = common_stones;
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

    // change player
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

      if( !is_any_move_possible(*current_player) )
      {
	next_player = true; // player can't move
	++players_skipped;
	if( players_skipped >= players.size() ) 
	{
	  current_player = prev_player;
	  current_player_index = prev_player_index;
 	  return false; // no player may move; choosing previous player
			// is important as workaround for deficiency of
			// network protocol to allow only current
			// player to undo moves
	}
      }
    }while( next_player );
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
    return true;
  }

  // AI rating uses this function; returns prev_player_indices
  std::list<int/*prev_player_index*/> Game::AI_set_current_player( Player &player )
  {
    std::list<int/*prev_player_index*/> ret;
    for( std::size_t i=0; i<players.size(); ++i )
      {
	if( get_current_player().id == player.id ) return ret;
	ret.push_front( prev_player_index );
	choose_next_player();
      }    
    return ret;
  }

  // AI rating uses this function;
  void Game::AI_unset_current_player( const std::list<int> &prev_player_indices )
  {
    std::list<int>::const_iterator it;
    for( it=prev_player_indices.begin(); it!=prev_player_indices.end(); ++it )
      {
	int prev_prev_player_index = *it;
	choose_prev_player( prev_prev_player_index );
      }
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
  // is_any_move_possible

  bool Game::is_any_move_possible()
  {
    std::vector<Player>::iterator it;
    for(it=players.begin(); it!=players.end(); ++it)
    {
      Player &player = *it;
      if( is_any_move_possible(player) ) return true;
    }
    return false;
  }

  bool Game::is_any_move_possible( const Player &player ) const
  {
    return get_possible_moves(player, /*only_one_flip_direction=*/false, /*is_flipped=*/false,
			      /*just_one=*/true).size() > 0; // Attention: currently mouse player cannot enter set moves for flipped stones
  }

  // **********************
  // get_possible_moves

  std::list<Set_Move> Game::get_possible_moves( const Player &player,
						bool only_one_flip_direction, bool is_flipped, bool just_one, 
						bool just_one_per_stone, int just_stone_ID,
						bool just_one_rotation_symmetric, bool random_order ) const
  {
    std::set<int/*stone_ID*/> stones_found;
    std::list<Set_Move> ret;
    std::list<Field_Pos> free_corners = board.get_free_corners(player); // TODO: make return corners for first move
    //** determine possible moves
    std::list<Field_Pos>::iterator it;
    for( it=free_corners.begin(); it!=free_corners.end(); ++it )
    {
      Field_Pos corner = *it;
      //std::cout << "free corner: " << corner.x << "/" << corner.y << std::endl;
      const std::map<int/*stone_ID*/, int/*count*/> &stone_counts = player.stones.get_stone_counts();
      std::map<int/*stone_ID*/, int/*count*/>::const_iterator it;
      for( it=stone_counts.begin(); it!=stone_counts.end(); ++it )
      {
	int stone_ID = it->first;
	int count = it->second;
	if( count < 1 ) continue;
	if( just_one_per_stone && does_include(stones_found,stone_ID) ) continue;
	if( just_stone_ID >=0 && just_stone_ID != stone_ID ) continue;
	const Stone_Type &stone_type = board.get_stone_type(stone_ID);
	bool found_move_for_stone=false;
	// try to place each sub field of the stone on the corner field in any direction of the stone
	for( int flip=0; flip<=1; ++flip )
	{
	  if( only_one_flip_direction && (bool(flip) != is_flipped)) continue;

	  for( int d=(flip?Field_Iterator::right_flipped : Field_Iterator::right); 
	       d<=(flip?Field_Iterator::bottom_flipped : Field_Iterator::bottom); ++d )
	  {
	    Field_Iterator::Direction dir = Field_Iterator::Direction(d);
	    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > &sub_fields
	      = stone_type.get_sub_fields(dir);
	    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it2,it3;
	    // try to place each sub field of the stone on the corner field
	    for( it2=sub_fields.begin(); it2!=sub_fields.end(); ++it2 )
	    {
	      unsigned sub_x = it2->first;
	      unsigned sub_y = it2->second;
	      Field_Pos origin(corner.x-int(sub_x),corner.y-int(sub_y));
	      //std::cout << "  sub: " << dir << " " << sub_x << " " << sub_y << std::endl;
	      bool valid = true;
	      // check that all sub fields are empty
	      for( it3=sub_fields.begin(); it3!=sub_fields.end(); ++it3 )
	      {
		Field_Pos pos(origin.x+it3->first,origin.y+it3->second);
		Field_Iterator p(pos,&board);
		if( !p.is_valid_field() ) { valid=false; break; } // not valid
		if( *p != field_empty ) { valid=false; break; } // not valid
	      }
	      if( valid )
	      {
		// check that all sub fields are not directly adjacent to fields of this player
		for( it3=sub_fields.begin(); it3!=sub_fields.end(); ++it3 )
		{
		  for(int i=-1; i<=1; i+=2) 
		  {
		    for(int j=-1; j<=1; j+=2) 
		    {
		      Field_Pos pos(origin.x+it3->first,origin.y+it3->second);
		      if( j<0 ) pos.x += i;
		      else pos.y += i;
		      Field_Iterator p(pos,&board);
		      if( p.is_valid_field() )
			if( *p == player.field_type ) { valid=false; break; } // not valid
		    }
		  }
		}
		if(valid)
		{
		  ret.push_back(Set_Move(origin,stone_ID,dir));
		  if( just_one ) return ret;
		  found_move_for_stone = true;
		  stones_found.insert(stone_ID);
		  if(just_one_per_stone) break;
		}
	      }
	      if(found_move_for_stone && just_one_per_stone) break;
	    }
	    if( just_one_rotation_symmetric && stone_type.get_rotation_symmetric() ) break;
	    if( found_move_for_stone && just_one_per_stone ) break;
	  }
	  if( just_one_rotation_symmetric && stone_type.get_flip_symmetric() ) break;
	  if( found_move_for_stone && just_one_per_stone ) break;
	}
      }
    }
    if( random_order )
    {
      std::vector<std::list<Set_Move>::iterator> all_moves;
      all_moves.resize(ret.size(),ret.end());
      std::list<Set_Move>::iterator it;
      for( it=ret.begin(); it!=ret.end(); ++it )
      {
	//Set_Move &move = *it;
	// start with random index into all_moves array
	std::size_t random_idx = std::size_t(random(0,all_moves.size()));
	// forward random_idx to a position that is still empty
	while(all_moves[random_idx]!=ret.end()) {
	  ++random_idx; 
	  if( random_idx >= all_moves.size() ) random_idx = 0;
	}
	// insert element at position of random_idx
	all_moves[random_idx] = it;
      }

      std::list<Set_Move> ret2;
      std::vector<std::list<Set_Move>::iterator>::iterator it2;
      for( it2=all_moves.begin(); it2!=all_moves.end(); ++it2 )
      {
	std::list<Set_Move>::iterator ret_it = *it2;
	ret2.splice(ret2.end(),ret,ret_it);
      }

      ret.swap(ret2);
    }
    return ret;
  }

  // **********************
  // get_num_possible_moves

  int Game::get_num_possible_moves() // number of possible moves in current situation
  {
    int ret=0;
    Player &player = get_current_player();
    std::list<Field_Pos> free_corners = board.get_free_corners(player); // TODO: make return corners for first move
    //** determine possible moves
    std::list<Field_Pos>::iterator it;
    for( it=free_corners.begin(); it!=free_corners.end(); ++it )
    {
      Field_Pos corner = *it;
      //std::cout << "free corner: " << corner.x << "/" << corner.y << std::endl;
      const std::map<int/*stone_ID*/, int/*count*/> &stone_counts = player.stones.get_stone_counts();
      std::map<int/*stone_ID*/, int/*count*/>::const_iterator it;
      for( it=stone_counts.begin(); it!=stone_counts.end(); ++it )
      {
	int stone_ID = it->first;
	int count = it->second;
	if( count < 1 ) continue;
	const Stone_Type &stone_type = board.get_stone_type(stone_ID);
	// try to place each sub field of the stone on the corner field in any direction of the stone
	for( int flip=0; flip<=1; ++flip )
	{
	  for( int d=(flip?Field_Iterator::right_flipped : Field_Iterator::right); 
	       d<=(flip?Field_Iterator::bottom_flipped : Field_Iterator::bottom); ++d )
	  {
	    Field_Iterator::Direction dir = Field_Iterator::Direction(d);
	    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > &sub_fields
	      = stone_type.get_sub_fields(dir);
	    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it2,it3;
	    // try to place each sub field of the stone on the corner field
	    for( it2=sub_fields.begin(); it2!=sub_fields.end(); ++it2 )
	    {
	      unsigned sub_x = it2->first;
	      unsigned sub_y = it2->second;
	      Field_Pos origin(corner.x-int(sub_x),corner.y-int(sub_y));
	      //std::cout << "  sub: " << dir << " " << sub_x << " " << sub_y << std::endl;
	      bool valid = true;
	      // check that all sub fields are empty
	      for( it3=sub_fields.begin(); it3!=sub_fields.end(); ++it3 )
	      {
		Field_Pos pos(origin.x+it3->first,origin.y+it3->second);
		Field_Iterator p(pos,&board);
		if( !p.is_valid_field() ) { valid=false; break; } // not valid
		if( *p != field_empty ) { valid=false; break; } // not valid
	      }
	      if( valid )
	      {
		// check that all sub fields are not directly adjacent to fields of this player
		for( it3=sub_fields.begin(); it3!=sub_fields.end(); ++it3 )
		{
		  for(int i=-1; i<=1; i+=2) 
		  {
		    for(int j=-1; j<=1; j+=2) 
		    {
		      Field_Pos pos(origin.x+it3->first,origin.y+it3->second);
		      if( j<0 ) pos.x += i;
		      else pos.y += i;
		      Field_Iterator p(pos,&board);
		      if( p.is_valid_field() )
			if( *p == player.field_type ) { valid=false; break; } // not valid
		    }
		  }
		}
		if(valid)
		{
		  ++ret;
		}
	      }
	    }
	    if( true/*just_one_rotation_symmetric*/ && stone_type.get_rotation_symmetric() ) break;
	  }
	  if( true/*just_one_rotation_symmetric*/ && stone_type.get_flip_symmetric() ) break;
	}
      }
    }
    return ret;
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
  // Set_Move
  // ----------------------------------------------------------------------------

  Move::Move_Type Set_Move::get_type() const
  {
    return set_move;
  }
  void Set_Move::do_move( Game &game )
  {
    do_move(game,false);
  }
  void Set_Move::do_move( Game &game, bool no_update_player )
  {
    assert( check_move(game) );
    if(!game.board.is_valid_stone_type(stone_type_ID)) return; // protect against malitious network input
    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields
      = game.board.get_stone_type(stone_type_ID).get_sub_fields(dir);
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned sub_x = it->first;
      unsigned sub_y = it->second;
      Field_Iterator p1(Field_Pos(pos.x+sub_x,pos.y+sub_y),&game.board);
      assert( p1.is_valid_field() );
      assert( *p1 == field_empty );
      game.board.field[pos.x+sub_x][pos.y+sub_y] = game.current_player->field_type;
    }
    if(!no_update_player)
    {
      int stone_count = game.current_player->stones.get_stone_count(stone_type_ID);
      assert(stone_count > 0);
      game.current_player->stones.set_stone_count(stone_type_ID,stone_count-1);
    }
  }

  void Set_Move::undo_move( Game &game )
  {
    undo_move(game,false);
  }

  void Set_Move::undo_move( Game &game, bool no_update_player )
  {
    if(!game.board.is_valid_stone_type(stone_type_ID)) return; // protect against malitious network input
    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields
      = game.board.get_stone_type(stone_type_ID).get_sub_fields(dir);
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned sub_x = it->first;
      unsigned sub_y = it->second;
      Field_Iterator p1(Field_Pos(pos.x+sub_x,pos.y+sub_y),&game.board);
      assert( p1.is_valid_field() );
      if(!no_update_player) assert( *p1 == game.current_player->field_type );
      game.board.field[pos.x+sub_x][pos.y+sub_y] = field_empty;
    }
    if(!no_update_player)
    {
      int stone_count = game.current_player->stones.get_stone_count(stone_type_ID);
      assert(stone_count >= 0);
      game.current_player->stones.set_stone_count(stone_type_ID,stone_count+1);
    }
  }

  // true: move ok
  bool Set_Move::check_move( Game &game ) const 
  {
    // check whether stone type is valid
    if(!game.board.is_valid_stone_type(stone_type_ID)) return false; // protect against malitious network input
    // check whether it is first stone to be placed
    bool first = false;
    if( game.current_player->stones.get_total_stone_count() == game.common_stones.get_total_stone_count() )
      first = true;
    // check stone count
    int stone_count = game.current_player->stones.get_stone_count(stone_type_ID);
    check_fail_reason = STONE_NOT_AVAILABLE;
    if(stone_count < 1) return false;
    // check free space and check whether one field is corner
    bool corner_field = false;
    const std::list<std::pair<unsigned/*x*/,unsigned/*y*/> > sub_fields
      = game.board.get_stone_type(stone_type_ID).get_sub_fields(dir);
    std::list<std::pair<unsigned/*x*/,unsigned/*y*/> >::const_iterator it;
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned sub_x = it->first;
      unsigned sub_y = it->second;
      Field_Iterator p1(Field_Pos(pos.x+sub_x,pos.y+sub_y),&game.board);
      check_fail_reason = OUTSIDE;
      if(!p1.is_valid_field()) return false;
      check_fail_reason = OVERLAP;
      if(*p1 != field_empty) return false;
      if( p1.is_corner() ) corner_field = true;
    }
    // check no adjacent field from player
    for( it=sub_fields.begin(); it!=sub_fields.end(); ++it )
    {
      unsigned sub_x = it->first;
      unsigned sub_y = it->second;
      Field_Iterator p1(Field_Pos(pos.x+sub_x,pos.y+sub_y),&game.board);
      for( int dir = Field_Iterator::right;
	   dir <= Field_Iterator::bottom; ++dir )
      {
	Field_Iterator p2 = p1.Next( Field_Iterator::Direction(dir) );
	if(p2.is_valid_field()) {
	  check_fail_reason = OWN_ADJACENT;
	  if(*p2 == game.current_player->field_type) return false; // adjacent field not allowed
	}
      }
    }
    if( first )
      {
	// the first stone must be placed in a corner
	if(!corner_field) std::cout << "check fail reason: FIRST_NO_CORNER" << std::endl;
	check_fail_reason = FIRST_NO_CORNER;
	if( !corner_field ) return false;
      }
    else
      {
	// check at least one diagonally-adjacent field from player
	bool diagonally_adjacent_found = false;
	const std::list<std::pair<int/*x*/,int/*y*/> > corners
	  = game.board.get_stone_type(stone_type_ID).get_corners(dir);
	std::list<std::pair<int/*x*/,int/*y*/> >::const_iterator it2;
	for( it2=corners.begin(); it2!=corners.end(); ++it2 )
	  {
	    int sub_x = it2->first;
	    int sub_y = it2->second;
	    Field_Iterator p(Field_Pos(pos.x+sub_x,pos.y+sub_y),&game.board);
	    //std::cout << "check diagonal " << sub_x << "/" << sub_y << " valid=" << p.is_valid_field() << std::endl; 
	    if( p.is_valid_field() )
	      {
		if( *p == game.current_player->field_type )
		  {
		    diagonally_adjacent_found = true;
		    break;
		  }
	      }
	  }
	check_fail_reason = OWN_NO_DIAGONAL;
	if(!diagonally_adjacent_found) return false;
      }
    check_fail_reason = NO_VIOLATION;
    return true;
  }
  // true: type ok
  bool Set_Move::check_previous_move( Game &, Move *move ) const 
  {
    return move == 0;
  }
  bool Set_Move::may_be_first_move( Game & ) const
  {
    return true;
  }
  bool Set_Move::may_be_last_move( Game & ) const
  {
    return true;
  }

  std::escape_ostream &Set_Move::output( std::escape_ostream &eos ) const
  {
    eos << get_type() << pos.x << pos.y << stone_type_ID << int(dir);
    return eos;
  }
  bool Set_Move::input( std::escape_istream &eis )
  {
    dir = Field_Iterator::invalid_direction;
    int dir_int=-1;
    eis >> pos.x >> pos.y >> stone_type_ID >> dir_int;
    if( (pos.x < 0) || (pos.y < 0) || stone_type_ID < 0 || dir_int < 0 )
      return false;

    switch(dir_int)
    {
    case Field_Iterator::invalid_direction: 
    case Field_Iterator::right:
    case Field_Iterator::right_flipped:
    case Field_Iterator::top:
    case Field_Iterator::top_flipped:
    case Field_Iterator::left:
    case Field_Iterator::left_flipped:
    case Field_Iterator::bottom:
    case Field_Iterator::bottom_flipped:
      dir = Field_Iterator::Direction(dir_int);
    }
    
    return true;
  }
  
  Move *Set_Move::clone() const
  {
    return new Set_Move(*this);
  }
  
  Set_Move::Set_Move()
    : stone_type_ID(-1), dir(Field_Iterator::right)
  {
  }

  Set_Move::Set_Move( Field_Pos pos, int stone_type_ID, Field_Iterator::Direction dir )
    : pos(pos), stone_type_ID(stone_type_ID), dir(dir)
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
      : main_area(0), living(false)
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
	if( m2->is_living() )
	{
	  m1->set_living();
	}
	m2->set_main_area(m1);
      }
    }
    inline void set_living() 
    { 
      get_main_area()->living = true; 
    }
    inline bool is_living()
    {
      return get_main_area()->living;
    }
  private:
    Area *main_area;					 // connected area which stores all relevant data
    bool living;					 // whether this area includes a red stone
  };

  void Finish_Move::do_move( Game & )
  {
  }
  void Finish_Move::undo_move( Game & )
  {
  }
  // true: move ok
  bool Finish_Move::check_move( Game & ) const 
  {
    return true;
  }
  // true: type ok
  bool Finish_Move::check_previous_move ( Game &, Move *move ) const 
  {
    if( !move )
      return false;

    return false; // finish move not needed
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
    if( moves->back()->get_type() != Move::finish_move &&
	moves->back()->get_type() != Move::set_move ) 
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
	case Move::set_move:	move = new Set_Move(); break;
	case Move::finish_move:	move = new Finish_Move(); break;
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
    if( m1.stone_type_ID != m2.stone_type_ID ) return false;
    if( m1.dir != m2.dir ) return false;
    return true;
  }
  bool operator<( const Set_Move &m1, const Set_Move &m2 )
  {
    if( m1.pos.x < m2.pos.x ) return true;
    if( m1.pos.x > m2.pos.x ) return false;
    if( m1.pos.y < m2.pos.y ) return true;
    if( m1.pos.y > m2.pos.y ) return false;
    if( m1.stone_type_ID < m2.stone_type_ID ) return true;
    if( m1.stone_type_ID > m2.stone_type_ID ) return false;
    if( m1.dir < m2.dir ) return true;
    if( m1.dir > m2.dir ) return false;
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
	case Move::finish_move:
	  break;
	case Move::set_move:
	{
	  Set_Move *det_move = static_cast<Set_Move*>(move);
	  ret += long_to_string(det_move->stone_type_ID);
	  switch(det_move->dir)
	  {
	  case Field_Iterator::invalid_direction: 
	    ret += "i "; break;
	  case Field_Iterator::right:
	    ret += "r "; break;
	  case Field_Iterator::right_flipped:
	    ret += "rf "; break;
	  case Field_Iterator::top:
	    ret += "t "; break;
	  case Field_Iterator::top_flipped:
	    ret += "tf "; break;
	  case Field_Iterator::left:
	    ret += "l "; break;
	  case Field_Iterator::left_flipped:
	    ret += "lf "; break;
	  case Field_Iterator::bottom:
	    ret += "b "; break;
	  case Field_Iterator::bottom_flipped:
	    ret += "bf "; break;
	  }
	  ret += coordinate_translator->get_field_name(det_move->pos);
	}
	break;
      }
    }

    return ret;
  }

  Move_Sequence Standard_Move_Translator::decode( std::istream &is )
  {
    Move_Sequence sequence;

    int stone_type_ID;
    std::string dir_str;
    std::string word;
    is >> stone_type_ID;
    is >> dir_str;
    is >> word;

    // EOF, pass and resign may be ignored
    if( !is ) return sequence;
    if( word == "pass" ) return sequence;
    if( word == "resign" ) return sequence;

    Field_Iterator::Direction dir = Field_Iterator::invalid_direction;
    if( dir_str == "r" ) dir = Field_Iterator::right;
    else if( dir_str == "t") dir = Field_Iterator::top;
    else if( dir_str == "l") dir = Field_Iterator::left;
    else if( dir_str == "b") dir = Field_Iterator::bottom;
    else if( dir_str == "rf") dir = Field_Iterator::right_flipped;
    else if( dir_str == "tf") dir = Field_Iterator::top_flipped;
    else if( dir_str == "lf") dir = Field_Iterator::left_flipped;
    else if( dir_str == "bf") dir = Field_Iterator::bottom_flipped;

    if( word.size() >= 2 )
    {
      Field_Pos pos = coordinate_translator->get_field_pos( word );
      Set_Move *move = new Set_Move(pos,stone_type_ID,dir);
      sequence.add_move( move );
      //sequence.add_move( new Finish_Move() ); // no finish move needed
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

  int/*player index*/ Standard_Win_Condition::get_winner( Game &game ) const
  {
    if( game.is_any_move_possible() )
      return -1;

    //count stones:
    std::map<Field_State_Type,int> counts = game.board.count_fields();

    int max_num_fields = 0;
    int winner_index = -1;

    std::map<Field_State_Type,int>::iterator it;
    for( it=counts.begin(); it!=counts.end(); ++it )
    {
      Field_State_Type field_type = it->first;
      int num_fields = it->second;
      if( field_type >= field_player1 && field_type < field_player_end ) 
      {
	if( num_fields > max_num_fields ) 
	{
	  max_num_fields = num_fields;
	  winner_index = field_type - field_player1;
	} 
	else if( num_fields == max_num_fields )
	{
	  winner_index = -1;
	}
      }
    }

    return winner_index;
  }

  float/*player rank[1..n]*/ 
  Standard_Win_Condition::get_player_rank( Game &game, Player &player ) const
  {
    if( game.is_any_move_possible() )
      return -1;

    //count stones:
    std::map<Field_State_Type,int> counts = game.board.count_fields();
    std::map<int, std::set<Field_State_Type> > sorted_counts;

    std::map<Field_State_Type,int>::iterator it;
    for( it=counts.begin(); it!=counts.end(); ++it )
    {
      Field_State_Type field_type = it->first;
      int num_fields = it->second;
      if( field_type >= field_player1 && field_type < field_player_end ) 
      {
	sorted_counts[num_fields].insert(field_type);
      }
    }

    int rank = 1;
    std::map<int, std::set<Field_State_Type> >::reverse_iterator it2;
    for( it2=sorted_counts.rbegin(); it2!=sorted_counts.rend(); ++it2 ) // traverse counts in descending order
    {
      //int num_fields = it2->first;
      const std::set<Field_State_Type> &field_types = it2->second;
      if( does_include(field_types,player.field_type) )
	return rank + (1 - 1.0 / field_types.size());
      else
	rank += field_types.size();
    }

    return -1;
  }

  // ----------------------------------------------------------------------------
  // Standard_Ruleset
  // ----------------------------------------------------------------------------

  Standard_Ruleset::Standard_Ruleset()
    : Ruleset( standard,
	       Board( (const int*) standard_board, 
		      sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
		      sizeof(standard_board)    / sizeof(standard_board[0]), 
		      get_standard_stone_types(), Board::standard ),
	       Standard_Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       true /*undo possible*/, 4, 4 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
  }

  // ----------------------------------------------------------------------------
  // Small_Board_Ruleset
  // ----------------------------------------------------------------------------

  Small_Board_Ruleset::Small_Board_Ruleset()
    : Ruleset( small_board,
	       Board( (const int*) small_board_board, 
		      sizeof(small_board_board[0]) / sizeof(small_board_board[0][0]),
		      sizeof(small_board_board)    / sizeof(small_board_board[0]), 
		      get_standard_stone_types(), Board::small_board ),
	       Standard_Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       true /*undo possible*/, 2, 2 )
  {
    coordinate_translator = new Standard_Coordinate_Translator(board);
  }

  // ----------------------------------------------------------------------------
  // Micro_Ruleset
  // ----------------------------------------------------------------------------

  Micro_Ruleset::Micro_Ruleset()
    : Ruleset( custom,
	       Board( (const int*) micro_board, 
		      sizeof(micro_board[0]) / sizeof(micro_board[0][0]),
		      sizeof(micro_board)    / sizeof(micro_board[0]), 
		      get_standard_stone_types(), Board::custom ),
	       Micro_Common_Stones(),
	       new Standard_Win_Condition(), 0 /*coordinate translator init below */,
	       true /*undo possible*/, 2, 4 )
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

  Sequence_Generator::Sequence_Generator( Game &game )
    : game(game), auto_undo(true), state(begin)
  {
  }

  // synthesize move sequence from clicks
  Sequence_Generator::Sequence_State Sequence_Generator::add_click
  ( Field_Pos pos, bool is_flipped )
  {
    move_done = 0;
    Field_Iterator p1( pos, &game.board );

    switch( state )
    {
      case begin:
	return error_impossible_yet;
      case stone_picked:
	if( Board::is_empty(*p1) )			 // clicked on stone?
	{
	  set_pos = pos;
	  state = stone_pinpointed;
	  // check whether there is only one way of rotating the piece (1x1)
	  const Stone_Type &stone_type = game.board.get_stone_type(picked_stone_type_ID);
	  if( stone_type.query_sub_field(Stone_Type::TOP_LEFT, Field_Iterator::right) 
	      != stone_type.query_sub_field(Stone_Type::BOTTOM_RIGHT, Field_Iterator::right) )
	    return hold2; // there are two points to be placed
	  // fall through since field has no freedom in orientation (1x1)
	}
	else
	  return error_can_t_set_here;
	// fall through since field has no freedom in orientation (1x1)
      case stone_pinpointed:
	if( Board::is_empty(*p1) )			 // clicked on stone?
	{
	  const Stone_Type &stone_type = game.board.get_stone_type(picked_stone_type_ID);
	  Field_Pos set_pos2 = pos;
	  std::pair<Field_Iterator::Direction,Field_Pos/*origin*/> res 
	    = stone_type.get_orientation(set_pos,set_pos2,is_flipped);
	  if( res.first == Field_Iterator::invalid_direction ) 
	  {
	    return error_invalid_orientation;
	  }

	  Set_Move *move = new Set_Move( res.second, picked_stone_type_ID, res.first );
	  if( !move->check_move(game) )
	  {
	    delete move;
	    rule_violation = move->get_check_fail_reason();
	    return error_rule_violation;
	  }

	  if( !sequence.add_move(game,move) )		 // check whether move may be added yet
	  {
	    delete move;
	    return error_rule_violation;
	  }
	  move->do_move(game);
	  move_done = move;
	  
	  state = move_finished;
	  return finished;
	}
	else
	  return error_can_t_set_here;
	break;
      case move_finished:
	return fatal_error;
    }
    assert( false );
    return fatal_error;
  }

  Sequence_Generator::Sequence_State Sequence_Generator::add_click_player_stone
  ( int player_id, int stone_type_ID, int stone_index, bool /*is_flipped*/ )
  {
    move_done = 0;
    if( state != begin )
      return error_impossible_yet;

    if( player_id != game.current_player->id )
      return error_wrong_player;

    if( game.current_player->stones.get_stone_count(stone_type_ID) <= 0 )
      return fatal_error;		 // clicked on stone which is not there?!?

    picked_player_ID = player_id;
    picked_stone_type_ID = stone_type_ID;
    picked_stone_index = stone_index;
    state = stone_picked;

    return hold1;
  }

  Sequence_Generator::Sequence_State Sequence_Generator::undo_click()
  {
    switch( state )
    {
      case begin:
	// nothing to be done (already at beginning of sequence)
	break;
      case stone_picked:
	state = begin;
	break;
      case stone_pinpointed:
	state = stone_picked;
	return hold1;
      default:
	assert(false);
    }
    return another_click;
  }

  // only interprete hold1/hold2/other(another_click)
  Sequence_Generator::Sequence_State Sequence_Generator::get_sequence_state()
  {
    switch( state )
    {
      case stone_picked:
	return hold1;
      case stone_pinpointed:
	return hold2;
      case begin:
      case move_finished:
	return another_click;
    }
    return another_click;
  }

  Move::Move_Type Sequence_Generator::get_required_move_type()
  {
    if( state == move_finished )
      return Move::no_move;

    return Move::set_move;
  }

  // call if get_sequence_state() == hold1 || hold2
  std::list<Field_Pos> Sequence_Generator::get_possible_clicks(bool is_flipped)
  {
    std::list<Field_Pos> ret;
    std::set<Field_Pos> fields;

    switch( state )
    {
      case stone_picked:
	{
	  // consider using get_picked_player_ID() instead of current_player
	  int stone_ID = get_picked_stone_type_ID();
	  std::list<Set_Move> moves = game.get_possible_moves(*game.current_player,
							      /*only_one_flip_direction=*/true,is_flipped, 
							      /*just_one=*/false,
							      /*just_one_per_stone=*/false, stone_ID,
							      /*just_one_rotation_symmetric=*/true,/*random_order=*/false);
	  std::list<Set_Move>::const_iterator it;
	  for( it=moves.begin(); it!=moves.end(); ++it )
	  {
	    const Set_Move &move = *it;
	    const Stone_Type &stone_type = game.board.get_stone_type(stone_ID);
	    std::pair<unsigned/*x*/,unsigned/*y*/> sub_field 
	      = stone_type.query_sub_field(Stone_Type::TOP_LEFT,move.dir);
	    Field_Pos click_pos = Field_Pos(move.pos.x + sub_field.first, move.pos.y + sub_field.second);
	    fields.insert(click_pos);
	  }
	}
	break;
      case stone_pinpointed:
	{
	  // consider using get_picked_player_ID() instead of current_player
	  int stone_ID = get_picked_stone_type_ID();
	  Field_Pos set_pos = get_set_pos();
	  std::list<Set_Move> moves = game.get_possible_moves(*game.current_player,
							      /*only_one_flip_direction=*/true,is_flipped, 
							      /*just_one=*/false,
							      /*just_one_per_stone=*/false, stone_ID,
							      /*just_one_rotation_symmetric=*/false,/*random_order=*/false);
	  std::list<Set_Move>::const_iterator it;
	  for( it=moves.begin(); it!=moves.end(); ++it )
	  {
	    const Set_Move &move = *it;
	    const Stone_Type &stone_type = game.board.get_stone_type(stone_ID);
	    std::pair<unsigned/*x*/,unsigned/*y*/> sub_field 
	      = stone_type.query_sub_field(Stone_Type::TOP_LEFT,move.dir);
	    Field_Pos first_click_pos = Field_Pos(move.pos.x + sub_field.first, move.pos.y + sub_field.second);
	    if( first_click_pos == set_pos )
	    {
	      std::pair<unsigned/*x*/,unsigned/*y*/> sub_field2 
		= stone_type.query_sub_field(Stone_Type::BOTTOM_RIGHT,move.dir);
	      Field_Pos second_click_pos = Field_Pos(move.pos.x + sub_field2.first, move.pos.y + sub_field2.second);
	      fields.insert(second_click_pos);
	    }
	  }
	}
	break;
      default:
	break;
    }

    // copy fields to ret
    std::set<Field_Pos>::const_iterator it;
    for( it=fields.begin(); it!=fields.end(); ++it )
      ret.push_back(*it);

    return ret;
  }

  // call if get_sequence_state() == another_click
  std::list<std::pair<int/*playerID*/,int/*stoneID*/> > 
  Sequence_Generator::get_possible_stone_clicks(bool is_flipped)
  {
    std::list<std::pair<int/*playerID*/,int/*stoneID*/> > ret;

    switch( state )
    {
      case stone_picked:
	return ret;
      case stone_pinpointed:
	return ret;
      default:
	{
	  std::list<Set_Move> moves = game.get_possible_moves(*game.current_player,
							      /*only_one_flip_direction=*/true,is_flipped,
							      /*just_one=*/false, /*just_one_per_stone=*/true);
	  std::list<Set_Move>::const_iterator it;
	  for( it=moves.begin(); it!=moves.end(); ++it )
	  {
	    const Set_Move &move = *it;
	    ret.push_back(std::make_pair(game.current_player->id,move.stone_type_ID));
	  }
	}
	break;
    }

    return ret;
  }

  int Sequence_Generator::get_picked_player_ID() const
  {
    switch( state )
    {
      case stone_picked:
      case stone_pinpointed:
	return picked_player_ID;
      case begin:
      case move_finished:
	return -1;
    }
    assert(false);
    return -1;
  }
  int Sequence_Generator::get_picked_stone_type_ID() const
  {
    switch( state )
    {
      case stone_picked:
      case stone_pinpointed:
	return picked_stone_type_ID;
      case begin:
      case move_finished:
	return -1;
    }
    assert(false);
    return -1;
  }
  int Sequence_Generator::get_picked_stone_index() const
  {
    switch( state )
    {
      case stone_picked:
      case stone_pinpointed:
	return picked_stone_index;
      case begin:
      case move_finished:
	return -1;
    }
    assert(false);
    return -1;
  }
  Field_Pos Sequence_Generator::get_set_pos() const
  {
    switch( state )
    {
      case stone_pinpointed:
	return set_pos;
      case begin:
      case stone_picked:
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

    if( pos.x < 26 ) {
      char letter = 'a' + pos.x;
      name += letter;
    } else {
      char letter1 = 'a' + pos.x/26;
      char letter2 = 'a' + pos.x%26;
      name += letter1;
      name += letter2;
    }
    name += long_to_string( pos.y + 1 );
    return name;
  }

  Field_Pos Standard_Coordinate_Translator::get_field_pos ( std::string name )
  {
    Field_Pos pos;
    if( name.size() >= 2 )
    {
      int x=-1,y=-1;
      std::string num_string = name.substr(1);
      // start interpreting with the assumption of a 1-letter case
      if( (name[0] >= 'a' && name[0] <= 'z') )
	x = name[0] - 'a';
      else if( (name[0] >= 'A' && name[0] <= 'Z') )
	x = name[0] - 'A';
      else
	return pos; // error
      // check whether it is 2-letter case
      if( name.size() >= 3 && ((name[1] >= 'a' && name[1] <= 'z')  || (name[1] >= 'A' && name[1] <= 'Z') ) ) 
      {
	num_string = name.substr(2); // handle 2-letter case
	if( (name[1] >= 'a' && name[1] <= 'z') )
	  x = x*26 + name[1] - 'a';
	else if( (name[1] >= 'A' && name[1] <= 'Z') )
	  x = x*26 + name[1] - 'A';
	else
	  return pos; // error
      }
      // decode y-coordinate
      std::pair<long,unsigned/*digits*/> num = string_to_long( num_string );
      if( num.second == num_string.size() )
      {
	y = num.first-1;
      }
      if( x >= 0 && y >= 0 )
      {
	pos.x = x;
	pos.y = y;
      }
    }
    return pos;
  }

  Coordinate_Translator *Standard_Coordinate_Translator::clone()
  {
    return new Standard_Coordinate_Translator( orig_board );
  }

  std::string DEBUG_translate_move(const Move_Sequence& sequence)
  {
    Board board( (const int*) standard_board, 
		 sizeof(standard_board[0]) / sizeof(standard_board[0][0]),
		 sizeof(standard_board)    / sizeof(standard_board[0]), 
		 get_standard_stone_types(), Board::standard );
    Standard_Coordinate_Translator coordinate_translator(board);
    Standard_Move_Translator move_translator(&coordinate_translator);
    return move_translator.encode(sequence);
  }
}

