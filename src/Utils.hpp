#pragma once

#include "Utils.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <memory>
#include <string>
#include <vector>
#include <iostream>


/*
	Standard Type Clarification
*/

using std::shared_ptr;
using std::make_shared;
using std::string;
using std::vector;


/*
Type Definition
*/

//using uint64 = unsigned long long;

using PageNum = unsigned long long;			// Identify a page in a file

using SlotNum = unsigned int;		// Identify a record in a page 

using DataPtr = shared_ptr<char>;

using VoidPtr = shared_ptr<void>;

/*
	Enums
*/
enum RETCODE {

	COMPLETE = 0,

	EOFFILE,
	NOMEM,            // out of memory
	NOBUF,            // out of buffer space
	INCOMPLETEREAD,   // incomplete read of page from file
	INCOMPLETEWRITE,  // incomplete write of page to file
	HDRREAD,          // incomplete read of header from file
	HDRWRITE,         // incomplete write of header to file
							// Internal PF errors:
	PAGEINBUF,        // new allocated page already in buffer
	HASHNOTFOUND,     // hash table entry not found
	HASHPAGEEXIST,    // page already exists in hash table
	INVALIDNAME,      // invalid file name

	PAGELOCKNED,       // page lockned in buffer
	PAGENOTINBUF,     // page to be unlockned is not in buffer
	PAGEUNLOCKNED,     // page already unlockned
	PAGEFREE,         // page already free
	INVALIDPAGE,      // invalid page number
	INVALIDPAGEFILE,
	FILEOPEN,         // file handle already open
	CLOSEDFILE,       // file is closed

	INVALIDSCAN,		// invalid scan request
	INVALIDRECORDFILE,	// invalid record file
	EOFSCAN,		// end of a scan
};

enum AttrType {
	INT,
	FLOAT,
	STRING
};

enum CompOp {
	EQ_OP, //	equal (i.e., attribute = value)
	LT_OP, //	less - than (i.e., attribute < value)
	GT_OP, //	greater - than (i.e., attribute > value)
	LE_OP, //	less - than - or -equal (i.e., attribute <= value)
	GE_OP, //	greater - than - or -equal (i.e., attribute >= value)
	NE_OP, //	not- equal (i.e., attribute <> value)
	NO_OP	, //no comparison (when value is a null pointer)
};

namespace Utils{ 
	
	/*
		Limitations
	*/

	const size_t IDENTIFYSTRINGLEN = 32;

	const size_t MAXNAMELEN = 32;

	const PageNum MAXPAGECOUNT = 2 << 31;

	const char PAGEFILEIDENTIFYSTRING[IDENTIFYSTRINGLEN] = "MicroSQL PageFile";

	const char RECORDFILEIDENTIFYSTRING[IDENTIFYSTRINGLEN] = "MicroSQL RecordFile";

	const char PAGEIDENTIFYSTRING[IDENTIFYSTRINGLEN] = "MicroSQL Page";

	/*
		Server Settings
	*/

	std::string DataDir;				// data files directory

	std::string BaseDir;				// software base directory

	bool skip_grant_tables;			// 

	std::string DEFAULTCHARSET = "utf8";

	const size_t PAGESIZE = 4096;		// page size

	const size_t BUFFERSIZE = 40;			// number of pages in buffer

	/*
		Utility Functions
	*/
	
	string GetRetcodeMessage (RETCODE code) {
		switch ( code ) {
		case NOMEM: return "out of memory"; break;
		case NOBUF: return "out of buffer space"; break;
		case INCOMPLETEREAD: return "incomplete read of page from file"; break;
		case INCOMPLETEWRITE: return "incomplete write of page to file"; break;
		case HDRREAD: return "incomplete read of header from file"; break;
		case HDRWRITE: return "incomplete write of header to file"; break;
		case PAGEINBUF: return "new allocated page already in buffer"; break;
		case HASHNOTFOUND: return "hash table entry not found"; break;
		case HASHPAGEEXIST: return "page already exists in hash table"; break;
		case INVALIDNAME: return "invalid file name"; break;
		case COMPLETE: return "complete"; break;
		case EOFFILE: return "end of file"; break;
		case PAGELOCKNED: return "page lockned in buffer"; break;
		case PAGENOTINBUF: return "page to be unlockned is not in buffer"; break;
		case PAGEUNLOCKNED: return "page already unlockned"; break;
		case PAGEFREE: return "page already free"; break;
		case INVALIDPAGE: return "invalid page number"; break;
		case INVALIDPAGEFILE: return "invalid page file"; break;
		case FILEOPEN: return "file handle already open"; break;
		case CLOSEDFILE: return "file is closed"; break;
		case INVALIDSCAN: return "invalid scan"; break;
		case INVALIDRECORDFILE: return "invalid record file"; break;
		case EOFSCAN: return "scan ended"; break;
		default: return "Unknown RETCODE"; break;
		}
	}


	void PrintRetcode (RETCODE code) {
		std::cout << Utils::GetRetcodeMessage (code) << std::endl;
	}



}



namespace CompMethod {

	int compare_string (void *value1, void* value2, size_t attrLength) {
		return strncmp (reinterpret_cast< char* >( value1 ) , reinterpret_cast< char* >( value2 ) , attrLength);
	}

	static int compare_int (void *value1, void* value2, size_t attrLength) {
		if ( ( *reinterpret_cast< int* >( value1 )  < *reinterpret_cast< int* >( value2 )  ) )
			return -1;
		else if ( ( *reinterpret_cast< int* >( value1 )  > *reinterpret_cast< int* >( value2 )  ) )
			return 1;
		else
			return 0;
	}

	int compare_float (void *value1, void* value2, size_t attrLength) {
		if ( ( *reinterpret_cast< float* >( value1 )  < *reinterpret_cast< float* >( value2 )  ) )
			return -1;
		else if ( ( *reinterpret_cast< float* >( value1 )  > *reinterpret_cast< float* >( value2 )  ) )
			return 1;
		else
			return 0;
	}

	bool equal (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) == *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) == *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) == 0 );
		}
	}

	bool less_than (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) < *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) < *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) < 0 );
		}
	}

	bool greater_than (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) > *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) > *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) > 0 );
		}
	}

	bool less_than_or_eq_to (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) <= *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) <= *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) <= 0 );
		}
	}

	bool greater_than_or_eq_to (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) >= *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) >= *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) >= 0 );
		}
	}

	bool not_equal (void * value1, void * value2, AttrType attrtype, size_t attrLength) {
		switch ( attrtype ) {
		case FLOAT: return ( *reinterpret_cast< float* >( value1 ) != *reinterpret_cast< float* >( value2 ) );
		case INT: return ( *reinterpret_cast< int* >( value1 ) != *reinterpret_cast< int* >( value2 ) );
		default:
			return ( strncmp (reinterpret_cast< char* >( value1 ), reinterpret_cast< char* >( value2 ), attrLength) != 0 );
		}
	}

}
