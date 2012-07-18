#include "tcpInterface.hpp"

using namespace boost::asio;

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		TCP Interface constructor/destructor
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
tcpInterface::tcpInterface( boost::asio::io_service* service, boost::function<void (tcpSession* session, char* data)> rx_handler )
	{
		this->io_service = service;
		this->rx_handler = rx_handler;
		this->acceptor = new ip::tcp::acceptor( *(this->io_service), ip::tcp::endpoint( ip::tcp::v4(), DEFAULT_TCP_PORT ) );
		
		//this->socket = new ip::tcp::socket( *(this->io_service) );

		this->session = new tcpSession( this->io_service, this->rx_handler );
		this->acceptor->async_accept
			(
				*(this->session->socket()),
				boost::bind
					(
						&tcpInterface::handle_accept,
						this,
						boost::asio::placeholders::error
					)
			);
	}

tcpInterface::~tcpInterface( void )
	{
		delete this->session;
		delete this->acceptor;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Function to handle accepting new connections
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void tcpInterface::handle_accept( const boost::system::error_code& error )
	{
		std::cout << "accepted a connection\n";
		this->session->start();
		this->session = new tcpSession( this->io_service, this->rx_handler );
		this->acceptor->async_accept
			(
				*(this->session->socket()),
				boost::bind
					(
						&tcpInterface::handle_accept,
						this,
						boost::asio::placeholders::error
					)
			);
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Initiate a new session with a target
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
tcpSession* tcpInterface::start_session( ip::tcp::endpoint& destination )
	{
		tcpSession* session = new tcpSession( this->io_service, this->rx_handler );

		//session->socket()->connect( *destination );
		session->connect( destination );

		return session;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Synchronous Data Sending/Receiving
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
 void tcpInterface::send_file( std::string address, std::string path )
	{
		boost::asio::ip::tcp::endpoint tmp( boost::asio::ip::address::from_string(address), DEFAULT_TCP_PORT);
		tcpSession* session = this->start_session( tmp );
		session->send_file( path );
	}

void tcpInterface::write_command( char cmd )
	{
		char* send_cmd = new char[1];
		send_cmd[0] = cmd;
		//this->socket->write_some( boost::asio::buffer( send_cmd, 1 ) );
		delete[] send_cmd;
	}

void tcpInterface::write( char* data, size_t length )
	{
		//this->socket->write_some( boost::asio::buffer( data, length ) );
	}