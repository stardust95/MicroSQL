#pragma once

/*
	1. QueryManager可以直接操作这个类的成员函数, 用于执行数据的更改
	2. 每个打开的RecordFile单独一个Buffer, 需要用到某个页面时先访问BufferManager要

*/

#include "Utils.hpp"
#include "Record.hpp"
#include "BufferManager.hpp"

class RecordFile {

public:

	struct RecordFileHeader {								// stored in the first page (PageNum = 0) of every data file
		char identifyString[Utils::IDENTIFYSTRINGLEN];			// "MicroSQL RecordFile", 32 bytes
		size_t recordSize;				// uint, 4 bytes
		size_t recordsPerPage;
		//PageNum	pageCount;			// ull, 8 bytes
		//PageNum firstFreePage;
		DataPtr bitMap;					// the length of bitmap = recordsPerPage; each record use 1 bit

		RecordFileHeader ( ) {
			strcpy_s (identifyString, Utils::RECORDFILEIDENTIFYSTRING);
			bitMap = nullptr;
		}

	};
	
	RecordFile ( );
	RecordFile (const PageFile &);
	RecordFile (const PageFilePtr &);
	~RecordFile ( );

	/*
	The file must be opened before any other operation
	*/
	RETCODE Open ( );

	RETCODE GetRec (const RecordIdentifier &rid, Record &rec) const;
	// Get a record
	RETCODE InsertRec (const char *pData, RecordIdentifier &rid);       // Insert a new record,
													  //   return record id
	RETCODE DeleteRec (const RecordIdentifier&rid);                    // Delete a record
	RETCODE UpdateRec (const Record &rec);              // Update a record
	RETCODE ForcePages (PageNum pageNum) const; // Write dirty page(s) to disk

	RETCODE ReadHeader ( );
	RETCODE GetHeader (RecordFileHeader & header) const;			// to write into the first page
	RETCODE GetPageFilePtr (PageFilePtr & ptr) const;

	bool isValidRecordFile ( ) const;

	/*
		Static Functions
	*/
	static size_t CalcRecordPerPage (size_t recordSize);
	static size_t CalcBitMapBytesPerPage (size_t recordPerPage);
	static RETCODE GetRecordPageAndSlot (const RecordIdentifier & id, PageNum & page, SlotNum & slot);		// call id.GetSlotNum() and id.GetPageNum()
	
	/*
		BitMap Functions
	*/
	RETCODE ChangeBit (size_t bit, bool val);
	RETCODE GetBit (size_t bit, bool & val);

private:

	PageNum pageCount ( ) const;

	RecordFileHeader _header;			// store in the first page, PageNum = 0

	BufferManagerPtr _bufMgr;

};

using RecordFilePtr = shared_ptr<RecordFile>;

RecordFile::RecordFile () {

}

inline RecordFile::RecordFile (const PageFile & pageFile) {
	//_pageFile = make_shared<PageFile> (pageFile);
	_bufMgr = make_shared<BufferManager> (pageFile);
}

inline RecordFile::RecordFile (const PageFilePtr & ptr) {
	//_pageFile = ptr;
	_bufMgr = make_shared<BufferManager> (ptr);
}

RecordFile::~RecordFile ( ) {

}

inline RETCODE RecordFile::GetRec (const RecordIdentifier & rid, Record & rec) const {
	RETCODE result;
	PageNum pageNum;
	SlotNum slotNum;

	// locate the page and slot by rid
	if ( (result = rid.GetPageNum (pageNum)) || (result = rid.GetSlotNum (slotNum)) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	//if ( pageNum > _header.pageCount || slotNum > _header.recordsPerPage )			// if the request file page is larger than amount
	//	return RETCODE::EOFFILE;

	// request the page from buffer
	PagePtr page;
	if ( result = _bufMgr->GetPage (pageNum, page) ) {		
		Utils::PrintRetcode (result);
		return result;
	}

	// get the page data
	DataPtr pData;
	if ( result = page->GetData (pData) ) {							
		Utils::PrintRetcode (result);
		return result;
	}

	// return the requested record data
	if ( result = rec.SetData (rid, pData.get ( ) + slotNum * _header.recordSize, _header.recordSize) ) {	
		Utils::PrintRetcode (result);
		return result;
	}

	return result;
}

inline RETCODE RecordFile::InsertRec (const char * pData, RecordIdentifier & rid) {
	RETCODE result;
	Record rec;

	

	return result;
}

inline RETCODE RecordFile::DeleteRec (const RecordIdentifier & rid) {
	RETCODE result;
	SlotNum slotNum;
	PageNum pageNum;

	if ( result = GetRecordPageAndSlot (rid, pageNum, slotNum) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	PagePtr page;
	
	if ( result = _bufMgr->GetPage(pageNum, page) ) {
		Utils::PrintRetcode (result);
		return result;
	}
	
	

	return result;
}

inline RETCODE RecordFile::UpdateRec (const Record & rec) {
	return RETCODE ( );
}

inline RETCODE RecordFile::ForcePages (PageNum pageNum) const {
	return RETCODE ( );
}

inline RETCODE RecordFile::ReadHeader ( ) {

	PagePtr page;
	DataPtr pData;
	RETCODE result;

	if ( ( result = _bufMgr->GetPage(0, page) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	if ( ( page->GetData (pData) ) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	size_t sizeHeaderNoBitMap = sizeof (RecordFileHeader) - sizeof (RecordFileHeader::bitMap);

	memcpy_s (reinterpret_cast<void*>( &_header ), sizeHeaderNoBitMap, pData.get ( ), sizeHeaderNoBitMap);

	size_t sizeBitMap = RecordFile::CalcBitMapBytesPerPage (_header.recordsPerPage);

	_header.bitMap = shared_ptr<char> ( new char[sizeBitMap]() );

	memcpy_s (reinterpret_cast<void*>(_header.bitMap.get ( )), sizeBitMap, pData.get ( ) + sizeHeaderNoBitMap, sizeBitMap);

	if ( strcmp (_header.identifyString, Utils::PAGEFILEIDENTIFYSTRING) != 0 )
		return RETCODE::INVALIDNAME;

	return RETCODE::COMPLETE;
}

inline RETCODE RecordFile::GetHeader (RecordFileHeader & header) const {
	header = _header;
	return RETCODE::COMPLETE;
}

inline RETCODE RecordFile::GetPageFilePtr (PageFilePtr & ptr) const {
	RETCODE result;
	if ( result = _bufMgr->GetPageFilePtr (ptr) ) {
		Utils::PrintRetcode (result);
		return result;
	}

	return RETCODE::COMPLETE;
}

inline bool RecordFile::isValidRecordFile ( ) const {
	return strcmp (_header.identifyString, Utils::RECORDFILEIDENTIFYSTRING) == 0;
}

inline size_t RecordFile::CalcRecordPerPage (size_t recordSize) {
	return Utils::PAGESIZE / recordSize;
}

inline size_t RecordFile::CalcBitMapBytesPerPage (size_t recordPerPage) {
	return recordPerPage / 8 + ( recordPerPage%8 != 0 );
}

inline RETCODE RecordFile::GetRecordPageAndSlot (const RecordIdentifier & id, PageNum & page, SlotNum & slot) {
	RETCODE result;

	if ( (result = id.GetPageNum (page)) || (result = id.GetSlotNum (slot)) ) {
		return result;
	}

}

inline RETCODE RecordFile::ChangeBit (size_t bit, bool val) {
	if ( val ) {
		*( _header.bitMap.get ( ) + bit / 8 ) |= ( 0x80 >> ( bit % 8 ) );
	} else {
		*( _header.bitMap.get ( ) + bit / 8 ) &= ~( 0x80 >> ( bit % 8 ) );
	}
	
	return RETCODE::COMPLETE;
}

inline RETCODE RecordFile::GetBit (size_t bit, bool & val) {
	
	val = (( *( _header.bitMap.get ( ) + bit / 8 ) & ( 0x80 >> ( bit % 8 ) ) ) >> ( 7 - ( bit % 8 ) )) == 1;

	return RETCODE::COMPLETE;
}
