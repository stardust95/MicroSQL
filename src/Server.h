#pragma once

#include "Setting.hpp"
#include "ConnectionManager.hpp"
#include "IndexManager.hpp"
#include "PageManager.hpp"
#include "QueryManager.hpp"
#include "SQLManager.hpp"
#include "StorageManager.hpp"
#include "TransactionManager.hpp"


class Server {
public:
	Server ( );
	~Server ( );

private:

	PageManager _pageMgr;

	SQLManager _sqlMgr;

	QueryManager _queryMgr;

};

Server::Server ( ) {
}

Server::~Server ( ) {
}
