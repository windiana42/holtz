/*
 * network.hpp
 * 
 * Netzwork access according to the BGP 1.00a protocol (see docs/ for
 * state diagrams of master and slave nodes)
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

#define GAME_NAME_ZERTZ "/GIPFPROJECT/ZERTZ"
#define GAME_NAME_DVONN "/GIPFPROJECT/DVONN"
#define GAME_NAME_BLOKS "/GIPFPROJECT/BLOKS"
#define GAME_NAME_RELAX "/GIPFPROJECT/RELAX"

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

#if (defined(VERSION_ZERTZ) && !defined(__ZERTZ_NETWORK__)) || \
  (defined(VERSION_DVONN) && !defined(__DVONN_NETWORK__)) || \
  (defined(VERSION_BLOKS) && !defined(__BLOKS_NETWORK__)) || \
  (defined(VERSION_RELAX) && !defined(__RELAX_NETWORK__))

#if defined(VERSION_ZERTZ)
#  define __ZERTZ_NETWORK__
//#  warning "using zertz..."
#elif defined(VERSION_DVONN)
#  define __DVONN_NETWORK__
//#  warning "using dvonn..."
#elif defined(VERSION_BLOKS)
#  define __BLOKS_NETWORK__
//#  warning "using bloks..."
#elif defined(VERSION_RELAX)
#  define __RELAX_NETWORK__
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
  class Network_Connection_Handler;
  class Basic_Network_Server
  {
  public:
    typedef int Connection_Id;
    Basic_Network_Server() : connection_handler(0) {}
    virtual ~Basic_Network_Server() {}
    void set_connection_handler(Network_Connection_Handler *handler) { connection_handler=handler; }
    // virtual functions:
    virtual void close_connections() = 0;
    virtual bool may_disconnect_id( Connection_Id id ) = 0;
    virtual void disconnect_id( Connection_Id id ) = 0;
    virtual void allow_connect( bool allow ) = 0;
  protected:
    Network_Connection_Handler *connection_handler;
  };

  class Network_Connection_Handler
  {
  public:
    virtual void new_connection( std::string name, Basic_Network_Server::Connection_Id conn_id ) = 0;
    virtual void closed_connection( Basic_Network_Server::Connection_Id conn_id ) = 0;
    virtual void destroy() = 0;

    virtual ~Network_Connection_Handler() {}
  };

  class Network_Manager_BGP100c_Server;
  class Network_Manager_BGP100c_Client;
}

namespace holtz
{
  // wxzertz.hpp:
  class Game_Window;
}

#if defined(VERSION_ZERTZ)
#  undef VERSION_ZERTZ
#  include "msg_net.hpp"
#  include "zertz/zertz.hpp"
#  include "zertz/wxzertz.hpp"
#  include "zertz/manager.hpp"
#  include "zertz/bgp.hpp"
#  define VERSION_ZERTZ
#elif defined(VERSION_DVONN)
#  undef VERSION_DVONN
#  include "msg_net.hpp"
#  include "dvonn/dvonn.hpp"
#  include "dvonn/wxdvonn.hpp"
#  include "dvonn/manager.hpp"
#  include "dvonn/bgp.hpp"
#  define VERSION_DVONN
#elif defined(VERSION_BLOKS)
#  undef VERSION_BLOKS
#  include "msg_net.hpp"
#  include "bloks/bloks.hpp"
#  include "bloks/wxbloks.hpp"
#  include "bloks/manager.hpp"
#  include "bloks/bgp.hpp"
#  define VERSION_BLOKS
#elif defined(VERSION_RELAX)
#  undef VERSION_RELAX
#  include "msg_net.hpp"
#  include "relax/relax.hpp"
#  include "relax/wxrelax.hpp"
#  include "relax/manager.hpp"
#  include "relax/bgp.hpp"
#  define VERSION_RELAX
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
  using namespace holtz;

  class Network_Manager_BGP100a_Server : public wxEvtHandler,
					 public Basic_Network_Server,
					 public Game_Setup_Manager, 
					 public Player_Input, public Player_Output,
					 public Message_Network_Handler<BGP::Message>,
					 public Message_Network_Server_Handler<BGP::Message>
  {
  public:
    Network_Manager_BGP100a_Server( Game_Manager &, Game_UI_Manager & );
    ~Network_Manager_BGP100a_Server();

    // connection commands
    bool setup_server( int port ); // returns true for success

    //---------------------------------------------
    // commands inherited from Basic_Network_Server

    virtual void close_connections();
    virtual bool may_disconnect_id( Connection_Id id );
    virtual void disconnect_id( Connection_Id id );
    virtual void allow_connect( bool allow );

    //-------------------------------------------
    // commands inherited from Game_Setup_Manager

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
    virtual void ready();      // ready with adding players
    // game commands
    virtual std::list<Player> enable_player_feedback(); // returns players before feedback
    virtual void disable_player_feedback(); // disables feedback about player changes
    virtual bool can_choose_board(); // whether this player can choose a board to play
    virtual bool can_enter_player_setup(); // whether the player setup can be entered
    virtual Game_State can_start();	// is everyone ready and number of players ok?
    virtual void start_game(); // call only when can_start() == true
    virtual Answer_Type ask_new_game(); // request to play new game
    virtual Answer_Type ask_undo_moves(int n=2); // request to undo n half moves
    virtual void force_new_game(bool on_own=false); // force new game (may close connections)
    virtual void stop_game();  // stop game
    virtual void game_setup_entered();  // game setup entered 
    virtual bool is_allow_game_setup_abort(); // is it allowed to abort the game setup?

    // special variants of game commands
    void start_prepared_game(); // call only when can_start() == true

    //-------------------------------------------
    // commands inherited from Player Input

    virtual Player_State determine_move() throw(Exception);
    virtual Move_Sequence get_move();
    virtual long get_used_time();

    //-------------------------------------------
    // commands inherited from Player Output

    virtual void report_move( const Move_Sequence & );

    //------------------------------------------------
    // commands inherited from Message_Network_Handler

    // is called when message arrives
    virtual void process_message( Message_Network<BGP::Message> *connection, 
				  BGP::Message *message );
				// message must be deleted,
				// message==0: invalid message arrived
    // is called when connection is established (for Message_Network_Client only)
    virtual void on_connect( Message_Network<BGP::Message> *connection );
    // is called when connection was closed or couldn't be established
    virtual void on_lost( Message_Network<BGP::Message> *connection );
    // is called when an error occured
    virtual void on_error( Message_Network<BGP::Message> *connection );

    //-------------------------------------------------------
    // commands inherited from Message_Network_Server_Handler

    virtual void new_connection( Message_Network<BGP::Message> *message_network );  
				// message_network must be deleted!
				// ->set_handler() should be called

    //----------------
    // event handlers
    
    void on_timer(wxTimerEvent& event);
    void on_done(wxTimerEvent& event);
    void on_connection_timer( Message_Network<BGP::Message> *connection ); 
  private:
    enum Display_Phase{ DISPLAY_INIT, DISPLAY_SETUP, DISPLAY_PLAYING };
    // BGP 1.00a protocol states 
    enum Protocol_State{ BGP_UNCONNECTED, BGP_CONNECTED, BGP_HANDSHAKE, BGP_CHOOSE_ROOM,
				 BGP_INFO, BGP_SETUP, BGP_ADD_PLAYER, BGP_REMOVE_PLAYER, 
				 BGP_READY, BGP_OTHERS_TURN, BGP_HIS_TURN, 
				 BGP_ASK_UNDO, BGP_ACCEPT_UNDO, 
				 BGP_ASK_NEW_GAME, BGP_ACCEPT_NEW_GAME,
				 BGP_ERROR };
    std::string to_string(Protocol_State);
/*bgp100c
    enum Protocol_State{ BGP_UNCONNECTED, BGP_CONNECTED, BGP_HANDSHAKE, BGP_CHOOSE_ROOM,
				 BGP_INFO, BGP_SETUP, BGP_ASK_CHANGE, BGP_ACCEPT_CHANGE, 
				 BGP_READY, BGP_READY_ACCEPT, BGP_OTHERS_TURN, BGP_HIS_TURN, 
				 BGP_ASK_UNDO, BGP_ACCEPT_UNDO, 
				 BGP_ASK_NEW_GAME, BGP_ACCEPT_NEW_GAME,
				 BGP_ERROR };
*/

    class Timer : public wxTimer {
    public:
      Timer() : notifiee(0), connection(0) {}
      void init(Network_Manager_BGP100a_Server* n,
		Message_Network<BGP::Message>*  c) {notifiee=n; connection=c;}
      void start(int milliseconds) { Start(milliseconds,wxTIMER_ONE_SHOT); }
      void stop() { Stop(); }
      // inherited from wxTimer
      virtual void Notify() { notifiee->on_connection_timer(connection); }
    private:
      Network_Manager_BGP100a_Server* notifiee;
      Message_Network<BGP::Message>*  connection;
    };

    class Connection_State {
    private:
      Timer *private_timer;	
    public:
      Connection_Id id;
      std::set<int> controlled_player_ids;
      std::set<int> abandoned_player_ids;
      Protocol_State state;
      std::string name;
      std::string nick_name;
      bool disconnecting;	// whether server is in progress to disconnect this connection
      bool final_setup;		// whether connection requested final setup report
      bool move_reminder;	// whether connection requested move reminders on its turn
      bool in_init;		// whether connection is still in initialization
      bool is_ready;		// whether connection is ready to leave setup procedures
      BGP::Phase_Type phase;

      Timer &timer;

      Connection_State();
      Connection_State(const Connection_State &s);
      ~Connection_State();
      Connection_State &operator=(const Connection_State &s);
    };

    void continue_game();	// continue game after interruption / waiting on event
    void discard(Message_Network<BGP::Message>* connection);  
				// discard influence on server state
    void disconnect(Message_Network<BGP::Message>* connection,bool send_disconnect=true);
				// disconnect connection and optionally send msg before
    void broadcast(BGP::Message*, BGP::Phase_Type phase = BGP::phase_playing, 
		   bool check_phase=true, Message_Network<BGP::Message>* skip_connection=0);
 				// broadcase message to all connections in <phase>
    void start_timer(int milliseconds);
    void stop_timer();
    unsigned get_client_count();
    BGP::Phase_Type get_game_phase();
    int do_add_player( const Player &, Message_Network<BGP::Message>* connection=0 );    
					    // actually add player and return ID
    void do_remove_player( int player_id ); // actually remove player
    bool do_player_up( int player_id );	    // actually move player up
    bool do_player_down( int player_id );   // actually move player down
    bool do_ask_new_game( Message_Network<BGP::Message>* asking_connection=0 );
    bool do_ask_undo( int n, Message_Network<BGP::Message>* asking_connection=0 );
    void check_all_ready();		    // check whether all clients are ready 
    bool check_all_answered(bool answer_local=true); // check whether all clients answered a request
    void setup_players();		    // setup players for local use
    void cleanup_players();
    bool process_move( const Move_Sequence&, Message_Network<BGP::Message>* connection=0, 
		       int player_id=-1, bool delay_continue_game=false );  
					    // process a move and initiate consequences
    void process_deferred_messages();       // process all messages that were deferred

    Game_Manager &game_manager;
    Game_UI_Manager &ui_manager;
    Game_Setup_Display_Handler *display_handler;
    Game game;

    wxTimer timer;

    Message_Network_Server<BGP::Message>                      msg_net_server;      
    std::map<Message_Network<BGP::Message>*,Connection_State> msg_net_connection_state;
    std::map<int,Message_Network<BGP::Message>*>	      player_id_connection;
    std::map<Connection_Id,Message_Network<BGP::Message>*>    id_connection;
    unsigned max_connections; 
    bool allow_visitors;
    int response_timeout;	// timeout for other side to respond in milliseconds

    std::list<Player> players;
    std::map<int,std::list<Player>::iterator> id_player; // Table id->player
    int current_id; int current_player_id;
    std::set<int> own_player_ids;
    std::set<int> abandoned_player_ids;

    Display_Phase display_phase;
    BGP::Phase_Type game_phase;
    bool disable_new_connections;
    bool is_ready;
    Move_Sequence sequence;	// determined move sequence
    bool sequence_available;   
    bool awaiting_move;
    bool asking_new_game;
    bool asking_undo;
    bool asking_done;
    bool answer;
    unsigned clients_in_setup, clients_ready;
    unsigned clients_asked;
    Message_Network<BGP::Message>* asking_client;
    int asking_n;
    Message_Network<BGP::Message>* possibly_interrupted_connection;
    std::list<std::pair<int/*player_id*/,Move_Sequence> > pending_moves;
    std::map<int/*player_id*/,
	     std::pair<Move_Sequence,Message_Network<BGP::Message>*> > pending_player_move;
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> > deferred_messages;
    bool delayed_move_visualization;

    DECLARE_EVENT_TABLE() //**/
  };

  class Network_Manager_BGP100a_Client : public wxEvtHandler,
					 public Game_Setup_Manager, 
					 public Player_Input, public Player_Output,
					 public Message_Network_Handler<BGP::Message>
					 
  {
  public:
    Network_Manager_BGP100a_Client( Game_Manager &, Game_UI_Manager & );
    ~Network_Manager_BGP100a_Client();

    // connection commands
    bool connect_to_server( std::string host, int port ); // returns true for success
    void close_connection();

    //-------------------------------------------
    // commands inherited from Game_Setup_Manager

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
    virtual void ready();      // ready with adding players
    // game commands
    virtual std::list<Player> enable_player_feedback(); // returns players before feedback
    virtual void disable_player_feedback(); // disables feedback about player changes
    virtual bool can_choose_board(); // whether this player can choose a board to play
    virtual bool can_enter_player_setup(); // whether the player setup can be entered
    virtual Game_State can_start();	// is everyone ready and number of players ok?
    virtual void start_game(); // call only when can_start() == true
    virtual Answer_Type ask_new_game(); // request to play new game
    virtual Answer_Type ask_undo_moves(int n=2); // request to undo n half moves
    virtual void force_new_game(bool on_own=false); // force new game (may close connections)
    virtual void stop_game();  // stop game
    virtual void game_setup_entered();  // game setup entered
    virtual bool is_allow_game_setup_abort(); // is it allowed to abort the game setup?

    // special variants of game commands
    void start_prepared_game(); // call only when can_start() == true

    //-------------------------------------------
    // commands inherited from Player Input

    virtual Player_State determine_move() throw(Exception);
    virtual Move_Sequence get_move();
    virtual long get_used_time();

    //-------------------------------------------
    // commands inherited from Player Output

    virtual void report_move( const Move_Sequence & );

    //------------------------------------------------
    // commands inherited from Message_Network_Handler

    // is called when message arrives
    virtual void process_message( Message_Network<BGP::Message> *connection, 
				  BGP::Message *message );
				// message must be deleted,
				// message==0: invalid message arrived
    // is called when connection is established (for Message_Network_Client only)
    virtual void on_connect( Message_Network<BGP::Message> *connection );
    // is called when connection was closed or couldn't be established
    virtual void on_lost( Message_Network<BGP::Message> *connection );
    // is called when an error occured
    virtual void on_error( Message_Network<BGP::Message> *connection );

    //----------------
    // event handlers
    
    void on_timer(wxTimerEvent& event);
    void on_done(wxTimerEvent& event);
  private:
    enum Display_Phase{ DISPLAY_INIT, DISPLAY_SETUP, DISPLAY_PLAYING };
    // BGP 1.00a protocol states 
    enum Protocol_State{ BGP_UNCONNECTED, BGP_HANDSHAKE, BGP_ROOMS,
				 BGP_ASK_ROOM, BGP_GET_PHASE, BGP_SETUP_PREPARE_1, 
				 BGP_PLAY_PREPARE_1, BGP_PLAY_PREPARE_2, BGP_PLAY_PREPARE_3A,
				 BGP_PLAY_PREPARE_3B, BGP_PLAY_PREPARE_4A, BGP_TAKE_PLAYER, 
				 BGP_ASK_TAKE_PLAYER, BGP_SETUP, BGP_ADD_PLAYER, BGP_REMOVE_PLAYER, 
				 BGP_READY, BGP_OTHERS_TURN, BGP_MY_TURN,
				 BGP_ASK_UNDO, BGP_ACCEPT_UNDO, 
				 BGP_ASK_NEW_GAME, BGP_ACCEPT_NEW_GAME,
				 BGP_ERROR, BGP_ERROR_RECOVERY };
    std::string to_string(Protocol_State);

    class Connection_State {
    public:
      std::set<int> controlled_player_ids;
      Protocol_State state;
      std::string name;
      std::string nick_name;
      bool in_setup;
      bool is_ready;

      Connection_State() : in_setup(false), is_ready(false) {}
    };

    void continue_game();
    void disconnect(bool send_disconnect=true);
    void start_timer(int milliseconds);
    void stop_timer();
    BGP::Phase_Type get_game_phase();
    void do_add_player( const Player&, bool local );   // actually add player
    void do_remove_player( int player_id ); // actually remove player
    void setup_players();
    void cleanup_players();
    bool process_setup( BGP::Setup );
    bool process_moves( std::list<std::pair<Move_Sequence,int/*player index*/> > );
    bool process_move( const Move_Sequence&, bool local, int player_id=-1, 
		       bool delay_continue_game=false );
    void process_deferred_messages();       // process all messages that were deferred

    Game_Manager &game_manager;
    Game_UI_Manager &ui_manager;
    Game_Setup_Display_Handler *display_handler;
    Game game;

    wxTimer timer;

    Message_Network_Client<BGP::Message> *msg_net_client;
    int response_timeout;	// timeout for other side to respond in milliseconds

    std::list<Player> players;
    std::map<int,std::list<Player>::iterator> id_player; // Table id->player
    std::set<int> own_player_ids;

    bool connection_lost;
    Display_Phase display_phase;
    BGP::Phase_Type game_phase;
    Connection_State conn_state;
    Move_Sequence sequence;		// determined move sequence
    bool sequence_available;   
    bool awaiting_move;
    bool asking_new_game;
    bool asking_undo;
    int asking_n;
    Player requested_add_player;
    int requested_remove_player_id;
    std::set<int>::iterator requested_take_player_id;
    std::list<std::pair<int/*player_id*/,Move_Sequence> > pending_moves;
    std::map<int/*player_id*/,Move_Sequence> pending_player_move;
    std::list<std::pair<Message_Network<BGP::Message>*,BGP::Message*> > deferred_messages;
    int error_recovery_pongs;
    unsigned error_recovery_attempts;
    bool delayed_move_visualization;

    DECLARE_EVENT_TABLE() //**/
  };
}
#endif
