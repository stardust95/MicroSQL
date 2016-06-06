#pragma once

#include <string>

namespace Setting {

	std::string datadir;				// data files directory

	std::string basedir;				// software base directory

	bool skip_grant_tables;			// 

	std::string default_chaset = "utf8";

	size_t page_size = 4096;		// page size

	size_t buffer_size = 40;			// number of pages in buffer

}
