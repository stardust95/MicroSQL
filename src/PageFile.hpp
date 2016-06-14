#pragma once

/*
	1. 每个表单独存放于一个文件中
	2. 每个文件的第一个Page用于存放文件头信息(PageFileHeader结构体)
	


*/

#include "Utils.hpp"
#include "Page.hpp"

#include <map>
#include <fstream>


class PageFile {

	//friend class RecordFile;

public:

	struct PageFileHeader {

		size_t numPages;

	};

	PageFile ( );
	PageFile (const PageFile & file);
	PageFile (const char *);
	~PageFile ( );
	
	RETCODE GetFirstPage (PagePtr &pageHandle) ;   // Get the first page
	RETCODE GetLastPage (PagePtr &pageHandle) ;   // Get the last page

	RETCODE GetNextPage (PageNum current, PagePtr &pageHandle) ;
	// Get the next page
	RETCODE GetPrevPage (PageNum current, PagePtr &pageHandle) ;
	// Get the previous page
	RETCODE GetThisPage (PageNum pageNum, PagePtr &pageHandle) ;
	// Get a specific page
	RETCODE AllocatePage (PagePtr &pageHandle);				     // Allocate a new page
	RETCODE DisposePage (PageNum pageNum);                   // Dispose of a page 
	
	//RETCODE ReadHeader ( );

	RETCODE Open ( );
	RETCODE Close ( );

	std::ifstream & stream ( ) ;

private:

	string _filename;

	std::ifstream _stream;
	


};

using PageFilePtr = std::shared_ptr<PageFile> ;

inline PageFile::PageFile ( ) {
}

PageFile::PageFile (const char * name) {
	_filename = name;

}

PageFile::~PageFile ( ) {
	if ( _stream.is_open ( ) )
		this->Close ( );
}

inline PageFile::PageFile (const PageFile & file) {		// TODO: How to copy the stream?
	_filename = file._filename;
	
}

inline RETCODE PageFile::GetFirstPage (PagePtr & pageHandle) {
	GetThisPage (0, pageHandle);
	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::GetLastPage (PagePtr & pageHandle) {


	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::GetNextPage (PageNum current, PagePtr & pageHandle) {

	GetThisPage (current + 1, pageHandle);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::GetThisPage (PageNum pageNum, PagePtr & pageHandle) {

	

	size_t offset = static_cast<size_t>( (pageNum+1) * Utils::PAGESIZE );

	_stream.seekg (offset, _stream.beg);

	pageHandle = make_shared<Page> ( );

	pageHandle->_pData = shared_ptr<char> (new char[Utils::PAGESIZE]());

	_stream.read (pageHandle->_pData.get(), Utils::PAGESIZE);

	return RETCODE ( );
}

inline RETCODE PageFile::AllocatePage (PagePtr & pageHandle) {

	

	return RETCODE ( );
}

inline RETCODE PageFile::Open ( ) {
	
	_stream.open (this->_filename, std::ifstream::binary | std::ifstream::in);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::Close ( ) {
	if ( !_stream.is_open ( ) )
		return RETCODE::CLOSEDFILE;

	_stream.close ( );

	return RETCODE::COMPLETE;
}

inline std::ifstream & PageFile::stream ( ) {

	return _stream;
}
