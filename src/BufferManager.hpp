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

	//RETCODE CreateFile (const char * fileName);       // Create a new file
	//RETCODE DestroyFile (const char * fileName);       // Destroy a file
	RETCODE OpenFile (const char * fileName, PageFilePtr & fileHandle);		// Open a file
	//RETCODE CloseFile (PageFile &fileHandle);				// Close a file
	//RETCODE AllocateBlock (DataPtr&buffer);              // Allocate a new scratch page in buffer
	//RETCODE DisposeBlock (DataPtrbuffer);               // Dispose of a scratch page

	RETCODE AllocatePage (PagePtr );

	RETCODE GetPage (const string & filename, PageNum page, PagePtr & pBuffer);		// get and lock the target page to one thread

	RETCODE ReadPage (const string & filename, PageNum page, char * dest) const;

	RETCODE WritePage (const string & filename, PageNum page, char * source) const;

	RETCODE MarkDirty (const char * filename, PageNum page);

	RETCODE UnlockPage (const string & filename, PageNum page);

	RETCODE FlushPages (const char *);

private:
	
	HashTable _bufferTbl;

	std::unordered_map<PagePtr, size_t> _lockMap;

	std::unordered_map<PagePtr, bool> _dirtyMap;

};

using PageManagerPtr = std::shared_ptr<BufferManager> ;

BufferManager::BufferManager ( ) {

}

BufferManager::~BufferManager ( ) {

}

inline RETCODE BufferManager::OpenFile (const char * fileName, PageFilePtr & fileHandle) {

	fileHandle = make_shared<PageFile>(fileName);

	fileHandle->Open ( );

	return RETCODE ( );
}

inline RETCODE BufferManager::AllocatePage (PagePtr pg) {
	//PagePtr pg;
	string file;
	PageNum num;

	pg->GetFileName (file);
	pg->GetPageNum (num);

	if ( _bufferTbl.Find (file, num, pg) != HASHNOTFOUND ) {			// The page is already in buffer
		return RETCODE::COMPLETE;
	}

	_bufferTbl.Insert (file, num, pg);		// TODO: Replace page when full

	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::GetPage (const string & filename, PageNum page, PagePtr & ptr) {

	RETCODE result = _bufferTbl.Find (filename, page, ptr);

	if ( result == RETCODE::COMPLETE ) {
		return RETCODE::COMPLETE;
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

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) != RETCODE::COMPLETE ) {
		return result;
	}

	DataPtr pdata;

	ptr->GetData (pdata);

	memcpy_s (dest, Utils::PAGESIZE, pdata.get(), Utils::PAGESIZE);

	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::WritePage (const string & filename, PageNum page, char * source) const {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) != RETCODE::COMPLETE ) {
		return result;
	}

	ptr->SetData (source);
	
	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::MarkDirty (const char * filename, PageNum page) {

	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) != RETCODE::COMPLETE ) {
		return result;
	}

	_dirtyMap[ptr] = true;

	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::UnlockPage (const string & filename, PageNum page) {
	PagePtr ptr;
	RETCODE result;

	if ( ( result = _bufferTbl.Find (filename, page, ptr) ) != RETCODE::COMPLETE ) {
		return result;
	}

	if ( _lockMap[ptr] == 0 ) {
		return RETCODE::PAGEUNLOCKNED;
	}

	_lockMap[ptr] -= 1;

	return RETCODE::COMPLETE;
}

inline RETCODE BufferManager::FlushPages (const char * file) {
	RETCODE result;
	vector<PagePtr> vec;

	_bufferTbl.FindAll (file, vec);

	PageFile fileHandle (file);

	if ( (result = fileHandle.Open ( ) ) != RETCODE::COMPLETE ) {
		return result;
	}

	for ( auto item : vec ) {
		if ( _lockMap[item] > 0 )
			return RETCODE::PAGELOCKNED;

		if( _dirtyMap[item] )
			fileHandle.AllocatePage (item);
	
	}

	return RETCODE::COMPLETE;
}
