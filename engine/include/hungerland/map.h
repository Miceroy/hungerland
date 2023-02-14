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
#include <hungerland/texture.h>
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

		const int getTileId(size_t layerId, size_t x, size_t y) const;

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

		typedef std::vector< std::vector<glm::vec3> > MapCollision;

		MapCollision checkCollision(const std::string& layerName, const glm::vec3 position, glm::vec3 halfSize) const;


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


	/*template<typename Func>
	static inline bool rowAnd(const Map::MapCollision& col, size_t y, Func f) {
		for(size_t x=0; x<col[y].size(); ++x){
			if(!f(col[y][x])) return false;
		}
		return true;
	}*/

	static inline auto getRowSum(const Map::MapCollision& col, size_t y) {
		float res = -1;
		if(y<col.size()) {
			for(size_t x=0; x<col[y].size(); ++x){
				if(col[y][x].y >= 0.0f) {
					if(res<0){
						res = col[y][x].y;
					} else {
						res += col[y][x].y;
					}
				}
			}
		}
		return res;
	}

	static inline auto getColSum(const Map::MapCollision& col, size_t x) {
		float res = -1;
		for(size_t y=0; y<col.size(); ++y) {
			if(col[y][x].x >= 0.0f) {
				if(res<0){
					res = col[y][x].x;
				} else {
					res += col[y][x].x;
				}
			}
		}
		return res;
	}

	/*template<typename Func>
	static inline bool colAnd(const Map::MapCollision& col, size_t x, Func f) {
		for(size_t y=0; y<col.size(); ++y){
			if(!f(col[y][x])) return false;
		}
		return true;
	}

	template<typename Func>
	static inline bool colOr(const Map::MapCollision& col, size_t x, Func f) {
		for(size_t y=0; y<col.size(); ++y){
			if(f(col[y][x])) return true;
		}
		return false;
	}*/

	///
	/// \brief isPenetrating
	/// \param col
	/// \return
	///
	bool isPenetrating(const Map::MapCollision& col);

	///
	/// \brief hungerland::map::load
	/// \param f
	/// \param mapFile
	///
	template<typename MapType, typename LoadTextureFunc>
	std::shared_ptr<MapType> load(LoadTextureFunc loadTexture, const std::string& mapFile, bool repeat) {
		return std::make_shared<MapType>(mapFile, [loadTexture,repeat](const std::string& imageFile) {
			auto texture = loadTexture(imageFile);
			texture->setRepeat(repeat);
			return texture;
		});
	}

	///
	/// \brief render
	/// \param map
	/// \param projectionMatrix
	/// \param cameraDelta
	///
	void draw(const Map& map, const glm::mat4& projectionMatrix, const glm::vec2& cameraDelta);

}
}
