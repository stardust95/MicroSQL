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

	RETCODE result;

	PageFileHeader header;
	PageFilePtr pageFile;
	PagePtr page;

	if ( fileName == nullptr )
		return RETCODE::CREATEFAILED;

	if ( Utils::IsFileExist (fileName) ) {
		return RETCODE::FILEEXISTS;
	}

	std::ofstream newfile (fileName);

	newfile.close ( );

	pageFile = make_shared<PageFile> (fileName);

	if ( result = pageFile->AllocatePage (page) ) {			// allocate the header page
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	
	if ( result = pageFile->SetHeader (header) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE PageFileManager::DestroyFile (const char * fileName) {

	if ( remove (fileName) ) {
		return RETCODE::INCOMPLETEWRITE;
	}

	return RETCODE::COMPLETE;
}

inline RETCODE PageFileManager::OpenFile (const char * fileName, PageFilePtr & fileHandle) {

	RETCODE result;

	fileHandle = make_shared<PageFile> (fileName);

	if ( result = fileHandle->ReadHeader ( ) ){
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE PageFileManager::CloseFile (PageFilePtr & fileHandle) {
	RETCODE result;

	if ( !fileHandle->IsOpen ( ) ) {
		return RETCODE::COMPLETE;
	}

	if ( result = fileHandle->Close ( ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}
