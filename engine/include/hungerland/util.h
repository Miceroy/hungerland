/*=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=
 MIT License

 Copyright (c) 2022 Mikko Romppainen (kajakbros@gmail.com)

 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:

 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.

 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=*/
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
	/// \brief WARN
	/// \param text
	///
	void WARN(const std::string& text) ;

	///
	/// \brief ERR
	/// \param text
	///
	void ERR(const std::string& text) ;

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
		return genNM<T>([&value](size_t x, size_t y){
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
			hungerland::util::ERR("Assertation failed at file (Line:+line+): \"" + file+ "\"");
		}
	}
} // End - namespace util

} // End - namespace hungerland
