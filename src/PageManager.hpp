#pragma once

#include "Utils.hpp"
#include "PageFile.hpp"


class PageManager {
public:

	PageManager ( );
	~PageManager ( );


	PF::RETCODE CreateFile (const char *fileName);       // Create a new file
	PF::RETCODE DestroyFile (const char *fileName);       // Destroy a file
	PF::RETCODE OpenFile (const char *fileName, PageFile &fileHandle);
	// Open a file
	PF::RETCODE CloseFile (PageFile &fileHandle);  // Close a file
	PF::RETCODE AllocateBlock (char *&buffer);              // Allocate a new scratch page in buffer
	PF::RETCODE DisposeBlock (char *buffer);               // Dispose of a scratch page

private:

};

typedef std::shared_ptr<PageManager> PageManagerPtr;

PageManager::PageManager ( ) {

}

PageManager::~PageManager ( ) {

}
