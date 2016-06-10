#pragma once

/*
	1. 主程序(Server)只能通过 OpenFile (const char *fileName, RecordFilePtr &fileHandle); 所返回的fileHandle访问数据库的数据
	2. 只管理数据库各个表的文件, 与Buffer无关
*/

#include "Utils.hpp"
#include "Record.hpp"
#include "RecordFile.hpp"
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

	if ( recordSize >= Utils::PAGESIZE )
		return RETCODE::INVALIDPAGEFILE;

	_pfMgr->CreateFile (fileName);



	return RETCODE::COMPLETE;
}

inline RETCODE RecordFileManager::DestroyFile (const char * fileName) {
	RETCODE result;

	if ( result = _pfMgr->DestroyFile (fileName) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	return result;
}

inline RETCODE RecordFileManager::OpenFile (const char * fileName, RecordFilePtr & fileHandle) {
	RETCODE result;
	PageFilePtr ptr;

	if ( ( result = _pfMgr->OpenFile (fileName, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	fileHandle = make_shared<RecordFile> (ptr);

	return result;
}

inline RETCODE RecordFileManager::CloseFile (RecordFilePtr & fileHandle) {
	RETCODE result;
	PageFilePtr ptr;
	if ( ( result = fileHandle->GetPageFilePtr (ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	if ( ( result = _pfMgr->CloseFile (ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	return result;
}


