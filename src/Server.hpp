#pragma once

#include "Utils.hpp"
//#include "ConnectionManager.hpp"
#include "IndexManager.hpp"
#include "QueryManager.hpp"
//#include "SQLManager.hpp"
//#include "TransactionManager.hpp"
#include "RecordFileManager.hpp"
#include "SystemManager.hpp"

class Server {
public:
	Server ( );
	~Server ( );

	

private:

	IndexManagerPtr _indexMgr;

	RecordFileManagerPtr _recordMgr;

	SystemManagerPtr _systemMgr;

};

Server::Server ( ) {
}

Server::~Server ( ) {
}
