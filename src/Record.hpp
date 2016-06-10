#pragma once
/*
	1. SlotNum, PageNum均开始自0
*/
#include "Utils.hpp"
#include "RecordIdentifier.hpp"

class Record {
public:
	Record ( );
	~Record ( );

	RETCODE GetIdentifier (RecordIdentifier & id) const;

	RETCODE GetData (DataPtr & pData) const;

	RETCODE GetSize (size_t & size) const;

	RETCODE SetData (const RecordIdentifier & id, char *, size_t);

private:

	RecordIdentifier _id;

	DataPtr _pData;					// points to an address in memory of _size

	size_t _size;

};

using RecordPtr = shared_ptr<Record>;

Record::Record ( ) {
}

Record::~Record ( ) {
}

inline RETCODE Record::GetIdentifier (RecordIdentifier & id) const {
	id = _id;
	return RETCODE::COMPLETE;
}

inline RETCODE Record::GetData (DataPtr & pData) const {
	pData = _pData;
	return RETCODE::COMPLETE;
}

inline RETCODE Record::GetSize (size_t & size) const {
	size = _size;
	return RETCODE::COMPLETE;
}

inline RETCODE Record::SetData (const RecordIdentifier & id, char * pData, size_t size) {
	_id = id;
	
	_pData = DataPtr(pData);

	_size = size;

	return RETCODE::COMPLETE;
}
