#ifndef FTM_UDP_INTERFACE
#define FTN_UDP_INTERFACE

#include <iostream>
#include <string>
#include <cstddef>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "ftm_log.hpp"
#include "LanwaveNetworkConstants.hpp"

//#define DEFAULT_UDP_PORT	12348
#define DEFAULT_RX_SIZE	64
#define DEFAULT_TX_SIZE	64

using namespace boost::asio;

class udpInterface
	{
	private:
		// io service object
		boost::asio::io_service* io_service;

		// log object
		ftm_log* log;
		
		// pointer to the service thread and thread kill var
		boost::thread* service_thread;
		bool kill_thread;
		bool free_memory;

		// send and receive internal data buffers
		char* rx_data;
		char* tx_data;
		size_t rx_size;
		size_t tx_size;

		unsigned long rx_packets_total;
		unsigned long tx_packets_total;
		unsigned long rx_bytes_total;
		unsigned long tx_bytes_total;

		// send and receive sockets
		ip::udp::socket*	tx_socket;
		ip::udp::socket*	rx_socket;

		// endpoints representing us and the target machine
		ip::udp::endpoint*	self;
		ip::udp::endpoint*	target;
		ip::udp::endpoint*	sender;
		//unsigned short tx_port;
		//unsigned short rx_port;
		
		// the service function which starts the io_service for the udp broadcast.
		void run( void );

		// receiving data function
		void listen_on_rx_socket( void );

		// send/receive data handlers
		void handle_rx_data( const boost::system::error_code& error, size_t bytes_transferred );
		void handle_tx_data( const boost::system::error_code& error, size_t bytes_transferred );

		// pointer to external data handler
		boost::function<void (ip::udp::endpoint* sender, char* data)> rx_handler;
		
	public:
		// constructor/destructor
		udpInterface(	boost::asio::io_service* service,
						boost::function<void (ip::udp::endpoint* sender, char* data)> handler,
						const short rx_port = DEFAULT_UDP_PORT );
		~udpInterface( void );
		
		// launch the service thread
		bool launch( void );
		void shutdown( void );
		
		// set/get the target address/port
		void set_target_address( const std::string address );
		void set_target_port( const short port );
		std::string get_target( void );
		void set_self_port( const short port );

		// change the send/receive buffer size.
		void	set_rx_buffer_size( const size_t length );
		size_t	get_rx_buffer_size( void );
		void	set_tx_packet_size( const size_t length );
		size_t	get_tx_packet_size( void );
		
		// memory functions
		void set_free( bool opt );
		
		// sending data functions
		void send_data( char* data );
		void send_data( char* data, size_t data_size );
		void send_data( const std::string address, char* data );
		void send_data( const std::string address, char* data, size_t data_size );

		// statistics on the network
		unsigned long total_bytes_rx( void );
		unsigned long total_bytes_tx( void );
		unsigned long total_packets_rx( void );
		unsigned long total_packets_tx( void );
	};
#endif