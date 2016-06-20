#pragma once

#include "Utils.hpp"
#include "HashTable.hpp"
#include "PageFile.hpp"

#include <unordered_map>
/*
	Buffer Manager
	1. 每个BufferManager管理一个文件(PageFilePtr)的Buffer


*/

class BufferManager {
public:

	BufferManager ( );
	
	BufferManager (const PageFile &);

	BufferManager (const PageFilePtr &);

	~BufferManager ( );

	RETCODE GetPage (PageNum page, PagePtr & pBuffer);		// get and lock the target page to one thread

	RETCODE ReadPage (PageNum page, char * dest) ;		// read a page from the disk file

	RETCODE WritePage (PageNum page, char * source) const;	// write a page to the disk file

	RETCODE MarkDirty (PageNum page);

	RETCODE LockPage (PageNum page);
	
	RETCODE UnlockPage (PageNum page);

	RETCODE ForcePage (PageNum page);

	RETCODE FlushPages ( );

	RETCODE GetPageFilePtr (PageFilePtr & ptr) const;

	RETCODE AllocatePage (PagePtr & page);			

	RETCODE DisposePage (PageNum page);

private:

	PageFilePtr _pageFile;

	HashTable _bufferTbl;

	std::unordered_map<PageNum, size_t> _lockMap;

	std::unordered_map<PageNum, bool> _dirtyMap;

};

using BufferManagerPtr = std::shared_ptr<BufferManager> ;

BufferManager::BufferManager ( ) {
	_pageFile = nullptr;
}

BufferManager::BufferManager (const PageFile & page) {
	_pageFile = make_shared<PageFile> (page);

}

BufferManager::BufferManager (const PageFilePtr & ptr) {
	_pageFile = ptr;
}


BufferManager::~BufferManager ( ) {

	this->FlushPages ( );



}

/*
	create a new page and write to file
*/
inline RETCODE BufferManager::AllocatePage ( PagePtr & page) {
	RETCODE result;

	if ( result = _pageFile->AllocatePage (page) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	PageNum num;
	
	page->GetPageNum (num);
	
	if ( result = _bufferTbl.Insert (num, page) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	_lockMap[num] = 1;
	_dirtyMap[num] = false;

	return result;
}

inline RETCODE BufferManager::DisposePage (PageNum page) {
	RETCODE result;

	if ( _bufferTbl.Delete (page) != RETCODE::HASHNOTFOUND ) {
		_dirtyMap.erase (page);
		_lockMap.erase (page);
	}

	if ( result = _pageFile->DisposePage (page) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}

/*
	Main Function to get page
*/
inline RETCODE BufferManager::GetPage ( PageNum page, PagePtr & ptr) {

	RETCODE result;
/*
	if ( page >= _pageFile->GetNumPage ( ) ) {

		return RETCODE::PAGENUMNOTFOUND;
	}
*/
	if ( _bufferTbl.Find ( page, ptr) == RETCODE::HASHNOTFOUND  ) {
		if ( result = _pageFile->GetThisPage (page, ptr) ) {
			Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
			return result;
		}
		_bufferTbl.Insert (page, ptr);
	}

	if ( (result = LockPage (page)) && result != RETCODE::PAGELOCKNED ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}

/*
	Read but not lock the page
*/
inline RETCODE BufferManager::ReadPage ( PageNum page, char * dest) {		

	PagePtr ptr;
	RETCODE result;


	if ( ( result = GetPage(page, ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	char * pdata;

	ptr->GetData (pdata);

	memcpy_s (dest, Utils::PAGESIZE, pdata, Utils::PAGESIZE);

	return result;
}

/*
	Unused
*/
inline RETCODE BufferManager::WritePage ( PageNum page, char * source) const {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = ptr->SetData (source) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = _pageFile->ForcePage (page, ptr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	return result;
}

inline RETCODE BufferManager::MarkDirty ( PageNum page) {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	_dirtyMap[page] = true;

	return result;
}

inline RETCODE BufferManager::LockPage (PageNum page) {

	if ( _lockMap[page] == 0 ) {				// the requested page is not using by any thread
		_lockMap[page] = 1;					// lock the page
	} else {												// if the page is already locked
		return RETCODE::PAGELOCKNED;
	}

	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::UnlockPage ( PageNum page) {
	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( _lockMap[page] == 0 ) {			// if the page has not lock
		return RETCODE::PAGEUNLOCKNED;
	}
/*
	Unlock != Delete
	if ( ( result = _bufferTbl.Delete ( page) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
*/

	_lockMap[page] -= 1;
	_dirtyMap[page] = false;

	return result;
}

inline RETCODE BufferManager::ForcePage (PageNum page) {
	
	PagePtr pagePtr;
	RETCODE result;

	if ( result = _bufferTbl.Find (page, pagePtr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	return _pageFile->ForcePage (page, pagePtr);

}

inline RETCODE BufferManager::FlushPages () {			// TODO: How to write page to disk file
	RETCODE result = RETCODE::COMPLETE;
	vector<PageNum> vec;
	PagePtr page;
/*
	if ( (result = _pageFile->OpenWrite ( ) ) && result != RETCODE::FILEOPEN ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}*/

	_bufferTbl.Keys (vec);

	for ( auto item : vec ) {			// flush all pages
		if ( _dirtyMap[item] ){		// if the page is modified, write to the disk file
			_bufferTbl.Find (item, page);
			if ( result = _pageFile->ForcePage (item, page) ) {
				Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
				return result;
			}
			_dirtyMap[item] = false;
		}
	}

	return result;
}

inline RETCODE BufferManager::GetPageFilePtr (PageFilePtr & ptr) const {

	ptr = _pageFile;

	return RETCODE::COMPLETE;
}
