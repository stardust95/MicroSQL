#pragma once

#include "Utils.hpp"
#include "IndexHandle.hpp"
#include "IndexScan.hpp"
#include "PageFileManager.hpp"

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

	PageFileManagerPtr _pfMgr;

};

using IndexManagerPtr = shared_ptr<IndexManager>;

IndexManager::IndexManager ( ) {
}

IndexManager::~IndexManager ( ) {
}

inline RETCODE IndexManager::CreateIndex (const char * fileName, int indexNo, AttrType attrType, int attrLength) {

	PageFilePtr pagefile;

	_pfMgr->CreateFile (fileName);

	_pfMgr->OpenFile (fileName, pagefile);

	BufferManagerPtr bufMgr = make_shared<BufferManager> ( );

	IndexHandlePtr index = make_shared<IndexHandle> (attrType, attrLength);

	return RETCODE::COMPLETE;
}
