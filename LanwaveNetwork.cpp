/*************************************************************************************************
 *		LanWave
 *	LanwaveNetwork class
 ************************************************************************************************/
#include "LanwaveNetwork.hpp"

using namespace boost::asio;
/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Constructor and destructor
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
LanwaveNetwork::LanwaveNetwork( void )
	{
		// initialise the uuid for this node
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->uuid = boost::uuids::random_generator()();

		// initialise properties
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->properties = new boost::property_tree::ptree();
		//this->properties->put("uuid", boost::uuids::random_generator()() );
		this->properties->put("id.relative", -1);
		this->properties->put("id.static", -1);

		// create the service thread and set some flags for threading.
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->service = new boost::asio::io_service();
		this->kill_thread = false;
		this->ignore_self_broadcast = false;
		
		// set the nodes rank to slave and put an empty self ip address and no friend ips.
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		//this->node_rank = SLAVE;
		this->properties->put("self.rank", "slave");
		this->ip_address = "";
		this->properties->put("self.address", "");
		this->friend_ip = "YOU HAVE NO FRIENDS!\n";

		// initialise the vis file path
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->properties->put("file.vis-path", "");
		//this->intsplat_rx_handler = boost::bind( &LanwaveNetwork::null_intsplat_rx_handler, this, _1, _2 );

		this->intsplat_rx_handlers = std::vector< boost::function<void (int* data, size_t length)> >();

		// set up the herald for discovering and talking to other machines on the network.
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->h_tmr = new boost::asio::deadline_timer(*(this->service));
		this->last_master = new ip::udp::endpoint( ip::address_v4::from_string("0.0.0.0"), DEFAULT_UDP_PORT );
		this->herald_message = HCMD_ANNOUNCE;
		this->herald_interface = new udpInterface( this->service, boost::bind( &LanwaveNetwork::herald_rx_handler, this, _1, _2 ), HERALD_PORT );
		this->herald_interface->set_target_address("broadcast");
		this->herald_interface->set_rx_buffer_size(17);
		this->herald_interface->set_tx_packet_size(17);
		this->herald_interface->set_target_port(HERALD_PORT);
		this->h_map = new std::set<ip::udp::endpoint>();
		this->h_fast_discovery_count = 0;

		// initialise the network map
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->network_map = new networkMap();
		this->map = new networkMapInterface( this->network_map );
		this->prev_node = NULL;
		this->next_node = NULL;
		
		//	create the udp interface
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		////*(this->log) << "$Starting UDP-Interface...";
		this->udp = new udpInterface( this->service, boost::bind( &LanwaveNetwork::udp_rx_handler, this, _1, _2 ) );
		this->udp->set_target_address("broadcast");
		//this->udp->set_free( true );
		////*(this->log) << "$UDP-Interface launched.";
		
		//	create the tcp interface
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->tcp = new tcpInterface( this->service, boost::bind( &LanwaveNetwork::tcp_rx_handler, this, _1, _2 ) );
		
		//this->debug_initialise();

		//	launch everything
		// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
		this->launch();
	}

LanwaveNetwork::~LanwaveNetwork( void )
	{
		//delete this->h_strand;
		delete this->h_tmr;
		delete this->herald_interface;
		delete this->tcp;
		delete this->udp;
		//delete this->log;
		delete this->h_map;
		delete this->map;
		delete this->network_map;
		delete this->service;
		delete this->service_thread;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		UDP Sending/Receiving Functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::send_float( float data )
	{
		char* buffer = (char*)malloc( 20 );
		sprintf( buffer, "%2.10f", data );
		std::cout << buffer << endl;
		this->udp->send_data( buffer, 20 );
	}

void LanwaveNetwork::send_int_array( int* data, size_t size )
	{
		char* c_data = new char[4*size];
		memcpy( c_data, data, 4*size );
		
		if ( this->udp->get_tx_packet_size() < 4*size )
			this->udp->set_tx_packet_size( 4*size );
		
		this->udp->send_data( c_data );
	}

void LanwaveNetwork::send_int_array_loopback( int* data, size_t size )
	{
		char* c_data = new char[4*size];
		memcpy( c_data, data, 4*size );
		
		if ( this->udp->get_tx_packet_size() < 4*size )
			this->udp->set_tx_packet_size( 4*size );

		//this->udp->send_data( c_data );
		this->udp_rx_handler( new ip::udp::endpoint( ip::address_v4::loopback(), DEFAULT_UDP_PORT ), c_data );
	}

void LanwaveNetwork::add_receive_handler( boost::function<void (int* data, size_t length)> handler )
	{
		//this->intsplat_rx_handler = handler;

		this->intsplat_rx_handlers.push_back( handler );
	}

void LanwaveNetwork::set_tcp_rx_handler( boost::function<void (char cmd, tcpSession* session)> handler )
	{
		this->upper_tcp_rx_handler = handler;
	}

void LanwaveNetwork::null_intsplat_rx_handler( int* data, size_t length )
	{
		return;
	}

// binds the receive function handler. call this and pass it the visualisation function to handle the received data.
//void LanwaveNetwork::bind_receive_handler( boost::function<void (double data)> handler )
//	{ this->receive_handler = handler; }

void LanwaveNetwork::udp_rx_handler( ip::udp::endpoint* sender, char* data )
	{
		if ( this->ignore_self_broadcast )
			return;
		
		//std::cout << "udp got data. from " << *(sender) << endl;
		size_t length, bytes = this->udp->get_rx_buffer_size();
		
		length = bytes/sizeof(int);
		
		int* n_data = new int[length];
		memcpy( n_data, data, sizeof(int)*length );
		
		//(this->intsplat_rx_handler)( n_data, length );

		for ( unsigned int i = 0; i < this->intsplat_rx_handlers.size(); i++ )
			(this->intsplat_rx_handlers[i])( n_data, length );

		//std::cout << "bytes: " << bytes << ", length: " << length << endl;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		TCP Sending/Receiving Functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::tcp_rx_handler( tcpSession* session, char* data )
	{
		std::cout << "tcp got data yeaaa!\n";

		if ( data[0] == TSCMD_PING )
			{
				std::cout << "We were TCP pinged\n";

				session->write_command( TSCMD_OK );

				delete session;
			}
		else if ( data[0] == TSCMD_UPDATE_RID )
			{
				std::cout << "oh we want to update the rid?\n";
				
				int num_length = session->read_command();
				int new_rid = atoi( session->read( num_length ) );

				std::cout << "data length is: " << num_length << "\n";
				std::cout << "new relative id is: " << new_rid << "\n";

				this->relative_id( new_rid );

				if ( this->upper_tcp_rx_handler != NULL )
					(this->upper_tcp_rx_handler)( data[0], session );
				else
					session->close();
			}
		else if ( data[0] == TSCMD_UPDATE_SID )
			{
				std::cout << "oh we want to update the sid instead?\n";
				
				int num_length = session->read_command();
				int new_sid = atoi( session->read( num_length ) );

				std::cout << "data length is: " << num_length << "\n";
				std::cout << "set static id to: " << new_sid << "\n";

				this->static_id( new_sid );

				if ( this->upper_tcp_rx_handler != NULL )
					(this->upper_tcp_rx_handler)( data[0], session );
				else
					session->close();
			}
		else if ( this->upper_tcp_rx_handler != NULL )
			(this->upper_tcp_rx_handler)( data[0], session );
		else
			session->close();
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		File transfer functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
bool LanwaveNetwork::send_file( int id, std::string path )
	{
		std::string target;
		if ( id == -1 )
			target = "127.0.0.1";
		else
			target = this->network_map->getEndpoint( id )->address().to_string();

		try
			{
				this->tcp->send_file( target, path );
			}
		catch ( std::exception& e )
			{
				std::cout << "Oops! There was a problem syncing. Message: " << e.what() << "\n";
				this->process_exception( e );
			}
		return true;
	}

void LanwaveNetwork::async_send_file( std::string path )
	{
	}

void LanwaveNetwork::file_rx_handler( tcpSession* session )
	{
		//char cmd = session->read_command();
		//session->read_command();
		//std::string path( session->read(1024) );
	}

void LanwaveNetwork::set_vis( std::string vis )
	{
		for ( std::map<int, LWNode*>::iterator iter = this->network_map->map.begin(); iter != this->network_map->map.end(); iter++ )
			{
				if ( iter->second->endpoint->address() != ip::address_v4::loopback() )
					{
						std::cout << vis << " => " << *(iter->second->endpoint) << std::endl;
						try
							{
								ip::tcp::endpoint tmp( iter->second->endpoint->address(), DEFAULT_TCP_PORT );
								tcpSession* slave_session = this->tcp->start_session( tmp );					

								slave_session->write_command( TSCMD_SELECT_VIS );
								slave_session->write_command( (char)(vis.length()+1) );
								slave_session->write( vis.c_str(), (size_t)(vis.length()+1) );
								


								//if ( slave_session->read_command() == TSCMD_CLOSE )
								//	delete slave_session;
							}
						catch ( std::exception& e )
							{
								std::cout << "Oops! problem sending new vis identifier!" << std::endl;
								this->process_exception( e );
							}
					}
				else
					std::cout << "this is us. do not send this to anyone." << std::endl;
			}
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		ID functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
std::string LanwaveNetwork::get_uuid( void )
	{
		std::stringstream str;
		str << this->uuid;
		return str.str();
	}

int LanwaveNetwork::get_static_id( void )
	{
		return this->properties->get<int>("id.static");
	}

void LanwaveNetwork::static_id( int sid )
	{
		this->properties->put("id.static", sid);
	}

int LanwaveNetwork::get_relative_id( void )
	{
		return this->properties->get<int>("id.relative");
	}

void LanwaveNetwork::relative_id( int rid )
	{
		this->properties->put("id.relative", rid);
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		IP and Rank functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
bool LanwaveNetwork::rank( std::string new_rank )
	{
		if ( new_rank != this->properties->get<std::string>("self.rank") )
			{
				if ( new_rank == "master" )
					{
						if ( this->last_master->address() != ip::address_v4::from_string("0.0.0.0") )
							{
								std::cout << "Oops! Looks like there is already a master. Ya snooze, ya loose!" << std::endl;
								return false;
							}

						delete this->h_map;
						this->h_map = new std::set<ip::udp::endpoint>();
						this->network_map->clearMap();
						this->network_map->addNode( new ip::udp::endpoint( ip::address_v4::loopback(), DEFAULT_UDP_PORT ) );
						this->static_id( 1 );

						this->properties->put("self.rank", "master");

						char* announce_reply = new char[1];
						announce_reply[0] = HCMD_AM_MASTER;
						this->herald_interface->send_data( announce_reply, 1 );

						std::cout << "bam\n";
						this->udp->set_target_address("broadcast");
						this->h_fast_discovery_count = 0;
						this->herald_announce();
					}
				else
					{
						std::cout << "boom\n";
						this->udp->set_target_address("127.0.0.1");

						if ( this->properties->get<std::string>("self.rank") == "master" )
							{
								char* announce_reply = new char[1];
								announce_reply[0] = HCMD_MASTER_DEATH;
								this->herald_interface->send_data( announce_reply, 1 );
							}
						this->properties->put("self.rank", "slave");

						this->last_master->address( ip::address_v4::from_string("0.0.0.0") );
					}

				this->properties->put("self.rank", new_rank);
			}
		return true;
	}

std::string LanwaveNetwork::rank( void )
	{
		return this->properties->get<std::string>("self.rank");
	}

std::string LanwaveNetwork::get_self_ip_address( void )
	{
		if ( this->ip_address == "" )
			return "<Self check has not been run>";
		else
			return this->ip_address;
	}

ip::udp::endpoint* LanwaveNetwork::get_master( void )
	{
		//std::cout << "so lets try and find the master if possible" << std::endl << flush;
		if ( this->rank() != "master" )
			{
				//std::cout << "who is da mastaaaa!\n" << flush;

				char* announce_reply = new char[1];
				announce_reply[0] = HCMD_WHO_IS_MASTER;
				this->herald_interface->send_data( announce_reply, 1 );

				//boost::asio::deadline_timer t( *(this->service), boost::posix_time::milliseconds(20));
				//t.wait();
				//sleep(1);
				//std::cout << "do we make it this far?" << std::endl;
				return this->last_master;
			}
		return this->last_master;
	}

bool LanwaveNetwork::is_there_a_master( void )
	{
		if ( this->rank() == "master" || this->get_master()->address() != ip::address_v4::from_string("0.0.0.0") )
			return true;

		return false;
	}

std::string LanwaveNetwork::get_friend( void )
	{
		return this->friend_ip;
	}

void LanwaveNetwork::add_node_by_ip( std::string address )
	{
		ip::udp::endpoint* sender = new ip::udp::endpoint( ip::address::from_string( address) , DEFAULT_UDP_PORT );
		this->h_map->insert( *sender );

		int node_id;

		if ( this->rank() == "master" )
			{
				node_id = this->network_map->addNode( sender );
				this->assign_static_id( node_id );
				//this->network_map->addNode( sender );
			}
	}

void LanwaveNetwork::print_hmap( void )
	{
		std::cout << "H_map:\n";
		for ( set<ip::udp::endpoint>::iterator iter = this->h_map->begin(); iter != this->h_map->end(); iter++ )
			std::cout << *iter << "\n";
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Vis files
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::set_vis_path( std::string path )
	{
		if ( this->file_exists( path ) )
			this->properties->put("file.vis-path", path);
		else
			std::cout << "File was not found.";
	}

bool LanwaveNetwork::file_exists( std::string path )
	{
		std::ifstream ifile(path.c_str());
		return ifile;
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Network Synchronisation
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::sync( void )
	{
		if ( this->rank() == "slave" )
			return;

		for ( std::map<int, LWNode*>::iterator iter = this->network_map->map.begin(); iter != this->network_map->map.end(); iter++ )
			{
				std::cout << "node ip: " << *(iter->second->endpoint) << "\n";
				this->send_file( iter->first, this->properties->get<std::string>("file.vis-path") );
			}
	}

void LanwaveNetwork::update( void )
	{

	}

void LanwaveNetwork::check_network( void )
	{
		std::list<int> bad_nodes;

		for ( std::map<int, LWNode*>::iterator iter = this->network_map->map.begin(); iter != this->network_map->map.end(); iter++ )
			{
				std::cout << "checking node: " << *(iter->second->endpoint) << "\n";

				try
					{
						ip::tcp::endpoint tmp( iter->second->endpoint->address(), DEFAULT_TCP_PORT );
						tcpSession* session = this->tcp->start_session( tmp );

						session->write_command( TSCMD_PING );

						if ( session->read_command() != TSCMD_OK )
							bad_nodes.push_back( iter->first );

						delete session;
					}
				catch ( std::exception& e )
					{
						bad_nodes.push_back( iter->first );
					}
			}

		for ( list<int>::iterator iter = bad_nodes.begin(); iter != bad_nodes.end(); iter++ )
			{
				std::cout << "bad node! " << *iter << "\n";
				this->network_map->removeNode(*iter);
			}
	}

void LanwaveNetwork::assign_static_id( int id )
	{
		try
			{
				ip::tcp::endpoint tmp( this->network_map->getEndpoint(id)->address(), DEFAULT_TCP_PORT );
				tcpSession* slave_session = this->tcp->start_session( tmp );

				std::stringstream sid;
				sid << id;						

				slave_session->write_command( TSCMD_UPDATE_SID );
				slave_session->write_command( (char)(sid.str().length()+1) );
				slave_session->write( sid.str().c_str(), (size_t)(sid.str().length()+1) );
				
				if ( slave_session->read_command() == TSCMD_CLOSE )
					delete slave_session;
			}
		catch ( std::exception& e )
			{
				std::cout << "Oops! There was a problem updating the static id for node " << id << ".\n";
				this->process_exception( e );
			}
	}

void LanwaveNetwork::assign_static_ids( void )
	{
		for ( std::map<int, LWNode*>::iterator iter = this->network_map->map.begin(); iter != this->network_map->map.end(); iter++ )
			{
				std::cout << (*iter).first << " => (sid=" << iter->first << ",addr=" << *(iter->second->endpoint) << ")\n";

				try
					{
						ip::tcp::endpoint tmp( iter->second->endpoint->address(), DEFAULT_TCP_PORT );
						tcpSession* slave_session = this->tcp->start_session( tmp );

						std::stringstream sid;
						sid << iter->first;						

						slave_session->write_command( TSCMD_UPDATE_SID );
						slave_session->write_command( (char)(sid.str().length()+1) );
						slave_session->write( sid.str().c_str(), (size_t)(sid.str().length()+1) );
						
						if ( slave_session->read_command() == TSCMD_CLOSE )
							delete slave_session;
					}
				catch ( std::exception& e )
					{
						std::cout << "Oops! There was a problem updating the static id for: " << (*iter).first << " => (sid=" << iter->first << ",addr=" << *(iter->second->endpoint) << ")\n";
						this->process_exception( e );
					}
			}
	}

void LanwaveNetwork::assign_relative_ids( void )
	{
		for ( std::map<int, LWNode*>::iterator iter = this->network_map->map.begin(); iter != this->network_map->map.end(); iter++ )
			{
				std::cout << (*iter).first << " => (rid=" << iter->second->relativeposition << ",addr=" << *(iter->second->endpoint) << ")" << std::endl;

				if ( iter->second->endpoint->address() == ip::address_v4::loopback() )
					std::cout << "this is us. do not send this to anyone." << std::endl;

				if ( iter->second->relativeposition != iter->second->lastposition && iter->second->endpoint->address() != ip::address_v4::loopback() )
					{
						try
							{
								ip::tcp::endpoint tmp( iter->second->endpoint->address(), DEFAULT_TCP_PORT );
								tcpSession* slave_session = this->tcp->start_session( tmp );

								std::stringstream rid;
								rid << iter->second->relativeposition;						

								slave_session->write_command( TSCMD_UPDATE_RID );
								slave_session->write_command( (char)(rid.str().length()+1) );
								slave_session->write( rid.str().c_str(), (size_t)(rid.str().length()+1) );
								
								if ( slave_session->read_command() == TSCMD_CLOSE )
									delete slave_session;
							}
						catch ( std::exception& e )
							{
								std::cout << "Oops! There was a problem updating the relative id for: " << (*iter).first << " => (sid=" << iter->first << ",addr=" << *(iter->second->endpoint) << ")\n";
								this->process_exception( e );
							}
					}
			}
	}

void LanwaveNetwork::debug_initialise( void )
	{
		std::cout << "Pre setting certain debugging values" << "\n";
		this->rank( "master" );

		this->add_node_by_ip("127.0.0.1");
		this->add_node_by_ip("192.168.1.2");
		this->add_node_by_ip("192.168.1.3");
		this->add_node_by_ip("192.168.1.4");
		this->add_node_by_ip("192.168.1.5");
		this->add_node_by_ip("192.168.1.6");
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		The Herald
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::herald( void )
	{
		char* announce = new char[1];
		announce[0] = HCMD_ANNOUNCE_REPLY;
		this->herald_interface->send_data( announce, 1 );

		//std::cout << "what is herald saying?: " << (int)this->herald_message << std::endl;
		this->herald_message = HCMD_ANNOUNCE;

		if ( !this->kill_thread )
			{
				if ( this->h_fast_discovery_count < HERALD_FAST_ANNOUNCES )
					{
						this->h_tmr->expires_from_now( boost::posix_time::seconds(HERALD_FAST_ANNOUNCE_DELAY) );
						this->h_fast_discovery_count++;
					}
				else
					this->h_tmr->expires_from_now( boost::posix_time::seconds( HERALD_ANNOUNCE_DELAY ) );

				this->h_tmr->async_wait( boost::bind( &LanwaveNetwork::herald, this ) );
			}
		delete[] announce;
	}

void LanwaveNetwork::herald_rx_handler( ip::udp::endpoint* sender, char* data )
	{
		//std::cout << "Herald: We have some data indahouse! from: " << sender->address() << endl;
		//std::cout << "lets see what the data was: " << (int)data[0] << endl;

		sender->port( DEFAULT_PORT );
		
		if ( sender->address().to_string() != this->ip_address )
			{
				switch ( data[0] )
				{
					//	when we receive an announce
					case HCMD_ANNOUNCE:
						if ( this->h_map->find( *sender ) == this->h_map->end() )
							{
								//this->h_map->insert( *sender );
								//
								//if ( this->node_rank == MASTER )
								//	this->network_map->addNode( sender );
								this->add_node_by_ip( sender->address().to_string() );

								if ( this->rank() == "master" )
									{
										int sid = this->network_map->addNode( sender );
										this->assign_static_id( sid );
										this->assign_static_id( sid );
										this->assign_static_id( sid );
									}

								char* announce_reply = new char[1];
								announce_reply[0] = HCMD_ANNOUNCE_REPLY;
								this->herald_interface->send_data( announce_reply, 1 );
								//std::cout << "sender not in map. added and replied.\n";
								delete[] announce_reply;
							}
						break;
					//	reply to an announce
					case HCMD_ANNOUNCE_REPLY:
						if ( this->h_map->find( *sender ) == this->h_map->end() )
							{
								this->h_map->insert( *sender );
								
								if ( this->rank() == "master" )
									{
										int sid = this->network_map->addNode( sender );
										this->assign_static_id( sid );
										this->assign_static_id( sid );
										this->assign_static_id( sid );
									}

								//std::cout << "received announce reply. sender not in map. added.\n";
							}
						break;
					case HCMD_WHO_IS_MASTER:
						//std::cout << "who is the master?\n";
						if ( this->rank() == "master" )
							{
								//std::cout << "IS YOU!\n";

								char* announce_reply = new char[1];
								announce_reply[0] = HCMD_AM_MASTER;
								this->herald_interface->send_data( announce_reply, 1 );
							}
						break;
					case HCMD_AM_MASTER:
						//std::cout << "found the master! " << *sender << std::endl;

						if ( this->rank() != "master" )
							{
								delete this->h_map;
								this->h_map = new std::set<ip::udp::endpoint>();
								this->last_master->address( sender->address() );

								this->h_fast_discovery_count = 0;
								this->herald_announce();

								//std::cout << "he da masta noob" << std::endl;

								if ( this->upper_tcp_rx_handler != NULL )
									(this->upper_tcp_rx_handler)( TSCMD_MASTER_NEW, NULL );
							}
						//else
							//std::cout << "We have duplicate master nodes on the network." << std::endl;
						break;
					case HCMD_MASTER_DEATH:
						//std::cout << "Master has gone?" << std::endl;
						if ( this->last_master->address() == sender->address() )
							{
								this->static_id(-1);
								this->relative_id(-1);
								this->last_master->address( ip::address_v4::from_string("0.0.0.0") );
								//std::cout << "The master left! :(" << std::endl;

								if ( this->upper_tcp_rx_handler != NULL )
									(this->upper_tcp_rx_handler)( TSCMD_MASTER_YIELD, NULL );
							}
						break;
					//	when we want to determin our own ip address
					case HCMD_SELF_CHECK:
						if ( this->ip_address == "" )
							{
								std::vector<char> sent_uuid( data+1, data+17 );
								std::vector<char> self_uuid( this->uuid.begin(), this->uuid.end() );

								if ( sent_uuid == self_uuid )
									this->ip_address = sender->address().to_string();
							}
						break;
					case TSCMD_SHOW_SID:
						if ( this->upper_tcp_rx_handler != NULL )
							(this->upper_tcp_rx_handler)( TSCMD_SHOW_SID, NULL );
						break;
				}
			}
	}

void LanwaveNetwork::herald_ping_self( void )
	{
		this->ip_address = "";
		char* payload = new char[1+uuid.size()];

		payload[0] = HCMD_SELF_CHECK;
		//memcpy(payload+1,&(id), uuid.length() );
		std::copy( this->uuid.begin(), this->uuid.end(), payload+1 );

		this->herald_interface->send_data( payload );
		delete[] payload;
	}

void LanwaveNetwork::broadcast_command( char cmd )
	{
		this->herald_message = cmd;
		this->herald_announce();
	}

void LanwaveNetwork::herald_announce( void )
	{
		this->h_tmr->expires_from_now( boost::posix_time::seconds(0) );
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Exception accounting
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::process_exception( std::exception& e )
	{
		std::string msg( e.what() );

		if ( msg.find("Connection refused") != std::string::npos )
			{
				std::cout << "Dropping node from network map.\n";
				this->check_network();
				return;
			}

		std::cout << "Unknown exception: " << msg << "\n";
	}

/* ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
 ~		Thread related functions
 ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~*/
void LanwaveNetwork::launch( void )
	{
		this->udp->launch();
		this->service_thread = new boost::thread( boost::bind( &LanwaveNetwork::run, this ) );
		this->herald_interface->launch();
		this->herald_ping_self();
		this->herald();
	}

void LanwaveNetwork::run( void )
	{
		//*(this->log) << "IO Service thread starting";
		this->service->run();
		//boost::asio::io_service::work work( *(this->service) );
	}

void LanwaveNetwork::shutdown( void )
	{
		////*(this->log) << "$Shutting down network...";
		this->kill_thread = true;
		////*(this->log) << "$Sending stop signals to threads...";
		//this->h_tmr->expires_from_now(boost::posix_time::seconds(0));
		
		//*(this->log) << "$Waiting for threads to stop...";
		
		//this->herald_thread->join();
		this->service->stop();
		this->service_thread->join();

		//*(this->log) << "$All threads stopped.";
		//*(this->log) << "$Network shut down.";
	}