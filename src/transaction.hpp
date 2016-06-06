#pragma once

#include "Utils.hpp"

class Transaction {
public:

	enum State {
		Active, Commited, Aborted
	};

	State _state;

	uint64 _id;

	Transaction ( );
	~Transaction ( );

	

private:

};

Transaction::Transaction ( ) {
}

Transaction::~Transaction ( ) {
}
