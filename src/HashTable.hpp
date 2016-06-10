#pragma once

#include "Utils.hpp"
#include "Utils.hpp"
#include "PageFile.hpp"

#include <unordered_map>


using PageMap = std::unordered_multimap<PageNum, PagePtr>;

class HashTable {
public:

	RETCODE Find (const string & file, PageNum page, PagePtr &) const;

	RETCODE FindAll (const string & file, vector<PagePtr> & vec) const;

	RETCODE Insert (const string & file, PageNum page, PagePtr &);

	RETCODE Delete (const string & file, PageNum page);

private:

	PageMap::const_iterator _find (const string & file, PageNum page) const;

	PageMap _bufPages;

};

RETCODE HashTable::Find (const string & file, PageNum page, PagePtr & ptr) const {
	auto result = this->_find (file, page);
	if ( result != _bufPages.end ( ) ) {
		ptr = result->second;
		return RETCODE::COMPLETE;
	}
	return RETCODE::HASHNOTFOUND;
}

RETCODE HashTable::Insert (const string & file, PageNum page, PagePtr & ptr) {
	
	if ( this->_find (file, page) == _bufPages.end ( ) )
		return RETCODE::HASHPAGEEXIST;
	
	ptr = make_shared<Page> ( );


	return RETCODE::COMPLETE;
}

inline RETCODE HashTable::Delete (const string & file, PageNum page) {
	auto range = _bufPages.equal_range (page);
	string tmp ;
	for ( auto it = range.first; it != range.second; it++ )
		if ( it->second != nullptr && it->second->GetFileName (tmp) == RETCODE::COMPLETE && tmp == file ) {
			return RETCODE::COMPLETE;
		}

	return RETCODE::HASHNOTFOUND;
}

inline PageMap::const_iterator HashTable::_find (const string & file, PageNum page) const {
	auto range = _bufPages.equal_range (page);
	string tmp;
	for ( auto it = range.first; it != range.second; it++ )
		if ( it->second != nullptr && 
			it->second->GetFileName (tmp) == RETCODE::COMPLETE && tmp == file ) {
			return it;
		}
	return _bufPages.end();
}
