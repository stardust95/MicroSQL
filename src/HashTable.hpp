#pragma once

#include "Utils.hpp"
#include "PageFile.hpp"

#include <unordered_map>



class HashTable {
public:

	using PageMap = std::unordered_map<PageNum, PagePtr>;

	RETCODE Find ( PageNum page, PagePtr &) const;

	RETCODE Insert ( PageNum page, PagePtr &);

	RETCODE Delete ( PageNum page);

	RETCODE Keys (vector<PageNum> & vec) const;

private:

	PageMap::const_iterator find ( PageNum page) const;

	PageMap _bufPages;

};

RETCODE HashTable::Find ( PageNum page, PagePtr & ptr) const {
	auto result = this->find (page);
	if ( result != _bufPages.end ( ) ) {		// has found in table
		ptr = result->second;
		return RETCODE::COMPLETE;
	}
	return RETCODE::HASHNOTFOUND;
}

RETCODE HashTable::Insert ( PageNum page, PagePtr & ptr) {
	
	if ( this->find (page) != _bufPages.end ( ) )
		return RETCODE::HASHPAGEEXIST;
	
	_bufPages.insert ({ page, ptr });

	return RETCODE::COMPLETE;
}

inline RETCODE HashTable::Delete ( PageNum page) {
	auto it = find (page);
	
	if( it == _bufPages.end() )
		return RETCODE::HASHNOTFOUND;

	_bufPages.erase (it);

	return RETCODE::COMPLETE;
}

inline RETCODE HashTable::Keys (vector<PageNum> & vec) const {
	
	for ( auto item : _bufPages ) {
		vec.push_back (item.first);
	}

	return RETCODE::COMPLETE;
}

inline HashTable::PageMap::const_iterator HashTable::find ( PageNum page) const {
	return _bufPages.find(page);
}
