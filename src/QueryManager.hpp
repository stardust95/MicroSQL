#pragma once
/*
	1. 主要用于处理用户输入的所有命令

*/

#include "Utils.hpp"
#include "Iterator.hpp"
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

	RETCODE IsValid ( ) const;
	// Choose between filescan and indexscan for first operation - leaf level of
	// operator tree
	// to see if NLIJ is possible, join condition is passed down
	Iterator* GetLeafIterator (const char *relName,
							   int nConditions,
							   const Condition conditions[],
							   int nJoinConditions = 0,
							   const Condition jconditions[] = NULL,
							   int order = 0,
							   RelAttr* porderAttr = NULL);

	RETCODE MakeRootIterator (Iterator*& newit,
						 int nSelAttrs,
						 int nRelations, const char * const relations[],
						 int order, RelAttr orderAttr,
						 bool group, RelAttr groupAttr) const;

	RETCODE PrintIterator (Iterator* it) const;

	void GetCondsForSingleRelation (int nConditions,
									Condition conditions[],
									char* relName,
									int& retCount, Condition*& retConds) const;

	// get conditions that involve both relations. intermediate relations are
	// possible from previous joins done so far - hence relsSoFar.
	void GetCondsForTwoRelations (int nConditions,
								  Condition conditions[],
								  int nRelsSoFar,
								  char* relations[],
								  char* relName2,
								  int& retCount, Condition*& retConds) const;


	SystemManagerPtr smm;

	IndexManagerPtr ixm;

	RecordFileManagerPtr rmm;

};

using QueryManagerPtr = shared_ptr<QueryManager>;

inline QueryManager::QueryManager (const SystemManager & sysMgr, const IndexManager & indexMgr, const RecordFileManager & recMgr) {


}

QueryManager::~QueryManager ( ) {
}

inline RETCODE QueryManager::Select (int nSelAttrs, const RelAttr selAttrs[], int nRelations, const char * const relations[], int nConditions, const Condition conditions[]) {
	
}

inline RETCODE QueryManager::Insert (const char * relName, int nValues, const Value values[]) {




	return RETCODE::COMPLETE;
}
