/*
 * network.hpp
 * 
 * Netzwork access
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
#include <wx/socket.h>

#ifndef __HOLTZ_NETZWORK__
#define __HOLTZ_NETZWORK__

namespace holtz
{
  class Network_Exception;
  class Network_Manager;

  class Network_Connection_Handler
  {
  public:
    virtual void new_connection( wxIPV4address host, wxSocketBase *socket ) = 0;
    virtual void closed_connection( wxSocketBase *socket ) = 0;

    virtual ~Network_Connection_Handler();
  };


  // wxholtz.hpp:
  class Game_Window;
}

#include "holtz.hpp"
#include "wxholtz.hpp"

namespace holtz
{
  class Network_Exception : public Exception
  {
  public:
    std::string name;
  };

  class Network_Manager : public wxEvtHandler, public Player_Setup_Manager, 
			  public Player_Input, public Player_Output
  {
  public:
    Network_Manager( Game & );
    Network_Manager( Game &, Game_Window & );
    ~Network_Manager();

    // connection commands
    bool setup_server( wxIPV4address port ) throw(Network_Exception); // setup of server ok
    bool setup_client( wxIPV4address host ) throw(Network_Exception);
    void close_connection();
    bool may_disconnect( wxSocketBase *socket );
    void disconnect( wxSocketBase *socket );
    void set_connection_handler( Network_Connection_Handler * );

    // player setup manager interface
    virtual void set_player_handler( Player_Handler * );
    virtual void new_game();
    virtual void stop_game();
    virtual bool add_player( std::string name, Player::Player_Type, Player::Help_Mode );
    virtual bool remove_player( int id );
    virtual bool player_up( int id );
    virtual bool player_down( int id );
    virtual bool change_ruleset( Ruleset::type, Ruleset& );
    virtual bool ready();		// ready with adding players; game may start

    // Player_Input functions
    virtual Player_State determine_move() throw(Exception);
    virtual Sequence get_move();
    // Player_Output functions
    virtual void report_move( const Sequence & );

    void on_network( wxSocketEvent &event );
    void on_done( wxTimerEvent &event );
  private:
    void on_connect( wxSocketBase& sock );
    void on_input( wxSocketBase& sock );
    void on_output( wxSocketBase& sock ); // start output
    void on_lost( wxSocketBase& sock );

    void recount_client_stat();	
    void check_state();		// forces some actions in certain states

    typedef enum Message_Type{ msg_handshake, msg_handshake_deny,
			       msg_player_request, msg_player_ack, msg_player_deny, 
			       msg_ready,
			       msg_player_setup, msg_player_setup_ack,
			       msg_start_game,
			       msg_report_move,
			       msg_undo_request, msg_undo_ack, msg_undo_deny,
			       msg_abort_game,
			       msg_illegal_request,
			       msg_player_added, msg_player_removed,
			       msg_player_up, msg_player_down, msg_player_change_deny,
			       msg_ruleset, msg_ruleset_deny,
			       msg_new_game };

    void write_string( wxSocketBase& sock, std::string );
    std::string read_string( wxSocketBase& sock );
    void write_int( wxSocketBase& sock, int );
    int read_int( wxSocketBase& sock );
    void write_message_type( wxSocketBase& sock, Message_Type );
    Message_Type read_message_type( wxSocketBase& sock );
    void write_player( wxSocketBase& sock, Player );
    Player read_player( wxSocketBase& sock );
    void write_move( wxSocketBase& sock, Sequence );
    Sequence read_move( wxSocketBase& sock );
    void write_board( wxSocketBase& sock, Board );
    Board read_board( wxSocketBase& sock );
    void write_ruleset( wxSocketBase& sock, Ruleset & );
    Ruleset *read_ruleset( wxSocketBase& sock );

    void continue_game();

    void report_player_added( Player *player );
    void report_player_removed( Player *player );
    void report_player_up( Player *player );
    void report_player_down( Player *player );
    void report_players( wxSocketBase &sock );
    void report_ruleset_change();

    Game &game;
    Game_Window *game_window;
    bool gui_connected;
    Sequence sequence;		// determined move sequence

    wxSocketServer *server;
    wxSocketClient *client;

    Network_Connection_Handler *connection_handler;
    Player_Handler *player_handler;

    typedef enum Network_Mode{ mode_undefined, mode_server, mode_client, mode_connecting };
    Network_Mode mode;

    typedef enum Protocol_State{ begin, handshake, request_player, is_ready, players_ready, game_started,
				 accept_move, wait_for_move, move_received, game_stop };
    Protocol_State state;

    std::list<Player> players;
    std::map<int,std::list<Player>::iterator> id_player; // Table id->player
    int current_id;
    std::list<Player> own_players; // only players added by this host
    std::map<int,std::list<Player>::iterator> id_own_player; // Table id->own_player
    std::string requested_player_name;	
    Player::Player_Type requested_player_type;	

    Ruleset *ruleset;
    Ruleset::type ruleset_type;

    class Client
    {
    public:
      Client();

      wxSocketBase *socket;
      std::list<std::list<Player>::iterator> players;

      Protocol_State state;
    };
    friend class Client;

    std::map<wxSocketBase*,Client> clients;
    std::map<int,Client*> id_client; // Table id->client
    unsigned max_clients;
    unsigned clients_ready, clients_player_setup_ack;

    unsigned max_string_size;
    long timeout_sequencial_read;

    wxIPV4address localhost;

    //DECLARE_EVENT_TABLE();
  };

  enum 
  {
    NETWORK_EVENT=200
  };
}
#endif
