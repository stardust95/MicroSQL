#pragma once

/*
	1. 用一个数据结构同时表示叶节点和内部结点, 区别仅在于rids
	对于叶节点, rids即定位一个Record的Page和Slot
	对于内部结点, rids.page定位子节点的PageNum
	2. 对于B+树, 除了根之外每个结点(内部/叶)包含[floor(m/2), m-1]个key
	每个结点最多有m个child指针(总是比key个数多1)
	3. 在析构时要把数据复制到pagePtr的pData

*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"

#include <list>

class BpTreeNode;

using BpTreeNodePtr = shared_ptr<BpTreeNode>;

struct BpTreeNodeHeader {

	char identifyChar;				// 'I' for internal node, 'L' for leaf node

	size_t maxKeys;					// the order of bptree
	size_t numKeys;

	PageNum parentNode;

	PageNum page;

	PageNum prevNode;		// only for leaf nodes
	PageNum nextNode;

	AttrType type;

	size_t attrLen;

	BpTreeNodeHeader() {
		identifyChar = 'B';
		attrLen = maxKeys = numKeys = 0 ;
		parentNode = page = prevNode = nextNode = Utils::UNKNOWNPAGENUM;
	}

};

class BpTreeNode {
	
	friend class IndexHandle;

public:


	using Comparator = int (*) ( void *, void *, size_t );

	BpTreeNode (AttrType , size_t , PagePtr, bool );

	~BpTreeNode ( );

	RETCODE Insert (void* newKey, const RecordIdentifier & rid);

	RETCODE Delete (void * newKey);
	RETCODE Delete (size_t pos);

	//RETCODE Delete (size_t slot);

	/* 
		return COMPLETE if the key is in the node
		if the key is duplicated, return the right most keyPos
		if the rid is specified, only return COMPLETE when key and rid both matched
		return KEYNOTEXIST if cannot find a match key
	*/

	PageNum GetRight ( ) const;
	RETCODE SetRight (PageNum);
	PageNum GetLeft ( ) const;
	RETCODE SetLeft (PageNum);


	RETCODE FindKey (void * key, const RecordIdentifier & rid, size_t & keyPos) const;
	size_t FindKey (void * key, const RecordIdentifier & rid = INVALIDRID) const;
	size_t FindKeyPosFit (void * key) const;
		
	void * LargestKey ( ) const;
	void * SmallestKey ( ) const;

	RETCODE SetKey (size_t keyPos, void * key);
	RETCODE GetKey (size_t keyPos, void* & key) const;
	void * GetKey (size_t keyPos) const;
	
	RecordIdentifier GetRid (size_t keyPos) const;
	RecordIdentifier GetRid (void * key) const;
	RETCODE SetRid (size_t keyPos, const RecordIdentifier &) const;

	RETCODE Split (BpTreeNode & rhs);
	RETCODE Merge (const BpTreeNode & rhs);

	RETCODE Print ( ) const;

	size_t GetMaxKeys ( ) const;
	void SetMaxKeys (size_t val );

	size_t GetNumKeys ( ) const;
	void SetNumKeys (size_t val );

	void SetPrev (PageNum );
	PageNum GetPrev ( )const;

	void SetNext (PageNum);
	PageNum GetNext ( ) const;

	void SetParent (PageNum);
	PageNum GetParent ( ) const;

	PageNum GetPageNum ( ) const;
	RecordIdentifier GetPageRid ( ) const;

	bool IsSorted ( ) const;

	bool IsLeaf ( ) const;

	RETCODE CopyKeyTo (size_t pos, void * dst) const;

	int comp (void *, void *) const;

private:

	size_t attrLen ( )const;

	void * keyAt (size_t) const;

	RecordIdentifier ridAt (size_t) const;

	RETCODE writePage ( ) const;

	Comparator _comp;

	/*
		Contains only 4 members
	*/

	BpTreeNodeHeader header;

	void * keys;							// the array of record identifiers (value); 
												// for internal nodes, rids[i].page is a pointer to the i-th children, rids[i].slot = 0
												// for leave nodes, rid[i].page and rid[i].slot indicates the record whose target attribute is to keys[i]

	RecordIdentifier * rids;

	PagePtr pagePtr;			// should not write into page

};

/*
	if newNode is true, write the data to the page
	if newNode is false, read header from the page
*/
BpTreeNode::BpTreeNode (AttrType _type, size_t _attrLen, PagePtr  _page, bool newNode) {
	char* pData;
	RETCODE result;

	if ( result = _page->GetData (pData) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return;
	}

	pagePtr = _page;
	
	pagePtr->GetPageNum (header.page);

	if ( newNode ) {

		header.type = _type;

		header.attrLen = _attrLen;

		switch ( header.type ) {
		case INT:
			_comp = CompMethod::compare_int;
			break;
		case FLOAT:
			_comp = CompMethod::compare_float;
			break;
		case STRING:
			_comp = CompMethod::compare_string;
			break;
		default:
			_comp = nullptr;
			break;
		}

		memcpy_s (pData, sizeof (BpTreeNodeHeader), reinterpret_cast< const void* >( &header ), sizeof (BpTreeNodeHeader));

	} else {		// is node new node, read the header info from file

		memcpy_s (reinterpret_cast< void* >( &header ), sizeof (BpTreeNodeHeader), pData, sizeof (BpTreeNodeHeader));

	}

	keys = pData+sizeof(header);

	pData += sizeof (header);

	rids = reinterpret_cast< RecordIdentifier *>( pData + attrLen ( ) * header.numKeys );

	return;
}

inline BpTreeNode::~BpTreeNode ( ) {
	if ( this->pagePtr != nullptr ) {
		this->writePage ( );
	} else {
		Utils::PrintRetcode (RETCODE::INCOMPLETEWRITE, __FUNCTION__, __LINE__);
	}
}

inline RETCODE BpTreeNode::Insert (void * newKey, const RecordIdentifier & rid) {

	if ( GetNumKeys ( ) + 1 >= GetMaxKeys ( ) ) {
		return RETCODE::NODEKEYSFULL;
	}

	size_t i;

	for ( i = this->GetNumKeys()-1; i >=0 ; --i ) {
		if ( comp (newKey, keyAt(i) ) >= 0 ) {
			break;
		}
		SetKey (i + 1, GetKey (i));
		rids[i + 1] = rids[i];
	}

	SetKey (i, newKey);
	rids[i] = rid;

	SetNumKeys (GetNumKeys ( ) + 1);

	
	return RETCODE::COMPLETE;
}

inline RETCODE BpTreeNode::Delete ( void * key) {

	if ( key == nullptr )
		return RETCODE::BADKEY;

	for ( size_t i = 0; i < this->GetNumKeys ( ); ++i ) {
		if ( comp (keyAt (i), key) == 0 ) {
			
			for ( size_t j = i; j < this->GetNumKeys ( ) - 1; j++ ) {
				SetKey (j, GetKey (j + 1));
				rids[j] = rids[j + 1];
			}

			SetNumKeys (GetNumKeys ( ) - 1);
			return RETCODE::COMPLETE;
		}
	}

	return RETCODE::KEYNOTFOUND;

}

inline RETCODE BpTreeNode::Delete (size_t pos) {
	return this->Delete(keyAt(pos));
}

inline bool BpTreeNode::IsSorted ( ) const {

	for ( size_t i = 1; i < this->GetNumKeys ( ); ++i ) {
		if ( comp (keyAt(i-1) , keyAt(i)) > 0 ) {		// if is not increasing order
			return false;
		}
	}

	return true;
}

bool BpTreeNode::IsLeaf ( ) const {
	return header.identifyChar == 'L';
}

inline RETCODE BpTreeNode::CopyKeyTo (size_t pos, void * dst) const {

	if ( pos >= GetNumKeys ( ) ) {
		return RETCODE::OUTOFRANGE;
	}

	if ( dst == nullptr )
		return RETCODE::BADKEY;

	memcpy_s (dst, attrLen ( ), keyAt (pos), attrLen ( ));

	return RETCODE::COMPLETE;
}


inline int BpTreeNode::comp (void * p1, void * p2) const {
	assert (p1 != nullptr && p2 != nullptr);

	return _comp(p1, p2, attrLen());
}

inline size_t BpTreeNode::attrLen ( ) const {
	return header.attrLen;
}

inline void * BpTreeNode::keyAt (size_t i) const {
	if ( i < this->GetNumKeys ( ) )
		return reinterpret_cast< char* >( keys) + i*attrLen ( );
		//return reinterpret_cast< char* >( keys.get() ) + i*attrLen ( );
	return nullptr;
}

inline RecordIdentifier BpTreeNode::ridAt (size_t i) const {
	
	if ( i < this->GetNumKeys ( ) )
		return rids[i];

	return INVALIDRID;
}

inline RETCODE BpTreeNode::writePage ( ) const {
	RETCODE result = RETCODE::COMPLETE;
	char * pData;

	if ( result = pagePtr->GetData (pData) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	memcpy_s (pData, sizeof (BpTreeNodeHeader), reinterpret_cast<const void* >( &header ), sizeof (BpTreeNodeHeader));

	pData += sizeof (BpTreeNodeHeader);

	size_t keySize = GetNumKeys ( ) * attrLen ( );

	memcpy_s (pData, keySize, keys, keySize);

	pData += GetNumKeys ( ) * attrLen ( );

	size_t ridSize = sizeof (GetNumKeys ( ) * sizeof (RecordIdentifier));

	memcpy_s (pData, ridSize, rids, ridSize);
	
	return result;
}

inline PageNum BpTreeNode::GetRight ( ) const {
	return header.nextNode;
}

inline RETCODE BpTreeNode::SetRight (PageNum page) {
	header.nextNode = page;
	return RETCODE::COMPLETE;
}

inline PageNum BpTreeNode::GetLeft ( ) const {
	return header.prevNode;
}

inline RETCODE BpTreeNode::SetLeft (PageNum page) {
	header.prevNode = page;
	
	return RETCODE::COMPLETE;
}

RETCODE BpTreeNode::FindKey (void * key, const RecordIdentifier & rid, size_t & keyPos) const {
	
	keyPos = Utils::UNKNOWNPOS;
	
	for ( size_t i = 0; i < this->GetNumKeys ( ); ++i ) {
		if ( comp (key, keyAt(i) ) == 0 && (rid == INVALIDRID || rid == ridAt(i)) ) {	// if rid is INVALIDRID, only compare the rid
			keyPos = i;
		}
	}
	
	return RETCODE::COMPLETE;
}

inline size_t BpTreeNode::FindKey (void * key, const RecordIdentifier & rid) const{

	for ( size_t i = 0; i < GetNumKeys ( ); i++ ) {
		if ( comp (keyAt (i), key) == 0 && ( rid == INVALIDRID || rid == ridAt (i) ) )
			return i;
	}
	return Utils::UNKNOWNPOS;
}

/*
	return position if key will fit in a particular position
	return INVALIDPOS if there was an error
	if there are dups - this will return rightmost position
*/
inline size_t BpTreeNode::FindKeyPosFit (void * key) const {
	for ( size_t i = GetNumKeys()-1; i >= 0 ; --i ){ 
		
		if ( keyAt (i) == nullptr )
			return Utils::UNKNOWNPOS;

		if ( comp (keyAt (i), key) == 0 )
			return i;
		else if ( comp (keyAt (i), key) < 0 )
			return i + 1;
	}
	return 0;
}

inline void * BpTreeNode::LargestKey ( ) const {
	if ( GetNumKeys ( ) == 0 )
		return nullptr;
	else
		return keyAt (GetNumKeys ( ) - 1);
}

inline void * BpTreeNode::SmallestKey ( ) const {
	if ( GetNumKeys ( ) == 0 )
		return nullptr;
	else
		return keyAt (0);
}

inline RETCODE BpTreeNode::GetKey (size_t keyPos, void * & key) const {

	if ( keyPos < this->GetNumKeys() ) {
		key = keyAt (keyPos);
		return RETCODE::COMPLETE;
	}

	return RETCODE::KEYNOTFOUND;
}

inline void * BpTreeNode::GetKey (size_t keyPos) const {
	if ( keyPos < this->GetNumKeys ( ) ) 
		return keyAt (keyPos);
	return nullptr;
}

inline RETCODE BpTreeNode::SetKey (size_t keyPos, void * key) {
	if ( keyPos < GetMaxKeys ( ) ) {
		memcpy_s (reinterpret_cast<char*>( keys ) + keyPos * header.attrLen, attrLen ( ), key, attrLen ( ));
		//memcpy_s ( reinterpret_cast<char*>(keys.get()) + keyPos * header.attrLen, attrLen ( ), key, attrLen ( ));
		return RETCODE::COMPLETE;
	}
	return RETCODE::NODEKEYSFULL;
}

inline RecordIdentifier BpTreeNode::GetRid (size_t pos) const {
	if ( pos < this->GetNumKeys ( ) )
		return rids[pos];
	return INVALIDRID;
}

inline RecordIdentifier BpTreeNode::GetRid (void * key) const {
	size_t keypos = this->FindKey(key);

	if ( keypos == Utils::UNKNOWNPOS )
		return INVALIDRID;

	return ridAt (keypos);

}

inline RETCODE BpTreeNode::SetRid (size_t keyPos, const RecordIdentifier & rid) const {
	if ( keyPos < GetMaxKeys ( ) ){ 
		rids[keyPos] = rid;
		return RETCODE::COMPLETE;
	}
	return RETCODE::NODEKEYSFULL;
}


/*
	Split this node and insert half backward buckets to rhs

*/

inline RETCODE BpTreeNode::Split (BpTreeNode & rhs) {
	RETCODE result;
	size_t numKeys = this->GetNumKeys ( );
	size_t rhsKeys = rhs.GetNumKeys ( );

	size_t leftistMovePos = ( numKeys + 1 ) / 2;
	size_t moveCount = numKeys - leftistMovePos;

	if ( moveCount + rhsKeys > rhs.GetMaxKeys() )
		return NODEKEYSFULL;

	// move this[leftistMovePos:end] after the end of rhs
	for ( size_t i = leftistMovePos; i < this->GetNumKeys ( ); ++i ) {
		if ( result = rhs.Insert ( keyAt(i), ridAt(i) ) ) {				// node insert failed 
			return result;
		}
		
	}

	//remove the remaining elements
	for ( size_t i = leftistMovePos; i < this->GetNumKeys ( ); ++i ) {
		if ( result = this->Delete( this->keyAt(i) ) ) {
			return result;
		}

	}

	rhs.SetNext (this->GetNext ( ));
	rhs.SetPrev (this->GetPageNum ( ));
	this->SetNext (rhs.GetPageNum ( ));

	return RETCODE::COMPLETE;
}

/*
	Merge this node with other node(move the buckets of rhs to this node)
	rhs must be a neighbor node(left or right)
*/
inline RETCODE BpTreeNode::Merge (const BpTreeNode & rhs) {

	RETCODE result;
	size_t numKeys = this->GetNumKeys ( );
	size_t rhsKeys = rhs.GetNumKeys ( );

	if ( numKeys + rhsKeys > this->GetMaxKeys()	)		// if this node is no enough space
		return NODEKEYSFULL;

	// move the buckets in rhs to this node
	for ( size_t i = 0; i < rhs.GetNumKeys ( ); i++ ) {
		if ( result = this->Insert ( rhs.keyAt(i), rhs.ridAt(i) ) ) {
			return result;
		}
	}

	if ( this->GetPageNum ( ) == rhs.GetPrev ( ) ) {		// if this is the previous node of rhs
		this->SetNext (rhs.GetNext ( ));
	} else {
		this->SetPrev (rhs.GetPrev ( ));
	}

	return RETCODE::COMPLETE;
}

inline RETCODE BpTreeNode::Print ( ) const {

	if ( header.type == INT ) {
		for ( size_t i = 0; i < this->GetNumKeys ( ); ++i ) {
			std::cout << *reinterpret_cast< int* >( keyAt(i) );
			std::cout << ":" << ridAt(i) << std::endl;
		}

	} else if ( header.type == FLOAT ) {
		for ( size_t i = 0; i < this->GetNumKeys ( ); ++i ) {
			std::cout << *reinterpret_cast< float* >( keyAt (i) );
			std::cout << ":" << ridAt (i) << std::endl;
		}


	} else {

		for ( size_t i = 0; i < this->GetNumKeys ( ); ++i ) {
			char * strptr = reinterpret_cast<char*>( keyAt(i) );
			for ( size_t j = 0; j < header.attrLen; j++ ) {
				std::cout << strptr[j];			// output one char
			}
			std::cout << ":" << ridAt (i) << std::endl;
		}

	}

	return RETCODE::COMPLETE;
}

inline size_t BpTreeNode::GetMaxKeys ( ) const {
	return header.maxKeys;
}

inline void BpTreeNode::SetMaxKeys (size_t val ) {
	header.maxKeys = val;
}

inline size_t BpTreeNode::GetNumKeys ( ) const {
	return header.numKeys;
}

inline void BpTreeNode::SetNumKeys (size_t val) {
	header.numKeys = val;
}

inline void BpTreeNode::SetPrev (PageNum page) {
	this->header.prevNode = page;
}

inline PageNum BpTreeNode::GetPrev ( ) const {
	return this->header.prevNode;
}


inline void BpTreeNode::SetNext (PageNum page) {
	this->header.nextNode = page;
}

inline PageNum BpTreeNode::GetNext ( ) const {
	return this->header.nextNode;
}


inline void BpTreeNode::SetParent (PageNum page) {
	this->header.parentNode = page;
}

inline PageNum BpTreeNode::GetParent ( ) const {
	return this->header.parentNode;
}

inline PageNum BpTreeNode::GetPageNum ( ) const {
	return this->header.page;
}

inline RecordIdentifier BpTreeNode::GetPageRid ( ) const {
	return RecordIdentifier { header.page, Utils::UNKNOWNSLOTNUM };
}
