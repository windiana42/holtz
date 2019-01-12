/*
 * wxbloks.hpp
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
#include <wx/sound.h>
#include <wx/html/helpctrl.h> // use HTML help

#ifndef __WXBLOKS__
#define __WXBLOKS__

#include "bloks.hpp"
#include "wxholtz.hpp"

namespace bloks
{
  class WX_GUI_Manager;
}

#include "manager.hpp"
#include "network.hpp"
#include "animations.hpp"
#include "ai.hpp"

namespace holtz
{
//#include "wxmain.hpp"
  class Game_Window;
}

namespace bloks
{
//#include "dialogs.hpp"
  // classes defined in dialogs.hpp
  class Game_Dialog;
  class Settings_Dialog;
}

// declare custom event type wx_EVT_BLOKS_NOTIFY
BEGIN_DECLARE_EVENT_TYPES()
#if defined(__WIN32__)
#  if defined(MY_WX_MAKING_DLL)
DECLARE_LOCAL_EVENT_TYPE(wxEVT_BLOKS_NOTIFY, 0)
#  else
DECLARE_LOCAL_EVENT_TYPE(wxEVT_BLOKS_NOTIFY, 0)
#  endif
#else
DECLARE_EVENT_TYPE(wxEVT_BLOKS_NOTIFY, 0)
#endif
END_DECLARE_EVENT_TYPES()

namespace bloks
{
  using namespace holtz;

  // ============================================================================
  // generic classes
  // ============================================================================

  struct Bitmap_Set
  {
    std::map<Field_State_Type,wxBitmap> field_bitmaps;
    std::map<int/*stone_type_ID*/,wxBitmap> stone_bitmaps;
    wxBitmap click_mark, stone_mark, field_mark;
    wxBitmap background;
  };

  class Bitmap_Handler
  {
  public:
    Bitmap_Set normal;
    Bitmap_Set rotated;

    Dimensions dimensions;

    void setup_field_stone_bitmaps();
    void setup_rotated_bitmaps();
  };

  // ============================================================================
  // panel classes
  // ============================================================================

  class Board_Panel : public Basic_Panel
  {
  public:
    struct Settings
    {
      bool rotate_board;	// display board rotated
      bool show_coordinates;
      wxFont coord_font;
      Settings( bool rotate_board = false, bool show_coordinates = true, 
		wxFont coord_font = wxNullFont );
    };

    Board_Panel( Settings &settings, Game_Manager &, WX_GUI_Manager &, Sequence_Generator* & );

    void calc_dimensions();
    void draw( wxDC &dc ) const;
    void draw_text( wxDC &dc ) const;
    void on_click( int x, int y ) const;

    Field_Pos		get_field( int x, int y ) const;
    std::pair<int,int>  get_field_pos( int col, int row ) const;
    Bitmap_Set         &get_bitmap_set() const;
  private:
    Settings &settings;

    Game_Manager &game_manager;
    WX_GUI_Manager &gui_manager;

    Bitmap_Handler &bitmap_handler;
    int board_x, board_y;
    Sequence_Generator* &sequence_generator;
  };

  class Stone_Panel : public Basic_Panel
  {
  public:
    struct Settings
    {
      bool rotate_stones;	// when board is rotated
      bool multiple_stones;	// false: display count on stone
      bool horizontal;
      int max_stones;		// for multiple stones
      wxFont stone_font;
      Settings( bool rotate_stones=false, bool multiple_stones=false, bool horizontal=true, 
		int max_stones=10, wxFont stone_font = wxNullFont );
    };
    enum Field_Pos_Type{ ORIGIN, EXACT_CENTER, EXTENT,     // virtual positions
			 CENTER, TOP_LEFT, BOTTOM_RIGHT }; // existing sub_field positions

    Stone_Panel( Settings &, Field_State_Type player_field, Stones &, Game_Manager &, 
		 WX_GUI_Manager &, Sequence_Generator* & );

    void calc_dimensions();
    void draw( wxDC &dc ) const;
    void draw_text( wxDC &dc ) const;
    void on_click( int x, int y ) const;
    void on_right_click( int x, int y ) const;

    std::pair<int,int> get_field_pos( int stone_index, int/*stone_type_ID*/, Field_Pos_Type ) const;
    std::pair<int,int> get_field_pos( std::pair<int/*stone_type_ID*/,int/*stone_index*/>, 
				      Field_Pos_Type ) const;
    std::pair<int/*stone_type_ID*/,int> get_stone( int x, int y ) const;
    inline bool get_is_flipped() const { return is_flipped; }
  private:
    Settings &settings;

    Field_State_Type player_field;
    Stones &stones;
    Game_Manager &game_manager;
    WX_GUI_Manager &gui_manager;

    Bitmap_Handler &bitmap_handler;
    Sequence_Generator* &sequence_generator;
    mutable bool is_flipped; // whether all stones are flipped upside down (attention: mutable because on_right_click is const => maybe remove const)
  };

  class Player_Panel : public Vertical_Sizer
  {
  public:
    struct Settings
    {
      Settings( const bloks::Stone_Panel::Settings &stone_settings,
		wxFont player_font = wxNullFont );

      bloks::Stone_Panel::Settings stone_settings;

      wxFont player_font;
    };

    Player_Panel( Settings &, Player &, Game_Manager &, WX_GUI_Manager &, Sequence_Generator* & );

    virtual void on_click( int x, int y ) const;

    inline const Stone_Panel &get_stone_panel() const { return stone_panel; }
    inline int get_id() const { return player.id; }
    inline bool get_is_flipped() const { return stone_panel.get_is_flipped(); }
  private:
    Settings &settings;

    Player &player;
    Game_Manager &game_manager;
    WX_GUI_Manager &gui_manager;

    Bitmap_Handler &bitmap_handler;
    Sequence_Generator* &sequence_generator;

    Stone_Panel stone_panel;

    class Header_Panel : public Basic_Panel
    {
    public:
      Header_Panel( Settings&, Player &player );
      void draw_text( wxDC &dc ) const;
      void calc_dimensions();
    private:
      Settings &settings;
      Player &player;
    };
    Header_Panel header_panel;
  };

  class Game_Panel : public Horizontal_Sizer
  {
  public:
    struct Settings
    {
      enum Arrangement_Type { arrange_standard, arrange_player_stones_right };
      Settings( const Board_Panel::Settings &board_settings, 
		const Player_Panel::Settings &player_settings,
		const Stone_Panel::Settings &common_stone_settings,
		Arrangement_Type arrangement = arrange_standard,
		wxString skin_file = wxT(""), wxString beep_file = wxT(""), 
		bool play_sound = false );
      Settings();

      Board_Panel::Settings  board_settings;
      Player_Panel::Settings player_settings;
      Stone_Panel::Settings  common_stone_settings;

      Arrangement_Type arrangement;
      wxString skin_file, beep_file;
      bool play_sound;
    };

    Game_Panel( Settings &, Game_Manager &, WX_GUI_Manager &, Sequence_Generator* & );
    ~Game_Panel();
    virtual void calc_dimensions();
    virtual void on_right_click( int /*x*/, int /*y*/ ) const;

    void remove_players();
    void add_player( Player &player );

    inline const Board_Panel &get_board_panel() const { return board_panel; }
    inline const std::list<Player_Panel*> &get_player_panels() const { return player_panels; }
    const Player_Panel *get_player_panel( int id ) const;
    const wxBitmap &get_background() const;
  private:
    void rearrange_panels();

    Settings &settings;

    Game_Manager &game_manager;
    WX_GUI_Manager &gui_manager;

    Bitmap_Handler &bitmap_handler;
    Sequence_Generator* &sequence_generator;

    Board_Panel board_panel;
    std::list<Player_Panel*> player_panels;
    Vertical_Sizer player_panel_sizer;
  };
  
  // ============================================================================
  // help classes
  // ============================================================================

  class Mouse_Handler : public wxEvtHandler, public Generic_Mouse_Input
  {
  public:
    Mouse_Handler( Game_Manager &, WX_GUI_Manager &, Sequence_Generator* & );

    // inherited from Generic_Mouse_Input
    virtual void init_mouse_input();
    virtual void disable_mouse_input();
    virtual long get_used_time();

    void on_animation_done( wxCommandEvent& );
  private:
    Game_Manager &game_manager;
    WX_GUI_Manager &gui_manager;
    Sequence_Generator* &sequence_generator_hook; // storage position for access 
						  // by Mouse event handler
    wxStopWatch stop_watch;
    long used_time;

    AI_Input *ai;		// for giving hints

    DECLARE_EVENT_TABLE()
  };

  // ============================================================================
  // game classes
  // ============================================================================

  /*! class WX_GUI_Manager
   *  standard GUI implementation for wxWindows
   */
  
  class WX_GUI_Manager : public Game_UI_Manager, public Horizontal_Sizer, 
			 public Variants_Display_Notifiee
  {
  public:
    WX_GUI_Manager( Game_Manager&, Game_Window&, Variants_Display_Manager& );
    ~WX_GUI_Manager();
    virtual void calc_dimensions();

    // interface inherited from Variants_Display_Notifiee
    virtual void selected_variant( std::list<unsigned> variant_id_path );

    // interface for game_manager
    virtual void setup_game_display(); // setup all windows to display game
    virtual void set_board( const Game &game );
    virtual void update_board( const Game &game );
    virtual void report_scores( const Game* game, 
				std::multimap<int/*score*/,const Player*> scores );
    virtual void report_winner( Player *player );
    virtual void report_error( wxString msg, wxString caption );
    virtual void report_information( wxString msg, wxString caption );
    virtual void reset();

    // interface for giving information to the user
    virtual void show_user_information( bool visible = true, bool do_refresh = true );
    virtual void give_hint( AI_Result ai_result );
    virtual void remove_hint();
    virtual void allow_user_activity();
    virtual void stop_user_activity();
    virtual void stop_animations();
    virtual void abort_all_activity();  // stop user activity and animations
    virtual void do_move_slowly( Move_Sequence sequence, wxEvtHandler *done_handler = 0, 
				 int event_id=-1, int abort_id=-2 ); // show user how move is done
    virtual void undo_move_slowly( wxEvtHandler *done_handler = 0, 
    				   int event_id=-1, int abort_id=-2 );// show user how move is undone
    virtual void show_status_text( wxString text ); // shows text in status bar
    virtual void beep();

    // interface for information access
    virtual Player_Input *get_user_input();

    // special display manipulation
    void set_mark( int x, int y );
    void add_stone_mark( std::pair<int/*x*/,int/*y*/> p ) { static_stone_mark_positions.push_back(p); }
    void add_stone_mark( int x, int y ) { static_stone_mark_positions.push_back(std::make_pair(x,y)); }
    void clear_stone_marks() { static_stone_mark_positions.clear(); }
    void add_field_mark( std::pair<int/*x*/,int/*y*/> p ) { static_field_mark_positions.push_back(p); }
    void add_field_mark( int x, int y ) { static_field_mark_positions.push_back(std::make_pair(x,y)); }
    void clear_field_marks() { static_field_mark_positions.clear(); }
    void draw_mark( wxDC &dc );
    wxColour get_background_colour();

    // change settings
    void load_settings();
    void save_settings();
    bool load_skin( wxString filename ); 
    bool load_beep( wxString filename );

    // ui event commands
    void mouse_click_left( int x, int y );
    void mouse_click_right( int x, int y );
    void refresh();

    inline Bitmap_Handler &get_bitmap_handler() { return bitmap_handler; }
    inline const Game_Panel &get_game_panel() const { return game_panel; }
    inline Game_Panel::Settings &get_game_settings() { return game_settings; }
    inline Mouse_Handler &get_mouse_handler() { return mouse_handler; }
    inline Game_Window &get_game_window() { return game_window; }
  private:
    friend class Game_Manager;
    Game_Manager &game_manager;
    Game_Window  &game_window;
    Variants_Display_Manager &variants_display_manager;

    Bitmap_Handler bitmap_handler;
    Game_Panel::Settings game_settings;
    Game_Panel game_panel;
    Sequence_Generator *sequence_generator;
    friend class Settings_Dialog;

    int click_mark_x, click_mark_y; // always changing mark
    std::list< std::pair<int,int> > static_stone_mark_positions; // stone marks used during sequence generation
    std::list< std::pair<int,int> > static_field_mark_positions; // stone marks used during sequence generation
    std::list< std::pair<int,int> > stone_mark_positions; // also in different color
    std::list< std::pair<int,int> > field_mark_positions; // permanent marks

    Move_Sequence_Animation *move_animation;
    AI_Result current_hint;	// move sequence recommented by the AI

    Mouse_Handler mouse_handler;
    bool user_activity_allowed;

#if wxUSE_SOUND
    wxSound sound_beep;
#endif
  };

}

#endif
