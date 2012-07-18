#ifndef FTM_TCP_SESSION
#define FTN_TCP_SESSION

#include <iostream>
#include <fstream>
#include <string>
#include <cstddef>
#include <cmath>
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/crc.hpp>
#include <boost/cstdint.hpp>

#include "LanwaveNetworkConstants.hpp"

using namespace boost::asio;

class tcpSession
	{
	private:
		// pointer to the io_service object
		boost::asio::io_service* io_service;
		
		// the session socket
		ip::tcp::socket* sock;
		
		// the data buffer
		char* data;
		
		void file_rx_handler( void );

		//	receive handler
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		boost::function<void ( tcpSession* self, char* data)> rx_handler;

		unsigned int number_of_packets( unsigned int bytes );
		unsigned int packet_size( unsigned int bytes );
		unsigned int length_to_write( unsigned int packet_no, unsigned int bytes );
		
	public:
		tcpSession( boost::asio::io_service* service, boost::function<void (tcpSession* self, char* data)> rx_handler );
		~tcpSession( void );
		
		// return the socket for this session
		ip::tcp::socket* socket( void );
		
		// start the session
		void start( void );

		void connect( ip::tcp::endpoint destination );
		
		void handle_read( const boost::system::error_code& error, size_t bytes_transferred );
		void handle_write( const boost::system::error_code& error );
		
		void send_file( std::string path );
		
		void write( const char* data, size_t length );
		void write_command( char cmd );
		char* read( unsigned int length );
		char read_command( void );
		void close( void );

		bool file_exists( const char* filename );

		char* int_to_chararr( unsigned int val );
		unsigned int chararr_to_int( char* ar );
	};

#endif