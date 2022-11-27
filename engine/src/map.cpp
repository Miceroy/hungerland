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
		const tmx::TileLayer& layer = *dynamic_cast<tmx::TileLayer*>(map.getLayers()[layerIndex].get());
		util::INFO("Creating map layer: index="+std::to_string(layerIndex)+", type=TileLayer, Name=\"" + layer.getName() + "\"");
		const auto& layerSize =  layer.getSize();
		const auto& layerTiles = layer.getTiles();
		const auto& tilesets = map.getTilesets();
		tileIds = util::gridNM(layer.getSize().x,layer.getSize().y, 0);
		tileFlags = util::gridNM(layer.getSize().x,layer.getSize().y, 0);
		// Create objects from non zero tile ids:
		for(auto ly = 0u; ly < layerSize.y; ++ly) {
			for(auto lx = 0u; lx < layerSize.x; ++lx) {
				auto tileIndex = ly * layerSize.x + lx;
				auto layerTile = layerTiles[tileIndex];
				if(layerTile.ID > 0) {
					tileIds[layerSize.y-ly-1][lx] = layerTile.ID;
					tileFlags[tileIds.size()-ly-1][lx] = layerTile.flipFlags;
					objects.push_back({lx,layerSize.y-ly-1});
				}
			}
		}
		util::INFO("Layer index="+std::to_string(layerIndex)+" has "+std::to_string(objects.size())+" objects");
		for(const auto& o : objects){
			util::INFO("x=" + std::to_string(o.x) + "y=" + std::to_string(o.y));

		}

		std::vector<float> layerPixels;
		bool tsUsed = false;
		for(auto tilesetId = 0u; tilesetId < tilesets.size(); ++tilesetId) {
			// Check each tile ID to see if it falls in the current tile set
			const auto& tileSet = tilesets[tilesetId];
			const auto& layerTiles = layer.getTiles();
			std::vector<float> layerPixels;
			bool tsUsed = false;
			for(auto ly = 0u; ly < layerSize.y; ++ly) {
				for(auto lx = 0u; lx < layerSize.x; ++lx) {
					auto tileIndex = ly * layerSize.x + lx;
					auto layerTileID = layerTiles[tileIndex].ID;
					auto firstGID = tileSet.getFirstGID();
					auto lastGID = firstGID + tileSet.getTileCount() - 1;
					if(layerTileID >= firstGID && layerTileID <= lastGID) {
						auto tileId = layerTileID - firstGID + 1;
						// Red channel: making sure to index relative to the tileset
						layerPixels.push_back(tileId);
						// Green channel: tile flips are performed on the shader
						layerPixels.push_back(layerTiles[tileIndex].flipFlags);
						layerPixels.push_back(0);
						layerPixels.push_back(0);
						tsUsed = true;
					} else {
						// Pad with empty space
						layerPixels.push_back(0);
						layerPixels.push_back(0);
						layerPixels.push_back(0);
						layerPixels.push_back(0);
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
				subset.colorLookup = std::make_shared<texture::Texture>(layerSize.x, layerSize.y, 4, &layerPixels[0]);
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
		util::INFO("Creating map layer: index="+std::to_string(layerIndex)+", type=ImageLayer, Name=\"" + layer.getName() + "\"");
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
		float texScaleX = float(bounds.width)/float(subset.texture->getWidth());
		float texScaleY = float(bounds.height)/float(subset.texture->getHeight());
		subset.mesh = quad::createImage(bounds.left, bounds.top, bounds.width, bounds.height, texScaleX, texScaleY);
	}

	/// Map
	Map::Map(const std::string& mapFilename, LoadTextureFuncType loadTexture)
		: m_clearColor(0.5,0.5,0.5,1)
		, m_map(std::make_shared<tmx::Map>())
		, m_tileLayerShader(shaders::createTileLayer())
		, m_imageLayerShader(shaders::createImageLayer())	{
		// Load map
		if(false == m_map->load(mapFilename)) {
			util::ERROR("Failed to load map file: \"" + mapFilename + "\"!");
		}
		util::INFO("Loaded Tiled map: " + mapFilename);

		m_clearColor.r = m_map->getBackgroundColour().r/255.0f;
		m_clearColor.g = m_map->getBackgroundColour().g/255.0f;
		m_clearColor.b = m_map->getBackgroundColour().b/255.0f;
		m_clearColor.a = m_map->getBackgroundColour().a/255.0f;
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
				util::INFO("Creating map layer: index="+std::to_string(layerIndex)+", type=Group, Name=\"" + m_map->getLayers()[layerIndex]->getName() + "\"");
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				const tmx::ImageLayer& layer = *dynamic_cast<tmx::ImageLayer*>(m_map->getLayers()[layerIndex].get());
				auto imgPath = layer.getImagePath();
				if(imgPath.size() > 0) {
					auto texture = loadTexture(imgPath);
					texture->setRepeat(true);
					if(texture == 0) {
						util::ERROR("Failed to load image texture file: \"" + imgPath + "\"!");
					}
					util::INFO("Loaded image texture: " + imgPath);
					m_imageTextures.push_back(texture);
				} else {
					m_imageTextures.push_back(0);
				}
			} else if(layerType == tmx::Layer::Type::Object) {
				util::INFO("Creating map layer: index="+std::to_string(layerIndex)+", type=Object, Name=\"" + m_map->getLayers()[layerIndex]->getName() + "\"");
				util::WARNING("Object layers are not supported in tmx-maps");
			} else {
				util::INFO("Creating map layer: index="+std::to_string(layerIndex)+", type=Unknown, Name=\"" + m_map->getLayers()[layerIndex]->getName() + "\"");
				util::ERROR("Unknown layer type in tmx-map!");
			}
		}

		// Create a drawable object for each layer:
		std::vector<std::string> layerNames;
		for(auto i = 0u; i < layers.size(); ++i) {
			const auto layerType = layers[i]->getType();
			if(layerType == tmx::Layer::Type::Tile) {
				m_layerNames[layers[i]->getName()] = i;
				layerNames.push_back(layers[i]->getName());
				m_allLayersMap.push_back({0,m_tileLayers.size()});
				m_tileLayers.push_back(std::make_shared<TileLayer>(*m_map, i, m_tilesetTextures));
			} else if(layerType == tmx::Layer::Type::Group) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				m_layerNames[layers[i]->getName()] = i;
				layerNames.push_back(layers[i]->getName());
				m_allLayersMap.push_back({1,m_bgLayers.size()});
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

	glm::vec3 Map::checkCollision(const glm::vec3 position, glm::vec3 halfSize, float speed) const {
		// Ckeck map limits
		auto mapSize = getMapSize();
		mapSize.x -= 1;
		mapSize.y -= 1;
		float posX = position.x;
		float posY = position.y;
		glm::vec3 normal(0);
		if(posX < 0)			normal.x =  1;
		if(posX > mapSize.x)	normal.x = -1;
		if(posY < 0)			normal.y =  1;
		if(posY > mapSize.y)	normal.y = -1;
		if(glm::dot(normal,normal) > 0) {
			return speed*glm::normalize(normal);
		}

		// Check map layer collisions
		auto layer = getLayerIndex("PlatformTiles");
		posX += 0.5;
		posY += 0.5;
		normal = glm::vec3(0);
		auto left	= posX - 0.5;
		auto right	= posX + 0.5;
		auto bottom	= posY - 0.5;
		auto top	= posY + 0.5;
		// bottom half
		//util::INFO("x="+std::to_string(posX)+"y="+std::to_string(posY));
		if(getTileId(layer, right, bottom)) {
			normal.x -= 1;
			normal.y += 1;
			util::INFO("Collision r bottom");
		}
		if(getTileId(layer, posX, bottom)) {
			normal.x += 0;
			normal.y += 1;
			util::INFO("Collision x bottom");
		}
		if(getTileId(layer, left, bottom)) {
			normal.x += 1;
			normal.y += 1;
			util::INFO("Collision l bottom");
		}
		// top half
		if(getTileId(layer, left, top)) {
			normal.x += 1;
			normal.y -= 1;
			util::INFO("Collision l top");
		}
		if(getTileId(layer, posX, top)) {
			normal.x -= 0;
			normal.y -= 1;
			util::INFO("Collision x top");
		}
		if(getTileId(layer, right, top)) {
			normal.x -= 1;
			normal.y -= 1;
			util::INFO("Collision r top");
		}
		// Left / right
		if(getTileId(layer, left, posY)) {
			normal.x += 1;
			normal.y += 0;
			util::INFO("Collision l y");
		}
		if(getTileId(layer, right, posY)) {
			normal.x -= 1;
			normal.y -= 0;
			util::INFO("Collision r y");
		}

		auto sign = [](float v) {
			if(v==0) return 0.0f;
			return std::signbit(v) ? -1.0f : 1.0f;
		};

		if(glm::dot(normal,normal) > 0) {
			// Select biggest normal to be actual normal
			if(std::abs(normal.x) > std::abs(normal.y)) {
				normal.x = sign(normal.x);
				normal.y = 0;
			} else if(std::abs(normal.x) < std::abs(normal.y)) {
				normal.x = 0;
				normal.y = sign(normal.y);
			} else {
				normal.x = sign(normal.x);
				normal.y = sign(normal.y);
				normal = glm::normalize(normal);
			}
			util::INFO("Collision normal: <"+std::to_string(normal.x)+", "+std::to_string(normal.y)+">");
			return speed*normal;
		}
		return glm::vec3(0);
	}

	const size_t Map::getNumLayers() const {
		return m_tileLayers.size();
	}

	size_t Map::getLayerIndex(const std::string& name) const {
		auto it = m_layerNames.find(name);
		if(it == m_layerNames.end()) {
			util::ERROR("Required layer named \n"+name+"\n not found from map");
		}
		return it->second;
	}

	const int Map::getTileId(size_t layerId, size_t x, size_t y) const {
		auto tileLayerId = m_allLayersMap[layerId][1];
		assert(tileLayerId < m_tileLayers.size());
		const auto& layer = m_tileLayers[tileLayerId];
		return layer->tileIds[y][x];
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
	}

	void draw(const ImageLayer& layer, shader::ShaderPass shader, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		auto& subset = layer.subset;
		if(subset.used) {
			applyLayerSubset(subset, shader, matProjection, cameraDelta);
			shader.setUniform("repeat", subset.repeat.x, subset.repeat.y);
			shader.setUniform("image", 0);
			subset.texture->bind(0);
			assert(subset.mesh != 0);
			quad::drawImage(*subset.mesh);
		}
	}

	void draw(const TileLayer& layer, shader::ShaderPass shader, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
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
				quad::drawImage(*subset.mesh);
			}
		}
	}

	void draw(const Map& map, const std::vector<float>& matProjection, const glm::vec2& cameraDelta) {
		// Clear screen:
		auto clearColor = map.getClearColor();
		auto a = clearColor.a;
		clearColor *= a;
		clearColor.a = a;
		glClearColor(clearColor.r, clearColor.g, clearColor.b, clearColor.a);
		checkGLError();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		checkGLError();

		// Enable alpha blending:
		glEnable(GL_BLEND);
		checkGLError();
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		checkGLError();

		// Render all layers:
		for(size_t layerId=0; layerId<map.getAllLayers().size(); ++layerId) {
			auto type = map.getAllLayers()[layerId][0];
			auto index = map.getAllLayers()[layerId][1];
			if(type==0) {
				map.m_tileLayerShader->use([&](shader::ShaderPass shader) {
					draw(*map.getTileLayers()[index], shader, matProjection, cameraDelta);
				});
			} else if(type==1) {
				map.m_imageLayerShader->use([&](shader::ShaderPass shader) {
					draw(*map.getImageLayers()[index], shader, matProjection, cameraDelta);
				});
			}
		}
	}
}
}
