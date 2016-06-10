#pragma once

/*
	1. 只管理各种文件, 与Buffer无关
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

	newfile.open (fileName);



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
		Utils::PrintRetcode (result);
		return result;
	}

	return RETCODE::COMPLETE;
}
