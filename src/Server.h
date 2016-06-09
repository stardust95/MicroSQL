#pragma once

#include "Setting.hpp"
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

	BufferManager _pageMgr;

	SQLManager _sqlMgr;

	IndexManager _indexMgr;

	QueryManager _queryMgr;

};

Server::Server ( ) {
}

Server::~Server ( ) {
}
