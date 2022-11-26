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

	class Shader {
	public:
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


}
}
