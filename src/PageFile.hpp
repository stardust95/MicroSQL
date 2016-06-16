#pragma once

/*
	1. 每个表单独存放于一个文件中
	2. 每个文件的第一个Page(PageNum = 0)用于存放文件头信息(PageFileHeader结构体)
	3. 每个Page的前sizeof(PageHeader)个字节存这个Page的信息
	4. 

*/

#include "Utils.hpp"
#include "Page.hpp"

#include <map>
#include <fstream>

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

class PageFile {

	//friend class RecordFile;

public:


	//PageFile ( );
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


PageFile::PageFile (const char * name) {
	_filename = name;

}

/*
	in destructor should not interrupt
*/
PageFile::~PageFile ( ) {
	
	PagePtr headerPage;
	RETCODE result;

	result = this->GetThisPage (0, headerPage);
	assert (result == RETCODE::COMPLETE);

	memcpy_s (headerPage->GetDataRawPtr(), sizeof (PageFileHeader), reinterpret_cast< void* >( &headerPage ), sizeof (PageFileHeader));

	result = this->ForcePage (0);
	assert (result == RETCODE::COMPLETE);

	if ( _stream.is_open ( ) )
		this->Close ( );
}

inline PageFile::PageFile (const PageFile & file) {		// TODO: How to copy the stream?
	_filename = file._filename;
	header = file.header;
}

inline RETCODE PageFile::GetFirstPage (PagePtr & pageHandle) {
	return GetThisPage (1, pageHandle);
}

inline RETCODE PageFile::GetLastPage (PagePtr & pageHandle) {

	return GetThisPage (header.pageCount-1, pageHandle);
}

inline RETCODE PageFile::GetNextPage (PageNum current, PagePtr & pageHandle) {

	GetThisPage (current + 1, pageHandle);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::GetPrevPage (PageNum current, PagePtr & pageHandle) {
	return GetThisPage(current-1, pageHandle);
}

/*
	pageNum >= 1
*/
inline RETCODE PageFile::GetThisPage (PageNum pageNum, PagePtr & pageHandle) {

	if ( !IsOpen ( ) )
		this->Open ( );

	if ( pageNum >= header.pageCount )
		return RETCODE::EOFFILE;

	if ( pageNum < 1 )
		return RETCODE::INVALIDPAGE;

	//size_t offset = static_cast< size_t >( ( pageNum + 1 ) * Utils::PAGESIZE );	// the first page is used 
	size_t offset = static_cast< size_t >( pageNum * Utils::PAGESIZE );	// the first page is used 

	_stream.seekg (offset, _stream.beg);

	pageHandle = make_shared<Page> ( );

	pageHandle->OpenPage (_stream);

	_stream.read (pageHandle->GetDataRawPtr(), Utils::PAGESIZE);

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

	pageHandle->Create (header.pageCount);		// the actual using page starts from number 1

	header.pageCount;

	memset (pageHandle->GetDataRawPtr(), 0, Utils::PAGESIZE);

	_stream.seekp (0, _stream.end);

	_stream.write ( pageHandle->GetDataRawPtr() , Utils::PAGESIZE);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::DisposePage (PageNum pageNum) {
	
	PagePtr pageHandle;
	RETCODE result;

	if ( result = this->GetThisPage (pageNum, pageHandle) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
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
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	_stream.seekp (sizeof (PageFileHeader) + pageNum * Utils::PAGESIZE);

	_stream.write (pageHandle->GetDataRawPtr(), Utils::PAGESIZE);

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
