#pragma once
#include <hungerland/types.h>

namespace hungerland {
namespace texture {

	class Texture {
	public:
		Texture(int width, int height, int nrChannels, const uint8_t* data);
		Texture(int width, int height, int nrChannels, const float* data);
		Texture(int width, int height, bool isDepthTexture);
		~Texture();

		void bind(unsigned textureIndex);
		void setRepeat(bool repeat);
		void setFiltering(bool filter);

		unsigned getId() const;
		unsigned getWidth() const;
		unsigned getHeight() const;

	private:
		unsigned	m_textureId;	// Texture id
		unsigned	m_width;
		unsigned	m_height;

		// Copy not allowed
		Texture() = delete;
		Texture(const Texture&) = delete;
		Texture& operator=(const Texture&) = delete;
	};
}
}
