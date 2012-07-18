#include "networkMapInterface.hpp"

networkMapInterface::networkMapInterface( networkMap* theNetworkMap )
	{this->theNetworkMap = theNetworkMap;}

const unsigned short networkMapInterface::moveUp( const unsigned short id )
	{return theNetworkMap->moveUp( id );}

const unsigned short networkMapInterface::moveDown( const unsigned short id )
	{return theNetworkMap->moveDown( id );}

const bool networkMapInterface::setNewRelativePosition( const unsigned short id, const unsigned short newPos )
	{return theNetworkMap->setNewRelativePosition( id, newPos );}

const bool networkMapInterface::hideNode( const unsigned short id )
	{return theNetworkMap->hideNode( id );}

const bool networkMapInterface::showNode( const unsigned short id )
	{return theNetworkMap->showNode( id );}

const unsigned short networkMapInterface::getNumberOfNodes( void )
	{return theNetworkMap->getNumberOfNodes();}

void networkMapInterface::setGraphicsUpdateFunction( void (*updatefunction)() )
	{theNetworkMap->setGraphicsUpdateFunction( updatefunction );}

const unsigned short networkMapInterface::findPosition( const unsigned short pos )
	{return theNetworkMap->findPosition( pos );}

void networkMapInterface::printNetwork( void )
	{return theNetworkMap->printNetwork();}