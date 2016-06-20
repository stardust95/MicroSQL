#pragma once

/*
	1. 主程序(Server)只能通过 OpenFile (const char *fileName, RecordFilePtr &fileHandle); 所返回的fileHandle访问数据库的数据
	2. 只管理数据库各个表的文件, 与Buffer无关
*/

#include "Utils.hpp"
#include "Record.hpp"
#include "RecordFile.hpp"
#include "RecordFileScan.hpp"
#include "PageFileManager.hpp"


class RecordFileManager {

public:
	RecordFileManager ( );
	~RecordFileManager ( );

	RETCODE CreateFile (const char *fileName, size_t recordSize);
	RETCODE DestroyFile (const char *fileName);
	RETCODE OpenFile (const char *fileName, RecordFilePtr &fileHandle);

	RETCODE CloseFile (RecordFilePtr &fileHandle);

private:

	PageFileManagerPtr _pfMgr;

};

using RecordFileManagerPtr = shared_ptr<RecordFileManager>;

RecordFileManager::RecordFileManager ( ) {
	_pfMgr = make_shared<PageFileManager> ( );

}

RecordFileManager::~RecordFileManager ( ) {
}

inline RETCODE RecordFileManager::CreateFile (const char * fileName, size_t recordSize) {

	if ( fileName == nullptr )
		return RETCODE::INVALIDNAME;

	if ( recordSize >= Utils::PAGESIZE - sizeof(RecordPageHeader) )
		return RETCODE::INVALIDPAGEFILE;

	RETCODE result;

	if ( result = _pfMgr->CreateFile (fileName) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	PageFilePtr pageFile;
	BufferManagerPtr bufMgr;

	if ( result = _pfMgr->OpenFile (fileName, pageFile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	bufMgr = make_shared<BufferManager> (pageFile);

	PagePtr headerPage;		// header page of RecordFile
	char * pData;
	
	if ( result = bufMgr->AllocatePage (headerPage) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = headerPage->GetData (pData) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	RecordFileHeader header;

	header.firstFreePage = Utils::UNKNOWNPAGENUM;
	header.numPages = 1;	// header page
	header.recordSize = recordSize;

	memcpy_s (pData, sizeof (RecordFileHeader),
			  reinterpret_cast< const void * >( &header ), sizeof (RecordFileHeader));

	PageNum headerPageNum;

	headerPage->GetPageNum (headerPageNum);
	
	

	if ( result = bufMgr->ForcePage (headerPageNum) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = bufMgr->UnlockPage (headerPageNum) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = _pfMgr->CloseFile (pageFile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	return RETCODE::COMPLETE;
}

inline RETCODE RecordFileManager::DestroyFile (const char * fileName) {
	RETCODE result;

	if ( result = _pfMgr->DestroyFile (fileName) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE RecordFileManager::OpenFile (const char * fileName, RecordFilePtr & fileHandle) {
	RETCODE result;
	PageFilePtr ptr;
	BufferManagerPtr bufMgr;

	if ( ( result = _pfMgr->OpenFile (fileName, ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	bufMgr = make_shared<BufferManager> (ptr);

	fileHandle = make_shared<RecordFile> ();
	fileHandle->Open (bufMgr);

	return result;
}

inline RETCODE RecordFileManager::CloseFile (RecordFilePtr & fileHandle) {
	RETCODE result;
	PageFilePtr ptr;
	if ( ( result = fileHandle->GetPageFilePtr (ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	if ( ( result = _pfMgr->CloseFile (ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}


