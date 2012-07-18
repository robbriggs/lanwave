/*************************************************************************************************
 *		LanWave
 *
 ************************************************************************************************/
#ifndef NETWORK_MAP
#define NETWORK_MAP

#include <iostream>
#include <string>
#include <stdio.h>
#include <map>
#include <utility>

#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>  
#include <boost/serialization/map.hpp>

#include "LWNode.hpp"
#include "networkMapInterface.hpp"

using namespace std;

//using boost:asio::ip::udp

// This is the main class as stores every node on the network. It will
// be accessed by both the networking class and the graphics
class networkMap
{
		friend class boost::serialization::access;
		friend class LanwaveNetwork;

	private:
		// This defines a hashtable which is hashed by an integer (the
		// the 'id', and stores a LWNode.
		typedef std::map<int, LWNode*> MapNodes;
		typedef std::map<int, int> MapIDs;

		// This is the hashtable.
		MapIDs locations;
		
		
		// The function to be called when the network is updated
		void (*graphicsupdatefunction)( void );

		// Other data about the collection of nodes.
		int nextID;
		int numberOfNodes;
		int nodesShowing;
		
		// Private Functions
		template<typename Archive>
		void serialize( Archive &ar );

	public:
		// The constructor
		networkMap( void );
		~networkMap( void );

		// Edit phyisical network structure
		int addNode( udp::endpoint* input_endpoint ); // Returns node id
		void removeNode( const unsigned short id );
		void clearMap();

		// Edit relative network structure
		const unsigned short moveUp( const unsigned short id );// Returns id, or -1 if error
		const unsigned short moveDown( const unsigned short id );// Returns id,or -1 if error
		const bool setNewRelativePosition( const unsigned short id, const unsigned short newPos );
		const bool hideNode( const unsigned short id );
		const bool showNode( const unsigned short id );

		// Getters for network information
		const unsigned short getNumberOfNodes( void );
		void setGraphicsUpdateFunction( void (*updatefunction)() );

		// Getters for individual node
		udp::endpoint* getEndpoint( const int id );
		const unsigned short findPosition( const unsigned short pos );

		// Debugging functions
		void printNetwork( void ); // For debugging
		void unitTest( void );
		MapNodes map;	// Agon wuz ere
		void updateGraphics( void ); // Updates the graphics of changes
};

#endif
