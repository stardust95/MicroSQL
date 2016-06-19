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
											//int        indexNo,
											AttrType   attrType,
											int        attrLength);
	RETCODE DestroyIndex (const char *fileName);          // Destroy index
											
	RETCODE OpenIndex (const char *fileName,          // Open index
											//int        indexNo,
										IndexHandlePtr &indexHandle);
	RETCODE CloseIndex (IndexHandlePtr &indexHandle);  // Close index

private:

	PageFileManagerPtr _pfMgr;

};

using IndexManagerPtr = shared_ptr<IndexManager>;

IndexManager::IndexManager ( ) {
}

IndexManager::~IndexManager ( ) {
}

inline RETCODE IndexManager::CreateIndex (const char * fileName, AttrType attrType, int attrLength) {

	if ( !( attrType == FLOAT || attrType == INT || attrType == STRING ) || fileName == nullptr )
		return RETCODE::CREATEFAILED;

	if ( ( attrType == FLOAT && attrLength != 4 ) || ( attrType == INT && attrLength != 4 ) )
		return RETCODE::CREATEFAILED;

	PageFilePtr pagefile;
	BufferManagerPtr bufMgr;
	RETCODE result;

	if ( (result = _pfMgr->CreateFile (fileName)) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = _pfMgr->OpenFile (fileName, pagefile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	bufMgr = make_shared<BufferManager> (pagefile);

	PagePtr headerPage;					// pageNum = 1;
	char * pData;

	if ( result = bufMgr->AllocatePage (headerPage) ) {	
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = headerPage->GetData (pData) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	IndexHeader header;
	header.attrType = attrType;
	header.attrLength = attrLength;
	header.numPages = 1;		// must have one header page
	header.numMaxKeys = ( Utils::PAGESIZE - sizeof (BpTreeNodeHeader) ) / ( sizeof (attrLength) + sizeof (RecordIdentifier) );
	header.rootPage = -1;
	header.height = 0;

	memcpy_s (pData, sizeof (IndexHeader), reinterpret_cast< void* >( &header ), sizeof (IndexHeader));

	PageNum page;
	headerPage->GetPageNum (page);

	if ( result = bufMgr->MarkDirty (page) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	// write the header page to disk
	if ( (result = bufMgr->UnlockPage (page)) && result != RETCODE::PAGEUNLOCKNED ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = bufMgr->ForcePage (page) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = _pfMgr->CloseFile (pagefile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}

inline RETCODE IndexManager::OpenIndex (const char * fileName, IndexHandlePtr & indexHandle) {
	RETCODE result;
	PageFilePtr pageFile;

	if ( result = _pfMgr->OpenFile (fileName, pageFile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	BufferManagerPtr bufMgr = make_shared<BufferManager> (pageFile);

	indexHandle = make_shared<IndexHandle> ( );

	if ( result = indexHandle->Open (bufMgr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = _pfMgr->CloseFile (pageFile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}
