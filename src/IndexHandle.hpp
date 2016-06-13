#pragma once

/*
	1. IndexHandle用于操作(查询, 插入, 删除)一个索引, 与大多数数据库采用的索引相同, 当前其内部实现也为B+树
	2. 每个结点的大小为Utils::PAGESIZE, 存放的Key
	3. 一个文件当做一段连续的内存, 指向子节点的指针就是在文件中的偏移
	
*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"

struct IndexHeader {				// the information of every index
	AttrType attrType;
	
	size_t attrLength;

};

struct NodeHeader {

	char identifyChar;				// 'I' for internal node, 'L' for leaf node

	size_t KeysCount;

	PageNum parent;

};

struct InternalNode {

	NodeHeader header;

	VoidPtr keys;				// the array of keys

	PageNumPtr childs;		// the array of child nodes

};

struct LeafNode {

	NodeHeader header;

	VoidPtr keys;		// the array of keys

	RecordIdentifierPtr rids;		// the array of record identifiers (value)

	PageNum prevLeaf;

	PageNum nextLeaf;

};

class IndexHandle {
public:

	using Comparator = int (*) ( void *, void *, size_t);

	IndexHandle ( );

	~IndexHandle ( );
	
	RETCODE InsertEntry (void *pData, const RecordIdentifier & rid);  // Insert new index entry

	RETCODE DeleteEntry (void *pData, const RecordIdentifier & rid);  // Delete index entry
	
	RETCODE ForcePages ( );                             // Copy index to disk

private:

	/*
		B+Tree Functions
	*/

	/*
		B+Tree Members
	*/

	PageNum rootPage;

	/*
		IndexHandle Members
	*/

	IndexHeader _header;

	BufferManagerPtr _bufMgr;
	
	bool _headerModified;

	bool _isOpenHandle;

	

};

IndexHandle::IndexHandle ( ) {
}

IndexHandle::~IndexHandle ( ) {
}
