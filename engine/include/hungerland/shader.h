#pragma once
#include <glad/gl.h>		// Include glad
#include <string>           // Inlude std::string
#include <vector>           // Inlude std::string

namespace hungerland {

	class Shader {
	public:
		Shader(const std::string& vertexShaderString, const std::string& fragmentShaderString);
		~Shader();

		template<typename F>
		void use(F f){
			bind();
			f();
			unbind();
		}
		void setUniformv(const std::string& name, const std::vector<float>& v);
		void setUniform(const std::string& name, float v);
		void setUniform(const std::string& name, float x, float y);
		void setUniform(const std::string& name, float x, float y, float z);
		void setUniform(const std::string& name, float x, float y, float z, float w);
		//void setUniformm(const std::string& name, const std::vector<float>& m, bool transposed=false);
		void setUniformm(const std::string& name, const float* m, bool transposed=false);
		void setUniform(const std::string& name, int value);

	private:

		void bind();
		void unbind();
		GLint m_shaderProgram;	// Handle to the shader program
	};

}
