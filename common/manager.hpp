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

#if (defined(VERSION_ZERTZ) && defined(VERSION_DVONN))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_DVONN defined"
#endif
#if (defined(VERSION_ZERTZ) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_BLOKS defined"
#endif
#if (defined(VERSION_ZERTZ) && defined(VERSION_RELAX))
#  error "something went wrong in include sequence: VERSION_ZERTZ and VERSION_RELAX defined"
#endif
#if (defined(VERSION_RELAX) && defined(VERSION_DVONN))
#  error "something went wrong in include sequence: VERSION_RELAX and VERSION_DVONN defined"
#endif
#if (defined(VERSION_RELAX) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_RELAX and VERSION_BLOKS defined"
#endif
#if (defined(VERSION_DVONN) && defined(VERSION_BLOKS))
#  error "something went wrong in include sequence: VERSION_DVONN and VERSION_BLOKS defined"
#endif

#if (defined(VERSION_ZERTZ) && !defined(__ZERTZ_MANAGER__)) || \
  (defined(VERSION_DVONN) && !defined(__DVONN_MANAGER__)) || \
  (defined(VERSION_BLOKS) && !defined(__BLOKS_MANAGER__)) || \
  (defined(VERSION_RELAX) && !defined(__RELAX_MANAGER__))

#if defined(VERSION_ZERTZ)
#  define __ZERTZ_MANAGER__
//#  warning "using zertz..."
#elif defined(VERSION_DVONN)
#  define __DVONN_MANAGER__
//#  warning "using dvonn..."
#elif defined(VERSION_BLOKS)
#  define __BLOKS_MANAGER__
//#  warning "using bloks..."
#elif defined(VERSION_RELAX)
#  define __RELAX_MANAGER__
//#  warning "using relax..."
#else
#  error "Please define either VERSION_ZERTZ or VERSION_DVONN or VERSION_BLOKS or VERSION_RELAX"
#endif

#if defined(VERSION_ZERTZ)
namespace zertz
#elif defined(VERSION_DVONN)
namespace dvonn
#elif defined(VERSION_BLOKS)
namespace bloks
#elif defined(VERSION_RELAX)
namespace relax
#endif
{
  class Game_Manager;
  class Game_UI_Manager;
  class Game_Setup_Manager;
  class Standalone_Game_Setup_Manager;
}

#if defined(VERSION_ZERTZ)
#  undef VERSION_ZERTZ
#  include "zertz/ai.hpp"
#  include "zertz/zertz.hpp"
#  define VERSION_ZERTZ
#elif defined(VERSION_DVONN)
#  undef VERSION_DVONN
#  include "dvonn/ai.hpp"
#  include "dvonn/dvonn.hpp"
#  define VERSION_DVONN
#elif defined(VERSION_BLOKS)
#  undef VERSION_BLOKS
#  include "bloks/ai.hpp"
#  include "bloks/bloks.hpp"
#  define VERSION_BLOKS
#elif defined(VERSION_RELAX)
#  undef VERSION_RELAX
#  include "relax/ai.hpp"
#  include "relax/relax.hpp"
#  define VERSION_RELAX
#endif

#include <wx/string.h>

#if defined(VERSION_ZERTZ)
namespace zertz
#elif defined(VERSION_DVONN)
namespace dvonn
#elif defined(VERSION_BLOKS)
namespace bloks
#elif defined(VERSION_RELAX)
namespace relax
#endif
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
    virtual void enter_player_setup() = 0; // allow to enter the player setup process
    virtual void everything_ready() = 0;
    virtual void aborted() = 0;
    virtual void game_started() = 0;
    virtual bool ask_new_game( wxString who ) = 0; // other player asks for a new game (true: accept)
    virtual bool ask_undo_moves( wxString who, int n = 2 ) = 0; // other player asks to undo a move
								// (true: accept)
  };

  /*! abstract class Game_Setup_Manager
   *  manages the setup of a game
   */
  class Game_Setup_Manager
  {
  public:
    enum Game_State { everyone_ready, not_ready, too_few_players, too_many_players };
    enum Answer_Type { accept, deny, wait_for_answer };
    enum Type { alone, server, client };
    virtual Type get_type() = 0;
    virtual void set_display_handler( Game_Setup_Display_Handler * ) = 0;

    // board commands
    virtual Answer_Type ask_change_board( const Game &game ) = 0;
    virtual const Game &get_board()   = 0;
    virtual const std::list<Player> &get_players() = 0;
    // player commands
    virtual bool add_player( const Player & ) = 0;
    virtual bool remove_player( const Player & ) = 0;
    virtual bool player_up( const Player & ) = 0;
    virtual bool player_down( const Player & ) = 0;
    virtual void ready() = 0;      // ready with adding players
    // game commands
    virtual std::list<Player> enable_player_feedback() = 0; // returns players before feedback
    virtual void disable_player_feedback() = 0; // disables feedback about player changes
    virtual bool can_choose_board() = 0; // whether this player can choose a board to play
    virtual bool can_enter_player_setup() = 0; // whether the player setup can be entered
    virtual Game_State can_start() = 0;	// is everyone ready and number of players ok?
    virtual void start_game() = 0; // call only when can_start() == true
    virtual Answer_Type ask_new_game() = 0; // request to play new game
    virtual Answer_Type ask_undo_moves(int n=2) = 0; // request to undo n half moves
    virtual void force_new_game(bool on_own=false) = 0; // force new game (may close connections)
    virtual void stop_game() = 0;  // stop game
    virtual void game_setup_entered() = 0;  // game setup entered 
    virtual bool is_allow_game_setup_abort() = 0; // is it allowed to abort the game setup?

    virtual ~Game_Setup_Manager();
  };

  /*! class Game_Manager
   *  manages central game loop
   */
  class Game_Manager
  {
  public:
    Game_Manager( int game_event_id, Game_UI_Manager *ui_manager=0 );
    ~Game_Manager();
    void set_ui_manager( Game_UI_Manager * );
    Game_UI_Manager *get_ui_manager() { return ui_manager; }

    void start_game();
    void continue_game();
    void new_game();		// request to start new game
    void force_new_game( bool on_own=false ); // start a new game (may disconnect game connection)
    void new_game_accepted();
    void new_game_denied();
    void undo_moves( int num_undo=-1 );
    void undo_accepted();
    void undo_denied();
    void do_undo_moves( int n=2 );
    void stop_game();

    void set_board  ( const Game& );
    //void set_players( const std::list<Player>& );

    AI_Input *get_hint_ai();
    AI_Input *get_player_ai();

    inline const Game &get_game() { return game; }
    inline Game_Setup_Manager *get_game_setup_manager() { return game_setup_manager; }

    inline void set_game_setup_manager( Game_Setup_Manager *sm )
    { game_setup_manager = sm; }
    inline void set_game_setup_display_handler( Game_Setup_Display_Handler *sm )
    { game_setup_display_handler = sm; }
    inline bool is_out_of_order_game() { return game.is_out_of_order(); }
  private:
    Game_Setup_Manager *game_setup_manager;
    Game_Setup_Display_Handler *game_setup_display_handler;
    Game_UI_Manager *ui_manager;
    Game game;
    bool valid_game, game_stopped;
    
    AI_Input *ai;		// may be replaced by multiple AIs

    bool undo_requested, new_game_requested;
    int num_undo_moves;
  // game specific event ID
    int game_event_id;
  };

  class Variants_Display_Notifiee
  {
  public:
    virtual ~Variants_Display_Notifiee() {}
    virtual void selected_variant( std::list<unsigned> variant_id_path ) = 0;
  };

  class Variants_Display_Manager
  {
  public:
    Variants_Display_Manager() : notifiee(0), allow_selection(false) {}
    virtual ~Variants_Display_Manager() {}

    virtual void reset() = 0;
    virtual void show_variant_tree(const Variant_Tree &,
				   Coordinate_Translator *) = 0;
    void set_notifiee(Variants_Display_Notifiee* _notifiee) { notifiee = _notifiee; }
    void set_allow_selection( bool allow ) { allow_selection = allow; }
  protected:
    Variants_Display_Notifiee* notifiee;
    bool allow_selection;
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
    virtual void report_scores( const Game* game, 
				std::multimap<int/*score*/,const Player*> scores ) = 0;
    virtual void report_winner( Player *player ) = 0;
    virtual void report_error( wxString msg, wxString caption ) = 0;
    virtual void report_information( wxString msg, wxString caption ) = 0;
    virtual void reset() = 0;

    // interface for giving information to the user
    virtual void show_user_information( bool visible = true, bool do_refresh = true ) = 0;
    virtual void give_hint( AI_Result ai_result ) = 0;
    virtual void remove_hint() = 0;
    virtual void allow_user_activity() = 0;
    virtual void stop_user_activity() = 0;
    virtual void stop_animations() = 0;
    virtual void abort_all_activity() = 0;
    virtual void do_move_slowly( Move_Sequence sequence, wxEvtHandler *done_handler = 0, 
				 int event_id=-1, int abort_id=-1 ) = 0;
				// show user how move is done
    virtual void undo_move_slowly( wxEvtHandler *done_handler = 0, 
				   int event_id=-1, int abort_id=-1 ) = 0;
				// show user how move is undone
    virtual void show_status_text( wxString text ) = 0; // shows text in status bar
    virtual void beep() = 0;
    virtual void refresh() = 0;

    // interface for information access
    virtual Player_Input *get_user_input() = 0;
    Game &get_display_game() { return display_game; }
    Variant *get_target_variant() { return target_variant; } 
				// attention: this pointer is only
				// valid for current display_game (exchanged every move)
    void clear_target_variant() { target_variant = 0; }

    Game_UI_Manager( const Game &game );
    virtual ~Game_UI_Manager();
  protected:
    Game display_game;

    Variant *target_variant;
  };

  /*! class Standalone_Game_Setup_Manager
   *  manages the setup of a standalone game
   */
  class Standalone_Game_Setup_Manager : public Game_Setup_Manager
  {
  public:
    Standalone_Game_Setup_Manager( Game_Manager & );
    ~Standalone_Game_Setup_Manager();

    virtual Type get_type();
    virtual void set_display_handler( Game_Setup_Display_Handler * );

    // board commands
    virtual Answer_Type ask_change_board( const Game &game );
    virtual const Game &get_board();
    virtual const std::list<Player> &get_players();
    // player commands
    virtual bool add_player( const Player & );
    virtual bool remove_player( const Player & );
    virtual bool player_up( const Player & );
    virtual bool player_down( const Player & );
    virtual void ready();	// ready with adding players
    // game commands
    virtual std::list<Player> enable_player_feedback(); // returns players before feedback
    virtual void disable_player_feedback(); // disables feedback about player changes
    virtual bool can_choose_board(); // whether this player can choose a board to play
    virtual bool can_enter_player_setup(); // whether the player setup can be entered
    virtual Game_State can_start(); // is everyone ready and number of players ok?
    virtual void start_game();  // call only when can_start() == true
    virtual Answer_Type ask_new_game(); // request to play new game
    virtual Answer_Type ask_undo_moves(int n=2); // request to undo n half moves
    virtual void force_new_game(bool on_own=false); // force new game (may close connections)
    virtual void stop_game();   // stop game
    virtual void game_setup_entered();  // game setup entered
    virtual bool is_allow_game_setup_abort(); // is it allowed to abort the game setup?
  private:
    Game_Manager &game_manager;
    Game_Setup_Display_Handler *display_handler;

    Game game;

    int current_id;
    std::list<Player> players;
    std::map<int,std::list<Player>::iterator> id_player; // Table id->player
  };

}

#endif
