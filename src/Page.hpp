#pragma once

/*
	1. 一个Page的实际存储大小为 PAGESIZEACTUAL
	2. 虽然只在内存中才使用Page的成员函数, 但在文件中存储的时候也要按一个Page的大小来写入文件(按Page存储)
	3. 传递数据的指针不需要SharedPtr, 用裸指针即可
	
*/


#include "Utils.hpp"
#include <fstream>

struct PageHeader {
	char identifyString[Utils::MAXNAMELEN];
	PageNum pageNum;
	bool isUsed;

	PageHeader ( ) {
		memset (identifyString, 0, sizeof ( identifyString ));
		strcpy_s (identifyString, Utils::PAGEIDENTIFYSTRING);
		isUsed = false;
		pageNum = Utils::UNKNOWNPAGENUM;
	}

};

class Page {

	friend class PageFile;

public:


	Page ( PageNum page = Utils::UNKNOWNPAGENUM);
	~Page ( );
	Page (const Page & page);

	RETCODE GetData (char * & pData) const;

	char * GetDataRawPtr ( ) const;

	RETCODE GetPageNum (PageNum & pageNum) const;

	bool GetIsUsed ( ) const;

	RETCODE SetData (char *);

	RETCODE SetPageNum (PageNum);

	RETCODE SetUsage (bool);

	RETCODE Create (PageNum page = Utils::UNKNOWNPAGENUM);

private:

	PageHeader _header;

	DataPtr _pData;							 // points to an address in memory of size Utils::page_size

};

using PagePtr = shared_ptr<Page>;
using PageNumPtr = shared_ptr<PageNum>;

Page::Page ( PageNum page) {
	_header.pageNum = page;
	
	_pData = nullptr;

}

Page::~Page ( ) {

}

inline Page::Page (const Page & page) {
	_header = page._header;
	_pData = page._pData;
}

inline RETCODE Page::GetData (char * &  pData) const {
	pData = _pData.get ( ) + sizeof (PageHeader);
	return RETCODE::COMPLETE;
}

inline char * Page::GetDataRawPtr ( ) const {
	return _pData.get ( ) + sizeof (PageHeader);
}

inline RETCODE Page::GetPageNum (PageNum & pageNum) const {
	pageNum = _header.pageNum;
	return RETCODE::COMPLETE;
}
//
//inline RETCODE Page::OpenPage (std::istream & stream) {
//	//RETCODE result;
//
//	if ( _pData == nullptr )
//		this->Create( );
//
//	stream.read (reinterpret_cast< char* >( &_header ), sizeof (Page::PageHeader));
//
//	if ( strcmp (_header.identifyString, Utils::PAGEIDENTIFYSTRING) != 0 ) {
//		return RETCODE::INVALIDPAGE;
//	}
//
//	stream.read (_pData.get ( ) + sizeof(Page::PageHeader), sizeof (Utils::PAGESIZE));
//
//	return RETCODE::COMPLETE;
//}

inline bool Page::GetIsUsed ( ) const {
	return _header.isUsed;
}

inline RETCODE Page::SetData (char * pdata) {
	if( _pData == nullptr )
		_pData = shared_ptr<char>( new char[Utils::PAGESIZE]() );			// allocate memory and pointed by a shared pointer
	
	memcpy_s (_pData.get ( ) + sizeof(PageHeader), Utils::PAGESIZE, pdata, Utils::PAGESIZE);			// read the souRETCODEe data
	
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
/*
	Memory allocation and initialize header
*/
inline RETCODE Page::Create (PageNum page) {

	_header.isUsed = true;
	_header.pageNum = page;

	_pData = shared_ptr<char> (new char[Utils::PAGESIZE + sizeof (PageHeader)]);

	memcpy_s (_pData.get ( ), sizeof (PageHeader), reinterpret_cast< void* >( &_header ), sizeof (PageHeader));

	return RETCODE::COMPLETE;
}
