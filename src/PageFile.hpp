#pragma once

/*
	1. 每个表单独存放于一个文件中
	2. 每个文件的第一个Page(PageNum = 0)用于存放文件头信息(PageFileHeader结构体)
	3. 每个Page的前sizeof(PageHeader)个字节存这个Page的信息
	4. PageFile需要考虑PageHeader
	5. 任何时候都必须写入一个Page的大小
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
		memset (identifyString, 0, sizeof (identifyString));
		strcpy_s (identifyString, Utils::PAGEFILEIDENTIFYSTRING);
		pageCount = 0;
		firstFreePage = 0;
	}
};

class PageFile {

	friend class PageFileManager;

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
	RETCODE ForcePage (PageNum page, const PagePtr & pageHande);

	PageNum GetNumPage ( ) const;

	RETCODE ReadHeader ( );

	RETCODE SetHeader (PageFileHeader h);

	bool IsOpen ( ) const;

	RETCODE Close ( );

private:

	const static int PAGESIZEACTUAL = Utils::PAGESIZE + sizeof (PageHeader);

	RETCODE OpenRead ( );
	RETCODE OpenWrite ( );

	std::fstream & stream ( );

	RETCODE GetHeaderPage (PagePtr & page);

	std::string _filename;

	std::fstream _stream;
	
	PageFileHeader header;

};

using PageFilePtr = std::shared_ptr<PageFile> ;


PageFile::PageFile (const char * name) {
	_filename = name;

}

/*
	in destructor should not interrupt
	refresh the header page
*/
PageFile::~PageFile ( ) {

	this->SetHeader (header);

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

	RETCODE result = RETCODE::COMPLETE;

	if ( !IsOpen ( ) )
		this->OpenRead ( );

	if ( pageNum >= header.pageCount )
		return RETCODE::EOFFILE;

	if ( pageNum < 1 )
		return RETCODE::INVALIDPAGE;

	size_t offset = static_cast< size_t >( pageNum * PAGESIZEACTUAL );	// the first page is used 

	_stream.seekg (offset , std::ios::beg);

	pageHandle = make_shared<Page> ( );

	pageHandle->Create (pageNum);

	_stream.read (pageHandle->_pData.get(), PAGESIZEACTUAL);

	if ( _stream.gcount ( ) != PAGESIZEACTUAL ) {
		Utils::PrintRetcode (RETCODE::INCOMPLETEREAD, __FUNCTION__, __LINE__, std::to_string(_stream.gcount ( )));
		//return RETCODE::INCOMPLETEREAD;
	}

	if ( this->IsOpen ( ) )
		this->Close ( );

	return result;
}

/*
	Allocate a new page at the end of file
	
	TODO:
		Find a free (unused) page to allocate, instead of append a new page to the file
*/
inline RETCODE PageFile::AllocatePage (PagePtr & pageHandle) {
	
	pageHandle = make_shared<Page> ( );

	pageHandle->Create (header.pageCount);		// the actual using page starts from number 1

	header.pageCount++;

	memcpy_s (reinterpret_cast< void* >( pageHandle->_pData.get ( ) ), sizeof (PageHeader),
						  reinterpret_cast<void*>( &pageHandle->_header ), sizeof (PageHeader));	

	memset (pageHandle->GetDataRawPtr(), 0, Utils::PAGESIZE);

	if ( !IsOpen ( ) )
		this->OpenWrite ( );

	_stream.seekp (0, std::ios::end);				// append the new page at the end of file

	auto before = _stream.tellp ( );

	_stream.write (reinterpret_cast< char* >( pageHandle->_pData.get() ), PAGESIZEACTUAL);

	if ( _stream.tellp() - before != PAGESIZEACTUAL ) {
		Utils::PrintRetcode (RETCODE::INCOMPLETEWRITE, __FUNCTION__, __LINE__, std::to_string (_stream.tellp ( ) - before));
	}

	if ( this->IsOpen ( ) )
		this->Close ( );

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

	//--header.pageCount;

	return RETCODE::COMPLETE;

}

/*
	Write a page to disk
*/

inline RETCODE PageFile::ForcePage (PageNum pageNum, const PagePtr & pageHandle) {

	RETCODE result = RETCODE::COMPLETE;

	if ( !IsOpen ( ) )
		this->OpenWrite ( );

	_stream.seekp (pageNum * PAGESIZEACTUAL, std::ios::beg);

	auto before = _stream.tellp ( );

	_stream.write (pageHandle->_pData.get(), PAGESIZEACTUAL);

	if ( _stream.tellp() - before != PAGESIZEACTUAL ) {
		result = RETCODE::HDRWRITE;
		Utils::PrintRetcode (RETCODE::HDRWRITE, __FUNCTION__, __LINE__, std::to_string (_stream.tellp ( ) - before));
	}

	this->Close ( );

	return result;
}

inline PageNum PageFile::GetNumPage ( ) const {
	return header.pageCount;
}

inline bool PageFile::IsOpen ( ) const {
	return _stream.is_open();
}

inline RETCODE PageFile::OpenRead ( ) {
	if ( IsOpen ( ) )
		return RETCODE::FILEOPEN;

	_stream.open (this->_filename, std::ios::binary | std::ios::in);

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::OpenWrite ( ) {
	if ( IsOpen ( ) )
		return RETCODE::FILEOPEN;

	_stream.open (this->_filename, std::ios::binary | std::ios::out | std::ios::in );

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


/*
	Assume that the file has written header
	Only read the pagefile header, not include page header
*/
inline RETCODE PageFile::ReadHeader ( ) {

	if ( !this->IsOpen ( ) )
		this->OpenRead ( );

	PagePtr page;
	//char buf[400];

	//memset (buf, 0, sizeof ( buf ));
	_stream.seekg (0+sizeof(PageHeader) , std::ios::beg);		// 0 page
	//_stream.seekg (, std::ios::beg);		// 0 page

	_stream.read (reinterpret_cast< char* >( &header ), sizeof (PageFileHeader));
	//_stream.read (reinterpret_cast< char* >( buf ), sizeof (buf));

	if ( _stream.gcount ( ) != sizeof (PageFileHeader) ) {		// should not close file now
		Utils::PrintRetcode (RETCODE::HDRREAD, __FUNCTION__, __LINE__);
		return RETCODE::HDRREAD;
	}

	this->Close ( );

	if ( strcmp (header.identifyString, Utils::PAGEFILEIDENTIFYSTRING) != 0 ) {
		return RETCODE::INVALIDPAGEFILE;
	}

	return RETCODE::COMPLETE;
}

inline RETCODE PageFile::SetHeader (PageFileHeader h) {
	RETCODE result = RETCODE::COMPLETE;

	if ( strcmp (h.identifyString, Utils::PAGEFILEIDENTIFYSTRING) ) {
		return RETCODE::INVALIDPAGEFILE;
	}

	PagePtr page;

	this->GetHeaderPage (page);

	memcpy_s ( page->GetDataRawPtr(), sizeof (PageFileHeader), reinterpret_cast< void* >( &h ), sizeof (PageFileHeader));

	if ( !this->IsOpen ( ) )
		this->OpenWrite ( );

	_stream.seekp ( 0 , std::ios::beg);			// header stored at the beginning of a file

	auto before = _stream.tellp ( );

	_stream.write (reinterpret_cast< const char* >( page->_pData.get() ), PAGESIZEACTUAL);

	if ( _stream.tellp() - before != PAGESIZEACTUAL ) {
		result = RETCODE::INCOMPLETEWRITE;
		Utils::PrintRetcode (RETCODE::INCOMPLETEWRITE, __FUNCTION__, __LINE__, std::to_string(_stream.tellp ( ) - before));
		//return RETCODE::INCOMPLETEWRITE;
	}

	if ( this->IsOpen ( ) )
		this->Close ( );

	return result;
}

inline RETCODE PageFile::GetHeaderPage (PagePtr & page) {

	RETCODE result = RETCODE::COMPLETE;
	/*
		Set this header to the page and get this page
	*/

	if ( !IsOpen ( ) )
		this->OpenRead ( );

	_stream.seekg (0, std::ios::beg);

	page = make_shared<Page> ( );

	page ->Create (0);

	_stream.read (page->_pData.get(), PAGESIZEACTUAL);

	if ( _stream.gcount ( ) != PAGESIZEACTUAL ) {
		Utils::PrintRetcode (RETCODE::INCOMPLETEREAD, __FUNCTION__, __LINE__, std::to_string (_stream.gcount ( )));
		//return RETCODE::INCOMPLETEREAD;
	}

	if ( this->IsOpen ( ) )
		this->Close ( );

	return result;
}
