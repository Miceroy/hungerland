#include <stdexcept>

namespace hungerland {

namespace util {

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

} // End - namespace util

} // End - namespace hungerland
