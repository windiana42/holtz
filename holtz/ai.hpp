/*
 * ai.hpp
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

#ifndef __HOLTZ_AI__
#define __HOLTZ_AI__

#include "holtz.hpp"

#include <wx/event.h>
#include <wx/timer.h>

#include <map>

namespace holtz
{
  struct Branch;

  class Position_Expanded_Handler
  {
  public:
    virtual bool expanded( Game &, std::list<Branch*>::iterator ) = 0; // return false: don't continue
    virtual ~Position_Expanded_Handler();
  };

  class Field_Permutation
  {
  public:
    Field_Permutation( Board &board );

    Field_Pos get_first();
    bool is_end();
    Field_Pos get_next();

    void new_context();
    void restore_context();
  private:
    std::vector<Field_Pos> position_vector;
    int num_valid_fields;

    struct Context
    {
      int start, jump, cur_pos;
      Context( int start, int jump, int cur_pos );
    };
    std::stack<Context> contexts;
    Context *current_context;
  };

  class Position
  {
  public:
    Position( Game & );
    Position( bool knock_out_possible );
    ~Position();

    void calc_following_positions( Game &, Field_Permutation &, 
				   std::vector<Stones::Stone_Type> &stone_types );
    inline bool is_expanded() { return expanded; }
    void set_expanded_handler( Position_Expanded_Handler * );

    bool knock_out_possible;
    std::list< Branch* > following_positions;
    bool expanded;

    double rating;
    std::multimap<double,std::list<Branch*>::iterator> sorted_positions;
    typedef std::pair<double,std::list<Branch*>::iterator> sorted_positions_element_type;
  private:
    // return false: don't continue
    bool add_knock_out_moves( Game &game, Sequence &sequence, 
			      Field_Iterator from, Field_Iterator over, Field_Iterator to );

    bool add_set_moves( Game &game, Field_Iterator to, Stones::Stone_Type type,
			Field_Permutation &field_permutation );
    bool add_branch( Game &game, Branch *branch );

    Position_Expanded_Handler *handler;
  };

  struct Branch
  {
    Sequence sequence;
    Position position;
    Branch( Sequence, Position );
  };

  struct AI_Result
  {
    Sequence sequence;
    double rating;
    long average_time;	// in milli seconds

    AI_Result();
    AI_Result( Sequence, double rating, long average_time );
  };

  class AI : public Position_Expanded_Handler
  {
  public:
    AI( Game &game );
    AI_Result get_move( Game &game );

    double rate_player( Player & );
    double rate_position( Game &, Position & );
    double depth_search( Game &, Position &, unsigned depth, double max, int min_max_vz,
			 bool knock_out_only );
  protected:
    bool should_stop( bool depth_finished );	// determines whether AI should stop now
    // position expanded return false: don't continue
    virtual bool expanded( Game &, std::list<Branch*>::iterator );  

    wxStopWatch stop_watch;
    long max_time, average_time, real_average_time;
    int num_measures;

    std::list<Player>::iterator current_player;
    unsigned cur_depth;

    std::vector<double> max_ratings; // for min-max strategy

    unsigned positions_checked;
    unsigned expanded_calls;

    const double rate_white, rate_gray, rate_black, rate_current_player_bonus;
    unsigned min_depth;
    bool deep_knocking_possible;// activates deep knocking strategy
  private:
    bool aborted;		// aborted depth because of time
    bool don_t_abort;
    bool deep_knocking;		// whether knock_out moves should be done to rate a position
    bool while_deep_knocking;
    unsigned max_depth_expanded;// the highest depth that was really expanded

    Field_Permutation field_permutation;    
    std::vector<Stones::Stone_Type> stone_types;

    //storage of recursion parameters:
    Position *position;
    unsigned depth; double max;
    int min_max_vz; bool knock_out_only;
  };

  class AI_Thread : public wxThread
  {
  public:
    AI_Thread( wxEvtHandler *handler, int id, Game &game );

    virtual ExitCode Entry();
  private:
    wxEvtHandler *handler;
    int id;
    Game &game;
    AI ai;
  };

  BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(wxEVT_REPORT_MOVE, wxEVT_USER_FIRST + 1) //**/
  END_DECLARE_EVENT_TYPES() //**/

  class Report_Move_Event : public wxEvent
  {
  public:
    typedef enum Type{ final_move, current_hint };

    Report_Move_Event();
    Report_Move_Event( int id, AI_Result, Type type = final_move );

    virtual wxEvent *Clone() const { return new Report_Move_Event(*this); }

  public:
    AI_Result ai_result;
    Type type;

  private:
    DECLARE_DYNAMIC_CLASS(Report_Move_Event) //**/
  };

  typedef void (wxEvtHandler::*Report_Move_Event_Function) ( Report_Move_Event &event );

  class Game_Window;

  class AI_Input : public Player_Input, public wxEvtHandler
  {
  public:
    AI_Input( Game &, Game_Window & );
    virtual Player_State determine_move() throw(Exception);
    virtual Sequence get_move();

    void on_report_move( Report_Move_Event & );
    void on_done( wxTimerEvent &event );
  protected:
    Game &game;
    Game_Window &game_window;
    Sequence sequence;
    bool ai_done, move_done;
  };

  enum
  {
    AI_REPORT_MOVE = 300,
    AI_CURRENT_HINT
  };
}

#endif
