#include "tcpSession.hpp"

using namespace boost::asio;

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		TCP Session constructor/destructor
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
tcpSession::tcpSession( boost::asio::io_service* service, boost::function<void (tcpSession* self, char* data)> rx_handler )
	{
		this->io_service = service;
		this->rx_handler = rx_handler;
		this->sock = new ip::tcp::socket( *(this->io_service) );
		this->data = new char[1024];
		std::cout << "constructed a new session\n";
	}

tcpSession::~tcpSession( void )
	{
		delete this->sock;
		delete[] this->data;
	}

void tcpSession::close( void )
	{
		this->write_command( TSCMD_CLOSE );
		std::cout << "Session closed.\n";
		delete this;
	}

void tcpSession::connect( ip::tcp::endpoint destination )
	{
		this->sock->connect( destination );
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Session synchronous messaging
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void tcpSession::write_command( char cmd )
	{
		char* send_cmd = new char[1];
		send_cmd[0] = cmd;
		this->sock->write_some( boost::asio::buffer( send_cmd, 1 ) );
		delete[] send_cmd;
	}

void tcpSession::write( const char* data, size_t length )
	{
		this->sock->write_some( boost::asio::buffer( data, length ) );
	}

char* tcpSession::read( unsigned int length )
	{
		char* data = new char[length];
		this->sock->read_some( boost::asio::buffer( data, length ) );
		return data;
	}

char tcpSession::read_command( void )
	{
		char* cmd = new char[1];
		this->sock->read_some( boost::asio::buffer( cmd, 1 ) );
		return *cmd;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		File handling function
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void tcpSession::send_file( std::string path )
	{
		std::cout << path << "\n";
		
		std::ifstream file( path.c_str(), std::ios::in|std::ios::binary|std::ios::ate );
		boost::crc_optimal<32, 0x1021, 0xFFFF, 0, false, false>  crc;

		std::ifstream::pos_type size = file.tellg();

		// oops! we got a bad path/empty file.
		if ( size < 1 )
			{
				std::cout << "bad file path/empty file\n";
				return;
			}
		
		size_t packets;
		//char* memblock = new char[1024];
		
		if ( size%1024 )
			packets = size/1024+1;
		else
			packets = size/1024;
		
		std::cout << "file size: " << size << "\n";
		std::cout << "packets:   " << packets << "\n";
		
		file.seekg (0, std::ios::beg);
		
		std::cout << "seeked the file ya\n";
		std::cout.flush();

		/**************************************************************************
		 *		Send file request
		 *************************************************************************/
		this->write_command( TSCMD_REQUEST_SEND_FILE );
		std::cout << "connect successful.\n";
		std::cout.flush();

		if ( this->read_command() == TSCMD_OK )
			std::cout << "peer said theyre good for file transfer\n";
		
		/**************************************************************************
		 *		Send path length
		 *************************************************************************/
		this->write_command( TSCMD_FILE_PATH );
		this->write_command( TSCMD_STREAM_LENGTH );

		unsigned int plen = (unsigned int)(path.size()+1);
		std::cout << "plen: " << plen << "\n";
		this->write( (char*)(&plen), 4 );

		/**************************************************************************
		 *		Send path
		 *************************************************************************/
		char* cpath = new char[path.size()+1];
		memcpy( cpath, path.c_str(), path.size()+1 );

		this->write( cpath , path.size()+1 );
		
		/**************************************************************************
		 *		Send file length
		 *************************************************************************/
		this->write_command( TSCMD_STREAM_LENGTH );
		this->write( (char*)(&size), 4 );
		
		/**************************************************************************
		 *		Send file
		 *************************************************************************/
		char* memblock = new char[1024];
		for ( unsigned int i = 0; i < packets; i++ )
			{
				file.read( memblock, 1024 );
				//std::cout << memblock;
				this->write( memblock, 1024 );

				crc.process_bytes( memblock, 1024 );
				std::cout << (unsigned int)(crc.checksum()) << "\n";

				this->write( this->int_to_chararr( (unsigned int)(crc.checksum()) ), 4 );
			}
		
		std::cout << "\nfile sent. closing session\n";
		file.close();
		
		/**************************************************************************
		 *		Close session
		 *************************************************************************/
		this->write_command( TSCMD_STREAM_END );
		std::cout << "finished at sending end.\n";
	}

void tcpSession::file_rx_handler( void )
	{
		/**************************************************************************
		 *		Send file request
		 *************************************************************************/
		std::cout << "    receiver is ok to accept the file...\n";
		this->write_command( TSCMD_OK );

		boost::crc_optimal<32, 0x1021, 0xFFFF, 0, false, false>  crc;

		/**************************************************************************
		 *		Send path length
		 *************************************************************************/
		if ( this->read_command() == TSCMD_FILE_PATH )
			{
				std::cout << "    reading the file path.\n";
			}

		this->read_command();

		char* cplen = this->read( 4 );
		unsigned int plen = *((int*)cplen);
		std::cout << "    path length: " << plen << "\n";

		/**************************************************************************
		 *		Send path
		 *************************************************************************/
		//delete[] data;
		char* pdata = this->read( plen );
		std::cout << "    target path: " << pdata << "\n";
		std::string path(pdata);
		
		while ( this->file_exists(path.c_str()) )
			{
				std::cout << "    Oh, the file seems to already exist!\n";
				path += ".1";
			}

		/**************************************************************************
		 *		Send file length
		 *************************************************************************/
		if ( this->read_command() == TSCMD_STREAM_LENGTH )
			{
				std::cout << "    receiving stream length (packets)\n";
			}
		//delete[] data;
		char* cflen = this->read( 4 );
		
		unsigned int fsize = *((int*)cflen);
		std::cout << "    file size: " << fsize << "\n";

		unsigned int packets = this->number_of_packets( fsize );
		std::cout << "    preparing to receive: " << packets << " packets\n";

		/**************************************************************************
		 *		Send file
		 *************************************************************************/
		char* data;
		std::ofstream file( path.c_str(), std::ios::out|std::ios::binary );
		unsigned int rem = 0;
		for ( unsigned int i = 0; i < packets; i++ )
			{
				data = this->read( 1024 );
		
				//if ( *data == TSCMD_CLOSE )
				//	return;

				file.write( data, this->length_to_write( i, fsize ) );
				//std::cout << "    i: " << i << ", size: " << size << ", length_to_write: " << this->length_to_write(i,size) << "\n";
				//std::cout << "    " << data;
				delete[] data;

				crc.process_bytes( data, 1024 );
				//std::cout << crc.checksum() << "\n";

				data = this->read(4);

				std::cout << "sent: " << this->chararr_to_int( data ) << "  us: " << (unsigned int)(crc.checksum()) << "\n";

				delete[] data;
			}
		std::cout << "\n";
		
		file.close();
		
		/**************************************************************************
		 *		Close session
		 *************************************************************************/
		if ( this->read_command() == TSCMD_STREAM_END )
			std::cout << "    successfully received file\n";
		
		std::cout << "    read all of the file\n    Closing session\n";
		delete this;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Command passing
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Start the session
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void tcpSession::start()
	{
		this->sock->async_read_some
			(
				boost::asio::buffer(this->data, 1024),
				boost::bind
					(
						&tcpSession::handle_read,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
			);
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Send and receive handlers
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void tcpSession::handle_read( const boost::system::error_code& error, size_t bytes_transferred )
	{
		//std::cout << "just received some data. writing...\n";
		//std::cout << "data: " << (int)(this->data[0]) << ", " << (int)(this->data[1]) << "\n";

		// std::cout << "data: ";

		// for (unsigned int i = 0; i < bytes_transferred; i++ )
		// 	std::cout << (int)(this->data[i]) << ", ";

		std::cout << "got something?\n";

		if ( std::string( this->data, 4 ) == "kill" )
			{
				std::cout << "received shutdown command. ending session.\n";
				delete this;
			}
		if ( this->data[0] == TSCMD_REQUEST_SEND_FILE )
			{
				std::cout << "    got a request for a file\n";
				this->file_rx_handler();
				return;
			}
		
		(this->rx_handler)( this, this->data );
	}

void tcpSession::handle_write(const boost::system::error_code& error)
	{
		//std::cout << "wrote a reply!\n";
		this->sock->async_read_some
			(
				boost::asio::buffer(this->data, 1024),
				boost::bind
					(
						&tcpSession::handle_read,
						this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::bytes_transferred
					)
			);
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Socket return function
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
ip::tcp::socket* tcpSession::socket( void )
	{
		return this->sock;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Helper Functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
unsigned int tcpSession::number_of_packets( unsigned int bytes )
	{
		if ( bytes%1024 )
			return bytes/1024+1;
		else
			return bytes/1024;
	}

bool tcpSession::file_exists( const char* filename )
	{
		std::ifstream ifile(filename);
		return ifile;
	}

unsigned int tcpSession::length_to_write( unsigned int packet_no, unsigned int bytes )
	{
		if ( packet_no+1 < this->number_of_packets( bytes ) )
			return 1024;
		else
			return bytes - 1024*(this->number_of_packets( bytes ) - 1);
	}

char* tcpSession::int_to_chararr( unsigned int val )
	{
		unsigned char* arr = new unsigned char[4];
		unsigned int mask = 0xFF;
		for ( int i = 0; i < 4; i++ )
			{
				arr[i] = (unsigned char)(val & mask);
				val = val >> 8;
			}
		return reinterpret_cast<char*>(arr);
	}

unsigned int tcpSession::chararr_to_int( char* ar )
	{
		unsigned char* arr = reinterpret_cast<unsigned char*>(ar);
		unsigned int val = 0;
		for ( int i = 0; i < 4; i++ )
			val += ((unsigned int)(arr[i]) << 8*i);

		return val;
	}