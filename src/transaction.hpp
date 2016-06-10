#pragma once

#include "Utils.hpp"

class Transaction {
public:

	enum State {
		Active, Commited, Aborted
	};

	State _state;



	Transaction ( );
	~Transaction ( );



private:

};

using TransactionPtr = shared_ptr<Transaction>;


Transaction::Transaction ( ) {
}

Transaction::~Transaction ( ) {
}
