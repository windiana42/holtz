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

    Field_Pos get_first( bool new_random = true );
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
			Field_Permutation &field_permutation, bool expand_first );
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
    int depth;
    long used_time, average_time;	// in milli seconds
    int num_measures;			// of average_time
    bool valid;

    AI_Result();
    AI_Result( Sequence, double rating, int depth, 
	       long used_time, long average_time, int num_measures );
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
    virtual bool need_stop_watch() { return true; }
    virtual bool should_stop( bool depth_finished );	// determines whether AI should stop now
    virtual void report_current_hint( AI_Result ) {}
    // position expanded return false: don't continue
    virtual bool expanded( Game &, std::list<Branch*>::iterator );  

    wxStopWatch stop_watch;
    long max_time, average_time, real_average_time;
    int num_measures;
    unsigned max_positions_expanded, max_depth;

    std::list<Player>::iterator current_player;
    unsigned cur_depth;

    std::vector<double> max_ratings; // for min-max strategy

    unsigned positions_checked;
    unsigned expanded_calls;

    const double rate_white, rate_gray, rate_black, rate_current_player_bonus;
    unsigned min_depth;
    bool deep_knocking_possible;// activates deep knocking strategy
  protected:
    bool aborted;		// aborted depth because of time
  private:
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

  class AI_Thread : public wxThread, public AI
  {
  public:
    AI_Thread( wxEvtHandler *handler, Game &game, long last_average_time = -1, 
	       int num_measures = 0, bool give_hints_only = false );

    virtual ExitCode Entry();
    virtual bool need_stop_watch() { return false; }
    virtual bool should_stop( bool depth_finished );	// determines whether AI should stop now
    virtual void report_current_hint( AI_Result hint );
  private:
    wxEvtHandler *handler;
    Game &game;
    bool give_hints_only;
  };

  BEGIN_DECLARE_EVENT_TYPES()
    DECLARE_EVENT_TYPE(EVT_AI_REPORT_MOVE, wxEVT_USER_FIRST + 1) //**/
    DECLARE_EVENT_TYPE(EVT_AI_REPORT_HINT, wxEVT_USER_FIRST + 2) //**/
    DECLARE_EVENT_TYPE(EVT_AI_FINISHED,    wxEVT_USER_FIRST + 3) //**/
  END_DECLARE_EVENT_TYPES() //**/
  
  class AI_Event : public wxEvent
  {
  public:
    AI_Event();
    AI_Event( WXTYPE type, AI_Thread * );
    AI_Event( AI_Result, WXTYPE type, AI_Thread * );

    virtual wxEvent *Clone() const { return new AI_Event(*this); }

  public:
    AI_Result ai_result;
    AI_Thread *thread;

  private:
    DECLARE_DYNAMIC_CLASS(AI_Event) //**/
  };
  
  typedef void (wxEvtHandler::*AI_Event_Function) ( AI_Event &event );

  class Game_Window;

  class AI_Input : public wxEvtHandler, public Player_Input
  {
  public:
    AI_Input( Game &, Game_Window & );
    ~AI_Input();

    virtual Player_State determine_move() throw(Exception);
    virtual Sequence get_move();
    void determine_hints();
    void abort();

    void on_report_move( AI_Event & );
    void on_report_hint( AI_Event & );
    void on_finished( AI_Event & );
    void on_animation_done( wxTimerEvent &event );
  protected:
    Game &game;
    Game_Window &game_window;
    Sequence sequence;
    bool ai_done, move_done;

    AI_Thread *thread;
    bool thread_active;
    bool give_hints;
  };
}

#endif
