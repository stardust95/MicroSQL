#pragma once

#include "Utils.hpp"
#include "Utils.hpp"
#include "PageFile.hpp"
#include "HashTable.hpp"

#include <unordered_map>
/*
	Buffer Manager

*/

class BufferManager {
public:

	BufferManager ( );
	~BufferManager ( );

	RETCODE AllocatePage (const char * filename, PageNum page, PagePtr & pBuffer );

	RETCODE GetPage (const string & filename, PageNum page, PagePtr & pBuffer);		// get and lock the target page to one thread

	RETCODE ReadPage (const string & filename, PageNum page, char * dest) const;

	RETCODE WritePage (const string & filename, PageNum page, char * source) const;

	RETCODE MarkDirty (const char * filename, PageNum page);

	RETCODE LockPage (const string & filename, PageNum page);
	
	RETCODE UnlockPage (const string & filename, PageNum page);

	RETCODE FlushPages (const char *);

private:
	
	HashTable _bufferTbl;

	std::unordered_map<PagePtr, size_t> _lockMap;

	std::unordered_map<PagePtr, bool> _dirtyMap;

};

using BufferManagerPtr = std::shared_ptr<BufferManager> ;

BufferManager::BufferManager ( ) {

}

BufferManager::~BufferManager ( ) {

}

inline RETCODE BufferManager::AllocatePage (const char * file, PageNum num, PagePtr & pBuffer) {
	RETCODE result;
	
	if ( ( result = _bufferTbl.Find (file, num, pBuffer) ) != HASHNOTFOUND ) {			// The page is already in buffer
		return RETCODE::PAGEINBUF;
	}

	_bufferTbl.Insert (file, num, pBuffer);		// TODO: Replace page when full

	return result;
}

inline RETCODE BufferManager::GetPage (const string & filename, PageNum page, PagePtr & ptr) {

	RETCODE result;

	if ( result = _bufferTbl.Find (filename, page, ptr) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	if ( _lockMap[ptr] == 0 ) {				// the requested page is not using by any thread
		_lockMap[ptr] = 1;
	} else {
		
	}

	return result;

}

inline RETCODE BufferManager::ReadPage (const string & filename, PageNum page, char * dest) const {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	DataPtr pdata;

	ptr->GetData (pdata);

	memcpy_s (dest, Utils::PAGESIZE, pdata.get(), Utils::PAGESIZE);

	return result;
}

inline RETCODE BufferManager::WritePage (const string & filename, PageNum page, char * source) const {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	ptr->SetData (source);
	
	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::MarkDirty (const char * filename, PageNum page) {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	_dirtyMap[ptr] = true;

	return result;
}

inline RETCODE BufferManager::UnlockPage (const string & filename, PageNum page) {
	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	if ( _lockMap[ptr] == 0 ) {
		return RETCODE::PAGEUNLOCKNED;
	}

	if ( ( result = _bufferTbl.Delete (filename, page) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	_lockMap[ptr] -= 1;
	_dirtyMap[ptr] = false;

	return result;
}

inline RETCODE BufferManager::FlushPages (const char * file) {
	RETCODE result;
	vector<PagePtr> vec;

	_bufferTbl.FindAll (file, vec);

	PageFile fileHandle (file);

	if ( (result = fileHandle.Open ( ) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	for ( auto item : vec ) {
		if ( _lockMap[item] > 0 )
			return RETCODE::PAGELOCKNED;

		if( _dirtyMap[item] )
			fileHandle.AllocatePage (item);
	
	}

	return result;
}
