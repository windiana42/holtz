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

#include <wx/wx.h>
#include <wx/config.h>
#include <wx/wave.h>
#include <wx/html/helpctrl.h> // use HTML help

#ifndef __WXHOLTZ__
#define __WXHOLTZ__

#include "holtz.hpp"

namespace holtz
{
  class Game_Window;

  // displays changes in the player setup
  class Player_Handler
  {
  public:
    virtual void player_added( const Player & ) = 0;
    virtual void player_removed( const Player & ) = 0;
    virtual void player_up( const Player & ) = 0;
    virtual void player_down( const Player & ) = 0;
    virtual void player_change_denied() = 0;
    virtual void ruleset_changed( Ruleset::type ) = 0;
    virtual void ruleset_change_denied() = 0;
    virtual void aborted() = 0;

    virtual ~Player_Handler();
  };

  // manages the setup of players
  class Player_Setup_Manager
  {
  public:
    virtual void set_player_handler( Player_Handler * ) = 0;
    virtual void new_game() = 0;
    virtual void stop_game() = 0;
    // player commands
    virtual bool add_player( std::string name, Player::Player_Type, Player::Help_Mode ) = 0;
    virtual bool remove_player( int id ) = 0;
    virtual bool player_up( int id ) = 0;
    virtual bool player_down( int id ) = 0;
    virtual bool change_ruleset( Ruleset::type, Ruleset ) = 0;
    virtual bool ready() = 0;		// ready with adding players; game may start

    virtual ~Player_Setup_Manager();
  };

  inline std::string wxstr_to_str( const wxString &str )
  {
#if wxUSE_UNICODE
    return str.mb_str( wxConvUTF8 ).data();
#else
    return str.c_str();
#endif
  }

  inline wxString str_to_wxstr( const std::string &str )
  {
#ifdef wxUSE_UNICODE
    return wxString(str.c_str(), wxConvUTF8);
#else
    return str.c_str();
#endif
  }
}

#include "network.hpp"
#include "dialogs.hpp"
#include "animations.hpp"
#include "ai.hpp"

namespace holtz
{
  class wxHoltz : public wxApp
  {
  public:
    // override base class virtuals
    // ----------------------------
    
    // this one is called on application startup and is a good place for the app
    // initialization (doing it here and not in the ctor allows to have an error
    // return: if OnInit() returns false, the application terminates)
    virtual bool OnInit();

    bool check_config();
    bool init_help(wxLocale&);
    wxHelpControllerBase& get_help_controller() { return help_controller; }

    ~wxHoltz();

  private:
    wxHtmlHelpController help_controller; // HTML help
    wxConfigBase* global_config; // so that it can be deleted
  };

  struct Dimensions
  {
    int field_width, field_height, field_packed_height;
    int caption_height, stone_offset;
    int board_stones_spacing, player_player_spacing, stones_player_spacing;

    Dimensions();
    Dimensions( char **field_xpm );
    Dimensions( wxConfigBase &config, wxBitmap &bitmap ); // load from configuration
  };

  class Game_Window;

  struct Bitmap_Set
  {
    std::map<Field_State_Type,wxBitmap> field_bitmaps;
    std::map<Stones::Stone_Type,wxBitmap> stone_bitmaps;
  };
  class Bitmap_Handler
  {
  public:
    Bitmap_Set normal;
    Bitmap_Set rotated;
    wxBitmap field_mark, field_mark2;
    wxBitmap background;

    Dimensions dimensions;

    void setup_field_stone_bitmaps();
    void setup_rotated_bitmaps();
  };

  class Board_Panel 
  {
  public:
    Board_Panel( Game &, Game_Window &, Bitmap_Handler &,
		 int x, int y, Sequence_Generator* & );

    int get_width() const;
    int get_height() const;
    void set_x( int x );
    void set_y( int y );
    inline int get_x() const { return x; }
    inline int get_y() const { return y; }
    void place_child_windows();

    void draw( wxDC &dc );
    void draw_text( wxDC &dc );

    void on_click( long x, long y );
    Field_Pos		get_field( int x, int y ) const;
    std::pair<int,int>  get_field_pos( int col, int row ) const;
    Bitmap_Set         &get_bitmap_set() const;
  private:
    Game &game;
    Game_Window &game_window;

    Bitmap_Handler &bitmap_handler;
    int x, y, board_x, board_y;
    Sequence_Generator* &sequence_generator;

    bool rotate_board;		// display board rotated
  };

  class Stone_Panel
  {
  public:
    Stone_Panel( Stones &, Game_Window &, Bitmap_Handler &,
		 int x, int y, Sequence_Generator* &, int max_stones = 10 );

    int get_width() const;
    int get_height() const;
    void set_x( int x );
    void set_y( int y );
    inline int get_x() const { return x; }
    inline int get_y() const { return y; }
    void place_child_windows();

    void draw( wxDC &dc );
    void draw_text( wxDC &dc );

    void on_click( long x, long y );
    std::pair<int,int> get_field_pos( int stone_num, Stones::Stone_Type type ) const;
  private:
    Stones &stones;
    Game_Window &game_window;

    Bitmap_Handler &bitmap_handler;
    int x,y;
    Sequence_Generator* &sequence_generator;

    bool rotate_stones;

    int max_stones;
  };

  class Player_Panel 
  {
  public:
    Player_Panel( Player &, Game_Window &, Bitmap_Handler &,
		  int x, int y, Sequence_Generator* & );

    int get_width() const;
    int get_height() const;
    void set_x( int x );
    void set_y( int y );
    inline int get_x() const { return x; }
    inline int get_y() const { return y; }
    void place_child_windows();

    void draw( wxDC &dc );
    void draw_text( wxDC &dc );

    void on_click( long x, long y );
    inline const Stone_Panel &get_stone_panel() const { return stone_panel; }
    inline int get_id() const { return player.id; }
  private:
    Player &player;
    Game_Window &game_window;

    wxFont player_font;
    //wxStaticText *caption_text;

    Bitmap_Handler &bitmap_handler;
    int x,y;
    Sequence_Generator* &sequence_generator;

    Stone_Panel stone_panel;
  };
  
  class Mouse_Handler : public Generic_Mouse_Input
  {
  public:
    Mouse_Handler( Game &, Game_Window &, Sequence_Generator* & );
    
    virtual void init_mouse_input();
    virtual void disable_mouse_input();
  private:
    Game_Window &game_window;
    Sequence_Generator* &sequence_generator_hook; // storage position for access by Mouse event handler

    AI_Input *ai;		// for giving hints
  };

  class Game_Window : public wxScrolledWindow
  {
  public:
    Game_Window( wxFrame *parent_frame, Game & );
    ~Game_Window();
    void on_close();

    int get_width() const;
    int get_height() const;
    void place_child_windows();

    void set_mark( int x, int y );
    void draw_mark( wxDC &dc );
    void show_user_information( bool visible = true, bool do_refresh = true );
    void give_hint( AI_Result ai_result );
    void remove_hint();

    void setup_new_game();
    void setup_network_game();
    void setup_standalone_game();
    void ask_new_game( wxString host );
    void reset( const Ruleset &ruleset );
    void new_game( std::list<Player> players, const Ruleset &ruleset );
    void continue_game();
    void stop_game();
    void allow_user_activity();
    void stop_user_activity();

    void do_move_slowly( Sequence sequence, wxEvtHandler *done_handler = 0, 
			 int event_id=-1 ); // show user how move is done

    void OnDraw( wxDC &dc );
    void on_erase_background( wxEraseEvent &event );
    void on_mouse_event( wxMouseEvent &event );
    void refresh();

    bool load_skin( wxString filename );
    void set_skin_file( wxString filename );
    void set_beep_file( wxString filename );
    void set_play_sound( bool );
    inline bool is_play_sound() { return play_sound; }
    void beep();

    inline wxFrame &get_frame() { return parent_frame; }
    inline AI_Input &get_ai_handler() { return ai; }
    inline Mouse_Handler &get_mouse_handler() { return mouse_handler; }
    inline Bitmap_Handler &get_bitmap_handler() { return bitmap_handler; }
    inline const Board_Panel &get_board_panel() const { return board_panel; }
    inline const Stone_Panel &get_stone_panel() const { return stone_panel; }
    inline const std::list<Player_Panel*> &get_player_panels() const { return player_panels; }
    const Player_Panel *get_player_panel( int id ) const;
  private:
    wxFrame &parent_frame;
    Bitmap_Handler bitmap_handler;

    Board_Panel board_panel;
    std::list<Player_Panel*> player_panels;
    Stone_Panel stone_panel;
    Mouse_Handler mouse_handler;
    AI_Input ai; // artificial inteligence handler for players

    Game &game;
    Sequence_Generator *sequence_generator; // handles mouse clicks
    Move_Sequence_Animation *move_animation;
    AI_Result current_hint;	// move sequence recommented by the AI

    Player_Setup_Manager *player_setup_manager;
    Network_Clients_Dialog *clients_dialog;      
    Player_Setup_Dialog *player_dialog;

    int field_mark_x, field_mark_y; // always changing mark
    std::list< std::pair<int,int> > field_mark_positions;  // permanent marks
    std::list< std::pair<int,int> > field_mark2_positions; // also in different color

    wxString skin_file, beep_file;
    bool play_sound;
#if wxUSE_WAVE
    wxWave sound_beep;
#endif

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
  };

  class Standalone_Player_Setup_Manager : public Player_Setup_Manager
  {
  public:
    Standalone_Player_Setup_Manager( Game_Window & );
    ~Standalone_Player_Setup_Manager();

    virtual void set_player_handler( Player_Handler * );
    virtual void new_game();
    virtual void stop_game();
    // player commands
    virtual bool add_player( std::string name, Player::Player_Type, Player::Help_Mode );
    virtual bool remove_player( int id );
    virtual bool player_up( int id );
    virtual bool player_down( int id );
    virtual bool change_ruleset( Ruleset::type, Ruleset );
    virtual bool ready();		// ready with adding players; game may start
  private:
    Game_Window &game_window;
    Player_Handler *player_handler;

    int current_id;
    std::list<Player> players;
    std::map<int,std::list<Player>::iterator> id_player; // Table id->player

    Ruleset ruleset;
    Ruleset::type ruleset_type;
  };

  // Define a new frame type: this is going to be our main frame
  class Main_Frame : public wxFrame
  {
  public:
    // ctor(s)
    Main_Frame( const wxString& title );
    ~Main_Frame();
    wxMenuBar* create_menu();

    void save_size_and_position();
    wxSize restore_size();
    wxPoint restore_position();

    // event handlers (these functions should _not_ be virtual)
    void on_new_game(wxCommandEvent& event);
    void on_standalone_game(wxCommandEvent& event);
    void on_server(wxCommandEvent& event);
    void on_client(wxCommandEvent& event);
    void on_network_game(wxCommandEvent& event);
    void on_quit(wxCommandEvent& event);
    void on_choose_skin(wxCommandEvent& event);
    void on_choose_beep(wxCommandEvent& event);
    void on_toggle_sound(wxCommandEvent& event);
	void on_help_contents(wxCommandEvent& event);
	void on_help_license(wxCommandEvent& event);
    void on_about(wxCommandEvent& event);
    void on_close(wxCloseEvent& event);

  private:
    Ruleset ruleset;
    Game game;
    Game_Window game_window;

    wxMenu *setting_menu;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE();
  };


  // ----------------------------------------------------------------------------
  // constants
  // ----------------------------------------------------------------------------

  // IDs for the controls and the menu commands
  enum
  {
    // menu items
    HOLTZ_NEW_GAME = 100,
    HOLTZ_STANDALONE_GAME,
    HOLTZ_NETWORK_GAME,
    HOLTZ_SKIN,
    HOLTZ_BEEP,
    HOLTZ_SOUND,
    HOLTZ_QUIT,
	HOLTZ_HELP_CONTENTS,
	HOLTZ_HELP_LICENSE,
    HOLTZ_ABOUT,

    DIALOG_OK,
    DIALOG_READY,
    DIALOG_CANCEL,
    DIALOG_ADD_PLAYER,
    DIALOG_REMOVE_PLAYER,
    DIALOG_PLAYER_UP,
    DIALOG_PLAYER_DOWN,
    DIALOG_DISCONNECT,
    DIALOG_HELP,
    DIALOG_RULESET,

    LISTBOX_DCLICK

  };
}

// declare the wxGetApp() function
DECLARE_APP(holtz::wxHoltz) //**/

#endif
