/*
 * animations.hpp
 * 
 * GUI implementation
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

#ifndef __HOLTZ_ANIMATIONS__
#define __HOLTZ_ANIMATIONS__

namespace holtz
{
  class Bitmap_Move_Animation;
  class Move_Sequence_Animation;
}

#include "wxholtz.hpp"
#include "holtz.hpp"

#include <vector>

namespace holtz
{
  class Bitmap_Move_Animation : public wxTimer
  {
  public:
    Bitmap_Move_Animation( Game_Window & );
    ~Bitmap_Move_Animation();

    // when to = (-1,-1) the bitmap is moved randomly out of sight to the top or left
    bool move( wxBitmap bitmap, wxPoint from, wxPoint to, unsigned steps=15, unsigned step_rate=20,
	       wxEvtHandler *done_handler = 0, int _event_id = -1 );
  private:
    bool step();		// do step (returns false if timer couldn't be set up)
    void finish();
    virtual void Notify();

    Game_Window &game_window;
    
    bool occupied;
    wxBitmap bitmap;
    unsigned steps, step_rate;
    wxPoint from, to;
    wxEvtHandler *done_handler;
    int event_id;

    int old_bg_x, old_bg_y;
    int old_x, old_y, pos_x, pos_y; // coordinates of last bitmap position
    int bg_width[2], bg_height[2];
    wxBitmap *background[2];
    wxMemoryDC *background_dc[2];
    int old_bg;

    unsigned current_step;
  };

  class Move_Sequence_Animation : public wxEvtHandler
  {
  public:
    Move_Sequence_Animation( Game_Window & );
    ~Move_Sequence_Animation();
    
    bool start( Sequence sequence, Game &game, wxEvtHandler *done_handler = 0, int event_id=-1 );

  private:
    bool step();		// do step (returns false if timer couldn't be set up)
    void on_done( wxTimerEvent &event ); // current sub-animation done
    void finish();

    Game_Window &game_window;
    Bitmap_Move_Animation bitmap_move_animation;

    Sequence sequence;
    Game *game;
    wxEvtHandler *done_handler;
    int event_id;

    typedef enum State_Type { begin, knock_out_jump, knock_out_collects, 
			      setting_stone, finish_sequence, finish_removes, 
			      finished };
    State_Type state;
    std::list<Move*>::const_iterator current_move;

    Field_State_Type save_field;
    std::list< std::pair<Field_Pos,Stones::Stone_Type> >::iterator current_removed_stone;
    std::list< std::pair<Field_Pos,Stones::Stone_Type> >::iterator end_removed_stones;

    //DECLARE_EVENT_TABLE();
  };

  enum
  {
    ANIMATION_DONE = 200
  };
}

#endif
