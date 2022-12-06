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
#include <cstddef>
#include <functional>
#include <string>           // Inlude std::string
#include <vector>           // Inlude std::vector
#include <memory>           // Inlude std::string
#include <array>

namespace hungerland {

	namespace aabb {
		template<typename Vec>
		struct AABB {
			Vec p;
			Vec hs;
		};

		template<typename Vec>
		static inline auto createAABB(const Vec& p, const Vec& hs) {
			return AABB<Vec>{p, hs};
		}

		template<typename Vec, typename AABB, typename AbsFunc>
		static inline auto getOverlap(AbsFunc absf, const AABB& o1, const AABB& o2) {
			auto d = o2.p - o1.p;
			d.x = std::abs(d.x);
			d.y = std::abs(d.y);
			d.z = std::abs(d.z);
			return o1.hs + o2.hs - d;
		}
	}

}
