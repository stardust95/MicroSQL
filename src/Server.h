#pragma once

#include "Utils.hpp"
#include "ConnectionManager.hpp"
#include "IndexManager.hpp"
#include "BufferManager.hpp"
#include "QueryManager.hpp"
#include "SQLManager.hpp"
#include "TransactionManager.hpp"


class Server {
public:
	Server ( );
	~Server ( );

private:

	PageManagerPtr _pageMgr;

	//SQLManagerPtr _sqlMgr;

	IndexManagerPtr _indexMgr;

	QueryManager _queryMgr;

};

Server::Server ( ) {
}

Server::~Server ( ) {
}
