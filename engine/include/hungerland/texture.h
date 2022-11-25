#pragma once
#include <stdint.h>
#include <memory>

namespace hungerland {

	class Texture {
	public:
		typedef std::shared_ptr<Texture> Ptr;
		Texture() : m_textureId(-1), m_width(0), m_height(0) {}
		Texture(int width, int height, int nrChannels, const uint8_t* data, bool repeat = false);
		Texture(int width, int height, int nrChannels, const float* data, bool repeat = false);
		Texture(int width, int height, bool isDepthTexture);
		~Texture();

		uint32_t getTextureId() const;
		auto getWidth() const {return m_width;}
		auto getHeight() const {return m_height;}

	private:
		uint32_t				m_textureId;	// Texture id
		int m_width;
		int m_height;
	};

}
