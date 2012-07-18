/*************************************************************************************************
 *		LanWave
 *
 ************************************************************************************************/
#include "networkMap.hpp"

// Initilzes the data in the class
networkMap::networkMap( void )
{
	this->nextID = 1;
	this->nodesShowing = 0;
	this->numberOfNodes = 0;
	this->map.begin();
	this->locations.begin();
}

networkMap::~networkMap( void )
{
	for ( int i = 0; i < nextID; i++ )
		delete map[i];
}

//		Boost Serialization functions
//	Daniel
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~
template<typename Archive>
void networkMap::serialize( Archive &ar )
	{
		ar & this->map;
		ar & this->locations;
	}
// ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~ ~


// Adds a node with a given endpoint to the collection
int networkMap::addNode( udp::endpoint* input_endpoint )
{
	map.insert(std::pair<int, LWNode*>(nextID, new LWNode(input_endpoint, this)));
	this->numberOfNodes++;
	return this->nextID++;
}

// Removes a node with a given id from the network and changes for
// the data in other nodes accordingly
void networkMap::removeNode( const unsigned short id )
{
	int i;
	int position_removed = (map[id])->relativeposition;
	LWNode* node;

	// Update all relative positions
	for ( i = 0; i < this->nextID; i++ )
	{
		node = map[i];
		if ( node != NULL && (node->relativeposition) > position_removed )
			(node->relativeposition)--;
	}

	this->map.erase( id );
	this->numberOfNodes--;
}

void networkMap::clearMap()
{
	for ( int i = 0; i < nextID; i++ )
		delete map[i];

	map.clear();
	locations.clear();
	this->nextID = 1;
	this->nodesShowing = 0;
	this->numberOfNodes = 0;
}

// Returns the endpoint corrsponding to the id given
udp::endpoint* networkMap::getEndpoint( const int id )
{
	return map[id]->endpoint;
}

// Returns the number of nodes in the network
const unsigned short networkMap::getNumberOfNodes()
{
	return numberOfNodes;
}

// Prints a list of all nodes in the network. Useful for debugging
void networkMap::printNetwork( void )
{
	LWNode* node;
	if ( this-> map.empty() )
		cout << "There are currently no nodes in the network.\n";
	else
	{ // ID | IP
		cout << "\nID \t | IP address \t | Relative Location\n";

		int size = this->nextID;
		for ( int i = 0; i < size; i++ )
		{
			node = this->map[i];
			if ( node == NULL )
				cout << "*** removed ***\n";
			else
			{
				cout << i << "\t | " << node->endpoint->address() << " \t | ";
				
				if ( node->relativeposition == -1 )
					cout << "?";
				else
					cout << node->relativeposition;
					
				cout << "\n";
			}
		}
	}
}

// Moves a node with a given id up one places in the relative
// positions. Returns -1 if error occurs.
const unsigned short networkMap::moveUp( const unsigned short id )
{
	int tmp_pos, id_of_pos;
	LWNode* node1 = map[id];
	if ( node1 == NULL )
		return -1;


	tmp_pos = node1->relativeposition;
	id_of_pos = findPosition(tmp_pos+1);
	if ( id_of_pos == -1 )
		return -1;

	LWNode* node2 = map[id_of_pos];

	(node1->relativeposition)++;
	(node2->relativeposition)--;
	
	return node1->relativeposition;
}

// Moves a node with a given id down one places in the relative
// positions. Returns -1 if error occurs.
const unsigned short networkMap::moveDown( const unsigned short id )
{
	int tmp_pos, id_of_pos;
	LWNode* node2 = map[id];
	if ( node2 == NULL )
		return -1;

	tmp_pos = node2->relativeposition;
	id_of_pos = findPosition(tmp_pos-1);
	if ( id_of_pos == -1 )
		return -1;

	LWNode* node1 = map[id_of_pos];
	(node2->relativeposition)--;
	(node1->relativeposition)++;
	
	return node2->relativeposition;
}

const bool networkMap::setNewRelativePosition( const unsigned short id, const unsigned short newPos )
{
	LWNode* node;
	int oldPos;
	int i;
	int position;

	if ( id < 0 || id >= nextID )
		return false;

	node = map[id];
	if ( node == NULL )
		return false;

	oldPos = node->relativeposition;
	
	if ( oldPos == -1 )
	{
		node->relativeposition = id;
	}
	else if ( newPos == oldPos )
		return false;
	else if ( newPos > oldPos )
	{
		for ( i = 0; i < this->nextID; i++ )
		{
			node = map[i];
			if ( node != NULL )
			{
				position = node->relativeposition;
				if ( position > oldPos && position <= newPos )
				{
					node->relativeposition--;
				}
			}
		}
	} else {
		for ( i = 0; i < this->nextID; i++ )
		{
			node = map[i];
			if ( node != NULL )
			{
				position = node->relativeposition;
				if ( position < oldPos && position >= newPos )
				{
					node->relativeposition++;
				}
			}
		}
	}

	map[id]->relativeposition = newPos;

	return true;
}

const bool networkMap::hideNode( const unsigned short id )
{
	if ( id < 0 || id >= nextID )
		return false;
	
	LWNode* node = this->map[id];
	if ( node == NULL || node->relativeposition == -1 )
		return false;
	
	node->relativeposition = -1;
	
	//for ( int i = 0; i  
	
	return true;
}

const bool networkMap::showNode( const unsigned short id )
{
	if ( id < 0 || id >= nextID )
		return false;
	
	LWNode* node = this->map[id];
	if ( node == NULL || node->relativeposition != -1 )
		return false;

	node->relativeposition = this->nodesShowing;

	locations.insert(std::pair<int, int>(-1, id));
	this->nodesShowing++;
	
	return true;
}

// Finds the id of a node with a given relative position.  Returns -1
// if an error occurs
const unsigned short networkMap::findPosition( const unsigned short pos )
{
	int i;
	LWNode* node;

	if ( pos < 0 || pos >= nextID )
		return -1;

	for ( i = 0; i < nextID; i++ )
	{
		node = this->map[i];
		if ( node != NULL && node->relativeposition == pos )
			return i;
	}

	return -1;
}

void networkMap::updateGraphics( void )
{
	(this->graphicsupdatefunction)();
}

void networkMap::setGraphicsUpdateFunction( void(*updatefunction)() )
{
	this->graphicsupdatefunction = updatefunction;
}

void printNetwork()
{
	cout << "HERE!!\n";
}

/*int main ()
{
	networkMap* nmap = new networkMap();
	nmap->addNode( new udp::endpoint );
	nmap->addNode( new udp::endpoint );
	nmap->addNode( new udp::endpoint );
	nmap->addNode( new udp::endpoint );
	nmap->addNode( new udp::endpoint );
	nmap->addNode( new udp::endpoint );
	nmap->printNetwork();
	
	nmap->showNode(3);
	nmap->printNetwork();
	
	nmap->showNode(0);
	nmap->printNetwork();
	
	nmap->showNode(5);
	nmap->printNetwork();
	
	nmap->hideNode(3);
	nmap->printNetwork();
	
	nmap->setNewRelativePosition(3,1);
	nmap->printNetwork();
	
	nmap->setNewRelativePosition(1,2);
	nmap->printNetwork();
	
	nmap->setNewRelativePosition(2,1);
	nmap->printNetwork();
	
	nmap->setNewRelativePosition(3,1);
	nmap->printNetwork();
	nmap->setNewRelativePosition(3,3);
	nmap->printNetwork();
	//nmap->setGraphicsUpdateFunction((nmap->printNetwork));
	//nmap->updateGraphics();

	nmap->removeNode( 0 );
	nmap->printNetwork();

	nmap->moveUp(2);
	nmap->printNetwork();

	nmap->moveDown(4);
	nmap->printNetwork();

	nmap->removeNode( 3 );
	nmap->printNetwork();
	return 0;
}*/
