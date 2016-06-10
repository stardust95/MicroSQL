#include "Utils.hpp"
#include "Server.hpp"

#include <iostream>
#include <vector>


int main ( ) {

	Server server;
	
	RecordFileManagerPtr recordMgr = make_shared<RecordFileManager> ( );

	recordMgr->CreateFile ("testfile", 100);
	
	RecordFilePtr file;
	recordMgr->OpenFile ("testfile", file);

	RecordIdentifier id{ 1, 1 };
	Record rec;

	file->GetRec (id, rec);

}

