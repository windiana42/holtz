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

#ifndef __DVONN_AI__
#define __DVONN_AI__

#include "dvonn.hpp"

#include <wx/event.h>
#include <wx/timer.h>

#include <map>

namespace dvonn
{
  struct Branch;

  class Position_Expanded_Handler
  {
  public:
    virtual bool expanded( Game &, std::list<Branch*>::iterator ) = 0; 
    				// return false: don't continue
    virtual ~Position_Expanded_Handler();
  };

  class Field_Permutation
  {
  public:
    Field_Permutation( const Board &board );

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
    Position( bool set_moves );
    ~Position();

    void calc_following_positions( Game &, Field_Permutation & );
    inline bool is_expanded() { return expanded; }
    void set_expanded_handler( Position_Expanded_Handler * );

    bool set_moves;
    std::list< Branch* > following_positions;
    bool expanded;

    double rating;
    std::multimap<double,std::list<Branch*>::iterator> sorted_positions;
    typedef std::pair<double,std::list<Branch*>::iterator> sorted_positions_element_type;
  private:
    // return false: don't continue
    bool add_jump_moves( Game &game, Field_Pos from );
    bool add_set_move( Game &game, Field_Pos to );
    bool add_branch( Game &game, Branch *branch );

    Position_Expanded_Handler *handler;
  };

  struct Branch
  {
    Move_Sequence sequence;
    Position position;
    Branch( Move_Sequence, Position );
  };

  struct AI_Result
  {
    Move_Sequence sequence;
    double rating;
    int depth;
    long used_time, average_time;	// in milli seconds
    int num_measures;			// of average_time
    bool valid;

    AI_Result();
    AI_Result( Move_Sequence, double rating, int depth, 
	       long used_time, long average_time, int num_measures );
  };

  class Distance_Map
  {
  public:
    struct Field {
      int distance;
      bool optimal;
      Field() : distance(-1), optimal(false) {}
    };
  public:
    Distance_Map( Board & );
    void find_closest_distances( bool across_empty=false );

    std::vector< std::vector< Field > > field; // field[x][y]
    std::list<Field_Pos> red_stones;
    std::list<Field_Pos> white_control;
    std::list<Field_Pos> black_control;
  private:
    Board &board;
  };

  class AI : public Position_Expanded_Handler
  {
  public:
    AI( const Game &game );
    AI_Result get_move( const Game &game );

    double rate_player( Game &, Player &, Distance_Map & );
    double rate_position( Game &, Position & );
    double depth_search( Game &, Position &, unsigned depth, double max, int min_max_vz,
			 bool jump_only );
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

    std::vector<Player>::iterator current_player;
    unsigned cur_depth;

    std::vector<double> max_ratings; // for min-max strategy

    unsigned positions_checked;
    unsigned expanded_calls;

    const double rate_alpha, rate_beta_no_good, rate_beta_not_now, rate_beta_red,
      rate_beta_add_bonus, rate_current_player_bonus;
    unsigned min_depth;
    bool deep_jumping_possible; // activates deep jumping strategy (currently not implemented)
  protected:
    bool aborted;		// aborted depth because of time
  private:
    bool don_t_abort;
    bool deep_jumping;		// whether more jump moves should be done to rate a position
    bool while_deep_jumping;
    unsigned max_depth_expanded;// the highest depth that was really expanded

    Field_Permutation field_permutation;    

    //storage of recursion parameters:
    Position *position;
    unsigned depth; double max;
    int min_max_vz; bool jump_only;
  };

  class AI_Thread : public wxThread, public AI
  {
  public:
    AI_Thread( wxEvtHandler *handler, const Game &game, long last_average_time = -1, 
	       int num_measures = 0, bool give_hints_only = false );

    virtual ExitCode Entry();
    virtual bool need_stop_watch() { return false; }
    virtual bool should_stop( bool depth_finished );	// determines whether AI should stop now
    virtual void report_current_hint( AI_Result hint );
  private:
    wxEvtHandler *handler;
    const Game &game;
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
  
  class Game_Manager;
  class Game_UI_Manager;

  typedef void (wxEvtHandler::*AI_Event_Function) ( AI_Event &event );

  class AI_Input : public wxEvtHandler, public Player_Input
  {
  public:
    AI_Input( Game_Manager&, Game_UI_Manager* );
    ~AI_Input();
    inline void set_ui_manager( Game_UI_Manager *ui ) { ui_manager = ui; }

    virtual Player_State determine_move() throw(Exception);
    virtual Move_Sequence get_move();
    virtual long get_used_time();
    void determine_hints();
    void abort();

    void on_report_move( AI_Event & );
    void on_report_hint( AI_Event & );
    void on_finished( AI_Event & );
    void on_animation_done( wxTimerEvent &event );
  protected:
    Game_Manager    &game_manager;
    Game_UI_Manager *ui_manager;

    Move_Sequence sequence;
    bool ai_done, move_done;
    long used_time;

    AI_Thread *thread;
    bool thread_active;
    bool give_hints;
  };
}

#endif
