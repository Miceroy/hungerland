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
#include <hungerland/map.h>
#include <hungerland/shader.h>
#include <hungerland/texture.h>
#include <hungerland/mesh.h>
#include <hungerland/util.h>
#include <hungerland/gl_utils.h>
#include <hungerland/graphics.h>
#include <glad/gl.h>

#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ImageLayer.hpp>

namespace hungerland {
namespace map {
	/// TileLayer
	TileLayer::TileLayer(const tmx::Map& map, size_t layerIndex, const std::vector<std::shared_ptr<texture::Texture> >& tilesetTextures) {
		const auto& mapSize = map.getTileCount();
		const auto& tilesets = map.getTilesets();
		const tmx::TileLayer& layer = *dynamic_cast<tmx::TileLayer*>(map.getLayers()[layerIndex].get());
		tileIds = util::gridNM(layer.getSize().x,layer.getSize().y, 0);
		tileFlags = util::gridNM(layer.getSize().x,layer.getSize().y, 0);
		for(auto tilesetId = 0u; tilesetId < tilesets.size(); ++tilesetId) {
			// Check each tile ID to see if it falls in the current tile set
			const auto& tileSet = tilesets[tilesetId];
			const auto& layerTiles = layer.getTiles();
			std::vector<float> mapPixels;
			bool tsUsed = false;
			for(auto y = 0u; y < mapSize.y; ++y) {
				for(auto x = 0u; x < mapSize.x; ++x) {
					auto tileIndex = y * mapSize.x + x;
					auto layerTileID = layerTiles[tileIndex].ID;
					auto firstGID = tileSet.getFirstGID();
					auto lastGID = firstGID + tileSet.getTileCount() - 1;
					if(layerTileID >= firstGID && layerTileID <= lastGID) {
						auto tileId = layerTileID - firstGID + 1;
						tileIds[tileIds.size()-y-1][x] = tileId;
						tileFlags[tileIds.size()-y-1][x] = layerTiles[tileIndex].flipFlags;
						// Red channel: making sure to index relative to the tileset
						mapPixels.push_back(tileId);
						// Green channel: tile flips are performed on the shader
						mapPixels.push_back(layerTiles[tileIndex].flipFlags);
						mapPixels.push_back(0);
						mapPixels.push_back(0);
						tsUsed = true;
					} else {
						// Pad with empty space
						mapPixels.push_back(0);
						mapPixels.push_back(0);
						mapPixels.push_back(0);
						mapPixels.push_back(0);
					}
				}
			}

			//if we have some data for this tile set, create the resources
			if(tsUsed) {
				auto bounds = map.getBounds();
				assert(tilesetId < tilesetTextures.size());
				TileSetSubset subset;
				subset.used = true;
				subset.opacity = layer.getOpacity();
				subset.offset.x = layer.getOffset().x;
				subset.offset.y = layer.getOffset().y;
				subset.tileMap = tilesetTextures[tilesetId];
				subset.colorLookup = std::make_shared<texture::Texture>(mapSize.x, mapSize.y, 4, &mapPixels[0]);
				subset.tileSize.x =  tileSet.getTileSize().x;
				subset.tileSize.y = tileSet.getTileSize().y;
				subset.tilesetSize.x = tileSet.getColumnCount();
				assert(tileSet.getTileCount() % tileSet.getColumnCount() == 0);
				subset.tilesetSize.y = tileSet.getTileCount()/tileSet.getColumnCount();

				subset.mesh = quad::createImage(bounds.left, bounds.top, bounds.width, bounds.height);
				subsets.push_back(subset);
			} else {
				subsets.push_back(TileSetSubset()); // Un used subset
			}
		}
	}


	/// ImageLayer
	ImageLayer::ImageLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<texture::Texture> >& imageTextures) {
		const tmx::ImageLayer& layer = *dynamic_cast<tmx::ImageLayer*>(map.getLayers()[layerIndex].get());
		subset.used = true;
		subset.opacity = layer.getOpacity();
		subset.texture = imageTextures[layerIndex];
		subset.offset.x = layer.getOffset().x;
		subset.offset.y = layer.getOffset().y;
		subset.repeat.x = layer.getRepeat().x;
		subset.repeat.y = layer.getRepeat().y;
		subset.parallaxFactor.x = layer.getParallax().x;
		subset.parallaxFactor.y = layer.getParallax().y;
		if(layer.hasTransparency()) {
			subset.transparentColor.push_back(layer.getTransparencyColour().r);
			subset.transparentColor.push_back(layer.getTransparencyColour().g);
			subset.transparentColor.push_back(layer.getTransparencyColour().b);
			subset.transparentColor.push_back(layer.getTransparencyColour().a);
		}
		// Create mesh
		auto texture = imageTextures[layerIndex];
		auto bounds = map.getBounds();
		subset.mesh = quad::createImage(bounds.left, bounds.top, bounds.width, bounds.height);
	}

	/// Map
	Map::Map(const std::string& mapFilename, LoadTextureFuncType loadTexture)
		: m_map(std::make_shared<tmx::Map>())
		, m_tileLayerShader(shaders::tileLayer())
		, m_imageLayerShader(shaders::imageLayer())	{
		// Load map
		if(false == m_map->load(mapFilename)) {
			util::ERROR("Failed to load map file: \"" + mapFilename + "\"!");
		}
		util::INFO("Loaded Tiled map: " + mapFilename);

		// Create tileset textures from map tilesets:
		for(const auto& ts : m_map->getTilesets()) {
			auto texture = loadTexture(ts.getImagePath());
			if(texture == 0) {
				util::ERROR("Failed to load tileset texture file: \"" + ts.getImagePath() + "\"!");
			}
			util::INFO("Loaded tileset texture: " + ts.getImagePath());
			m_tilesetTextures.push_back(texture);
		}

		// Create all rest textures for each layers:
		const auto& layers = m_map->getLayers();
		for(auto layerIndex = 0u; layerIndex < layers.size(); ++layerIndex) {
			const auto layerType = layers[layerIndex]->getType();
			if(layerType == tmx::Layer::Type::Tile) {

			} else if(layerType == tmx::Layer::Type::Group) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				const tmx::ImageLayer& layer = *dynamic_cast<tmx::ImageLayer*>(m_map->getLayers()[layerIndex].get());
				auto imgPath = layer.getImagePath();
				if(imgPath.size() > 0) {
					auto texture = loadTexture(imgPath);
					if(texture == 0) {
						util::ERROR("Failed to load image texture file: \"" + imgPath + "\"!");
					}
					util::INFO("Loaded image texture: " + imgPath);
					m_imageTextures.push_back(texture);
				} else {
					m_imageTextures.push_back(0);
				}
			} else if(layerType == tmx::Layer::Type::Object) {
				util::WARNING("Object layers are not supported in tmx-maps");
			} else {
				util::ERROR("Unknown layer type in tmx-map!");
			}
		}

		// Create a drawable object for each layer:
		for(auto i = 0u; i < layers.size(); ++i) {
			const auto layerType = layers[i]->getType();
			if(layerType == tmx::Layer::Type::Tile) {
				m_tileLayers.push_back(std::make_shared<TileLayer>(*m_map, i, m_tilesetTextures));
			} else if(layerType == tmx::Layer::Type::Group) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				m_bgLayers.push_back(std::make_shared<ImageLayer>(*m_map, i, m_imageTextures));
			} else if(layerType == tmx::Layer::Type::Object) {
				util::WARNING("Object layers are not supported in tmx-maps");
			} else {
				util::ERROR("Unknown layer type in tmx-map!");
			}
		}
	}

	size2d_t Map::getTileSize() const {
		auto& s = m_map->getTileSize();
		return {s.x, s.y};
	}

	size2d_t Map::getMapSize() const {
		auto& s = m_map->getTileCount();
		return {s.x, s.y};
	}

	const size_t Map::getNumLayers() const {
		return m_tileLayers.size();
	}

	const int Map::getTileId(size_t layer, size_t x, size_t y) const {
		return m_tileLayers[layer]->tileIds[y][x];
	}

	template<typename Subset>
	void applyLayerSubset(const Subset& subset, shader::ShaderPass shader, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		assert(subset.used);

		float paralX = 0;
		if(subset.parallaxFactor.x == 1.0f) {
			paralX = 0;
		} else if(subset.parallaxFactor.x < 1.0f) {
			paralX = (subset.parallaxFactor.x/1.0) * cameraDelta.x;
		} else {
			paralX = subset.parallaxFactor.x * cameraDelta.x;
		}
		float paralY = 0;
		if(subset.parallaxFactor.y == 1.0f) {
			paralY = 0;
		} else if(subset.parallaxFactor.y < 1.0f) {
			paralY = (subset.parallaxFactor.y/1.0) * cameraDelta.y;
		} else {
			paralY = subset.parallaxFactor.y * cameraDelta.y;
		}
		// Set map properties
		shader.setUniformm("P",			&matProjection[0], false);
		shader.setUniform( "offset",	subset.offset.x, subset.offset.y);
		shader.setUniform( "opacity",	subset.opacity);
		shader.setUniform( "parallax",	-paralX, paralY);

		assert(subset.mesh != 0);
		mesh::draw(*subset.mesh, GL_TRIANGLE_STRIP, 4);
	}

	void drawImageLayer(const Map& map, const ImageLayer& layer, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		map.m_imageLayerShader->use([&](shader::ShaderPass shader) {
			auto& subset = layer.subset;
			if(subset.used) {
				applyLayerSubset(subset, shader, matProjection, cameraDelta);
				shader.setUniform("repeat", subset.repeat.x, subset.repeat.y);
				shader.setUniform("image", 0);
				subset.texture->bind(0);

				assert(subset.mesh != 0);
				mesh::draw(*subset.mesh, GL_TRIANGLE_STRIP, 4);
			}
		});
	}

	void drawTileLayer(const Map& map, const TileLayer& layer, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		map.m_tileLayerShader->use([&](shader::ShaderPass shader) {
			for(const auto& subset : layer.subsets)	{
				if(subset.used) {
					applyLayerSubset(subset, shader, matProjection, cameraDelta);
					shader.setUniform("tileSize", subset.tileSize.x, subset.tileSize.y);
					shader.setUniform("tilesetSize", subset.tilesetSize.x, subset.tilesetSize.y);
					shader.setUniform("lookupMap", 0);
					subset.colorLookup->bind(0);
					shader.setUniform("tileMap", 1);
					subset.tileMap->bind(1);
					assert(subset.mesh != 0);
					mesh::draw(*subset.mesh, GL_TRIANGLE_STRIP, 4);
				}
			}
		});
	}

	void draw(const Map& map, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		glEnable(GL_BLEND);
		checkGLError();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		checkGLError();
		//glBlendEquation(GL_FUNC_ADD);
		//checkGLError();

		glClearColor(0,0,0,1);
		checkGLError();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();

		for(const auto& layer : map.getImageLayers()) {
			drawImageLayer(map, *layer, matProjection, cameraDelta);
		}

		for(auto& layer : map.getTileLayers()) {
			drawTileLayer(map, *layer, matProjection, cameraDelta);
		}


	}
}
}
