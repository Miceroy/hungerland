#pragma once
#include <string>
#include <vector>

#define HL_ASSERT(x) hungerland::util::fassert(x, #x, __FUNCTION__, __FILE__, __LINE__)

namespace hungerland {

namespace util {
	///
	/// \brief INFO
	/// \param text
	///
	void INFO(const std::string& text) ;

	///
	/// \brief WARNING
	/// \param text
	///
	void WARNING(const std::string& text) ;

	///
	/// \brief ERROR
	/// \param text
	///
	void ERROR(const std::string& text) ;

	///
	/// \brief readFile
	/// \param fileName
	/// \return
	///
	std::string readFile(const std::string& fileName);

	///
	/// \brief genNM
	/// \param f
	/// \param N
	/// \param M
	///
	template<typename T, typename F>
	static inline auto genNM(F f, std::size_t N, std::size_t M) {
		std::vector< std::vector<T> > res;
		for(std::size_t i=0; i<M; ++i) {
			res.push_back(std::vector<T>());
			for(std::size_t j=0; j<N; ++j) {
				res[i].push_back(f(j,i));
			}
		}
		return res;
	}

	///
	/// \brief gridNM
	/// \param N
	/// \param M
	/// \param value
	///
	template<typename T>
	static inline auto gridNM(std::size_t N, std::size_t M, T value) {
		return genNM<int>([&value](size_t x, size_t y){
			return value;
		}, N, M);
	}

	///
	/// \brief gridN
	/// \param N
	/// \param value
	///
	template<typename T>
	static inline auto gridN(std::size_t N, T value) {
		return gridNM(N,N,value);
	}

	static inline void fassert(bool cond, std::string expr, std::string func, std::string file, int line) {
		if(cond) {
			util::ERROR("Assertation failed at file (Line:+line+): \"" + file+ "\"");
		}
	}
} // End - namespace util

} // End - namespace hungerland
