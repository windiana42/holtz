/*
 * manager.hpp
 * 
 * management classes
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

#ifndef __DVONN_MANAGER__
#define __DVONN_MANAGER__

namespace dvonn
{
  class Game_Manager;
  class Game_UI_Manager;
  class Game_Setup_Manager;
  class Standalone_Game_Setup_Manager;
}

#include "dvonn.hpp"
//#include "ai.hpp"

#include <wx/string.h>

namespace dvonn
{
  /*! abstract class Game_Setup_Display_Handler
   *  displays everything during game setup
   */
  class Game_Setup_Display_Handler
  {
  public:
    virtual ~Game_Setup_Display_Handler();

    virtual void game_setup() = 0;
    // board commands
    virtual void set_board( const Game &game ) = 0;
    virtual bool ask_change_board( const Game &game, wxString who ) = 0;
    virtual void change_board( const Game &game ) = 0;
    virtual void board_change_accepted() = 0;
    virtual void board_change_denied() = 0;
    // player commands
    virtual void player_added( const Player & ) = 0;
    virtual void player_removed( const Player & ) = 0;
    virtual void player_up( const Player & ) = 0;
    virtual void player_down( const Player & ) = 0;
    virtual void player_change_denied() = 0;
    virtual void player_ready( const Player & ) = 0;
    // game commands
    virtual void everything_ready() = 0;
    virtual void aborted() = 0;
    virtual bool ask_new_game( wxString who ) = 0; // other player asks for a new game (true: accept)
    virtual bool ask_undo_moves( wxString who, int n=2 ) = 0; // other player asks to undo a move (true: accept)
  };

  /*! abstract class Game_Setup_Manager
   *  manages the setup of a game
   */
  class Game_Setup_Manager
  {
  public:
    typedef enum Game_State { everyone_ready, not_ready, too_few_players, too_many_players };
    typedef enum Answer_Type { accept, deny, wait_for_answer };
    typedef enum Type { alone, server, client };
    virtual Type get_type() = 0;
    virtual void set_display_handler( Game_Setup_Display_Handler * ) = 0;

    // board commands
    virtual Answer_Type ask_change_board( const Game &game ) = 0;
    virtual const Game &get_board()   = 0;
    virtual std::vector<Player> get_players() = 0;
    // player commands
    virtual bool add_player( const Player & ) = 0;
    virtual bool remove_player( const Player & ) = 0;
    virtual bool player_up( const Player & ) = 0;
    virtual bool player_down( const Player & ) = 0;
    virtual void ready() = 0;      // ready with adding players
    // game commands
    virtual Game_State can_start() = 0;	// is everyone ready and number of players ok?
    virtual void start_game() = 0; // call only when can_start() == true
    virtual Answer_Type ask_new_game() = 0; // request to play new game
    virtual Answer_Type ask_undo_moves( int n = 2 ) = 0; // request to undo a move
    virtual void new_game() = 0; // force new game (may close connections)
    virtual void stop_game() = 0; // stop game

    virtual ~Game_Setup_Manager();
  };

  /*! class Game_Manager
   *  manages central game loop
   */
  class Game_Manager
  {
  public:
    Game_Manager( Game_UI_Manager *ui_manager = 0 );
    void set_ui_manager( Game_UI_Manager * );

    void start_game();
    void continue_game();
    void ask_new_game();	// request to start new game
    void new_game_accepted();
    void new_game_denied();
    void ask_undo_moves( int n = 2 );
    void undo_accepted();
    void undo_denied();

    void new_game();		// start a new game (may disconnect game connection)
    void undo_moves( int n = 2 );
    void undo_move();
    void stop_game();

    void set_board  ( const Game& );
    void set_players( const std::vector<Player>& );
    /*
    AI_Input *get_hint_ai();
    AI_Input *get_player_ai();
    */
    inline const Game &get_game() { return game; }
    inline Game_Setup_Manager *get_game_setup_manager() { return game_setup_manager; }
    inline Game_UI_Manager    *get_ui_manager()         { return ui_manager; }

    inline void set_game_setup_manager( Game_Setup_Manager *sm ) // set by Game_Dialog
    { delete game_setup_manager; game_setup_manager = sm; }
  private:
    Game_Setup_Manager *game_setup_manager;
    Game_UI_Manager *ui_manager;
    Game game;

    //AI_Input ai;		// may be replaced by multiple AIs

    bool new_game_requested;
    int undo_moves_requested;	// number of undo moves requested
  };

  /*! abstract class Game_UI_Manager
   *  interface for user interfaces
   */
  class Game_UI_Manager
  {
  public:
    // interface for ui access from game_manager
    virtual void setup_game_display() = 0; // setup all windows to display game
    virtual void set_board( const Game &game ) = 0;
    virtual void update_board( const Game &game ) = 0;
    virtual void report_winner( Player *player ) = 0;
    virtual void report_error( wxString msg, wxString caption ) = 0;
    virtual void report_information( wxString msg, wxString caption ) = 0;
    virtual void reset() = 0;

    // interface for giving information to the user
    virtual void show_user_information( bool visible = true, bool do_refresh = true ) = 0;
    //virtual void give_hint( AI_Result ai_result ) = 0;
    virtual void remove_hint() = 0;
    virtual void allow_user_activity() = 0;
    virtual void stop_user_activity() = 0;
    virtual void do_move_slowly( Move_Sequence sequence, wxEvtHandler *done_handler = 0, 
				 int event_id=-1 ) = 0; // show user how move is done
    virtual void undo_move_slowly( wxEvtHandler *done_handler = 0, 
    				   int event_id=-1 ) = 0; // show user how move is undone
    virtual void show_status_text( wxString text ) = 0; // shows text in status bar
    virtual void beep() = 0;
    virtual void refresh() = 0;

    // interface for information access
    virtual Player_Input *get_user_input() = 0;
    inline Game &get_display_game() { return display_game; }

    Game_UI_Manager( const Game &game );
    virtual ~Game_UI_Manager();
  protected:
    Game display_game;
  };

  /*! class Standalone_Game_Setup_Manager
   *  manages the setup of a standalone game
   */
  class Standalone_Game_Setup_Manager : public Game_Setup_Manager
  {
  public:
    // fetches game from game_manager but initialize no players
    Standalone_Game_Setup_Manager( Game_Manager & );
    ~Standalone_Game_Setup_Manager();

    virtual Type get_type();
    virtual void set_display_handler( Game_Setup_Display_Handler * );

    // board commands
    virtual Answer_Type ask_change_board( const Game &game );
    virtual const Game &get_board();
    virtual std::vector<Player> get_players();
    // player commands
    virtual bool add_player( const Player & );
    virtual bool remove_player( const Player & );
    virtual bool player_up( const Player & );
    virtual bool player_down( const Player & );
    virtual void ready();	// ready with adding players
    // game commands
    virtual Game_State can_start(); // is everyone ready and number of players ok?
    virtual void start_game();  // call only when can_start() == true
    virtual Answer_Type ask_new_game(); // request to play new game
    virtual Answer_Type ask_undo_moves( int n = 2 ); // request to undo a move
    virtual void new_game();	// force new game (may close connections)
    virtual void stop_game();   // stop game
  private:
    Game_Manager &game_manager;
    Game_Setup_Display_Handler *display_handler;

    Game game;

    int current_id;
    std::deque<Player> players;
    std::map<int,std::deque<Player>::iterator> id_player; // Table id->player
  };

}

#endif
