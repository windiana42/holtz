/*
 * dialogs.hpp
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

#ifndef __WXZERTZ_DIALOG__
#define __WXZERTZ_DIALOG__

#define DEFAULT_PORT 6211

namespace zertz
{
  class Game_Dialog;
  class Settings_Dialog;
  class Network_Clients_Dialog;
}

#include "network.hpp"
#include "zertz.hpp"
#include "wxzertz.hpp"
#include "pbm.hpp"

#include <wx/wx.h>
#include <wx/wizard.h>
#include <wx/spinctrl.h>
#include <list>
#include <map>
#include <string>

namespace zertz
{
  // ============================================================================
  // Game_Dialog
  // ============================================================================

  class Game_Dialog;

  class Setup_Manager_Page : public wxWizardPage
  {
  public:
    Setup_Manager_Page( wxWizard *parent, Game_Dialog & );

    virtual wxWizardPage *GetPrev() const;
    virtual wxWizardPage *GetNext() const;
    virtual bool TransferDataFromWindow();

    void on_server_port( wxSpinEvent& );
    void on_client_port( wxSpinEvent& );
    void on_host_name( wxCommandEvent& );

    void restore();		// display stored game state
  private:
    Game_Dialog &game_dialog;
    bool changes;

    Network_Clients_Dialog *clients_dialog;

    wxRadioButton *alone, *network_server, *network_client, *don_t_change;
    wxTextCtrl *hostname;
    wxSpinCtrl *server_port, *client_port;
    bool alone_val, network_server_val, network_client_val;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Board_Page : public wxWizardPage
  {
  public:
    Board_Page( wxWizard *parent, Game_Dialog & );

    virtual wxWizardPage *GetPrev() const;
    virtual wxWizardPage *GetNext() const;
    virtual bool TransferDataFromWindow();
    wxWizardPage *get_last_board_page() const;

    void on_continue_game_choice  ( wxCommandEvent& event );
    void on_new_game_choice	  ( wxCommandEvent& event );

    void restore();		// display stored game state
  private:
    Game_Dialog &game_dialog;
    bool changes;

    wxRadioButton *new_game, *continue_game, *don_t_change;
    wxRadioBox *new_game_choice, *continue_game_choice;
    bool new_game_val, continue_game_val;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Custom_Board_Setup_Panel : public wxPanel
  {
  public:
    Custom_Board_Setup_Panel( wxWindow *parent, Game_Dialog & );
    ~Custom_Board_Setup_Panel();

    void on_change_board  ( wxCommandEvent& event );
    void on_change_win  ( wxCommandEvent& event );
    void on_spin_win ( wxSpinEvent& event );
    void on_change_stones  ( wxCommandEvent& event );
    void on_spin_stones ( wxSpinEvent& event );

    void restore();
    Game get_board();		// get board according to chosen settings
    bool is_changes() { return changes; }
    void set_changes( bool ch ) { changes = ch; }
  private:
    Game_Dialog &game_dialog;
    bool changes;

    wxRadioBox *board_choice;
    wxRadioBox *win_choice;
    wxSpinCtrl *win_white, *win_grey, *win_black, *win_all;
    wxRadioBox *stones_choice;
    wxSpinCtrl *stones_white, *stones_grey, *stones_black;

    DECLARE_EVENT_TABLE()
  };

  class Custom_Board_Page : public wxWizardPage
  {
  public:
    Custom_Board_Page( wxWizard *parent, Game_Dialog & );

    virtual wxWizardPage *GetPrev() const;
    virtual wxWizardPage *GetNext() const;
    virtual bool TransferDataFromWindow();

    void restore();		// display stored game state
  private:
    Game_Dialog &game_dialog;

    Custom_Board_Setup_Panel *custom_board_panel;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Load_Board_Page : public wxWizardPage
  {
  public:
    Load_Board_Page( wxWizard *parent, Game_Dialog & );

    virtual wxWizardPage *GetPrev() const;
    virtual wxWizardPage *GetNext() const;
    virtual bool TransferDataFromWindow();

    void restore();		// display stored game state
  private:
    bool scan_directory( wxString directory );
    void on_choose_directory( wxCommandEvent& event );
    void on_change_directory( wxCommandEvent& event );

    Game_Dialog &game_dialog;
    bool changes;

    wxRadioBox *pbm_choice;
    wxTextCtrl *pbm_directory;
    wxListBox  *pbm_game_list;

    wxString valid_directory;	// stores only a valid directory path or ""
    std::map< int, std::list< std::pair<PBM_Content,std::string> > > pbm_files; 
				// board_num -> last_move_num -> file
    int pbm_game_list_val;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Player_Setup_Panel : public wxPanel
  {
  public:
    Player_Setup_Panel( wxWindow *parent, Game_Dialog & );
    ~Player_Setup_Panel();

    void on_ready( wxCommandEvent& event );
    void on_player_name( wxCommandEvent& event );
    void on_add_player( wxCommandEvent& event );
    void on_remove_player( wxCommandEvent& event );
    void on_player_up( wxCommandEvent& event );
    void on_player_down( wxCommandEvent& event );

    // as player handler this dialog has to show results of player manipulations
    void player_added( const Player & );
    void player_removed( const Player & );
    void player_up( const Player & );
    void player_down( const Player & );
    void player_change_denied();
    void player_ready( const Player & );

    void restore();		// display stored game state
    void players_changed();	// default players changed
    void everything_ready();	// all players are ready
    void update_status_display();
  private:
    wxString get_default_name( int player_num );
    void set_player_name( wxString name );

    Game_Dialog &game_dialog;

    wxTextCtrl *player_name;
    wxCheckBox *ai;
    wxListBox *player_list;
    std::map<int,Player> item_player;   // item_number->id
    std::map<int,int> player_item; // id->item_number

    wxRadioBox *help_choice;
    wxStaticText *status_display;
    wxButton *ready_button;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Player_Page : public wxWizardPage
  {
  public:
    Player_Page( wxWizard *parent, Game_Dialog & );

    virtual wxWizardPage *GetPrev() const;
    virtual wxWizardPage *GetNext() const;
    virtual bool TransferDataFromWindow();

    void restore();		// display stored game state
    void players_changed();	// default players changed
    void everything_ready();	// all players are ready
    void set_ready(bool r) { ready=r; }
    bool is_ready() { return ready; }
  private:
    Game_Dialog &game_dialog;

    Player_Setup_Panel *player_setup_panel;

    wxNotebook *notebook;

    bool ready;

    friend class Game_Dialog;
    DECLARE_EVENT_TABLE() //**/
  };

  class Game_Dialog : public Game_Setup_Display_Handler
  {
  public:
    Game_Dialog( wxWindow *parent, Game_Manager&, WX_GUI_Manager& );
    ~Game_Dialog();

    virtual void game_setup();
    // board commands
    virtual void set_board( const Game &game );
    virtual bool ask_change_board( const Game &game, wxString who );
    virtual void change_board( const Game &game );
    virtual void board_change_accepted();
    virtual void board_change_denied();
    // player commands
    virtual void player_added( const Player & );
    virtual void player_removed( const Player & );
    virtual void player_up( const Player & );
    virtual void player_down( const Player & );
    virtual void player_change_denied();
    virtual void player_ready( const Player & );
    // game commands
    virtual void enter_player_setup();
    virtual void everything_ready();
    virtual void aborted();
    virtual bool ask_new_game( wxString who ); // other player asks for a new game (true: accept)
    virtual bool ask_undo_moves( wxString who, int n = 2 ); 
				// other player asks to undo a move (true: accept)
  private:
    void get_data_from_setup_manager();

    Game_Manager   &game_manager;
    WX_GUI_Manager &gui_manager;

    Game game;
    std::list<Player> players;
    Game_Setup_Manager *game_setup_manager;

    wxWizard		*wizard;
    Setup_Manager_Page  setup_manager_page;
    Board_Page		board_page;
    Custom_Board_Page   custom_board_page;
    Load_Board_Page     load_board_page;
    Player_Page		player_page;
    wxWizardPageSimple  dummy;
    wxSize best_size;

    friend class Setup_Manager_Page;
    friend class Board_Page;
    friend class Custom_Board_Setup_Panel;
    friend class Custom_Board_Page;
    friend class Load_Board_Page;
    friend class Player_Setup_Panel;
    friend class Player_Page;
  };

  // ============================================================================
  // Settings Dialog
  // ============================================================================

  class Display_Setup_Page : public wxPanel
  {
  public:
    Display_Setup_Page( wxWindow *parent, Settings_Dialog *, WX_GUI_Manager & );
    ~Display_Setup_Page();

    void restore_settings();
    void apply ();
  private:
    Settings_Dialog *dialog;
    WX_GUI_Manager &gui_manager;

    Game_Panel::Settings game_settings;

    wxRadioBox *orientation_choice;
    wxCheckBox *show_coordinates;
    wxRadioBox *arrangement_choice;
    wxCheckBox *multiple_player_stones;
    wxCheckBox *multiple_common_stones;

    DECLARE_EVENT_TABLE()	//**/
  };

  class Look_Feel_Page : public wxPanel
  {
  public:
    Look_Feel_Page( wxWindow *parent, Settings_Dialog *, WX_GUI_Manager & );
    ~Look_Feel_Page();

    void restore_settings();
    void apply();

    void on_choose_skin( wxCommandEvent& event );
    void on_change_skin( wxCommandEvent& event );
    void on_choose_beep( wxCommandEvent& event );
    void on_change_beep( wxCommandEvent& event );
    void on_choose_player_font( wxCommandEvent& event );
    void on_choose_coord_font( wxCommandEvent& event );
    void on_choose_stone_font( wxCommandEvent& event );
  private:
    Settings_Dialog *dialog;
    WX_GUI_Manager &gui_manager;

    wxTextCtrl *skin_file;
    wxTextCtrl *beep_file;
    wxCheckBox *play_sound;
    wxStaticText *player_font_name;
    wxStaticText *coord_font_name;
    wxStaticText *stone_font_name;

    wxString valid_skin_file;
    wxString valid_beep_file;
    wxFont player_font;
    wxFont coord_font;
    wxFont stone_font;

    DECLARE_EVENT_TABLE()	//**/
  };

  class Settings_Dialog : public wxDialog
  {
  public:
    Settings_Dialog( wxWindow *parent, WX_GUI_Manager &);

    void on_ok     ( wxCommandEvent& event );
    void on_apply  ( wxCommandEvent& event );
    void on_restore( wxCommandEvent& event );
    void on_cancel ( wxCommandEvent& event );

  private:
    WX_GUI_Manager &gui_manager;

    wxNotebook *notebook;
    Display_Setup_Page *display_page;
    Look_Feel_Page *look_feel_page;

    Game_Panel::Settings game_settings;
    friend class Display_Setup_Page;
    friend class Look_Feel_Page;

    DECLARE_EVENT_TABLE()	//**/
  };

  // ============================================================================
  // Network_Clients_Dialog
  // ============================================================================

  class Network_Clients_Dialog : public wxDialog, Network_Connection_Handler
  {
  public:
    Network_Clients_Dialog( wxWindow *parent, Basic_Network_Server &network_server );
    ~Network_Clients_Dialog();

    virtual void new_connection( std::string name, Basic_Network_Server::Connection_Id conn_id );
    virtual void closed_connection( Basic_Network_Server::Connection_Id conn_id );
    virtual void destroy();

    void on_disconnect( wxCommandEvent& event );
    void on_dclick( wxCommandEvent& event );
  private:
    wxListBox *client_list;
    std::map<void*,Basic_Network_Server::Connection_Id> client_data;
    std::map<Basic_Network_Server::Connection_Id,int> client_item;
    Basic_Network_Server &network_server;
    bool destroyed;

    // any class wishing to process wxWindows events must use this macro
    DECLARE_EVENT_TABLE()	//**/
  };

  // ============================================================================
  // Network_Connection_Dialog
  // ============================================================================

  class Network_Connection_Dialog : public wxDialog // modal dialog
  {
  public:
    Network_Connection_Dialog( wxWindow *parent );

    wxFlexGridSizer *sizer;
    wxRadioButton *server, *client;
    wxTextCtrl *hostname;
    wxSpinCtrl *port;
  private:
    DECLARE_EVENT_TABLE()
  };
}

#endif