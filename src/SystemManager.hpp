#pragma once

#include "Utils.hpp"
#include "IndexManager.hpp"
#include "RecordFileManager.hpp"
#include "PageFileManager.hpp"


// Used by Printer class
struct DataAttrInfo {
	char     relName[Utils::MAXNAMELEN + 1];  // Relation name
	char     attrName[Utils::MAXNAMELEN + 1]; // Attribute name
	int      offset;              // Offset of attribute 
	AttrType attrType;            // Type of attribute 
	int      attrLength;          // Length of attribute
	int      indexNo;             // Attribute index number
};

struct DataRelInfo {
	// Default constructor
	DataRelInfo ( ) {
		memset (relName, 0,Utils::MAXNAMELEN + 1);
	}

	DataRelInfo (char * buf) {
		memcpy (this, buf, DataRelInfo::size ( ));
	}

	// Copy constructor
	DataRelInfo (const DataRelInfo &d) {
		strcpy_s (relName, d.relName);
		recordSize = d.recordSize;
		attrCount = d.attrCount;
		numPages = d.numPages;
		numRecords = d.numRecords;
	};

	DataRelInfo& operator=(const DataRelInfo &d) {
		if ( this != &d ) {
			strcpy_s (relName, d.relName);
			recordSize = d.recordSize;
			attrCount = d.attrCount;
			numPages = d.numPages;
			numRecords = d.numRecords;
		}
		return ( *this );
	}

	static unsigned int size ( ) {
		return (Utils::MAXNAMELEN + 1 ) + 4 * sizeof (int);
	}

	static unsigned int members ( ) {
		return 5;
	}

	int      recordSize;            // Size per row
	int      attrCount;             // # of attributes
	int      numPages;              // # of pages used by relation
	int      numRecords;            // # of records in relation
	char     relName[Utils::MAXNAMELEN];    // Relation name
};


class SystemManager {
public:
	SystemManager (const IndexManagerPtr & ixm, 
								const RecordFileManagerPtr & rm);

	~SystemManager ( );

	static RETCODE CreateDb (const char * dbName, int attrCount, AttrInfo * attributes,
												PageFileManagerPtr & pfMgr, RecordFileManager & recMgr);
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


public:
	RETCODE IsValid ( ) const;

	// attributes is allocated and returned back with attrCount elements.
	// attrCount is returned back with number of attributes
	RETCODE GetFromTable (const char *relName,           // create relation relName
					 int&        attrCount,         // number of attributes
					 DataAttrInfo   *&attributes);  // attribute data

	// Get the first matching row for relName
	// contents are return in rel and the RecordIdentifier the record is located at is
	// returned in RecordIdentifier.
	// method returns SM_NOSUCHTABLE if relName was not found
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
					   RecordIdentifier& RecordIdentifier) const;

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

	std::string dbOpen;

	IndexManagerPtr indexMgr;

	RecordFileManagerPtr recMgr;

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


inline RETCODE SystemManager::CreateDb (const char * dbName, int attrCount, AttrInfo * attributes,
																		PageFileManagerPtr & pfMgr, RecordFileManager & recMgr) {
	RETCODE result = RETCODE::COMPLETE;

	std::stringstream cmd;

	cmd << "mkdir " << dbName;

	if ( system (cmd.str ( ).c_str ( )) ) {
		result = RETCODE::CREATEFAILED;
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	cmd.clear ( );
	cmd << "cd " << dbName;

	if ( system (cmd.str ( ).c_str ( )) ) {
		result = RETCODE::CREATEFAILED;
		Utils::PrintRetcode (result, __FUNCTION__, __LINE__);
		return result;
	}

	return result;
}

inline RETCODE SystemManager::OpenDb (const char * dbName) {
	
	return RETCODE::COMPLETE;
}

inline RETCODE SystemManager::CloseDb ( ) {
	return RETCODE ( );
}
