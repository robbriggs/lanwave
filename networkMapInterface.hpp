#ifndef NETWORK_MAP_INTERFACE
#define NETWORK_MAP_INTERFACE

#include "networkMap.hpp"

// This object is created by the network class, and passed to everyone else
class networkMapInterface
{
	private:
		networkMap* theNetworkMap;

	public:
		networkMapInterface( networkMap* theNetworkMap );

		const unsigned short moveUp( const unsigned short id );
		const unsigned short moveDown( const unsigned short id );
		const bool setNewRelativePosition( const unsigned short id, const unsigned short newPos );
		const bool hideNode( const unsigned short id );
		const bool showNode( const unsigned short id );
		const unsigned short getNumberOfNodes( void );
		void setGraphicsUpdateFunction( void (*updatefunction)() );
		const unsigned short findPosition( const unsigned short pos );
		void printNetwork( void );
};

#endif