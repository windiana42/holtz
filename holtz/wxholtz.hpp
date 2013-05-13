/*
 * wxholtz.hpp
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

#ifndef __WXHOLTZ__
#define __WXHOLTZ__

#include <wx/wx.h>
#include <wx/config.h>
#include <list>
#include <utility>

// declare custom event type wx_EVT_HOLTZ_NOTIFY
BEGIN_DECLARE_EVENT_TYPES()
#if defined(MY_WX_MAKING_DLL) && defined(__WIN32__)
DECLARE_EXPORTED_EVENT_TYPE(__declspec(dllexport),wxEVT_HOLTZ_NOTIFY, 0)
#else
DECLARE_EVENT_TYPE(wxEVT_HOLTZ_NOTIFY, 0)
#endif
END_DECLARE_EVENT_TYPES()


namespace holtz {

  struct Dimensions
  {
    int field_x, field_y, field_width, field_height, field_packed_width, field_packed_height;
    int stack_shift_x, stack_shift_y;
    int caption_height, stone_offset, board_x_offset, board_y_offset;
    int board_stones_spacing, player_player_spacing, stones_player_spacing;
    bool rotation_symmetric, rotate_background;

    Dimensions();
    Dimensions( const char **field_xpm );
    Dimensions( wxConfigBase &config, wxBitmap &bitmap ); // load from configuration
  };

  class Basic_Panel
  {
  public:
    Basic_Panel();
    virtual ~Basic_Panel();

    inline int get_x() const { assert(x>=0); return x; }
    inline int get_y() const { assert(y>=0); return y; }
    inline void set_x( int _x ) { x = _x; }
    inline void set_y( int _y ) { y = _y; }
    inline int get_width() const { assert(width>=0); return width; }
    inline int get_height() const { assert(height>=0); return height; }

    inline bool is_in( int _x, int _y ) const 
    { return (_x>=x)&&(_y>=y)&&(_x<x+width)&&(_y<y+height); }

    virtual void calc_dimensions() = 0;
    virtual void draw( wxDC & ) const {}
    virtual void draw_text( wxDC & ) const {}
    virtual void on_click( int /*x*/, int /*y*/ ) const {}
    virtual void on_right_click( int /*x*/, int /*y*/ ) const {}
  protected:
    int x,y;
    int width, height;
  };

  class Spacer : public Basic_Panel
  {
  public:
    Spacer( int width, int height );
    virtual void calc_dimensions() {}
  };
  
  class Horizontal_Sizer : public Basic_Panel
  {
  public:
    enum Align_Type { top, bottom, middle };
    Horizontal_Sizer( Align_Type align = top );
    ~Horizontal_Sizer();

    void add( Basic_Panel *, bool destroy_on_remove = false );
    void erase( Basic_Panel * );
    void clear();

    virtual void calc_dimensions();
    virtual void draw( wxDC &dc ) const;
    virtual void draw_text( wxDC &dc ) const;
    virtual void on_click( int x, int y ) const;
    virtual void on_right_click( int /*x*/, int /*y*/ ) const;
  private:
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> > elements;
    Align_Type align;
  };
  
  class Vertical_Sizer : public Basic_Panel
  {
  public:
    enum Align_Type { left, right, center };
    Vertical_Sizer( Align_Type align = left );
    ~Vertical_Sizer();

    void add( Basic_Panel *, bool destroy_on_remove = false );
    void erase( Basic_Panel * );
    void clear();

    virtual void calc_dimensions();
    virtual void draw( wxDC &dc ) const;
    virtual void draw_text( wxDC &dc ) const;
    virtual void on_click( int x, int y ) const;
    virtual void on_right_click( int /*x*/, int /*y*/ ) const;
  private:
    std::list< std::pair<Basic_Panel*,bool /*destroy*/> > elements;
    Align_Type align;
  };


}

#endif
