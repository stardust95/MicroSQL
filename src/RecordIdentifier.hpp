#pragma once

#include "Utils.hpp"


/*
	作为索引 key-value 中的value
*/

class RecordIdentifier {
public:
	RecordIdentifier ( );
	RecordIdentifier (PageNum, SlotNum);
	~RecordIdentifier ( );

	RETCODE GetPageNum (PageNum &) const;

	RETCODE GetSlotNum (SlotNum &) const;

private:

	PageNum _pageNum;

	SlotNum _slotNum;

};

using RecordIdentifierPtr = shared_ptr<RecordIdentifier>;

RecordIdentifier::RecordIdentifier ( ) {
}

inline RecordIdentifier::RecordIdentifier (PageNum page, SlotNum slot) {

	_pageNum = page;
	_slotNum = slot;
}

RecordIdentifier::~RecordIdentifier ( ) {
}

inline RETCODE RecordIdentifier::GetPageNum (PageNum & page) const {
	page = _pageNum;
	return RETCODE::COMPLETE;
}

inline RETCODE RecordIdentifier::GetSlotNum (SlotNum & slot) const {
	slot = _slotNum;
	return RETCODE::COMPLETE;
}
