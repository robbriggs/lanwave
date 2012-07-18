/************************************************************************************************
 *		LanWave - LWNode class - v1.2 -5/12/11
 *
 ************************************************************************************************/
//#include "networkMap.hpp"
#include "LWNode.hpp"
#include "networkMap.hpp"

using boost::asio::ip::udp;

class networkMap;

// Constructs the node with all the endpoint provided, and all other
// data generated
LWNode::LWNode( udp::endpoint* input_endpoint, networkMap* network )
{
	this->endpoint = new udp::endpoint(*input_endpoint);
	this->nmap = network;
	this->relativeposition = -1;
	this->locked = false;
}

LWNode::~LWNode( void )
{
	delete this->endpoint;
}
