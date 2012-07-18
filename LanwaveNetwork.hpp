/*************************************************************************************************
 *		LanWave
 *	LanwaveNetwork class
 ************************************************************************************************/
#ifndef LANWAVE_NETWORK
#define LANWAVE_NETWORK
#pragma once

#include <iostream>
#include <fstream>
#include <set>
#include <vector>

#include <boost/bind.hpp>
#include <boost/function.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>

#include "LanwaveNetworkConstants.hpp"
#include "herald.hpp"
#include "udpInterface.hpp"
#include "tcpInterface.hpp"
#include "networkMap.hpp"
#include "networkMapInterface.hpp"

using namespace boost::asio;

class LanwaveNetwork
	{
	private:
		// the uuid of this node
		boost::uuids::uuid uuid;
		
		// log object to write information to the log
		ftm_log* log;

		boost::property_tree::ptree* properties;

		// the main io service. Will be used for all io_service
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		boost::thread* service_thread;
		bool kill_thread;
		bool ignore_self_broadcast;
		
		// The Herald
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		boost::thread* herald_thread;
		udpInterface* herald_interface;
		boost::asio::deadline_timer* h_tmr;
		ip::udp::endpoint* last_master;
		char herald_message;
		void herald( void );
		void herald_rx_handler( ip::udp::endpoint* sender, char* data );
		int h_fast_discovery_count;
		
		// LanwaveNetworks own network map and sync list.
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		std::set<ip::udp::endpoint>* h_map;
		ip::udp::endpoint* prev_node;
		ip::udp::endpoint* next_node;
		//std::set<

		// some ip functions
		std::string ip_address;
		std::string friend_ip;
		
		// the tcp and udp interfaces
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		udpInterface* udp;
		//tcpInterface* tcp;
		
		// function receive handler
		//boost::function<void (double data)> receive_handler;
		
		boost::function<void (int* data, size_t length)> intsplat_rx_handler;
		void null_intsplat_rx_handler( int* data, size_t length );

		boost::function<void (char cmd, tcpSession* session)> upper_tcp_rx_handler;

		std::vector< boost::function<void (int* data, size_t length)> > intsplat_rx_handlers;
		//std::vector<boost::function< void (double data)> > double_receive_handlers;
		
		// the tcp and udp receive handlers
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void udp_rx_handler( ip::udp::endpoint* sender, char* data );
		void tcp_rx_handler( tcpSession* session, char* data );
		
		void file_rx_handler( tcpSession* session );
		void async_send_file( std::string path );

		void write( const char* data, size_t size );
		void write_command( char cmd );
		char* read( size_t size );
		char read_command( void );
		
		// thread launching functions
		void launch( void );
		void run( void );

		// debugging commands
		void debug_initialise( void );

		// file functions
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		bool file_exists( std::string path );

	public:
		boost::asio::io_service* service;
		tcpInterface* tcp;
		
		// Network map
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		networkMap* network_map;

		// constructor and destructor, thread handling
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		LanwaveNetwork( void );
		~LanwaveNetwork( void );

		// thread functions
		void shutdown( void );
		
		// herald functions
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void herald_announce( void );
		void herald_ping_self( void );

		// network map interface
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		networkMapInterface* map;
		
		// data sending/receiving
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void send_float( float data );
		void send_int_array( int* data, size_t size );
		void send_int_array_loopback( int* data, size_t size );

		// receive handler bindings
		void add_receive_handler( boost::function<void (int* data, size_t length)> handler );
		void set_tcp_rx_handler( boost::function<void (char cmd, tcpSession* session)> handler );

		//void add_receive_handler( boost::function<void (int* data)> handler );
		
		// id functions
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		std::string get_uuid( void );
		int get_static_id( void );
		void relative_id( int rid );
		int get_relative_id( void );
		void static_id( int sid );

		// ip functions
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		std::string get_self_ip_address( void );
		std::string get_friend( void );

		void add_node_by_ip( std::string address );
		void print_hmap( void );

		// file transfer functions
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		bool send_file( int id, std::string path );

		// synchronisation
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void sync( void );
		void update( void );
		void broadcast_command( char cmd );

		// master control
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void assign_static_id( int id );
		void assign_static_ids( void );
		void assign_relative_ids( void );
		void set_vis( std::string vis );
		void set_vis_path( std::string path );

		void check_network( void );

		// exception handling
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		void process_exception( std::exception& e );

		// set the race of the node
		ip::udp::endpoint* get_master( void );
		bool is_there_a_master( void );
		bool rank( std::string new_rank );
		std::string rank( void );
	};
#endif
