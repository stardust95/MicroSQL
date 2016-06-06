#pragma once
#include "Utils.hpp"

class Page {
public:
	Page ( );
	~Page ( );
	Page (const Page & page);

	PF::RETCODE getData (char * & pData) const;

	PF::RETCODE getPageNum (PageNum & pageNum) const;

private:



};

Page::Page ( ) {

}

Page::~Page ( ) {

}

class PageFile {
public:

	PageFile ( );
	~PageFile ( );
	PageFile (const PageFile & file);
	
	PF::RETCODE GetFirstPage (Page &pageHandle) const;   // Get the first page
	PF::RETCODE GetLastPage (Page &pageHandle) const;   // Get the last page

	PF::RETCODE GetNextPage (PageNum current, Page &pageHandle) const;
	// Get the next page
	PF::RETCODE GetPrevPage (PageNum current, Page &pageHandle) const;
	// Get the previous page
	PF::RETCODE GetThisPage (PageNum pageNum, Page &pageHandle) const;
	// Get a specific page
	PF::RETCODE AllocatePage (Page &pageHandle);         // Allocate a new page
	PF::RETCODE DisposePage (PageNum pageNum);                   // Dispose of a page 
	PF::RETCODE MarkDirty (PageNum pageNum) const;             // Mark a page as dirty
	PF::RETCODE UnpinPage (PageNum pageNum) const;             // Unpin a page
	PF::RETCODE ForcePages (PageNum pageNum) const; // Write dirty page(s) to disk

private:

};

typedef std::shared_ptr<Page> PagePtr;
typedef std::shared_ptr<PageFile> PageFilePtr;

PageFile::PageFile ( ) {
}

PageFile::~PageFile ( ) {
}
