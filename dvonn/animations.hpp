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

#ifndef __DVONN_ANIMATIONS__
#define __DVONN_ANIMATIONS__

namespace holtz
{
//#include "wxmain.hpp"
  class Game_Window;
}

namespace dvonn
{
  class Bitmap_Move_Animation;
  class Move_Sequence_Animation;
}

#include "wxdvonn.hpp"
#include "dvonn.hpp"

#include <vector>

namespace dvonn
{
  using namespace holtz;

  /*! class Bitmap_Move_Animation
   *  moves a bitmap continuously over the screen
   */
  class Bitmap_Move_Animation : public wxTimer
  {
  public:
    Bitmap_Move_Animation( Game_Window & );
    ~Bitmap_Move_Animation();

    // when to = (-1,-1) the bitmap is moved randomly out of sight to the top or left
    bool move( wxBitmap bitmap, wxPoint from, wxPoint to, unsigned steps, unsigned step_rate,
	       wxEvtHandler *done_handler, int _event_id, int _abort_id );
    // abort a running animation
    void abort();
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
    int event_id, abort_id;

    int old_bg_x, old_bg_y;
    int old_x, old_y, pos_x, pos_y; // coordinates of last bitmap position
    int bg_width[2], bg_height[2];
    wxBitmap *background[2];
    wxMemoryDC *background_dc[2];
    int old_bg;

    unsigned current_step;
  };

  /*! class Move_Sequence_Animation
   *  shows a zertz move as animation
   */
  class Move_Sequence_Animation : public wxEvtHandler
  {
  public:
    Move_Sequence_Animation( WX_GUI_Manager &, Game_Window & );
    ~Move_Sequence_Animation();
    
    // start to animate move
    bool start( Move_Sequence sequence, Game &game, wxEvtHandler *done_handler = 0, 
		int event_id = -1, int _abort_id = -2 );
    // start to animate undo move
    bool start_undo( Move_Sequence sequence, Game &game, wxEvtHandler *done_handler = 0, 
		     int event_id = -1, int _abort_id = -2 );

    // abort a running animation
    void abort();
  private:
    bool step();	// do step (returns false if timer couldn't be set up)
    bool step_undo();	// do step in undo-animation (returns false if timer couldn't be set up)
    void on_done( wxCommandEvent &event ); // current sub-animation done
    void on_aborted( wxCommandEvent &event ); // current sub-animation aborted
    void finish();

    WX_GUI_Manager &gui_manager;
    Game_Window &game_window;
    Bitmap_Move_Animation bitmap_move_animation;

    Move_Sequence sequence;
    Game *game;
    wxEvtHandler *done_handler;
    int event_id, abort_id;

    enum State_Type { begin, jump_move, setting_stone, setting_stone2, finish_sequence, 
			      finish_sequence2, finished };
    State_Type state;
    std::list<Move*>::const_iterator current_move;
    std::list<Move*>::const_reverse_iterator current_undo_move;

    std::deque<Field_State_Type> save_stack;
    std::list< std::pair<Field_Pos,std::deque<Field_State_Type> > >::iterator current_removed_stone;
    std::list< std::pair<Field_Pos,std::deque<Field_State_Type> > >::iterator end_removed_stones;
    std::vector<Player>::iterator current_player;	// especially needed for undo moves
    Stones *stones;
    bool undo;			// undo=true when animating undoing of move
    //DECLARE_EVENT_TABLE();
  };
}

#endif
