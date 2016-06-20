#pragma once

#include "Utils.hpp"
#include "IndexHandle.hpp"
#include "RecordIdentifier.hpp"

class IndexScan {
public:
	IndexScan ( );
	~IndexScan ( );
	
	RETCODE OpenScan (const IndexHandle & indexHandle, // Initialize index scan
										CompOp compOp,
										void * value);
	
	RETCODE GetNextEntry (RecordIdentifier & rid);                         // Get next matching entry
	
	RETCODE CloseScan ( );                                 // Terminate index scan

private:


};

IndexScan::IndexScan ( ) {
}

IndexScan::~IndexScan ( ) {
}

inline RETCODE IndexScan::OpenScan (const IndexHandle & indexHandle, CompOp compOp, void * value) {
	
}

inline RETCODE IndexScan::GetNextEntry (RecordIdentifier & rid) {
	return RETCODE ( );
}

inline RETCODE IndexScan::CloseScan ( ) {
	return RETCODE ( );
}
