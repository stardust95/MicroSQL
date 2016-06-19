#include "Utils.hpp"
#include "Server.hpp"
#include "IndexManager.hpp"


#include <iostream>
#include <vector>


using namespace std;

int main ( ) {

	IndexManagerPtr ixMgr = make_shared<IndexManager> ( );

	RecordFileManagerPtr recMgr = make_shared<RecordFileManager> ( );

	const char * filename = "table.attrName";
	
	SystemManagerPtr sysMgr = make_shared<SystemManager>(ixMgr, recMgr);

	//SystemManager::CreateDb ("testdb", 2, nullptr);

	system ("pause");
	return 0;
}

