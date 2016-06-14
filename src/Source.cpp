#include "Utils.hpp"
#include "Server.hpp"
#include "IndexManager.hpp"
#include "IndexHandle.hpp"
#include "BpTreeNode.hpp"


#include <iostream>
#include <vector>


int main ( ) {
	//IndexHandle handle;

	Server server;
	
	PagePtr page;

	BpTreeNode node(FLOAT, 4, page);

	node.Print ( );

}

