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

		char identifyString[Utils::IDENTIFYSTRINGLEN];			// "MicroSQL RecordFile", 32 bytes
		PageNum	pageCount;			// ull, 8 bytes
		PageNum firstFreePage;

		PageFileHeader ( ) {
			strcpy_s (identifyString, Utils::PAGEFILEIDENTIFYSTRING);
			pageCount = 0;
			firstFreePage = 0;
		}
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
	RETCODE ForcePage (PageNum page);

	//RETCODE ReadHeader ( );

	PageNum GetNumPage ( ) const;

	bool IsOpen ( ) const;

	RETCODE Open ( );
	RETCODE Close ( );

	std::fstream & stream ( ) ;

private:

	string _filename;

	std::fstream _stream;
	
	PageFileHeader header;

};

using PageFilePtr = std::shared_ptr<PageFile> ;
//
//inline PageFile::PageFile ( ) {
//
//	
//
//}

PageFile::PageFile (const char * name) {
	_filename = name;

}
/*
	should not interrupt
*/
PageFile::~PageFile ( ) {
	
	PagePtr headerPage;
	RETCODE result;

	result = this->GetThisPage (0, headerPage);
	assert (result == RETCODE::COMPLETE);

	memcpy_s (headerPage->GetData ( ).get ( ), sizeof (PageFileHeader), reinterpret_cast< void* >( &headerPage ), sizeof (PageFileHeader));

	result = this->ForcePage (0);
	assert (result == RETCODE::COMPLETE);

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

/*
	pageNum >= 1
*/
inline RETCODE PageFile::GetThisPage (PageNum pageNum, PagePtr & pageHandle) {

	if ( !IsOpen ( ) )
		this->Open ( );

	if ( pageNum >= header.pageCount )
		return RETCODE::EOFFILE;

	if ( pageNum <= 1 )
		return RETCODE::INVALIDPAGE;

	//size_t offset = static_cast< size_t >( ( pageNum + 1 ) * Utils::PAGESIZE );	// the first page is used 
	size_t offset = static_cast< size_t >( pageNum * Utils::PAGESIZE );	// the first page is used 

	_stream.seekg (offset, _stream.beg);

	pageHandle = make_shared<Page> ( );

	pageHandle->OpenPage (_stream);

	_stream.read (pageHandle->GetData().get(), Utils::PAGESIZE);

	return RETCODE ( );
}

/*
	Allocate a new page at the end of file
	
	TODO:
		Find a free (unused) page to allocate, instead of append a new page to the file
*/
inline RETCODE PageFile::AllocatePage (PagePtr & pageHandle) {
	
	if ( !IsOpen ( ) )
		this->Open ( );

	pageHandle = make_shared<Page> ( );

	++header.pageCount;

	pageHandle->Create (header.pageCount);		// the actual using page starts from number 1

	memset (pageHandle->GetData().get(), 0, Utils::PAGESIZE);

	_stream.seekp (0, _stream.end);

	_stream.write (pageHandle->GetData().get ( ), Utils::PAGESIZE);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::DisposePage (PageNum pageNum) {
	
	PagePtr pageHandle;
	RETCODE result;

	if ( result = this->GetThisPage (pageNum, pageHandle) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	pageHandle->SetUsage (false);

	--header.pageCount;

	return RETCODE::COMPLETE;

}

/*
	Write a page to disk
*/

inline RETCODE PageFile::ForcePage (PageNum pageNum) {

	PagePtr pageHandle;
	RETCODE result;
	
	if ( result = this->GetThisPage (pageNum, pageHandle) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	_stream.seekp (sizeof (PageFileHeader) + pageNum * Utils::PAGESIZE);

	_stream.write (pageHandle->GetData ( ).get ( ), Utils::PAGESIZE);

	return RETCODE::COMPLETE;
}

inline PageNum PageFile::GetNumPage ( ) const {
	return header.pageCount;
}

inline bool PageFile::IsOpen ( ) const {
	return _stream.is_open();
}

inline RETCODE PageFile::Open ( ) {
	if ( IsOpen ( ) )
		return RETCODE::FILEOPEN;

	_stream.open (this->_filename, std::ifstream::binary | std::ifstream::in);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::Close ( ) {
	if ( !_stream.is_open ( ) )
		return RETCODE::CLOSEDFILE;

	_stream.close ( );

	return RETCODE::COMPLETE;
}

inline std::fstream & PageFile::stream ( ) {

	return _stream;
}
