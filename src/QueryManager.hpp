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

struct RelAttr {
	char *relName;     // relation name (may be NULL) 
	char *attrName;    // attribute name              
	friend std::ostream &operator<<(std::ostream &s, const RelAttr &ra);
};

struct Value {
	AttrType type;     // type of value               
	void     *data;    // value                       
	friend std::ostream &operator<<(std::ostream &s, const Value &v);
};

struct Condition {
	RelAttr lhsAttr;      // left-hand side attribute                     
	CompOp  op;           // comparison operator                          
	int     bRhsIsAttr;   // TRUE if right-hand side is an attribute
						  //   and not a value
	RelAttr rhsAttr;      // right-hand side attribute if bRhsIsAttr = TRUE
	Value   rhsValue;     // right-hand side value if bRhsIsAttr = FALSE
	friend std::ostream &operator<<(std::ostream &s, const Condition &c);
};

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

};

using QueryManagerPtr = shared_ptr<QueryManager>;

inline QueryManager::QueryManager (const SystemManager & sysMgr, const IndexManager & indexMgr, const RecordFileManager & recMgr) {
}

QueryManager::~QueryManager ( ) {
}
