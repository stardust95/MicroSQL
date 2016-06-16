#pragma once

/*
	1. 只管理各种文件, 与Buffer无关
	2. File的第一个Page存PageFileHeader结构体



*/

#include "Utils.hpp"

class PageFileManager {
public:
	PageFileManager ( );
	~PageFileManager ( );


	RETCODE CreateFile (const char * fileName);       // Create a new file

	RETCODE DestroyFile (const char * fileName);       // Destroy a file

	RETCODE OpenFile (const char * fileName, PageFilePtr & fileHandle);		// Open a file

	RETCODE CloseFile (PageFilePtr &fileHandle);				// Close a file


private:

};

using PageFileManagerPtr = shared_ptr<PageFileManager>;

PageFileManager::PageFileManager ( ) {
}

PageFileManager::~PageFileManager ( ) {
}

inline RETCODE PageFileManager::CreateFile (const char * fileName) {

	std::ofstream newfile;
	
	if ( fileName == nullptr )
		return RETCODE::CREATEFAILED;

	newfile.open (fileName);

	if ( !newfile.is_open ( ) ) {
		return RETCODE::CREATEFAILED;
	}

	PageFileHeader header;
	PageFilePtr pageFile;
	PagePtr page;

	page->Create ( );
	header.pageCount = 1;
	
	memcpy_s (page->GetDataRawPtr ( ), sizeof (PageFileHeader), reinterpret_cast< void* >( &header ), sizeof (PageFileHeader));

	pageFile = make_shared<PageFile> ( fileName );
	
	newfile.close ( );

	return RETCODE::COMPLETE;
}

inline RETCODE PageFileManager::DestroyFile (const char * fileName) {

	if ( remove (fileName) ) {
		return RETCODE::INCOMPLETEWRITE;
	}

	return RETCODE::COMPLETE;
}

inline RETCODE PageFileManager::OpenFile (const char * fileName, PageFilePtr & fileHandle) {

	fileHandle = make_shared<PageFile> (fileName);

	fileHandle->Open ( );

	return RETCODE ( );
}

inline RETCODE PageFileManager::CloseFile (PageFilePtr & fileHandle) {
	RETCODE result;

	if ( ( result = fileHandle->Close ( ) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}
