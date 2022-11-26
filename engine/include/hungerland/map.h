#pragma once
#include <hungerland/types.h>
#include <functional>
#include <memory>
#include <hungerland/math.h>

namespace tmx {
	class TileLayer;
	class ImageLayer;
	class Map;
}

namespace hungerland {
	namespace shader {
		class Shader;
		class ShaderPass;
	}

	namespace texture {
		class Texture;
	}
	namespace mesh {
		class Mesh;
	}
}

namespace hungerland {
	///
	/// \brief The TileMap class
	///
	class TileMap {
	public:
		typedef std::function<std::shared_ptr<texture::Texture>(const std::string&)> LoadTextureFuncType;
		TileMap() {}
		TileMap(const std::string& mapFilename, LoadTextureFuncType loadTexture);
		~TileMap() {}
		void render(const std::vector<float>& projectionMatrix, const glm::vec2& cameraDelta) const;

		size2d_t getMapSize() const;
		size2d_t getTileSize() const;
		const size_t getNumLayers() const;
		const int getTileId(size_t layer, size_t x, size_t y) const;

	private:

		struct LayerSubset {
			std::shared_ptr<texture::Texture> texture;
			float opacity = 1.0f;
			int2d_t offset = {0,0};
			bool used = false;
			std::vector<float> tintColor;
			glm::vec2 parallaxFactor = {1,1};
		};

		struct TileSubset : public LayerSubset {
			std::shared_ptr<texture::Texture> lookup;
			std::shared_ptr<texture::Texture> texture;
			size2d_t tileSize;
			size2d_t tilesetCount;
		};

		struct ObjectSubset : public LayerSubset {
			std::shared_ptr<mesh::Mesh> mesh;
			std::shared_ptr<texture::Texture> texture;
			size2d_t size;
			std::vector<float> objectColor;
		};

		struct BackgroundSubset : public LayerSubset {
			std::shared_ptr<mesh::Mesh> mesh;
			std::shared_ptr<texture::Texture> texture;
			std::vector<float> transparentColor;
			size2d_t size;
			size2d_t repeat;
		};

		struct TileLayer {
		public:
			std::vector<TileSubset>	subsets;
			std::vector< std::vector<int> > tileIds;
			std::vector< std::vector<int> > tileFlags;
			TileLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<texture::Texture> >& tilesetTextures);
			void render(shader::ShaderPass& shader, mesh::Mesh* mesh) const;
			~TileLayer();
		};

		struct BackgroundLayer {
		public:
			BackgroundSubset subset;
			const tmx::ImageLayer* tmxLayer;
			BackgroundLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<texture::Texture> >& mapTextures);
			void render(shader::ShaderPass& shader, const glm::vec2& cameraDelta) const;
			~BackgroundLayer();
		};
		std::shared_ptr<tmx::Map>							m_map;
		std::vector< std::shared_ptr<texture::Texture> >	m_tilesetTextures;
		std::vector< std::shared_ptr<texture::Texture> >	m_imageTextures;
		std::shared_ptr<mesh::Mesh>				m_mapMesh;
		std::shared_ptr<shader::Shader>					m_tileShader;
		std::vector< std::shared_ptr<TileLayer> >			m_tileLayers;
		std::shared_ptr<shader::Shader>					m_imageShader;
		std::vector< std::shared_ptr<BackgroundLayer> >		m_bgLayers;
	};

	namespace map {
		///
		/// \brief map::load
		/// \param f
		/// \param mapFile
		///
		template<typename MapType, typename Functor>
		auto load(Functor f, const std::string& mapFile, bool repeat) {
			return MapType(mapFile, [f,repeat](const std::string& imageFile) {
				return f.loadTexture(imageFile, repeat);
			});
		}
	}
}
