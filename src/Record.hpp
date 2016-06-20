#pragma once
/*
	1. SlotNum, PageNum均开始自0
*/
#include "Utils.hpp"
#include "RecordIdentifier.hpp"

class Record {

	friend class RecordFile;

public:
	Record ( );
	Record (const RecordIdentifier & rid, char * ptr, size_t sz);
	~Record ( );

	RETCODE GetIdentifier (RecordIdentifier & id) const;

	RETCODE GetData (char * & pData) const;

	RETCODE GetSize (size_t & size) const;

private:
	
	RETCODE SetData (const RecordIdentifier & id, char *, size_t);

//private:

	RecordIdentifier _id;

	DataPtr _pData;					// points to an address in memory of _size

	size_t _size;

};

using RecordPtr = shared_ptr<Record>;

Record::Record () {
	_pData = nullptr;
	_id = UNKNOWNRID;
}

inline Record::Record (const RecordIdentifier & rid, char * ptr, size_t sz) {
	_id = rid;
	_size = sz;
	_pData = DataPtr (new char[_size] ( ));
	memcpy_s (_pData.get ( ), _size, ptr, _size);
}

Record::~Record ( ) {
	
}

inline RETCODE Record::GetIdentifier (RecordIdentifier & id) const {
	id = _id;
	return RETCODE::COMPLETE;
}

inline RETCODE Record::GetData (char * & pData) const {
	pData = _pData.get();
	return RETCODE::COMPLETE;
}

inline RETCODE Record::GetSize (size_t & size) const {
	size = _size;
	return RETCODE::COMPLETE;
}

inline RETCODE Record::SetData (const RecordIdentifier & rid, char * ptr, size_t sz) {
	_id = rid;
	_size = sz;
	_pData = DataPtr (new char[_size] ( ));
	memcpy_s (_pData.get ( ), _size, ptr, _size);

	return RETCODE::COMPLETE;
}
