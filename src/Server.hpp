#pragma once

#include "Utils.hpp"
//#include "ConnectionManager.hpp"
#include "QueryManager.hpp"

class Server {
public:
	Server ( );
	~Server ( );

	

private:

	QueryManagerPtr _queryMgr;

};

Server::Server ( ) {
}

Server::~Server ( ) {
}
