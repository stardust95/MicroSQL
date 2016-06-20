#include "Utils.hpp"
#include "Server.hpp"
#include "IndexManager.hpp"


#include <iostream>
#include <vector>


using namespace std;

int main ( ) {

	IndexManagerPtr ixMgr = make_shared<IndexManager> ( );

	RecordFileManagerPtr recMgr = make_shared<RecordFileManager> ( );

	PageFileManagerPtr pfMgr = make_shared<PageFileManager> ( );

	const char * filename = "table.attrName";
	
	SystemManagerPtr sysMgr = make_shared<SystemManager>(ixMgr, recMgr);

	//sysMgr->CreateDb ("testdb", pfMgr);

	system ("pause");
	return 0;
}

