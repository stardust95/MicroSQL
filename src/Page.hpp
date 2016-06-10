#pragma once

#include "Utils.hpp"
#include <fstream>

class Page {
public:

	struct PageHeader {
		char fileName[Utils::MAXNAMELEN];
		char tbName[Utils::MAXNAMELEN];
		PageNum pageNum;
		bool isUsed;
	};

	Page ( );
	~Page ( );
	Page (const Page & page);

	RETCODE GetData (DataPtr & pData) const;

	RETCODE GetPageNum (PageNum & pageNum) const;

	RETCODE ReadHeader (std::ifstream & stream);

	bool GetIsUsed ( ) const;

	RETCODE SetData (char *);

	RETCODE SetPageNum (PageNum);

	RETCODE SetUsage (bool);

	RETCODE GetTbName (string & name) const;

	RETCODE GetFileName (string & name) const;

	RETCODE SetTbName (const string & name);

private:

	PageHeader _header;

	DataPtr _pData;							 // points to an address in memory of size Utils::page_size

};

using PagePtr = shared_ptr<Page>;

Page::Page ( ) {
	_header.pageNum = -1;
	_pData = nullptr;
}

Page::~Page ( ) {

}

inline RETCODE Page::GetData (DataPtr & pData) const {
	pData = _pData;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::GetPageNum (PageNum & pageNum) const {
	pageNum = _header.pageNum;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::ReadHeader (std::ifstream & stream) {
	stream.read (reinterpret_cast< char* >( &_header ), sizeof (Page::PageHeader));

	return RETCODE::COMPLETE;
}


inline bool Page::GetIsUsed ( ) const {
	return _header.isUsed;
}

inline RETCODE Page::SetData (char * pdata) {
	_pData = shared_ptr<char>( new char[Utils::PAGESIZE]() );			// allocate memory and pointed by a shared pointer
	
	memcpy_s (_pData.get ( ), Utils::PAGESIZE, pdata, Utils::PAGESIZE);			// read the source data
	
	return RETCODE::COMPLETE;
}

inline RETCODE Page::SetPageNum (PageNum num) {
	_header.pageNum = num;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::SetUsage (bool val) {
	_header.isUsed = val;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::GetTbName (string & name) const {
	name = _header.tbName;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::GetFileName (string & name) const {
	name = _header.fileName;
	return RETCODE::COMPLETE;
}

inline RETCODE Page::SetTbName (const string & name) {
	strncpy_s (_header.tbName, name.c_str ( ), Utils::MAXNAMELEN);
	return RETCODE::COMPLETE;
}
