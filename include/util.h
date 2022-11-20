//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
//// CyberSpace - Design, art: Roope Romppainen, Code: Mikko Romppainen
////
////
//// Copyright (c) 2022 Mikko Romppainen.
////
//// =-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-= ////
#include <stdexcept>

namespace cs {

namespace csut {
	inline auto INFO(const std::string& text) {
		printf("INFO: %s\n", text.c_str());
		fflush(stdout);
	}

	inline auto WARNING(const std::string& text) {
		printf("WARNING: %s\n", text.c_str());
		fflush(stdout);
	}

	inline auto ERROR(const std::string& text) {
		printf("ERROR: %s\n", text.c_str());
		fflush(stdout);
		throw std::runtime_error("ERROR: " + text);
	}


} // End - namespace csut

} // End - namespace cs
