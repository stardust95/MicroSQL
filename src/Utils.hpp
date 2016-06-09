#pragma once

#include "Setting.hpp"

#include <boost/property_tree/json_parser.hpp>

#include <memory>
#include <string>

using uint64 = unsigned long long;
using PageNum = unsigned long long;

using std::shared_ptr;
using std::string;

namespace PF {

	const size_t identifyStringLen = 32;

	const size_t MaxNameLen = 32;

	const char PageFileIdentifyString[identifyStringLen] = "MicroSQL PageFile";

	const char PageIdentifyString[identifyStringLen] = "MicroSQL Page";

	enum RETCODE {
		PF_NOMEM,            // out of memory
		PF_NOBUF,            // out of buffer space
		PF_INCOMPLETEREAD,   // incomplete read of page from file
		PF_INCOMPLETEWRITE,  // incomplete write of page to file
		PF_HDRREAD,          // incomplete read of header from file
		PF_HDRWRITE,         // incomplete write of header to file
							 // Internal PF errors:
		PF_PAGEINBUF,        // new allocated page already in buffer
		PF_HASHNOTFOUND,     // hash table entry not found
		PF_HASHPAGEEXIST,    // page already exists in hash table
		PF_INVALIDNAME,      // invalid file name

		PF_COMPLETE = 0,

		PF_EOF,              // end of file
		PF_PAGElockNED,       // page lockned in buffer
		PF_PAGENOTINBUF,     // page to be unlockned is not in buffer
		PF_PAGEUNlockNED,     // page already unlockned
		PF_PAGEFREE,         // page already free
		PF_INVALIDPAGE,      // invalid page number
		PF_FILEOPEN,         // file handle already open
		PF_CLOSEDFILE       // file is closed
	};
}


namespace Utils{

}

