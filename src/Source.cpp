#include "Utils.hpp"
#include "Server.hpp"

#include <iostream>
#include <vector>


int main ( ) {

	Server server;
	
	RecordFileManagerPtr recordMgr = make_shared<RecordFileManager> ( );

	recordMgr->CreateFile ("testfile", 100);


}

