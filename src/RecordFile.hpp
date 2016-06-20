#pragma once

/*
	1. QueryManager可以直接操作这个类的成员函数, 用于执行数据的更改
	2. 每个打开的RecordFile单独一个Buffer, 需要用到某个页面时先访问BufferManager要
	3. 每个PageFile的第一个Page(PageNum=0)存PageFileHeader, 第二个Page(PageNum=1)存RecordFileHeaedr
	4. PageFile的每个普通的Page(PageNum>1)除了存PageHeader, 还要存一个RecordPageHeader用于记录一个页的信息
	5. Bitmap不需要作为Header或File的成员, 只需要用来解析一段Buffer
	6. 通过ReadHeader读取文件中的Header信息, 通过SaveHeader把当前内存中的Header存到文件中
*/

#include "Utils.hpp"
#include "Bitmap.hpp"
#include "Record.hpp"
#include "BufferManager.hpp"

struct RecordFileHeader {								// stored in the first page (PageNum = 0) of every data file
	char identifyString[Utils::IDENTIFYSTRINGLEN];			// "MicroSQL RecordFile", 32 bytes
	size_t recordSize;				// uint, 4 bytes, the total number of records
	size_t recordsPerPage;
	PageNum	numPages;			// ull, 8 bytes
	PageNum firstFreePage;
	RecordFileHeader ( ) {
		memset (identifyString, 0, sizeof ( identifyString ));
		strcpy_s (identifyString, Utils::RECORDFILEIDENTIFYSTRING);
		recordSize = recordsPerPage = 0;
	}

};

struct RecordPageHeader {
	PageNum nextFree;       // nextFree can be any of these values:
						//  - the number of the next free page
						//  - RM_PAGE_LIST_END if this is last free page
						//  - RM_PAGE_FULLY_USED if the page is not free
	DataPtr freeSlotMap; // A bitmap that tracks the free slots within 
						// the page
	SlotNum numSlots;
	SlotNum numFreeSlots;

	RecordPageHeader (int numSlots) : numSlots (numSlots), numFreeSlots (numSlots) {
		freeSlotMap = DataPtr(new char[this->mapsize ( )]());
	}

	~RecordPageHeader ( ) {

	}

	char * getFreeSlotMap ( ) const {
		return freeSlotMap.get ( );
	}

	int size ( ) const {
		return sizeof (nextFree) + sizeof (numSlots) + sizeof (numFreeSlots)
			+ Bitmap (numSlots).numChars ( )*sizeof (char);
	}
	int mapsize ( ) const {
		return this->size ( ) - sizeof (nextFree)
			- sizeof (numSlots) - sizeof (numFreeSlots);
	}
	int to_buf (char *& buf) const {
		memcpy (buf, &nextFree, sizeof (nextFree));
		memcpy (buf + sizeof (nextFree), &numSlots, sizeof (numSlots));
		memcpy (buf + sizeof (nextFree) + sizeof (numSlots),
						&numFreeSlots, sizeof (numFreeSlots));
		memcpy (buf + sizeof (nextFree) + sizeof (numSlots) + sizeof (numFreeSlots),
						freeSlotMap.get(), this->mapsize ( )*sizeof (char));
		return 0;
	}
	int from_buf (const char * buf) {
		memcpy (&nextFree, buf, sizeof (nextFree));
		memcpy (&numSlots, buf + sizeof (nextFree), sizeof (numSlots));
		memcpy (&numFreeSlots, buf + sizeof (nextFree) + sizeof (numSlots),
				sizeof (numFreeSlots));
		memcpy (freeSlotMap.get(),
				buf + sizeof (nextFree) + sizeof (numSlots) + sizeof (numFreeSlots),
				this->mapsize ( )*sizeof (char));
		return 0;
	}
};

class RecordFile {

public:

	
	RecordFile ( );
	~RecordFile ( );

	/*
	The file must be opened before any other operation
	*/
	RETCODE Open (const BufferManagerPtr & ptr );

	RETCODE InsertRec (const char *pData, RecordIdentifier &rid);       // Insert a new record, and return record id
	RETCODE DeleteRec (const RecordIdentifier&rid);                    // Delete a record
	RETCODE UpdateRec (const Record &rec);              // Update a record
	RETCODE GetRec (const RecordIdentifier &rid, Record &rec) const;

	RETCODE ForcePages (PageNum pageNum) const; // Write dirty page(s) to disk

	RETCODE GetPageHeader (PagePtr & page, RecordPageHeader & pHdr);
	RETCODE SetPageHeader (PagePtr & page, const RecordPageHeader & pHdr);

	RETCODE ReadHeader ( );
	RETCODE SaveHeader ( ) const;

	RETCODE GetHeader (RecordFileHeader & header) const;			// to write into the first page
	RETCODE GetPageFilePtr (PageFilePtr & ptr) const;

	bool isValidRecordFile ( ) const;

	/*
		Static Functions
	*/
	static RETCODE GetRecordPageAndSlot (const RecordIdentifier & id, PageNum & page, SlotNum & slot);		// call id.GetSlotNum() and id.GetPageNum()

	RETCODE GetNextFreeSlot (PagePtr & pagePtr, PageNum & page, SlotNum & slot) ;

	RETCODE GetNextFreePage (PageNum & page) ;

	bool IsValidRid (const RecordIdentifier & rid) const;

private:

	size_t recordsPerPage ( ) const;

	size_t recordSize ( ) const;

	size_t getOffsetBySlot (SlotNum slot) const;

	PageNum numPages ( ) const;

	SlotNum numSlots ( ) const;

	static const PageNum HEADERPAGE = 1;

	bool headerModified;

	bool isFileOpen;

	RecordFileHeader header;			// store in the first page, PageNum = 0

	BufferManagerPtr bufMgr;

};

using RecordFilePtr = shared_ptr<RecordFile>;

RecordFile::RecordFile () {
	bufMgr = nullptr;
	headerModified = false;
	isFileOpen = false;
}


RecordFile::~RecordFile ( ) {

	if ( headerModified ) {
		SaveHeader ( );
	}

}

inline RETCODE RecordFile::Open (const BufferManagerPtr & ptr) {

	RETCODE result = RETCODE::COMPLETE;

	if ( isFileOpen || bufMgr != nullptr ) {
		return RETCODE::FILEOPEN;
	}

	if ( ptr == nullptr ) {
		return RETCODE::INVALIDOPEN;
	}

	isFileOpen = true;
	bufMgr = ptr;
	headerModified = true;

	if ( result = ReadHeader ( ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;	
	}

	return result;
}

inline RETCODE RecordFile::GetRec (const RecordIdentifier & rid, Record & rec) const {
	RETCODE result;
	PageNum pageNum;
	SlotNum slotNum;

	// locate the page and slot by rid
	if ( (result = rid.GetPageNum (pageNum)) || (result = rid.GetSlotNum (slotNum)) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( pageNum > numPages() || slotNum > recordSize() )			// if the request file page is larger than amount
		return RETCODE::EOFFILE;

	// request the page from buffer
	PagePtr page;
	if ( result = bufMgr->GetPage (pageNum, page) ) {		
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	// get the page data
	char * pData;
	if ( result = page->GetData (pData) ) {							
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	// return the requested record data
	if ( result = rec.SetData (rid, pData + getOffsetBySlot(slotNum), header.recordSize) ) {	
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE RecordFile::InsertRec (const char * pData, RecordIdentifier & rid) {
	RETCODE result = RETCODE::COMPLETE;
	SlotNum slot;
	PageNum page;
	PagePtr pagePtr;
	RecordPageHeader pHdr (this->numSlots());

	if ( pData == nullptr ) {
		return RETCODE::BADRECORD;
	}

	if ( result = GetNextFreeSlot (pagePtr, page, slot) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( result = this->GetPageHeader (pagePtr, pHdr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	Bitmap bm (pHdr.getFreeSlotMap ( ), numSlots());

	char * pSlot = pagePtr->GetDataRawPtr ( ) + getOffsetBySlot (slot);

	rid = RecordIdentifier{ page, slot };

	memcpy_s (pSlot, recordSize ( ), pData, recordSize ( ));

	bm.reset (slot);
	pHdr.numFreeSlots--;

	if ( pHdr.numFreeSlots == 0 ) {
		header.firstFreePage = pHdr.nextFree;
		pHdr.nextFree = Utils::UNKNOWNPAGENUM;
	}

	bm.to_char_buf (pHdr.getFreeSlotMap ( ), bm.numChars ( ));

	if ( result = this->SetPageHeader (pagePtr, pHdr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE RecordFile::DeleteRec (const RecordIdentifier & rid) {
	RETCODE result;
	PageNum p;
	SlotNum s;

	if ( !this->IsValidRid (rid) )
		return RETCODE::BADRECORD;
	rid.GetPageNum (p);
	rid.GetSlotNum (s);

	PagePtr ph;
	RecordPageHeader pHdr (this->numSlots ( ));
	if ( ( result = bufMgr->GetPage (p, ph) ) ||
			( result = bufMgr->MarkDirty (p) ) ||
			( result = bufMgr->UnlockPage (p) ) ||		// Needs to be called every time GetThisPage is called.
			( result = this->GetPageHeader (ph, pHdr) )
		)
		return result;

	Bitmap b (pHdr.getFreeSlotMap(), this->numSlots ( ));

	if ( b.test (s) ) // already free
		return RETCODE::RECORDNOTFOUND;

	// TODO considering zero-ing record - IOs though
	b.set (s); // s is now free
	if ( pHdr.numFreeSlots == 0 ) {
		// this page used to be full and used to not be on the free list
		// add it to the free list now.
		pHdr.nextFree = header.firstFreePage;
		header.firstFreePage = p;
	}
	pHdr.numFreeSlots++;

	b.to_char_buf (pHdr.getFreeSlotMap(), b.numChars ( ));
	result = this->SetPageHeader (ph, pHdr);
	return result;
}

inline RETCODE RecordFile::UpdateRec (const Record & rec) {
	RecordIdentifier rid;
	rec.GetIdentifier (rid);
	PageNum p;
	SlotNum s;
	rid.GetPageNum (p);
	rid.GetSlotNum (s);

	if ( !this->IsValidRid (rid) )
		return RETCODE::BADRECORD;

	PagePtr ph;
	RETCODE result;

	RecordPageHeader pHdr (this->numSlots ( ));
	if ( ( result = bufMgr->GetPage (p, ph) ) ||
		( result = bufMgr->MarkDirty (p) ) ||
		( result = bufMgr->UnlockPage (p) ) ||
		( result = this->GetPageHeader (ph, pHdr) )
		)
		return result;

	Bitmap b (pHdr.getFreeSlotMap(), this->numSlots ( ));

	if ( b.test (s) ) // free - cannot update
		return RETCODE::RECORDNOTFOUND;

	char * pData;

	rec.GetData (pData);

	char * pSlot = pData + getOffsetBySlot(s);
	
	memcpy (pSlot, pData, this->recordSize ( ));

	return result;
}

inline RETCODE RecordFile::ForcePages (PageNum pageNum) const {
	return bufMgr->ForcePage(pageNum);
}

inline RETCODE RecordFile::GetPageHeader (PagePtr & page, RecordPageHeader & pHdr) {
	char * pData;
	RETCODE result = page->GetData (pData);
	pHdr.from_buf (pData);
	return result;
}

inline RETCODE RecordFile::SetPageHeader (PagePtr & page, const RecordPageHeader & pHdr) {
	char * pData;
	PageNum pageNum;
	RETCODE result;

	if ( (result = page->GetPageNum (pageNum)) || ( page->GetData (pData) )) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	pHdr.to_buf (pData);

	if ( result = bufMgr->ForcePage (pageNum) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE RecordFile::ReadHeader ( ) {

	PagePtr page;				// Header Page
	char* pData;
	RETCODE result;

	if ( ( result = bufMgr->GetPage(HEADERPAGE, page) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( ( page->GetData (pData) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	memcpy_s (reinterpret_cast<void*>( &header ), sizeof(RecordFileHeader), pData, sizeof(RecordFileHeader));

	if ( strcmp (header.identifyString, Utils::RECORDFILEIDENTIFYSTRING) != 0 )
		return RETCODE::INVALIDRECORDFILE;

	return RETCODE::COMPLETE;
}

inline RETCODE RecordFile::SaveHeader ( ) const {

	PagePtr rootpage;
	char * pData;
	RETCODE result = RETCODE::COMPLETE;

	if ( bufMgr == nullptr ) {
		Utils::PrintRetcode (RETCODE::HDRWRITE, __FUNCTION__, __LINE__);
	}

	if ( result = bufMgr->GetPage (HEADERPAGE, rootpage) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
	}

	if ( result = rootpage->GetData (pData) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
	}

	memcpy_s (pData, sizeof (RecordFileHeader), reinterpret_cast< const void * >( &header ), sizeof (RecordFileHeader));

	if ( result = bufMgr->ForcePage (HEADERPAGE) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
	}

	return result;
}

inline RETCODE RecordFile::GetHeader (RecordFileHeader & header) const {
	header = header;
	return RETCODE::COMPLETE;
}

inline RETCODE RecordFile::GetPageFilePtr (PageFilePtr & ptr) const {
	RETCODE result;
	if ( result = bufMgr->GetPageFilePtr (ptr) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return RETCODE::COMPLETE;
}

inline bool RecordFile::isValidRecordFile ( ) const {
	return strcmp (header.identifyString, Utils::RECORDFILEIDENTIFYSTRING) == 0;
}


inline RETCODE RecordFile::GetRecordPageAndSlot (const RecordIdentifier & id, PageNum & page, SlotNum & slot) {
	RETCODE result;

	if ( (result = id.GetPageNum (page)) || (result = id.GetSlotNum (slot)) ) {
		return result;
	}

}

inline RETCODE RecordFile::GetNextFreeSlot (PagePtr & pagePtr, PageNum & page, SlotNum & slot) {

	RETCODE result;

	RecordPageHeader pHdr (this->numSlots());

	if ( ( result = GetNextFreePage (page) ) || ( result = bufMgr->GetPage (page, pagePtr) )
		 || (result = bufMgr->UnlockPage(page) ) || ( result = this->GetPageHeader(pagePtr, pHdr)) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	Bitmap bm (pHdr.getFreeSlotMap(), numSlots());

	for ( size_t i = 0; i < this->numSlots(); i++ ) {
		if ( bm.test (i) ) {
			slot = i;
			return RETCODE::COMPLETE;
		}
	}

	return RETCODE::NODEKEYSFULL;
}

inline RETCODE RecordFile::GetNextFreePage (PageNum & pageNum) {
	RETCODE result;
	PagePtr ph;
	RecordPageHeader pHdr (this->numSlots ( ));
	PageNum p = Utils::UNKNOWNPAGENUM;

	if ( header.firstFreePage != Utils::UNKNOWNPAGENUM ) {
		// this last page on the free list might actually be full
		if ( ( result = bufMgr->GetPage (header.firstFreePage, ph) )
			|| ( result = ph->GetPageNum (p) )
			|| ( result = bufMgr->MarkDirty (p) )
			// Needs to be called everytime GetPage is called.
			|| ( result = bufMgr->UnlockPage (header.firstFreePage) )
			|| ( result = this->GetPageHeader (ph, pHdr) ) )
			return result;
		this->GetPageHeader (ph, pHdr);
	}

	if ( //we need to allocate a new page
		 // because this is the firs time
		header.numPages == 0 ||
		header.firstFreePage == Utils::UNKNOWNPAGENUM ||
		// or due to a full page
		//      (pHdr.numFreeSlots == 0 && pHdr.nextFree == RM_PAGE_FULLY_USED)
		( pHdr.numFreeSlots == 0 )
		) {

		if ( pHdr.nextFree == Utils::UNKNOWNPAGENUM) {
			// std::cerr << "RM_FileHandle::GetNextFreePage - Page Full!" << endl;
		}
		
		{
			char * pData;
			if ( ( result = bufMgr->AllocatePage (ph) ) || ( result = ph->GetData (pData) )
				|| ( result = ph->GetPageNum (pageNum) ) ) {
				Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
				return result;
			}

			RecordPageHeader phdr (this->numSlots ( ));
			phdr.nextFree = Utils::UNKNOWNPAGENUM;
			Bitmap b (this->numSlots ( ));
			b.set ( );
			b.to_char_buf (phdr.getFreeSlotMap ( ), b.numChars ( ));
			phdr.to_buf (pData);

		}

		// add page to the free list
		header.firstFreePage = pageNum;
		header.numPages++;
		assert (header.numPages > 1); // page num 1 would be header page
								   // std::cerr << "RM_FileHandle::GetNextFreePage hdr.numPages is " 
								   //           << hdr.numPages 
								   //           << " method " << this->GetNumPages()
								   //           << endl;
		headerModified = true;
		return RETCODE::COMPLETE; // pageNum is set correctly
	}
	// return existing free page
	pageNum = header.firstFreePage;

	return RETCODE::COMPLETE;
}

inline bool RecordFile::IsValidRid (const RecordIdentifier & rid) const{
	PageNum page;
	SlotNum slot;
	rid.GetPageNum (page);
	rid.GetSlotNum (slot);
	return page <= numPages ( ) && slot <= numSlots ( );

}


inline size_t RecordFile::recordsPerPage ( ) const {
	return header.recordsPerPage;
}

inline size_t RecordFile::recordSize ( ) const {
	return header.recordSize;
}

inline size_t RecordFile::getOffsetBySlot (SlotNum slot) const {
	return sizeof (RecordFileHeader) + static_cast<size_t>( header.recordSize * slot) ;
}

inline PageNum RecordFile::numPages ( ) const {
	return header.numPages;
}

inline SlotNum RecordFile::numSlots ( ) const {
	assert (recordSize ( ) != 0);

	return (Utils::PAGESIZE - sizeof (RecordPageHeader)) / recordSize();
}

