#pragma once

#include "Utils.hpp"
#include "BpTree.hpp"
#include "IndexHandle.hpp"
#include "IndexScan.hpp"

class IndexManager {
public:
	IndexManager ( );
	~IndexManager ( );

	RETCODE CreateIndex (const char *fileName,          // Create new index
											int        indexNo,
											AttrType   attrType,
											int        attrLength);
	RETCODE DestroyIndex (const char *fileName,          // Destroy index
											int        indexNo);
	RETCODE OpenIndex (const char *fileName,          // Open index
											int        indexNo,
											IndexHandle &indexHandle);
	RETCODE CloseIndex (IndexHandle &indexHandle);  // Close index

private:

};

using IndexManagerPtr = shared_ptr<IndexManager>;

IndexManager::IndexManager ( ) {
}

IndexManager::~IndexManager ( ) {
}