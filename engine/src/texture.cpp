#include <hungerland/texture.h>
#include <hungerland/gl_utils.h>
#include <glad/gl.h>
#include <stdio.h>
#include <assert.h>

namespace hungerland {
namespace texture {

	/*Texture::Texture()
		: m_textureId(0), m_width(0), m_height(0) {
	}*/

	Texture::Texture(int width, int height, int nrChannels, const GLubyte* data) : m_width(width), m_height(height) {
		// Create texture
		glGenTextures(1, &m_textureId);
		checkGLError();
		// Bind it for use
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();
		// set the texture data as RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels == 3 ? GL_RGB : GL_RGBA, width, height, 0, nrChannels == 3 ? GL_RGB : GL_RGBA, GL_UNSIGNED_BYTE, data);
		checkGLError();

		setRepeat(false);
		setFiltering(false);
	}

	Texture::Texture(int width, int height, int nrChannels, const float* data) : m_width(width), m_height(height) {
		// Create texture
		glGenTextures(1, &m_textureId);
		checkGLError();
		// Bind it for use
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();
		// set the texture data as RGBA
		glTexImage2D(GL_TEXTURE_2D, 0, nrChannels == 3 ? GL_RGB32F : GL_RGBA32F, width, height, 0, nrChannels == 3 ? GL_RGB : GL_RGBA, GL_FLOAT, data);
		checkGLError();

		setRepeat(false);
		setFiltering(false);
	}

	Texture::Texture(int width, int height, bool isDepthTexture) : m_width(width), m_height(height) {
		// Create texture
		glGenTextures(1, &m_textureId);
		checkGLError();
		// Bind it for use
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();

		if (isDepthTexture) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
			checkGLError();
		} else {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
			checkGLError();
		}

		setRepeat(false);
		setFiltering(false);
	}

	Texture::~Texture() {
		glDeleteTextures(1, &m_textureId);
		checkGLError();
	}

	void Texture::bind(unsigned textureIndex) {
		glActiveTexture(GL_TEXTURE0 + textureIndex);
		checkGLError();
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();
	}

	void Texture::setRepeat(bool repeat) {
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();
		if(repeat) {
			// set the texture wrapping options to repeat
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			checkGLError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			checkGLError();
		} else {
			// set the texture wrapping options to clamp
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			checkGLError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			checkGLError();
		}
	}

	void Texture::setFiltering(bool filter) {
		glBindTexture(GL_TEXTURE_2D, m_textureId);
		checkGLError();
		if(filter) {
			// set the texture filltering options to linear
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			checkGLError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST_MIPMAP_LINEAR);
			checkGLError();
		} else {
			// set the texture filltering options to nearest
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			checkGLError();
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			checkGLError();
		}
	}

	unsigned Texture::getId() const {
		return m_textureId;
	}

	unsigned Texture::getWidth() const {
		return m_width;
	}

	unsigned Texture::getHeight() const {
		return m_height;
	}
}
}
