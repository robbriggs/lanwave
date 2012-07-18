#ifndef FTM_TCP_INTERFACE
#define FTN_TCP_INTERFACE
#pragma once

#include <iostream>
#include <string>
#include <cstddef>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

#include "tcpSession.hpp"

using namespace boost::asio;

class tcpInterface
	{
	private:
		// pointer to the main io_service object
		boost::asio::io_service* io_service;

		// holds the current waiting session
		tcpSession* session;

		// the tcp acceptor
		ip::tcp::acceptor* acceptor;
		
		//	receive handler
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		boost::function<void (tcpSession* session, char* data)> rx_handler;

		void handle_accept( const boost::system::error_code& error );
	public:
		tcpInterface( boost::asio::io_service* service, boost::function<void (tcpSession* session, char* data)> rx_handler );
		~tcpInterface( void );
		
		tcpSession* start_session( ip::tcp::endpoint& destination );
		
		void write_command( char cmd );
		void write( char* data, size_t length );

		void send_file( std::string address, std::string path );
	};

#endif