#pragma once
#include "Utils.hpp"

#include <map>
#include <fstream>

class Page {
public:

	struct PageHeader {
		char identyfyString[PF::identifyStringLen];
		char tbName[PF::MaxNameLen];
		bool isUsed;
	};

	Page ( );
	~Page ( );
	Page (const Page & page);

	PF::RETCODE getData (char * & pData) const;

	PF::RETCODE getPageNum (PageNum & pageNum) const;

	PF::RETCODE readHeader (std::ifstream & stream);

	bool getIsUsed ( ) const;

	PF::RETCODE setData (char * );

	PF::RETCODE setPageNum (PageNum);

	PF::RETCODE setUsed (bool);

	PF::RETCODE getTbName (string & name) const;

	PF::RETCODE setTbName (const string & name);

private:

	PageHeader _header;

	PageNum _pageNum;

	char * _pData;

};

typedef shared_ptr<Page> PagePtr;

Page::Page ( ) {
	_pageNum = -1;
	_pData = nullptr;
}

Page::~Page ( ) {

}

inline PF::RETCODE Page::getData (char *& pData) const {
	pData = _pData;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::getPageNum (PageNum & pageNum) const {
	pageNum = _pageNum;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::readHeader (std::ifstream & stream) {
	stream.read (reinterpret_cast<char*>( &_header), sizeof (Page::PageHeader));
	return PF::RETCODE::PF_COMPLETE;
}


inline bool Page::getIsUsed ( ) const {
	return _header.isUsed;
}

inline PF::RETCODE Page::setData (char * pdata) {
	_pData = pdata;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::setPageNum (PageNum num) {
	_pageNum = num;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::setUsed (bool val) {
	_header.isUsed = val;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::getTbName (string & name) const {
	name = _header.tbName;
	return PF::RETCODE::PF_COMPLETE;
}

inline PF::RETCODE Page::setTbName (const string & name) {
	strncpy_s(_header.tbName, name.c_str(), PF::MaxNameLen);
	return PF::RETCODE::PF_COMPLETE;
}

class PageFile {
public:

	struct PageFileHeader {
		char identifyString[PF::identifyStringLen];			// "MicroSQL PageFile", 32 bytes
		char dbName[PF::MaxNameLen];				// 32 bytes
		PageNum	pageCount;			// ull, 8 bytes
		size_t pageSize;					// uint, 4 bytes
	};

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
	PF::RETCODE AllocatePage (Page &pageHandle);				     // Allocate a new page
	PF::RETCODE DisposePage (PageNum pageNum);                   // Dispose of a page 
	PF::RETCODE MarkDirty (PageNum pageNum) const;				// Mark a page as dirty
	PF::RETCODE UnlockPage (PageNum pageNum) const;          // Unlock a page
	PF::RETCODE ForcePages (PageNum pageNum) const;			// Write dirty page(s) to disk

private:

	string _filepath;

	std::ifstream _stream;

	std::map<PageNum, bool> _isDirty;

	std::map<PageNum, bool> _isLocked;
	
};

typedef std::shared_ptr<PageFile> PageFilePtr;

PageFile::PageFile ( ) {

}

PageFile::~PageFile ( ) {

}

inline PF::RETCODE PageFile::GetFirstPage (Page & pageHandle) const {
	GetThisPage (0, pageHandle);
	return PF::RETCODE::PF_COMPLETE;
}
