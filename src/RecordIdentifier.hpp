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

	friend bool operator == (const RecordIdentifier & lhs, const RecordIdentifier & rhs) {
		return lhs._pageNum == rhs._pageNum && lhs._slotNum == rhs._slotNum;
	}

	friend std::ostream & operator << (std::ostream & out, const RecordIdentifier & rid) {
		out << "(" << rid._pageNum << "," << rid._slotNum << ")";
		return out;
	}

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
