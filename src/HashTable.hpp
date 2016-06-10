#pragma once

#include "Utils.hpp"
#include "Utils.hpp"
#include "PageFile.hpp"

#include <unordered_map>



class HashTable {
public:

	struct HashEntry {
		string file;
		PagePtr page;
	};

	using PageMap = std::unordered_multimap<PageNum, HashEntry>;

	RETCODE Find (const string & file, PageNum page, PagePtr &) const;

	RETCODE FindAll (const string & file, vector<PagePtr> & vec) const;

	RETCODE Insert (const string & file, PageNum page, PagePtr &);

	RETCODE Delete (const string & file, PageNum page);

private:

	PageMap::const_iterator find (const string & file, PageNum page) const;

	PageMap _bufPages;

};

RETCODE HashTable::Find (const string & file, PageNum page, PagePtr & ptr) const {
	auto result = this->find (file, page);
	if ( result != _bufPages.end ( ) ) {		// has found in table
		ptr = result->second.page;
		return RETCODE::COMPLETE;
	}
	return RETCODE::HASHNOTFOUND;
}

RETCODE HashTable::Insert (const string & file, PageNum page, PagePtr & ptr) {
	
	if ( this->find (file, page) == _bufPages.end ( ) )
		return RETCODE::HASHPAGEEXIST;
	
	_bufPages.insert ({ page, { file, ptr } });

	return RETCODE::COMPLETE;
}

inline RETCODE HashTable::Delete (const string & file, PageNum page) {
	auto it = find (file, page);
	
	if( it == _bufPages.end() )
		return RETCODE::HASHNOTFOUND;

	_bufPages.erase (it);

	return RETCODE::COMPLETE;
}

inline HashTable::PageMap::const_iterator HashTable::find (const string & file, PageNum page) const {
	auto range = _bufPages.equal_range (page);
	string tmp;
	for ( auto it = range.first; it != range.second; it++ )
		if ( it->second.page != nullptr && it->second.file == file ) {
			return it;
		}
	return _bufPages.end();
}
