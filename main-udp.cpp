#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <string>
#include <boost/asio.hpp>

#include "udpInterface.hpp"
//#include "LanwaveNetwork.hpp"


using namespace std;

using boost::asio::ip::udp;

void receiving( char* data )
	{
		std::cout << "Double was: " << (int)(*data) << endl;
	}

void run( boost::asio::io_service* service )
	{
		boost::asio::io_service::work work( *(service) );
	}

int main( int argc, char* argv[] )
	{	
		std::cerr << "\033[1;31mUDP-Interface Testing Binary\033[0m\n\n";
		
		std::cout << "argc = " << argc << endl;
		for (int i = 0; i < argc; i++)
			cout << "\targv[" << i << "] = " << argv[i] << endl;
		
		string input_line;

		boost::asio::io_service* serv = new boost::asio::io_service();
		udpInterface* udp = new udpInterface(serv,receiving);
		//udp->set_target_address("broadcast");
		udp->set_tx_packet_size(17);
		udp->set_rx_buffer_size(13);
		udp->launch();
		
		char num = 12;
		
		std::cout << "Target Address: " << udp->get_target() << endl;
		
		while ( input_line != ":q" )
			{
				sleep(1);
				std::cout << "\nEnter Text: ";				
				getline(cin, input_line);
				cout << "1: " << (int)num << endl;
				udp->send_data( &num );
			}

		sleep(1);
		std::cout.flush();
		std::cout << "Bytes Sent:       " << udp->total_bytes_tx() << endl;
		std::cout << "Bytes Received:   " << udp->total_bytes_rx() << endl;
		std::cout << "Packets Sent:     " << udp->total_packets_tx() << endl;
		std::cout << "Packets Received: " << udp->total_packets_rx() << endl;
		
		udp->shutdown();
		delete udp;
		delete serv;
		return 0;
	}

