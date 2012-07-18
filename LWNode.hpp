/*************************************************************************************************
 *		LanWave - LWNode class - v1.2 - 5/11/12
 *
 ************************************************************************************************/
#ifndef LWNODE
#define LWNODE

#include <iostream>
#include <string>
#include <stdio.h>
#include <map>
#include <utility>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

using namespace std;
using boost::asio::ip::udp;

class networkMap;

// This is a node in the network, and store all the information about it.
// For security reasons, no-one will ever be given an object of it.
class LWNode
{
	private:
		networkMap* nmap;

	public:
		// This is the data it stores about each node.
		udp::endpoint* endpoint;
		int relativeposition;
		int lastposition;
		bool locked;
		
		// The constructor places the endpoint it, and generates all
		// the other data.
		LWNode( udp::endpoint* input_endpoint, networkMap* network );
		~LWNode( void );
};

#endif
