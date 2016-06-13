#pragma once

/*
	1. 该类用于给定条件扫描一个RecordFile文件中的所有Records
	2. TODO: 维护一个PagePtr, 不用每次都申请新的PagePtr
*/

#include "Utils.hpp"
#include "RecordFile.hpp"

class RecordFileScan {
public:

	const static PageNum BeginPage = 1;

	enum ScanState {
		Close, Open, End
	};

	struct ScanInfo {

		size_t recordsCount;

		size_t recordsPerPage;

		PageNum scanedPage;

		SlotNum scanedSlot;

		ScanState state;

	};

	RecordFileScan ( );
	~RecordFileScan ( );

	RETCODE OpenScan (const RecordFilePtr &fileHandle,  // Initialize file scan
											  AttrType			attrType,
											  size_t				attrLength,
											  size_t				attrOffset,
											  CompOp        compOp,
											  void          *value);

	RETCODE GetNextRec (Record &rec);                  // Get next matching record

	RETCODE CloseScan ( );                                // Terminate file scan
		
private:
	
	using Comparator = bool (*)( void*, void*, AttrType, size_t );
	
	Comparator _comp;

	RecordFilePtr _recFile;

	PagePtr _curPage;
	
	AttrType _attrType;
	
	size_t _attrLength;

	size_t _attrOffset;

	 VoidPtr _attrValue;

	 ScanInfo _scanInfo;

};

RecordFileScan::RecordFileScan ( ) {
	_scanInfo.state = ScanState::Close;
	_scanInfo.recordsCount = 0;
	_scanInfo.scanedPage = 0;
	_scanInfo.scanedSlot = 0;
	
	_recFile = nullptr;
	_curPage = nullptr;

}

RecordFileScan::~RecordFileScan ( ) {
	
}

inline RETCODE RecordFileScan::OpenScan (const RecordFilePtr & fileHandle, AttrType attrType, size_t attrLength, size_t attrOffset, CompOp compOp, void * value) {
	
	if ( _scanInfo.state == Open )
		return RETCODE::INVALIDSCAN;

	_recFile = fileHandle;

	if ( fileHandle == nullptr || !fileHandle->isValidRecordFile ( ) )
		return RETCODE::INVALIDPAGEFILE;

	RecordFile::RecordFileHeader header;

	_recFile->GetHeader (header);

	if ( value != nullptr ) {			// has condition

		if ( attrType != AttrType::INT && attrType != AttrType::FLOAT && attrType != AttrType::STRING )
			return INVALIDSCAN;

		if ( attrOffset + attrLength > header.recordSize || attrOffset < 0 )
			return RETCODE::INVALIDSCAN;

		switch ( compOp ) {
		case EQ_OP:
			_comp = CompMethod::equal;
			break;
		case LT_OP:
			_comp = CompMethod::less_than;
			break;
		case GT_OP:
			_comp = CompMethod::greater_than;
			break;
		case LE_OP:
			_comp = CompMethod::less_than_or_eq_to;
			break;
		case GE_OP:
			_comp = CompMethod::greater_than_or_eq_to;
			break;
		case NE_OP:
			_comp = CompMethod::not_equal;
			break;
		case NO_OP:
			_comp = nullptr;
			break;
		default:
			return RETCODE::INVALIDSCAN;
			break;
		}

		_attrType = attrType;

		_attrLength = attrLength;

		_attrOffset = attrOffset;

		if ( ( attrType == AttrType::INT || attrType == AttrType::FLOAT ) && attrLength != 4 )
			return RETCODE::INVALIDSCAN;

		_attrValue = shared_ptr<void> (reinterpret_cast< void* >( new char[attrLength] ( ) ));

		memcpy_s (_attrValue.get ( ), attrLength, value, attrLength);

	} 

	// initialize the status
	_scanInfo.state = Open;
	_scanInfo.recordsCount = header.recordSize;
	_scanInfo.scanedPage = BeginPage;
	_scanInfo.scanedSlot = 0;
	
	_curPage = nullptr;

	return RETCODE::COMPLETE;
}

inline RETCODE RecordFileScan::GetNextRec (Record & rec) {

	if ( _scanInfo.state != ScanState::Open )
		return RETCODE::INVALIDSCAN;
	else if ( _scanInfo.state == End )
		return RETCODE::EOFSCAN;
	
	Record tmpRec;
	RETCODE result;
	DataPtr recData;

	for ( ;; ) {

		if ( result = _recFile->GetRec (RecordIdentifier{ _scanInfo.scanedPage, _scanInfo.scanedSlot }, tmpRec) ) {
			Utils::PrintRetcode (result);

			if ( result == RETCODE::EOFFILE ) {
				_scanInfo.state = End;
				return RETCODE::EOFSCAN;
			}
			return result;
		}

		if ( ++_scanInfo.scanedSlot == _scanInfo.recordsPerPage ) {
			_scanInfo.scanedPage++;
			_scanInfo.scanedSlot = 0;
		}

		if ( result = tmpRec.GetData (recData) ) {
			Utils::PrintRetcode (result);
			return result;
		}

		if ( _comp == nullptr || _comp (recData.get ( ), _attrValue.get(), _attrType, _attrLength) ) {	// if satisfies the condition
			rec = tmpRec;
			break;
		}

	}

	return RETCODE::COMPLETE;
}

inline RETCODE RecordFileScan::CloseScan ( ) {
	_scanInfo.state = ScanState::Close;

	return RETCODE::COMPLETE;
}
