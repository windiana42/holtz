/*
 * wxholtz.cpp
 * 
 * GUI declaration
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

#define MY_WX_MAKING_DLL // for WX MSW using mingw cross-compile
#include "wxholtz.hpp"

// define custom event type wx_EVT_HOLTZ_NOTIFY
DEFINE_EVENT_TYPE(wxEVT_HOLTZ_NOTIFY)

namespace holtz {

  // ----------------------------------------------------------------------------
  // Dimensions
  // ----------------------------------------------------------------------------

  Dimensions::Dimensions()
    : field_x(0), field_y(0), field_width(51), field_height(51), 
      field_packed_width(44), field_packed_height(44),
      stack_shift_x(0), stack_shift_y(4),
      caption_height(30), stone_offset(20), board_x_offset(10), board_y_offset(10),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotation_symmetric(false), 
      rotate_background(false)
  {
  }

  Dimensions::Dimensions( const char **field_xpm )
    : field_x(0), field_y(0), stack_shift_x(0), stack_shift_y(3),      
      caption_height(30), stone_offset(20),
      board_x_offset(10), board_y_offset(10),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotation_symmetric(false), 
      rotate_background(false)
  {
    wxBitmap bitmap(field_xpm);
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)
    field_packed_width  = int(field_width  * 0.866);	// height * cos(30deg)
  }

  Dimensions::Dimensions( wxConfigBase &config, wxBitmap &bitmap )	// load from configuration
    : field_x(0), field_y(0), stack_shift_x(0), stack_shift_y(3),
      caption_height(30), stone_offset(20),
      board_x_offset(10), board_y_offset(70),
      board_stones_spacing(30), player_player_spacing(10), 
      stones_player_spacing(10), rotation_symmetric(false), 
      rotate_background(false)
  {
    field_width  = bitmap.GetWidth();
    field_height = bitmap.GetHeight();
    field_packed_height = int(field_height * 0.866);	// height * cos(30deg)
    field_packed_width  = int(field_width  * 0.866);	// height * cos(30deg)

    int buf;
    bool bool_buf;

    if( config.Read( wxT("field_x"), &buf ) ) field_x = buf;
    if( config.Read( wxT("field_y"), &buf ) ) field_y = buf;
    if( config.Read( wxT("field_width"), &buf ) ) field_width = buf;
    if( config.Read( wxT("field_height"), &buf ) ) field_height = buf;
    if( config.Read( wxT("field_packed_width"), &buf ) ) field_packed_width = buf;
    if( config.Read( wxT("field_packed_height"), &buf ) ) field_packed_height = buf;
    if( config.Read( wxT("stack_shift_x"), &buf ) ) stack_shift_x = buf;
    if( config.Read( wxT("stack_shift_y"), &buf ) ) stack_shift_y = buf;
    if( config.Read( wxT("caption_height"), &buf ) ) caption_height = buf;
    if( config.Read( wxT("stone_offset"), &buf ) ) stone_offset = buf;
    if( config.Read( wxT("board_stones_spacing"), &buf ) ) board_stones_spacing = buf;
    if( config.Read( wxT("player_player_spacing"), &buf ) ) player_player_spacing = buf;
    if( config.Read( wxT("stones_player_spacing"), &buf ) ) stones_player_spacing = buf;
    if( config.Read( wxT("rotation_symmetric"), &bool_buf ) ) rotation_symmetric = bool_buf;
    if( config.Read( wxT("rotate_background"), &bool_buf ) ) rotate_background = bool_buf;
  }

  // ----------------------------------------------------------------------------
  // Basic Panel
  // ----------------------------------------------------------------------------

  Basic_Panel::Basic_Panel()
    : x(-1), y(-1), width(-1), height(-1)
  {
  }
  Basic_Panel::~Basic_Panel()
  {
  }

  // ----------------------------------------------------------------------------
  // Spacer
  // ----------------------------------------------------------------------------

  Spacer::Spacer( int _width, int _height )
  {
    width = _width;
    height = _height;
  }

  // ----------------------------------------------------------------------------
  // Horizontal_Sizer
  // ----------------------------------------------------------------------------

  Horizontal_Sizer::Horizontal_Sizer( Align_Type align )
    : align(align)
  {
  }

  Horizontal_Sizer::~Horizontal_Sizer()
  {
    clear();
  }

  void Horizontal_Sizer::add( Basic_Panel *panel, bool destroy_on_remove )
  {
    elements.push_back( std::pair<Basic_Panel*,bool>(panel, destroy_on_remove) );
  }
  void Horizontal_Sizer::erase( Basic_Panel *panel )
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->first == panel )
      {
	if( element->second )
	  delete panel;

	elements.erase(element);
	break;
      }
    }
    calc_dimensions();
  }
  void Horizontal_Sizer::clear()
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->second )
	delete element->first;
    }
    elements.clear();
    width = 0;
    height = 0;
  }

  void Horizontal_Sizer::draw( wxDC &dc ) const
  {
    //wxBrush brush1(*wxBLACK,wxTRANSPARENT);
    //wxBrush brush2(*wxRED,wxTRANSPARENT);
    //dc.SetBrush(brush1);
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      //dc.DrawRectangle( element->first->get_x(), element->first->get_y(), 
      //		element->first->get_width(), element->first->get_height() );
      element->first->draw(dc);
    }
    //dc.SetBrush(brush2);
    //dc.DrawRectangle( get_x(), get_y(), get_width(), get_height() );
  }

  void Horizontal_Sizer::draw_text( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->draw_text(dc);
    }
  }
  
  void Horizontal_Sizer::on_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_x < element->first->get_x() + element->first->get_width() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }

  void Horizontal_Sizer::on_right_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_x < element->first->get_x() + element->first->get_width() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_right_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }
  void Horizontal_Sizer::calc_dimensions()
  {
    int cur_width = 0;
    int cur_height = 0;
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->set_x( x + cur_width );
      element->first->calc_dimensions();
      if( element->first->get_height() > cur_height )
	cur_height = element->first->get_height();
      cur_width += element->first->get_width();
    }
    width = cur_width;
    height = cur_height;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      switch(align)
      {
	case top:    element->first->set_y(y); break;
	case bottom: element->first->set_y(y +  height - element->first->get_height()); break;
	case middle: element->first->set_y(y + (height - element->first->get_height()) / 2); break;
      }
      element->first->calc_dimensions();
    }
  }

  // ----------------------------------------------------------------------------
  // Vertical_Sizer
  // ----------------------------------------------------------------------------

  Vertical_Sizer::Vertical_Sizer( Align_Type align )
    : align(align)
  {
  }

  Vertical_Sizer::~Vertical_Sizer()
  {
    clear();
  }

  void Vertical_Sizer::add( Basic_Panel *panel, bool destroy_on_remove )
  {
    elements.push_back( std::pair<Basic_Panel*,bool>(panel, destroy_on_remove) );
  }
  void Vertical_Sizer::erase( Basic_Panel *panel )
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->first == panel )
      {
	if( element->second )
	  delete panel;

	elements.erase(element);
	break;
      }
    }
    calc_dimensions();
  }
  void Vertical_Sizer::clear()
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      if( element->second )
	delete element->first;
    }
    elements.clear();
    width = 0;
    height = 0;
  }

  void Vertical_Sizer::draw( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      //dc.DrawRectangle( element->first->get_x(), element->first->get_y(), 
      //		  element->first->get_width(), element->first->get_height() );
      element->first->draw(dc);
    }
    //dc.DrawRectangle( get_x(), get_y(), get_width(), get_height() );
  }
  
  void Vertical_Sizer::draw_text( wxDC &dc ) const
  {
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->draw_text(dc);
    }
  }

  void Vertical_Sizer::on_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_y < element->first->get_y() + element->first->get_height() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }

  void Vertical_Sizer::on_right_click( int cl_x, int cl_y ) const
  {
    if( is_in(cl_x,cl_y) )
    {
      std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::const_iterator element;
      for( element = elements.begin(); element != elements.end(); ++element )
      {
	if( cl_y < element->first->get_y() + element->first->get_height() )
	{
	  if( element->first->is_in(cl_x,cl_y) )
	    element->first->on_right_click( cl_x, cl_y );
	  break;
	}
      }
    }
  }
  void Vertical_Sizer::calc_dimensions()
  {
    int cur_width = 0;
    int cur_height = 0;
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> >::iterator element;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      element->first->set_y( y + cur_height );
      element->first->calc_dimensions();
      if( element->first->get_width() > cur_width )
	cur_width = element->first->get_width();
      cur_height += element->first->get_height();
    }
    width = cur_width;
    height = cur_height;
    for( element = elements.begin(); element != elements.end(); ++element )
    {
      switch(align)
      {
	case left:   element->first->set_x(x); break;
	case right:  element->first->set_x(x +  width - element->first->get_width()); break;
	case center: element->first->set_x(x + (width - element->first->get_width()) / 2); break;
      }
      element->first->calc_dimensions();
    }
  }

}
