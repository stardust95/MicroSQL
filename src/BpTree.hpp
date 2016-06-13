#pragma once

/*
	1. 每个表的每个索引都用一棵B+树
	2. B+树的叶节点和内部结点使用不同的数据结构, 



*/

#include "Utils.hpp"
#include "RecordIdentifier.hpp"

#include <memory>
#include <algorithm>


//template<typename KeyType, typename ValueType, typename Comparator = std::less<KeyType>>
class BpTree {
public:

	using Comparator = int (*)( void*, void*, size_t);			// The type of compare function

	BpTree ( );

	BpTree (const BpTree &);
	
	~BpTree ( );

private:



};

using BpTreePtr = shared_ptr<BpTree>;


BpTree::BpTree ( ) {

}

inline BpTree::BpTree (const BpTree &) {
}

BpTree::~BpTree ( ) {

}
