#pragma once

/*
	1. 每个表的每个索引都用一棵B+树

*/

#include "Utils.hpp"

#include <memory>
#include <algorithm>

template<typename KeyType, typename ValueType, typename Comparator = std::less<KeyType>>
class BpTree {
public:

	struct Node {
		KeyType key;

	};

	BpTree ( );
	BpTree (const BpTree &);
	~BpTree ( );

	void Insert ( const KeyType & key, const ValueType & val ) {

	}



private:

};
template<typename KeyType, typename ValueType, typename Comparator>
using BpTreePtr = shared_ptr<BpTree<KeyType, ValueType, Comparator>>;

template<typename KeyType, typename ValueType, typename Comparator>
inline BpTree<KeyType, ValueType, Comparator>::BpTree ( ) {
}

template<typename KeyType, typename ValueType, typename Comparator>
inline BpTree<KeyType, ValueType, Comparator>::BpTree (const BpTree &) {
}
