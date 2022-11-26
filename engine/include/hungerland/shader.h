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
#include <hungerland/types.h>


namespace hungerland {
namespace shader {
	class Shader;
	class ShaderPass;

	typedef std::pair<std::string,std::vector<float> >Constant;
	typedef std::vector<shader::Constant> Constants;
	typedef std::function<void(ShaderPass)> RenderFunc;

	std::string to_string(const Constants& inputConstants);

	///
	/// \brief The ShaderPass class
	///
	/// @ingroup hungerland::shader
	/// @author Mikko Romppainen (kajakbros@gmail.com)
	///
	class ShaderPass {
	public:
		ShaderPass(const Shader& shader);
		void setUniformv(const std::string& name, const std::vector<float>& v);
		void setUniform(const std::string& name, float v);
		void setUniform(const std::string& name, float x, float y);
		void setUniform(const std::string& name, float x, float y, float z);
		void setUniform(const std::string& name, float x, float y, float z, float w);
		void setUniformm(const std::string& name, const float* m, bool transposed=false);
		void setUniform(const std::string& name, int value);
	private:
		const Shader& m_shader;
	};

	///
	/// \brief The Shader class
	///
	/// @ingroup hungerland::shader
	/// @author Mikko Romppainen (kajakbros@gmail.com)
	///
	class Shader {
	public:
		typedef std::shared_ptr<Shader> Ref;
		Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString);
		~Shader();

		void use(RenderFunc render) const;

		unsigned getId() const ;

	private:
		unsigned m_shaderProgram;	// Handle to the shader program

		// Copy not allowed
		Shader(const Shader&) = delete;
		Shader& operator=(const Shader&) = delete;
	};

	static inline Shader::Ref standardShader(const std::vector<shader::Constant>& constants, const std::string& surfaceShader, const std::string& globals) {
		return 0;
	}


}
}
