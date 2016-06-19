#pragma once
/*
	1. 主要用于处理用户输入的所有命令

*/

#include "Utils.hpp"
#include "RecordFile.hpp"
#include "SystemManager.hpp"
#include "IndexManager.hpp"
#include "RecordFileManager.hpp"

#include <iostream>

class QueryManager {
public:
	QueryManager (const SystemManager & sysMgr, const IndexManager & indexMgr, const RecordFileManager & recMgr);
	~QueryManager ( );

	RETCODE Select (int           nSelAttrs,        // # attrs in Select clause
							   const RelAttr selAttrs[],       // attrs in Select clause
							   int           nRelations,       // # relations in From clause
							   const char * const relations[], // relations in From clause
							   int           nConditions,      // # conditions in Where clause
							   const Condition conditions[]);  // conditions in Where clause
	RETCODE Insert (const char  *relName,           // relation to insert into
							   int         nValues,            // # values to insert
							   const Value values[]);          // values to insert
	RETCODE Delete (const char *relName,            // relation to delete from
							   int        nConditions,         // # conditions in Where clause
							   const Condition conditions[]);  // conditions in Where clause
	RETCODE Update (const char *relName,            // relation to update
								   const RelAttr &updAttr,         // attribute to update
								   const int bIsValue,             // 0/1 if RHS of = is attribute/value
								   const RelAttr &rhsRelAttr,      // attr on RHS of =
								   const Value &rhsValue,          // value on RHS of =
								   int   nConditions,              // # conditions in Where clause
								   const Condition conditions[]);  // conditions in Where clause

private:

	SystemManagerPtr sysMgr;

	IndexManagerPtr indexMgr;

	RecordFileManagerPtr recMgr;

};

using QueryManagerPtr = shared_ptr<QueryManager>;

inline QueryManager::QueryManager (const SystemManager & sysMgr, const IndexManager & indexMgr, const RecordFileManager & recMgr) {


}

QueryManager::~QueryManager ( ) {
}
