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
#include <hungerland/math.h>
#include <map>

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
namespace map {
	struct LayerSubset {
		float opacity = 1.0f;
		int2d_t offset = {0,0};
		bool used = false;
		//std::vector<float> tintColor;
		glm::vec2 parallaxFactor = {1,1};
	};

	struct TileSetSubset : public LayerSubset {
		std::shared_ptr<mesh::Mesh> mesh;
		std::shared_ptr<texture::Texture> tileMap;
		std::shared_ptr<texture::Texture> colorLookup;
		size2d_t tileSize = {0,0};
		size2d_t tilesetSize  = {0,0};
	};

	struct ObjectSubset : public LayerSubset {
		std::shared_ptr<mesh::Mesh> mesh;
		std::shared_ptr<texture::Texture> texture;
		size2d_t size;
		std::vector<float> objectColor;
	};

	struct ImageSubset : public LayerSubset {
		std::shared_ptr<mesh::Mesh> mesh;
		std::shared_ptr<texture::Texture> texture;
		std::vector<float> transparentColor;
		size2d_t size;
		size2d_t repeat;
	};

	class TileLayer {
	public:
		std::vector<size2d_t>	objects;
		std::vector<TileSetSubset>	subsets;
		std::vector< std::vector<int> > tileIds;
		std::vector< std::vector<int> > tileFlags;
		TileLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<texture::Texture> >& tilesetTextures);
	};

	class ImageLayer {
	public:
		ImageSubset subset;
		const tmx::ImageLayer* tmxLayer;
		ImageLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<texture::Texture> >& mapTextures);
	};

	///
	/// \brief The hungerland::map::Map class
	///
	class Map {
	public:
		typedef std::function<std::shared_ptr<texture::Texture>(const std::string&)> LoadTextureFuncType;

		Map(const std::string& mapFilename, LoadTextureFuncType loadTexture);

		size2d_t getMapSize() const;
		size2d_t getTileSize() const;
		const size_t getNumLayers() const;
		size_t getLayerIndex(const std::string& name) const;
		const int getTileId(size_t layer, size_t x, size_t y) const;

		const auto& getImageLayers() const {
			return m_bgLayers;
		}

		const auto& getTileLayers() const {
			return m_tileLayers;
		}

		auto getClearColor() const {
			return m_clearColor;
		}

		const auto& getAllLayers() const {
			return m_allLayersMap;
		}

		glm::vec3 checkCollision(const glm::vec3 position, glm::vec3 halfSize) const;


	public:
		std::shared_ptr<shader::Shader>						m_tileLayerShader;
		std::shared_ptr<shader::Shader>						m_imageLayerShader;
		//std::shared_ptr<mesh::Mesh>							m_mapMesh;
	private:
		glm::vec4											m_clearColor;
		std::shared_ptr<tmx::Map>							m_map;
		std::vector< std::shared_ptr<texture::Texture> >	m_tilesetTextures;
		std::vector< std::shared_ptr<texture::Texture> >	m_imageTextures;
		std::vector< std::shared_ptr<TileLayer> >			m_tileLayers;
		std::vector< std::shared_ptr<ImageLayer> >			m_bgLayers;
		std::map<std::string, size_t> m_layerNames;
		std::vector< std::array<size_t,2> > m_allLayersMap;
	};

	///
	/// \brief hungerland::map::load
	/// \param f
	/// \param mapFile
	///
	template<typename MapType, typename Functor>
	std::shared_ptr<MapType> load(Functor f, const std::string& mapFile, bool repeat) {
		return std::make_shared<MapType>(mapFile, [f,repeat](const std::string& imageFile) {
			return f.loadTexture(imageFile, repeat);
		});
	}

	///
	/// \brief render
	/// \param map
	/// \param projectionMatrix
	/// \param cameraDelta
	///
	void draw(const Map& map, const std::vector<float>& projectionMatrix, const glm::vec2& cameraDelta);

}
}
