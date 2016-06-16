#include "Utils.hpp"
#include "Server.hpp"
#include "IndexManager.hpp"


#include <iostream>
#include <vector>


using namespace std;

int main ( ) {

	IndexManagerPtr ixMgr = make_shared<IndexManager> ( );

	const char * filename = "table.attrName";

	IndexHandlePtr index;

	ixMgr->CreateIndex (filename, INT, 4);

	ixMgr->OpenIndex (filename, index);

	system ("pause");
	return 0;
}

