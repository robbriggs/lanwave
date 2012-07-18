#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "LanwaveNetwork.hpp"


using namespace std;

using boost::asio::ip::udp;

void receiving( int* data, size_t length )
	{
		std::cout << "Int arr was: ";
		
		for (unsigned int i = 0; i < length; i++)
			{
				std::cout << data[i] << ", ";
			}
		
		std::cout << endl;
	}

char* int_to_chararr( unsigned int val )
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

unsigned int chararr_to_int( char* ar )
	{
		unsigned char* arr = reinterpret_cast<unsigned char*>(ar);
		unsigned int val = 0;
		for ( int i = 0; i < 4; i++ )
			val += ((unsigned int)(arr[i]) << 8*i);

		return val;
	}

int main( int argc, char* argv[] )
	{
		std::cerr << "\033[1;31mUDP-Interface Testing Binary\033[0m\n\n";

		std::cout << "argc = " << argc << endl;
		for (int i = 0; i < argc; i++)
			cout << "\targv[" << i << "] = " << argv[i] << endl;

		string input_line;

		std::cout << "DEFAULT_PORT(UDP): " << DEFAULT_PORT << endl;
		std::cout << "DEFAULT_TCP_PORT:  " << DEFAULT_TCP_PORT << endl;
		std::cout << "HERALD_PORT:       " << HERALD_PORT << endl;
		
		LanwaveNetwork* network = new LanwaveNetwork();
		network->add_receive_handler( boost::bind( &receiving, _1, _2 ) );
		
		int* arr = new int[10];
		for (int i = 0; i < 10; i++)
			arr[i] = i;

		// std::string val;
		// std::cout << "enter a number: ";
		// getline( cin, val );

		// unsigned int var = (unsigned int)atoi(val.c_str());
		// std::cout << "before: " << var << "  after: " << chararr_to_int( int_to_chararr( var ) ) << "\n";

		// char* valar = int_to_chararr( var );
		// for ( int i = 0; i < 4; i++ )
		// 	std::cout << (unsigned int)((unsigned char)valar[i]) << " ";


		sleep(1);
		//char num = 12;
		//std::cout << "Our IP: " << network->get_self_ip_address() << endl;

		while ( input_line != "exit" )
			{
				//sleep(1);
				std::cout << "\nEnter Text: ";
				getline(cin, input_line);
				
				if ( input_line == "announce" )
					network->herald_announce();
				if ( input_line == "master" )
					{
						network->rank( "master" );
						std::cout << "You are now da masta\n";
					}
				if ( input_line == "send loopback" )
					{
						int* testarr = new int[8];
						testarr[0] = 1;
						testarr[1] = 2;
						testarr[2] = 3;
						testarr[3] = 4;
						testarr[4] = 1;
						testarr[5] = 2;
						testarr[6] = 3;
						testarr[7] = 4;

						network->send_int_array_loopback( testarr, 8 );
					}
				if ( input_line == "slave" )
					{
						network->rank( "slave" );
						std::cout << "You are now a slave.\n";
					}
				if ( input_line == "self rid" )
					{
						std::string new_rid;
						std::cout << "enter a new relative id for this node: ";
						getline( cin, new_rid );
						network->relative_id( atoi( new_rid.c_str() ) );
					}
				if ( input_line == "set rid" )
					{
						std::string sid, new_rid;
						std::cout << "enter the target sid: ";
						getline( cin, sid );
						std::cout << "enter the new rid for the target: ";
						getline( cin, new_rid );
						network->map->setNewRelativePosition( atoi( sid.c_str() ), atoi( new_rid.c_str() ) );
					}
				if ( input_line == "push rids" )
					{
						std::cout << "pushing\n";
						network->assign_relative_ids();
					}
				if ( input_line == "push sids" )
					{
						std::cout << "pushing sids\n";
						network->assign_static_ids();
					}
				if ( input_line == "sid" )
					{
						std::string new_sid;
						std::cout << "enter a new static id for this node: ";
						getline( cin, new_sid );
						network->static_id( atoi( new_sid.c_str() ) );
					}
				if ( input_line == "vis" )
					{
						std::string new_vis;
						std::cout << "enter a new path to a vis file: ";
						getline( cin, new_vis );
						network->set_vis_path( new_vis );
					}
				if ( input_line == "sync" )
					{
						network->sync();
					}
				if ( input_line == "display" )
					{
						std::cout << "    Printing Network Debugging Information:\n";
						std::cout << "IP Address: " << network->get_self_ip_address() << endl;
						std::cout << "Uuid: " << network->get_uuid() << endl;
						std::cout << "Static ID: " << network->get_static_id() << endl;
						std::cout << "Relative ID: " << network->get_relative_id() << endl;
						std::cout << "Rank: " << network->rank() << endl;
					}
				if ( input_line == "list" )
					{
						std::cout << "do you have friends?:" << endl;
						network->map->printNetwork();
						network->print_hmap();
					}
				if ( input_line == "send" )
					{
						std::cout << "sending data...\n";
						network->send_int_array( arr, 10 );
					}
				if ( input_line == "write" )
					{
						std::cout << "going to try some tcp messaging\n";
						network->tcp->write_command( (char)0x13 );
					}
				if ( input_line == "warr" )
					{
						char* arr = new char[10];
						for (int i = 0; i < 10; i++)
							arr[i] = (char)i;

						network->tcp->write(arr, 10);
					}
				if ( input_line == "file" )
					{
						std::string address_id;
						std::string filename;
						std::cout << "node id: ";
						getline( cin, address_id );
						std::cout << "path: ";
						getline( cin, filename );
						std::cout << "\nsending a file...\n";
						network->send_file( atoi(address_id.c_str()), filename );
					}
				if ( input_line == "help" )
					{
						std::cout << "    LanWave Network Test Program:\nUsage is: $command\n    where $command is one of the following:\n\nannounce: Announce the presence of this node.\nmaster: Sets this node to be the master node.\nsync: Synchronise data between this master and the rest.\ndisplay: prints information about this node.\nlist: prints the network map (lists this nodes friends).\nsend: udp broadcast some test data.\nwrite: tcp send a test command.\nfile: Sends a file. This command is followed by a promt for the target nodes id and then the path to the file.\nwarr: Send an array of data over tcp.\nhelp: Prints this help.\n";
					}
			}

		//sleep(1);

		network->shutdown();
		//sleep(1);

		delete[] arr;
		delete network;

		return 0;
	}

