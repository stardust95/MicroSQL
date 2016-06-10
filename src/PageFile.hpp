#pragma once

/*
	虽然只在内存中才使用Page的成员函数, 
	但在文件中存储的时候也要按一个Page的大小来写入文件(按Page存储)
*/

#include "Utils.hpp"
#include "Page.hpp"
#include <map>
#include <fstream>


class PageFile {
public:

	struct PageFileHeader {								// stored in the header of every data file
		char identifyString[Utils::IDENTIFYSTRINGLEN];			// "MicroSQL PageFile", 32 bytes
		char dbName[Utils::MAXNAMELEN];				// 32 bytes
		PageNum	pageCount;			// ull, 8 bytes
		size_t pageSize;					// uint, 4 bytes
	};

	PageFile ( );
	PageFile ( const char * );
	~PageFile ( );
	PageFile (const PageFile & file);
	
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
	
	RETCODE ReadHeader ( );

	RETCODE Open ( );
	RETCODE Close ( );

	std::ifstream & stream ( ) ;

private:

	PageFileHeader _header;

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

	size_t offset = sizeof (PageFileHeader) + static_cast<size_t>( pageNum * Utils::PAGESIZE );

	_stream.seekg (offset, _stream.beg);

	pageHandle = make_shared<Page> ( );

	return RETCODE ( );
}

inline RETCODE PageFile::ReadHeader ( ) {

	_stream.read (reinterpret_cast< char* >(&_header), sizeof (PageFileHeader));

	if ( strcmp (_header.identifyString, Utils::PAGEFILEIDENTIFYSTRING) != 0 )
		return RETCODE::INVALIDNAME;

	return RETCODE::COMPLETE;
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
