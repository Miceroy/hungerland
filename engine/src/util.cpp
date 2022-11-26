#include <hungerland/util.h>
#include <stdexcept>
#include <fstream>

namespace hungerland {

namespace util {

	void INFO(const std::string& text) {
		printf("INFO: %s\n", text.c_str());
		fflush(stdout);
	}

	void WARNING(const std::string& text) {
		printf("WARNING: %s\n", text.c_str());
		fflush(stdout);
	}

	void ERROR(const std::string& text) {
		printf("ERROR: %s\n", text.c_str());
		fflush(stdout);
		throw std::runtime_error("ERROR: " + text);
	}

	std::string readFile(const std::string& fileName){
		std::ifstream f(fileName);
		return std::string(std::istreambuf_iterator<char>(f), std::istreambuf_iterator<char>());
	}

} // End - namespace util

} // End - namespace hungerland
