#pragma once
/*
	.1 DataAttrInfo记录的是数据库中一个关系的一个属性的信息, 包括该属性所属的关系(表), 偏移量, 属性类型, 属性长度
	2. DataRelInfo记录的是数据库中某个关系本身的信息, 包括一条记录的大小, 属性(列)的个数, 总页数, 总记录数
	3. 每个DB创建时先创建两张表relcat和attrcat记录以分别这个数据库的表和每个表的属性信息

*/
#include "Utils.hpp"
#include "IndexManager.hpp"
#include "RecordFileManager.hpp"
#include "PageFileManager.hpp"

#include <set>

class SystemManager {
public:
	SystemManager (const IndexManagerPtr & ixm, 
								const RecordFileManagerPtr & rm);

	~SystemManager ( );

	RETCODE CreateDb (const char * dbName, PageFileManagerPtr & pfMgr);
	RETCODE OpenDb (const char *dbName);                // Open database
	RETCODE CloseDb ( );                                  // Close database
	RETCODE CreateTable (const char *relName,                // Create relation
											int        attrCount,
											AttrInfo   *attributes);
	RETCODE DropTable (const char *relName);               // Destroy relation
	RETCODE CreateIndex (const char *relName,                // Create index
											const char *attrName);
	RETCODE DropIndex (const char *relName,                // Destroy index
											const char *attrName);
	RETCODE Load (const char *relName,                // Load utility
								const char *fileName);
	RETCODE Help ( );                                  // Help for database
	RETCODE Help (const char *relName);               // Help for relation
	RETCODE Print (const char *relName);               // Print relation
	RETCODE Set (const char *paramName,              // Set system parameter
			const char *value);

private:
	RETCODE IsValid ( ) const;

	// attributes is allocated and returned back with attrCount elements.
	// attrCount is returned back with number of attributes
	RETCODE GetFromTable (const char *relName,           // create relation relName
					 int&        attrCount,         // number of attributes
					 DataAttrInfo   *&attributes);  // attribute data

	// Get the first matching row for relName
	// contents are return in rel and the RecordIdentifier the record is located at is
	// returned in RecordIdentifier.
	// method returns RETCODE::TABLENOTFOUND if relName was not found
	RETCODE GetRelFromCat (const char* relName,
					  DataRelInfo& rel,
					  RecordIdentifier& RecordIdentifier) const;

	// Get the first matching row for relName, attrName
	// contents are returned in attr
	// location of record is returned in RecordIdentifier
	// method returns SM_NOSUCHENTRY if attrName was not found
	RETCODE GetAttrFromCat (const char* relName,
					   const char* attrName,
					   DataAttrInfo& attr,
					   RecordIdentifier& rid) const;

	RETCODE GetNumPages (const char* relName) const;
	RETCODE GetNumRecords (const char* relName) const;

	// Semantic checks for various parts of queries
	RETCODE SemCheck (const char* relName) const;
	RETCODE SemCheck (const RelAttr& ra) const;
	//RETCODE SemCheck (const AggRelAttr& ra) const;
	RETCODE SemCheck (const Condition& cond) const;
	// for NULL relname - implicit relation name
	// return error if there is a clash and multiple relations have this attrName
	// user must free() ra.relName eventually
	RETCODE FindRelForAttr (RelAttr& ra, int nRelations,
					   const char * const
					   possibleRelations[]) const;

	// Load a single record in the buf into the table relName
	RETCODE LoadRecord (const char *relName,
									   int buflen,
									   const char buf[]);

	bool IsAttrIndexed (const char* relName, const char* attrName) const;

	// temp operations on attrcat to make index appear to be missing
	RETCODE DropIndexFromAttrCatAlone (const char *relName,
																  const char *attrName);

	RETCODE ResetIndexFromAttrCatAlone (const char *relName,
																	const char *attrName);

private:

	RecordFilePtr relFile;

	RecordFilePtr attrFile;

	bool IsDBOpen;

	IndexManagerPtr indexMgr;

	RecordFileManagerPtr recMgr;

	std::map<std::string, std::string> config;

};

using SystemManagerPtr = shared_ptr<SystemManager>;

SystemManager::SystemManager (const IndexManagerPtr & ixm, const RecordFileManagerPtr & rm){

	indexMgr = ixm;

	recMgr = rm;

	relFile = nullptr;

	attrFile = nullptr;

}

SystemManager::~SystemManager ( ) {
	relFile = nullptr;
	attrFile = nullptr;
	indexMgr = nullptr;
	recMgr = nullptr;
}


inline RETCODE SystemManager::CreateDb (const char * dbName, PageFileManagerPtr & pfMgr) {
	RETCODE result = RETCODE::COMPLETE;

	std::stringstream cmd;

	cmd << "mkdir " << dbName;

	if ( system (cmd.str ( ).c_str ( )) ) {
		result = RETCODE::CREATEFAILED;
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	cmd.str (std::string ( ));			// clear the stream
	cmd.clear ( );						// clear the failed flags

	std::string relcat_name = dbName;
	relcat_name += "\\relcat";
	std::string attrcat_name = dbName;
	attrcat_name += "\\attrcat";

	if ( ( result = recMgr->CreateFile (relcat_name.c_str(), DataRelInfo::size()) ) 
			|| (result = recMgr->OpenFile (relcat_name.c_str ( ), relFile)) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	// Create two relations(tables) to store the database catalog
	if ( ( result = recMgr->CreateFile (attrcat_name.c_str(), sizeof (DataAttrInfo)) ) || recMgr->OpenFile (attrcat_name.c_str(), attrFile) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	// initialize two tables (relations info) 
	DataRelInfo relcat_rel;
	strcpy_s  (relcat_rel.relName, "relcat");
	relcat_rel.attrCount = DataRelInfo::members ( );
	relcat_rel.recordSize = DataRelInfo::size ( );
	relcat_rel.numPages = 1;		// initially
	relcat_rel.numRecords = 2;		// initially only two tables: relcat & attrcat

	DataRelInfo attrcat_rel;
	strcpy_s  (attrcat_rel.relName, "attrcat");
	attrcat_rel.attrCount = DataAttrInfo::members ( );
	attrcat_rel.recordSize = DataAttrInfo::size ( );
	attrcat_rel.numPages = 1; // initially
	attrcat_rel.numRecords = DataAttrInfo::members ( ) + DataRelInfo::members ( );	 // initially only these attributes in total

	// store the two tables into relation file
	RecordIdentifier rid;		// not use in this function
	
	if ( ( result = relFile->InsertRec (reinterpret_cast< const char * >( &relcat_rel ), rid) ) || 
			(result = relFile->InsertRec(reinterpret_cast<const char *>(&attrcat_rel), rid) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	// initialize attributes info of two tables
	// relcat attrs
	DataAttrInfo a;
	strcpy_s  (a.relName, "relcat");
	strcpy_s  (a.attrName, "relName");
	a.offset = offsetof (DataRelInfo, relName);
	a.attrType = STRING;
	a.attrLength = Utils::MAXNAMELEN;
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
		

	strcpy_s  (a.relName, "relcat");
	strcpy_s  (a.attrName, "recordSize");
	a.offset = offsetof (DataRelInfo, recordSize);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "relcat");
	strcpy_s  (a.attrName, "attrCount");
	a.offset = offsetof (DataRelInfo, attrCount);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "relcat");
	strcpy_s  (a.attrName, "numPages");
	a.offset = offsetof (DataRelInfo, numPages);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "relcat");
	strcpy_s  (a.attrName, "numRecords");
	a.offset = offsetof (DataRelInfo, numRecords);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}


	// attrcat attrs
	strcpy_s  (a.relName, "attrcat");
	strcpy_s  (a.attrName, "relName");
	a.offset = offsetof (DataAttrInfo, relName);
	a.attrType = STRING;
	a.attrLength = Utils::MAXNAMELEN;
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}
	
	strcpy_s  (a.relName, "attrcat");
	strcpy_s  (a.attrName, "attrName");
	a.offset = offsetof (DataAttrInfo, relName) + Utils::MAXNAMELEN;
	a.attrType = STRING;
	a.attrLength = Utils::MAXNAMELEN;
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "attrcat");
	strcpy_s  (a.attrName, "offset");
	a.offset = offsetof (DataAttrInfo, offset);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "attrcat");
	strcpy_s  (a.attrName, "attrType");
	a.offset = offsetof (DataAttrInfo, attrType);
	a.attrType = INT;
	a.attrLength = sizeof (AttrType);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	strcpy_s  (a.relName, "attrcat");
	strcpy_s  (a.attrName, "attrLength");
	a.offset = offsetof (DataAttrInfo, attrLength);
	a.attrType = INT;
	a.attrLength = sizeof (int);
	if ( ( result = attrFile->InsertRec (( char* ) &a, rid) ) < 0 ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	if ( ( result = recMgr->CloseFile (relFile) ) || ( result = recMgr->CloseFile (attrFile) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE SystemManager::OpenDb (const char * dbName) {
	
	if ( dbName == nullptr ) {
		return RETCODE::INVALIDOPEN;
	}
	std::string relcat_name = dbName;
	relcat_name += "\\relcat";
	std::string attrcat_name = dbName;
	attrcat_name += "\\attrcat";

	RETCODE result;
	
	if ( ( result = recMgr->OpenFile (relcat_name.c_str ( ), relFile) )
		|| ( result = recMgr->OpenFile (attrcat_name.c_str ( ), attrFile) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	IsDBOpen = true;

	return RETCODE::COMPLETE;
}

inline RETCODE SystemManager::CloseDb ( ) {
	RETCODE result;

	if ( ( result = recMgr->CloseFile (relFile) ) || ( result = recMgr->CloseFile (attrFile) ) ) {
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	IsDBOpen = false;

	return RETCODE::COMPLETE;
}

inline RETCODE SystemManager::CreateTable (const char * relName, int attrCount, AttrInfo * attributes) {

	RETCODE result = RETCODE::COMPLETE;

	if ( relName == nullptr || attrCount <= 0 || attributes == nullptr ) {
		return RETCODE::CREATEFAILED;
	}

	if ( strcmp (relName, "relcat") == 0 || strcmp (relName, "attrcat") == 0 ) {
		return RETCODE::CREATEFAILED;
	}

	RecordIdentifier rid;
	std::set<std::string> uniq;

	DataAttrInfo * d = new DataAttrInfo[attrCount];
	int size = 0;
	for ( int i = 0; i < attrCount; i++ ) {
		d[i] = DataAttrInfo (attributes[i]);
		d[i].offset = size;
		size += attributes[i].attrLength;
		strcpy_s (d[i].relName, relName);

		if ( uniq.find (string (d[i].attrName)) == uniq.end ( ) )
			uniq.insert (string (d[i].attrName));
		else {
			// attrName was used already
			return RETCODE::CREATEFAILED;
		}

		if ( ( result = attrFile->InsertRec (( char* ) &d[i], rid) ) < 0 )
			return( result );
	}



	if ( ( result = recMgr->CreateFile (relName, size) ) )
		return( result );

	DataRelInfo rel;
	strcpy_s (rel.relName, relName);
	rel.attrCount = attrCount;
	rel.recordSize = size;
	rel.numPages = 1; // initially
	rel.numRecords = 0;

	if ( ( result = relFile->InsertRec (( char* ) &rel, rid) ) < 0 )
		return result;

	// attrfh.ForcePages();
	// relfh.ForcePages();
	delete[] d;

	return result;
}


inline RETCODE SystemManager::GetFromTable (const char *relName,           // create relation relName
					  int&        attrCount,         // number of attributes
					  DataAttrInfo   *&attributes) {

	if ( relName == NULL )
		return RETCODE::TABLENOTFOUND;

	void * value = const_cast< char* >( relName );
	RecordFileScan rfs;
	RETCODE rc = rfs.OpenScan (relFile, STRING, Utils::MAXNAMELEN, offsetof (DataRelInfo, relName),
							   EQ_OP, value);
	if ( rc != 0 ) return rc;

	Record rec;
	rc = rfs.GetNextRec (rec);
	if ( rc == RETCODE::EOFFILE )
		return RETCODE::TABLENOTFOUND; // no such table

	DataRelInfo * prel;
	rec.GetData (( char *& ) prel);

	rc = rfs.CloseScan ( );
	if ( rc != 0 ) return rc;

	attrCount = prel->attrCount;
	attributes = new DataAttrInfo[attrCount];

	RecordFileScan afs;
	rc = afs.OpenScan (attrFile, STRING, Utils::MAXNAMELEN, offsetof (DataAttrInfo, relName),
					   EQ_OP, value);

	int numRecs = 0;
	while ( 1 ) {
		Record rec;
		rc = afs.GetNextRec (rec);
		if ( rc == RETCODE::EOFFILE || numRecs > attrCount )
			break;
		DataAttrInfo * pattr;
		rec.GetData (( char*& ) pattr);
		attributes[numRecs] = *pattr;
		numRecs++;
	}

	if ( numRecs != attrCount ) {
		// too few or too many
		return RETCODE::INVALIDTABLE;
	}

	rc = afs.CloseScan ( );
	if ( rc != 0 ) return rc;

	return RETCODE::COMPLETE;

}

inline RETCODE SystemManager::GetRelFromCat (const char * relName, DataRelInfo & rel, RecordIdentifier & rid) const {
	if ( relName == NULL )
		return RETCODE::INVALIDTABLE;

	void * value = const_cast< char* >( relName );
	RecordFileScan rfs;
	RETCODE rc = rfs.OpenScan (relFile, STRING, Utils::MAXNAMELEN, offsetof (DataRelInfo, relName),EQ_OP, value);
	if ( rc != 0 ) return rc;

	Record rec;
	rc = rfs.GetNextRec (rec);
	if ( rc == RETCODE::EOFFILE )
		return RETCODE::TABLENOTFOUND; // no such table

	rc = rfs.CloseScan ( );
	if ( rc != 0 ) return rc;

	DataRelInfo * prel;
	rec.GetData (( char *& ) prel);
	rel = *prel;
	rec.GetIdentifier (rid);

	return RETCODE::COMPLETE;
}

inline RETCODE SystemManager::GetAttrFromCat (const char* relName, const char* attrName, DataAttrInfo& attr, RecordIdentifier& rid) const{
	if ( relName == NULL || attrName == NULL ) {
		return RETCODE::INVALIDTABLE;
	}

	RETCODE rc;
	RecordFileScan rfs;
	Record rec;
	DataAttrInfo * data = nullptr;
	if ( ( rc = rfs.OpenScan (attrFile,
							  STRING,
							  Utils::MAXNAMELEN,
							  offsetof (DataAttrInfo, relName),
							  EQ_OP,
							  ( void* ) relName) ) )
		return ( rc );

	bool attrFound = false;
	while ( rc != RETCODE::EOFFILE ) {
		rc = rfs.GetNextRec (rec);

		if ( rc != 0 && rc != RETCODE::EOFFILE )
			return ( rc );

		if ( rc != RETCODE::EOFFILE ) {
			rec.GetData (( char*& ) data);
			if ( strcmp (data->attrName, attrName) == 0 ) {
				attrFound = true;
				break;
			}
		}
	}

	if ( ( rc = rfs.CloseScan ( ) ) )
		return ( rc );

	if ( !attrFound )
		return RETCODE::BADATTR;

	attr = *data;
	rec.GetIdentifier (rid);

	return RETCODE::COMPLETE;
}


RETCODE SystemManager::IsValid ( ) const {
	bool ret = true;
	ret = ret && IsDBOpen;
	return ret ? RETCODE::COMPLETE : RETCODE::INVALIDOPEN;
}


RETCODE SystemManager::SemCheck (const char* relName) const {
	RETCODE invalid = IsValid ( ); if ( invalid ) return invalid;
	DataRelInfo rel;
	RecordIdentifier rid;
	return GetRelFromCat (relName, rel, rid);
}

RETCODE SystemManager::SemCheck (const RelAttr& ra) const {
	RETCODE invalid = IsValid ( ); if ( invalid ) return invalid;
	DataAttrInfo a;
	RecordIdentifier rid;
	return GetAttrFromCat (ra.relName, ra.attrName, a, rid);
}

// should be used only after lhsAttr and rhsAttr have been expanded out of NULL
RETCODE SystemManager::SemCheck (const Condition& cond) const {
	if ( ( cond.op < NO_OP ) ||
		cond.op > GE_OP )
		return RETCODE::BADOP;

	if ( cond.lhsAttr.relName == NULL || cond.lhsAttr.attrName == NULL )
		return RETCODE::ENTRYNOTFOUND;

	if ( cond.bRhsIsAttr == TRUE ) {
		if ( cond.rhsAttr.relName == NULL || cond.rhsAttr.attrName == NULL )
			return RETCODE::ENTRYNOTFOUND;
		DataAttrInfo a, b;
		RecordIdentifier rid;
		RETCODE rc = GetAttrFromCat (cond.lhsAttr.relName, cond.lhsAttr.attrName, a, rid);
		if ( rc != 0 ) return RETCODE::ENTRYNOTFOUND;
		rc = GetAttrFromCat (cond.rhsAttr.relName, cond.rhsAttr.attrName, b, rid);
		if ( rc != 0 ) return RETCODE::ENTRYNOTFOUND;

		if ( b.attrType != a.attrType )
			return RETCODE::TYPEMISMATCH;

	} else {
		DataAttrInfo a;
		RecordIdentifier rid;
		RETCODE rc = GetAttrFromCat (cond.lhsAttr.relName, cond.lhsAttr.attrName, a, rid);
		if ( rc != 0 ) return RETCODE::ENTRYNOTFOUND;
		if ( cond.rhsValue.type != a.attrType )
			return RETCODE::TYPEMISMATCH;
	}
	return RETCODE::COMPLETE;
}
