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
#include <hungerland/shader.h>		// Include class header
#include <hungerland/util.h>	// Include gl_utils for checkGLError
#include <hungerland/gl_utils.h>	// Include gl_utils for checkGLError
#include <glad/gl.h>
#include <stdio.h>			// Include stdio.h, which contains printf-function
#include <assert.h>

namespace hungerland {
namespace shader {

	std::string to_string(const std::vector<shader::Constant>& inputConstants) {
		std::string res;
		for(auto& c : inputConstants){
			auto& name = c.first;
			auto& shaderConstantValue = c.second;
			if(shaderConstantValue.size() == 1){
				res += "uniform float "+name+";\n";
			} else if(shaderConstantValue.size() == 2){
				res += "uniform vec2 "+name+";\n";
			} else if(shaderConstantValue.size() == 3){
				res += "uniform vec3 "+name+";\n";
			} else if(shaderConstantValue.size() == 4){
				res += "uniform vec4 "+name+";\n";
			} else {
				HL_ASSERT(1 <= shaderConstantValue.size() && shaderConstantValue.size() <= 4); // Invalid value length. Must be between 1 to 4
			}
		}
		return res;
	}

	ShaderPass::ShaderPass(const Shader& shader) : m_shader(shader) {
	}

	void ShaderPass::setUniformv(const std::string& name, const std::vector<float>& v){
		GLint loc = glGetUniformLocation(m_shader.getId(), name.c_str());
		if (loc < 0) {
			return; // Don't set the uniform value, if it not found
		}
		if(v.size()==1){
			glUniform1f(loc, v[0]);
		} else if(v.size()==2){
			glUniform2f(loc, v[0], v[1]);
		} else if(v.size()==3){
			glUniform3f(loc, v[0], v[1], v[2]);
		} else if(v.size()==4){
			glUniform4f(loc, v[0], v[1], v[2], v[3]);
		}
		checkGLError();
	}

	void ShaderPass::setUniform(const std::string& name, float v) {
		setUniformv(name,{v});
	}

	void ShaderPass::setUniform(const std::string& name, float x, float y) {
		setUniformv(name,{x,y});
	}

	void ShaderPass::setUniform(const std::string& name, float x, float y, float z) {
		setUniformv(name,{x,y,z});
	}

	void ShaderPass::setUniform(const std::string& name, float x, float y, float z, float w) {
		setUniformv(name,{x,y,z,w});
	}

	void ShaderPass::setUniformm(const std::string& name, const float* m, bool transposed) {
		GLint loc = glGetUniformLocation(m_shader.getId(), name.c_str());
		if (loc < 0) {
			return; // Don't set the uniform value, if it not found
		}
		glUniformMatrix4fv(loc, 1, transposed ? GL_TRUE:GL_FALSE, m);
		checkGLError();
	}

	void ShaderPass::setUniform(const std::string& name, int value) {
		GLint loc = glGetUniformLocation(m_shader.getId(), name.c_str());
		if (loc < 0) {
			return; // Don't set the uniform value, if it not found
		}
		glUniform1i(loc, value);
		checkGLError();
	}

	Shader::Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString)
		: m_shaderProgram(0) {
		checkGLError();
		// Create and compile vertex shader
		int vertexShader = glCreateShader(GL_VERTEX_SHADER);
		checkGLError();
		auto vs = vertexShaderString.c_str();
		glShaderSource(vertexShader, 1, &vs, 0);
		checkGLError();
		glCompileShader(vertexShader);
		checkGLError();

		// check for shader compile errors
		int success;
		char infoLog[512];
		glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
		checkGLError();
		if (!success) {
			// If failed, get error string using glGetShaderInfoLog-function.
			glGetShaderInfoLog(vertexShader, 512, 0, infoLog);
			checkGLError();
			util::ERR("Vertex shader compilation failed: \"" + std::string(infoLog) + "\"");
		}

		// Create and compile fragment shader
		int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
		checkGLError();
		auto fs = fragmentShaderString.c_str();
		glShaderSource(fragmentShader, 1, &fs, 0);
		checkGLError();
		glCompileShader(fragmentShader);
		checkGLError();
		// check for shader compile errors
		glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
		checkGLError();
		if (!success) {
			// If failed, get error string using glGetShaderInfoLog-function.
			glGetShaderInfoLog(fragmentShader, 512, 0, infoLog);
			checkGLError();
			util::ERR("Fragment shader compilation failed: \"" + std::string(infoLog) + "\"");
		}

		// link shaders
		m_shaderProgram = glCreateProgram();
		checkGLError();
		glAttachShader(m_shaderProgram, vertexShader);
		checkGLError();
		glAttachShader(m_shaderProgram, fragmentShader);
		checkGLError();
		glLinkProgram(m_shaderProgram);
		checkGLError();
		// check for linking errors
		glGetProgramiv(m_shaderProgram, GL_LINK_STATUS, &success);
		checkGLError();
		if (!success) {
			// If failed, get error string using glGetProgramInfoLog-function.
			glGetProgramInfoLog(m_shaderProgram, 512, 0, infoLog);
			checkGLError();
			util::ERR("Shader link failed: \"" + std::string(infoLog) + "\"");
		}

		// After linking, the shaders can be deleted.
		glDeleteShader(vertexShader);
		checkGLError();
		glDeleteShader(fragmentShader);
		checkGLError();
	}

	Shader::~Shader() {
		assert(m_shaderProgram != 0);
		// Delete shader program
		glDeleteProgram(m_shaderProgram);
		//checkGLError();
	}

	void Shader::use(RenderFunc render) const {
		assert(m_shaderProgram != 0);
		glUseProgram(m_shaderProgram);
		checkGLError();
		auto pass = ShaderPass(*this);
		render(pass);
		checkGLError();
		glUseProgram(0);
		checkGLError();
	}

	unsigned Shader::getId() const {
		return m_shaderProgram;
	}

}
}

