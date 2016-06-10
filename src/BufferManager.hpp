#pragma once

#include "Utils.hpp"
#include "HashTable.hpp"
#include "PageFile.hpp"

#include <unordered_map>
/*
	Buffer Manager

*/

class BufferManager {
public:

	BufferManager ( );
	
	BufferManager (const PageFile &);

	BufferManager (const PageFilePtr &);

	~BufferManager ( );

	RETCODE GetPage (PageNum page, PagePtr & pBuffer);		// get and lock the target page to one thread

	RETCODE ReadPage (PageNum page, char * dest) const;		// read a page from the disk file

	RETCODE WritePage (PageNum page, char * source) const;	// write a page to the disk file

	RETCODE MarkDirty (PageNum page);

	RETCODE LockPage (PageNum page);
	
	RETCODE UnlockPage (PageNum page);

	RETCODE FlushPages ( );

	RETCODE GetPageFilePtr (PageFilePtr & ptr) const;

private:

	RETCODE AllocatePage (PageNum page);			// load a new page to buffer table

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

}

inline RETCODE BufferManager::AllocatePage ( PageNum num) {
	RETCODE result;
	PagePtr pBuffer;

	_pageFile->GetThisPage (num, pBuffer);
	
	if ( ( result = _bufferTbl.Find ( num, pBuffer) ) != HASHNOTFOUND ) {			// The page is already in buffer
		Utils::PrintRetcode (result);
		return RETCODE::PAGEINBUF;
	}

	if( result = _bufferTbl.Insert ( num, pBuffer ) ){ 		// TODO: Replace page when full
		Utils::PrintRetcode (result);
		return result;
	}

	_lockMap[num] = 0;
	_dirtyMap[num] = false;

	return result;
}

inline RETCODE BufferManager::GetPage ( PageNum page, PagePtr & ptr) {

	RETCODE result;

	if ( result = _bufferTbl.Find ( page, ptr) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	if ( result = LockPage (page) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	return result;
}

/*
	Read but not lock the page
*/
inline RETCODE BufferManager::ReadPage ( PageNum page, char * dest) const {		

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	DataPtr pdata;

	ptr->GetData (pdata);

	memcpy_s (dest, Utils::PAGESIZE, pdata.get(), Utils::PAGESIZE);

	return result;
}

/*
	Unused
*/
inline RETCODE BufferManager::WritePage ( PageNum page, char * source) const {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	ptr->SetData (source);
	
	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::MarkDirty ( PageNum page) {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find ( page, ptr) ) ) {
		Utils::PrintRetcode (result);
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
		Utils::PrintRetcode (result);
		return result;
	}

	if ( _lockMap[page] == 0 ) {			// if the page has not lock
		return RETCODE::PAGEUNLOCKNED;
	}

	if ( ( result = _bufferTbl.Delete ( page) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	_lockMap[page] -= 1;
	_dirtyMap[page] = false;

	return result;
}

inline RETCODE BufferManager::FlushPages () {			// TODO: How to write page to disk file
	RETCODE result;
	vector<PageNum> vec;

	if ( (result = _pageFile->Open ( ) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	for ( auto item : vec ) {			// flush all pages
		//if ( _lockMap[item] > 0 )		
			//return RETCODE::PAGELOCKNED;

		//if( _dirtyMap[item] )		// if the page is modified, write to the disk file
			//fileHandle.AllocatePage (item);
	
	}

	return result;
}

inline RETCODE BufferManager::GetPageFilePtr (PageFilePtr & ptr) const {

	ptr = _pageFile;

	return RETCODE::COMPLETE;
}
