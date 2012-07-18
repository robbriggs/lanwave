#include "udpInterface.hpp"

using namespace boost::asio;

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		UDP Interface constructor/destructor
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
udpInterface::udpInterface(	boost::asio::io_service* service,
							boost::function<void (ip::udp::endpoint* sender, char* data)> handler,
							const short rx_port )
	{
		// create a new log object to log events and debugging information
 		//this->log = new ftm_log("udp-interface");

		// initialise basic vars
		this->rx_packets_total = 0;
		this->tx_packets_total = 0;
		this->rx_bytes_total = 0;
		this->tx_bytes_total = 0;
 		this->kill_thread = false;
 		this->free_memory = false;
 		
		this->rx_data = new char[DEFAULT_RX_SIZE];
		this->rx_size = DEFAULT_RX_SIZE;
		this->tx_size = DEFAULT_TX_SIZE;

		//*(this->log) << "$Listen endpoint created.";
		
 		this->io_service = service;

 		// Set up the transmit and receive sockets
 		this->tx_socket = new ip::udp::socket( *(this->io_service) );
 		this->tx_socket->open( ip::udp::v4() );
 
 		this->rx_socket = new ip::udp::socket( *(this->io_service) );
 		this->rx_socket->open( ip::udp::v4() );
		
		this->sender = new ip::udp::endpoint();
		
 		this->self = new ip::udp::endpoint( ip::udp::v4(), rx_port );
 		//*(this->log) << "$Self address created.";
		//this->rx_port = rx_port;
 
 		this->target = new ip::udp::endpoint( ip::address_v4::loopback(), DEFAULT_UDP_PORT );
		boost::asio::socket_base::broadcast option( false );
		this->tx_socket->set_option(option);
		//this->tx_port = DEFAULT_UDP_PORT;
		
 		//*(this->log) << "$Target address = ";
		//*(this->log) << this->get_target();

		//std::cout << "Self address = " << *(this->self) << endl;
 		
 		this->rx_socket->bind( *(this->self) );
 		//*(this->log) << "$Listen socket bound.";
 		this->rx_handler = handler;
 		
 		//*(this->log) << "$UDP-Interface constructed.";
	}

udpInterface::~udpInterface( void )
	{
		delete this->tx_socket;
		delete this->rx_socket;
		delete this->target;
		delete this->self;
		delete this->sender;
		delete[] this->rx_data;
		//delete this->log;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Configuration Functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void udpInterface::set_target_address( const std::string address )
	{
		if ( !address.compare( this->target->address().to_string() ) )
			return;
		
		if ( address.compare("broadcast") == 0 )
			{
				this->target->address( ip::address_v4::broadcast() );
				
				boost::asio::socket_base::broadcast option( true );
				this->tx_socket->set_option(option);
			}
		else
			{
				this->target->address( ip::address_v4::from_string(address) );
				
				boost::asio::socket_base::broadcast option( false );
				this->tx_socket->set_option(option);
			}
	}

std::string udpInterface::get_target( void )
	{
		//std::stringstream addr;
		//addr << *(this->target);
		//return addr.str();
		return this->target->address().to_string();
	}

void udpInterface::set_target_port( const short port )
	{ this->target->port(port); }

void udpInterface::set_self_port( const short port )
	{ this->self->port(port); }

void udpInterface::set_rx_buffer_size( const size_t length )
	{
		this->rx_size = length;
		delete[] this->rx_data;
		this->rx_data = new char[length];
		//this->rx_data = (char*)realloc( this->rx_data, this->rx_size );
	}

size_t udpInterface::get_rx_buffer_size( void )
	{ return this->rx_size; }

void udpInterface::set_tx_packet_size( const size_t length )
	{ this->tx_size = length; }

size_t udpInterface::get_tx_packet_size( void )
	{ return this->tx_size; }

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Sending/Receiving Functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void udpInterface::send_data( char* data )
	{
		this->tx_data = data;
		this->tx_socket->async_send_to(
				boost::asio::buffer( this->tx_data, this->tx_size ),
				*(this->target),
				boost::bind
					(
						&udpInterface::handle_tx_data,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
			);
	}

void udpInterface::send_data( char* data, size_t data_size )
	{
		size_t old_length = this->tx_size;
		this->tx_size = data_size;
		this->send_data( data );
		this->tx_size = old_length;
	}

void udpInterface::send_data( const std::string address, char* data )
	{
		std::string old_address = this->get_target();
		this->set_target_address( address );
		this->send_data( data );
		this->set_target_address( old_address );
	}

void udpInterface::send_data( const std::string address, char* data, size_t data_size )
	{
		size_t old_length = this->tx_size;
		std::string old_address = this->get_target();
		this->tx_size = data_size;
		this->set_target_address( address );
		this->send_data( data );
		
		this->tx_size = old_length;
		this->set_target_address( old_address );
	}

// receiving data function
void udpInterface::listen_on_rx_socket( void )
	{
		this->rx_socket->async_receive_from
			(
				boost::asio::buffer( this->rx_data, this->rx_size ),
				*(this->sender),
				boost::bind
					(
						&udpInterface::handle_rx_data,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
			);
	}

// send/receive data handlers
void udpInterface::handle_rx_data( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if ( this->rx_handler != NULL )
			(this->rx_handler)( this->sender, this->rx_data );
		
		this->rx_bytes_total += bytes_transferred;
		this->rx_packets_total++;
		this->listen_on_rx_socket();
	}

void udpInterface::handle_tx_data( const boost::system::error_code& error, size_t bytes_transferred )
	{
		if ( this->free_memory )
			delete this->tx_data;
		
		this->tx_bytes_total += bytes_transferred;
		this->tx_packets_total++;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Statistics related functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
// get the total number of bytes transfered
unsigned long udpInterface::total_bytes_rx( void )
	{ return this->rx_bytes_total; }
unsigned long udpInterface::total_bytes_tx( void )
	{ return this->tx_bytes_total; }

// get the total number of packets transfered
unsigned long udpInterface::total_packets_rx( void )
	{ return this->rx_packets_total; }
unsigned long udpInterface::total_packets_tx( void )
	{ return this->tx_packets_total; }

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Memory functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void udpInterface::set_free( bool opt )
	{ this->free_memory = opt; }

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Thread related functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
bool udpInterface::launch( void )
	{
		// check if we have a receive handler. if not return false as we cant run without one
		if ( this->rx_handler == NULL )
			return false;
		else
			this->listen_on_rx_socket();
		
		// launch the network thread.
		//this->service_thread = new boost::thread( boost::bind( &udpInterface::run, this ) );
		return true;
	}

void udpInterface::run( void )
	{
		//*(this->log) << "$Network thread launched!";
		//boost::asio::io_service::work work( *(this->io_service) );
		this->io_service->run();
	}

void udpInterface::shutdown( void )
	{
		////*(this->log) << "$Shutting down network...";
		this->kill_thread = true;
		////*(this->log) << "$Sending stop signals to threads...";
		this->io_service->stop();
		////*(this->log) << "$Waiting for threads to stop...";
		this->service_thread->join();
		////*(this->log) << "$All threads stopped.";
		////*(this->log) << "$Network shut down.";
	}