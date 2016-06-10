#pragma once

#include "Utils.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <memory>
#include <string>
#include <vector>
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

using uint64 = unsigned long long;
using PageNum = unsigned long long;
using DataPtr = shared_ptr<char>;

/*
	Enums
*/
enum RETCODE {

	COMPLETE = 0,
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
	CLOSEDFILE       // file is closed
};

namespace Utils{ 
	
	/*
		Limit
	*/

	const size_t IDENTIFYSTRINGLEN = 32;

	const size_t MAXNAMELEN = 32;

	const char PAGEFILEIDENTIFYSTRING[IDENTIFYSTRINGLEN] = "MicroSQL PageFile";

	//const char PAGEIDENTIFYSTRING[IDENTIFYSTRINGLEN] = "MicroSQL Page";

	/*
		Server Settings
	*/

	std::string DataDir;				// data files directory

	std::string BaseDir;				// software base directory

	bool skip_grant_tables;			// 

	std::string DEFAULTCHARSET = "utf8";

	size_t PAGESIZE = 4096;		// page size

	size_t BUFFERSIZE = 40;			// number of pages in buffer

	/*
		Utility Functions
	*/

	string GetRECODEMessage (RETCODE code) {
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
		case EOF: return "end of file"; break;
		case PAGELOCKNED: return "page lockned in buffer"; break;
		case PAGENOTINBUF: return "page to be unlockned is not in buffer"; break;
		case PAGEUNLOCKNED: return "page already unlockned"; break;
		case PAGEFREE: return "page already free"; break;
		case INVALIDPAGE: return "invalid page number"; break;
		case INVALIDPAGEFILE: return "invalid page file"; break;
		case FILEOPEN: return "file handle already open"; break;
		case CLOSEDFILE: return "file is closed"; break;
		default: return "Unknown RETCODE"; break;
		}
	}
}


