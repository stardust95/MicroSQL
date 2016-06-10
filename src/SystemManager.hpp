#pragma once

#include "Utils.hpp"

// Used by SM_Manager::CreateTable
struct AttrInfo {
	char     *attrName;           // Attribute name
	AttrType attrType;            // Type of attribute
	int      attrLength;          // Length of attribute
};

// Used by Printer class
struct DataAttrInfo {
	char     relName[Utils::MAXNAMELEN + 1];  // Relation name
	char     attrName[Utils::MAXNAMELEN + 1]; // Attribute name
	int      offset;              // Offset of attribute 
	AttrType attrType;            // Type of attribute 
	int      attrLength;          // Length of attribute
	int      indexNo;             // Attribute index number
};


class SystemManager {
public:
	SystemManager ( );
	~SystemManager ( );

	RETCODE OpenDb (const char *dbName);                // Open database
	RETCODE CloseDb ( );                                  // Close database
	RETCODE CreateTable (const char *relName,                // Create relation
					int        attRETCODEount,
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

};

using SystemManagerPtr = shared_ptr<SystemManager>;

SystemManager::SystemManager ( ) {
}

SystemManager::~SystemManager ( ) {
}
