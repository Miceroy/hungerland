#include <hungerland/map.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <mikroplot/graphics.h>
#include <mikroplot/shader.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <tmxlite/ImageLayer.hpp>
#include <hungerland/util.h>
#include <glm/gtc/matrix_transform.hpp>
#include "gl_check.h"

namespace hungerland {
	static std::string spriteVSSource(){
		return
			std::string("#version 330 core\n") +
			"layout (location = 0) in vec2 inPosition;\n"
			"layout (location = 1) in vec4 inTexCoord;\n"
			"uniform mat4 P;\n"
			"uniform vec2 offset = vec2(0, 0);\n"
			"uniform vec2 parallax = vec2(1, 1);\n"
			"out vec2 texCoord;"
			"out vec2 worldPos;"
			"void main() {\n"
			"    vec4 pos = vec4(inPosition.x+parallax.x+offset.x,inPosition.y+parallax.y+offset.y, 0.0, 1.0);\n"
			"    gl_Position = P*pos;\n"
			"    texCoord  = inTexCoord.xy;\n"
			"    worldPos.x = inTexCoord.z;\n"
			"    worldPos.y = inTexCoord.w;\n"
			"}\n";
	}

	static std::string gbShader() {
		return
			std::string("#version 330 core\n") +
			"in vec2 texCoord;\n"
			"uniform sampler2D image;\n"
			"uniform float u_opacity;\n"
			"uniform vec2 u_repeat = vec2(0);\n"
			"void main() {\n"
			"	vec4 color = vec4(0, 0, 0, 0);\n"
			"   float doPixel = 1;\n"
			"	if(u_repeat.x == 0 && (texCoord.x < 0.0 || texCoord.x > 1.0)) { \n"
			"	     doPixel = 0;\n"
			"	}"
			"	if(u_repeat.y == 0 && (texCoord.y < 0.0 || texCoord.y > 1.0)) { \n"
			"	     doPixel = 0;\n"
			"	}"
			"   if(doPixel > 0.5) { \n"
			"        color = texture(image, texCoord);\n"
			"	}\n"
			"   color.a = min(color.a, u_opacity);\n"
			"	gl_FragData[0] = color;\n"
			"}\n";
	}

	static std::string imageShader() {
		return
			std::string("#version 330 core\n") +
			"in vec2 texCoord;\n"
			"uniform sampler2D image;\n"
			"uniform float u_opacity;\n"
			"void main() {\n"
			"	vec4 color = vec4(0, 0, 0, 0);\n"
			"	color = texture(image, texCoord);\n"
			"	color.a = min(color.a, u_opacity);\n"
			"	gl_FragData[0] = color;\n"
			"}\n";
	}

	static std::string tilemapShader() {
		return
			std::string("#version 330 core\n") +
			"#define FLIP_HORIZONTAL 8u\n"
			"#define FLIP_VERTICAL 4u\n"
			"#define FLIP_DIAGONAL 2u\n"
			"in vec2 texCoord;\n"
			"uniform sampler2D tileMap;\n"
			"uniform sampler2D lookupMap;\n"
			"uniform vec2 u_tileSize;\n"
			"uniform vec2 u_tilesetCount ;\n"
			"uniform float u_opacity;\n"
			// Fixes rounding imprecision on AMD cards
			"const float epsilon = 0.000005;\n"
			"void main() {\n"
			"	vec4 values = texture(lookupMap, texCoord);\n"
			"	float flip = values.g;\n"
			"	if(values.r > 0.0) {\n"
			"		float index = float(values.r) - 1.0;\n"
			"		vec2 position = vec2(mod(index + epsilon, u_tilesetCount.x), floor((index / u_tilesetCount.x) + epsilon)) / u_tilesetCount;\n"

			"		vec2 texelSize = vec2(1.0) / textureSize(lookupMap, 0);\n"
			"		vec2 offset = mod(texCoord, texelSize);\n"
			"		vec2 ratio = offset / texelSize;\n"
			"		offset = ratio * (1.0 / u_tileSize);\n"
			"		offset *= u_tileSize / u_tilesetCount;\n"
	#if 1
			"		vec2 tileSize = vec2(1.0) / u_tilesetCount;\n"
			"		if(flip == FLIP_DIAGONAL) {\n"
			"			float temp = offset.x;\n"
			"			offset.x = offset.y;\n"
			"			offset.y = temp;\n"
			"			temp = tileSize.x / tileSize.y;\n"
			"			offset.x *= temp;\n"
			"			offset.y /= temp;\n"
			"			offset.x = tileSize.x - offset.x;\n"
			"			offset.y = tileSize.y - offset.y;\n"
			"		} else if(flip == FLIP_VERTICAL) {\n"
			"			offset.y = tileSize.y - offset.y;\n"
			"		} else if(flip == FLIP_HORIZONTAL) {\n"
			"			offset.x = tileSize.x - offset.x;\n"
			"		}\n"
	#endif
			"		vec4 color = texture(tileMap, position + offset);"
			"		color.a = min(color.a, u_opacity);\n"
			"		gl_FragData[0] = color;\n"
			"	} else {\n"
			"		gl_FragData[0] = vec4(0,0,0,0);\n"
			"	}\n"
			"}";
	}

	TileMap::TileLayer::TileLayer(const tmx::Map& map, size_t layerIndex, const std::vector<std::shared_ptr<mikroplot::Texture> >& tilesetTextures) {
		const auto& mapSize = map.getTileCount();
		const auto& tilesets = map.getTilesets();
		const tmx::TileLayer& layer = *dynamic_cast<tmx::TileLayer*>(map.getLayers()[layerIndex].get());
		tileIds = mikroplot::gridNM(layer.getSize().x,layer.getSize().y, 0);
		tileFlags = mikroplot::gridNM(layer.getSize().x,layer.getSize().y, 0);
		for(auto tilesetId = 0u; tilesetId < tilesets.size(); ++tilesetId) {
			// Check each tile ID to see if it falls in the current tile set
			const auto& tileSet = tilesets[tilesetId];
			const auto& layerTiles = layer.getTiles();
			std::vector<float> pixels;
			bool tsUsed = false;
			for(auto y = 0u; y < mapSize.y; ++y) {
				for(auto x = 0u; x < mapSize.x; ++x) {
					auto tileIndex = y * mapSize.x + x;
					assert(tileIndex < layerTiles.size());
					auto layerTileID = layerTiles[tileIndex].ID;
					auto firstGID = tileSet.getFirstGID();
					auto lastGID = firstGID + tileSet.getTileCount() - 1;
					if(layerTileID >= firstGID && layerTileID <= lastGID) {
						auto tileId = layerTileID - firstGID + 1;
						tileIds[tileIds.size()-y-1][x] = tileId;
						tileFlags[tileIds.size()-y-1][x] = layerTiles[tileIndex].flipFlags;
						// Red channel: making sure to index relative to the tileset
						pixels.push_back(tileId);
						// Green channel: tile flips are performed on the shader
						pixels.push_back(layerTiles[tileIndex].flipFlags);
						pixels.push_back(0);
						pixels.push_back(0);
						tsUsed = true;
					} else {
						// Pad with empty space
						pixels.push_back(0);
						pixels.push_back(0);
						pixels.push_back(0);
						pixels.push_back(0);
					}
				}
			}

			//if we have some data for this tile set, create the resources
			if(tsUsed) {
				assert(tilesetId < tilesetTextures.size());
				TileSubset subset;
				subset.used = true;
				subset.texture = tilesetTextures[tilesetId];
				subset.lookup = std::make_shared<mikroplot::Texture>(mapSize.x, mapSize.y, 4, &pixels[0]);
				subset.tileSize.x =  tileSet.getTileSize().x;
				subset.tileSize.y = tileSet.getTileSize().y;
				subset.tilesetCount.x = tileSet.getColumnCount();
				assert(tileSet.getTileCount() % tileSet.getColumnCount() == 0);
				subset.tilesetCount.y = tileSet.getTileCount()/tileSet.getColumnCount();
				subset.opacity = layer.getOpacity();
				subset.offset.x = layer.getOffset().x;
				subset.offset.y = layer.getOffset().y;
				subsets.push_back(subset);
			} else {
				subsets.push_back(TileSubset()); // Un used subset
			}
		}
	}


	TileMap::TileLayer::~TileLayer() {

	}


	void TileMap::TileLayer::render(mikroplot::Shader* shader, mikroplot::mesh::Mesh* mesh) const {
		for(const auto& ss : subsets)	{
			if(ss.used) {
				// Set map properties
				shader->setUniform("offset", ss.offset.x, ss.offset.y);
				shader->setUniform("tileMap", 0);
				shader->setUniform("lookupMap", 1);
				shader->setUniform("u_tileSize", ss.tileSize.x, ss.tileSize.y);
				shader->setUniform("u_tilesetCount", ss.tilesetCount.x, ss.tilesetCount.y);
				shader->setUniform("u_opacity", ss.opacity);

				glActiveTexture(GL_TEXTURE0);
				mikroplot::checkGLError();
				glBindTexture(GL_TEXTURE_2D, ss.texture->getTextureId());
				mikroplot::checkGLError();

				glActiveTexture(GL_TEXTURE1);
				mikroplot::checkGLError();
				glBindTexture(GL_TEXTURE_2D, ss.lookup->getTextureId());
				mikroplot::checkGLError();

				mikroplot::mesh::render(*mesh, GL_TRIANGLE_STRIP, 4);
			}
		}
	}

	// ImageLayer

	TileMap::BackgroundLayer::BackgroundLayer(const tmx::Map& map, size_t layerIndex, const std::vector< std::shared_ptr<mikroplot::Texture> >& imageTextures) {
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
		auto bounds = map.getBounds();
		auto texture = imageTextures[layerIndex];
		std::vector<float> positions = {
			bounds.left,				bounds.top,
			bounds.left + bounds.width, bounds.top,
			bounds.left,				bounds.top + bounds.height,
			bounds.left + bounds.width,	bounds.top + bounds.height,
		};
		float sx=float(bounds.width)/texture->getWidth();
		float sy=1.075*float(bounds.height)/texture->getHeight();
		float wx=float(bounds.width) - bounds.left;
		float wy=float(bounds.height) - bounds.top;
		std::vector<float> texCoords = {
			0.f, 0.f,   0.0f, 0.0f,
			sx,  0.f,   wx,   0.0f,
			0.f, sy,    0.0f, wy,
			sx,  sy,    wx,   wy,
		};
		subset.mesh = mikroplot::mesh::create(positions, 2, texCoords, 4);
	}

	TileMap::BackgroundLayer::~BackgroundLayer() {

	}

	void TileMap::BackgroundLayer::render(mikroplot::Shader* shader, const glm::vec2& cameraDelta) const {
		if(subset.used) {
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
			shader->setUniform("image", 0);
			shader->setUniform("offset", subset.offset.x, subset.offset.y);
			shader->setUniform("u_opacity", subset.opacity);
			shader->setUniform("u_repeat", subset.repeat.x, subset.repeat.y);
			shader->setUniform("parallax", -paralX, paralY);
			glActiveTexture(GL_TEXTURE0);
			mikroplot::checkGLError();
			glBindTexture(GL_TEXTURE_2D, subset.texture->getTextureId());
			mikroplot::checkGLError();

			mikroplot::mesh::render(*subset.mesh, GL_TRIANGLE_STRIP, 4);
		}
	}





	TileMap::TileMap(const std::string& mapFilename, std::function<mikroplot::Texture::Ptr(const std::string&)> loadTexture)
		: m_map(std::make_shared<tmx::Map>())
		, m_tileShader(std::make_shared<mikroplot::Shader>(spriteVSSource(),tilemapShader()))
		, m_imageShader(std::make_shared<mikroplot::Shader>(spriteVSSource(),gbShader()))	{
		// Load map
		if(false == m_map->load(mapFilename)) {
			util::ERROR("Failed to load map file: \"" + mapFilename + "\"!");
		}
		util::INFO("Loaded Tiled map: " + mapFilename);

		// Create tileset textures from map tilesets.
		for(const auto& ts : m_map->getTilesets()) {
			auto texture = loadTexture(ts.getImagePath());
			if(texture == 0) {
				util::ERROR("Failed to load tileset texture file: \"" + ts.getImagePath() + "\"!");
			}
			util::INFO("Loaded tileset texture: " + ts.getImagePath());
			m_tilesetTextures.push_back(texture);
		}

		{
			auto bounds = m_map->getBounds();
			std::vector<float> positions = {
				bounds.left,				bounds.top,
				bounds.left + bounds.width, bounds.top,
				bounds.left,				bounds.top + bounds.height,
				bounds.left + bounds.width,	bounds.top + bounds.height,
			};
			float sx=1;
			float sy=1;
			float wx=float(bounds.width);
			float wy=float(bounds.height);
			std::vector<float> texCoords = {
				0.f, 0.f,   0.0f, 0.0f,
				sx,  0.f,   wx,   0.0f,
				0.f, sy,    0.0f, wy,
				sx,  sy,    wx,   wy,
			};
			m_mapMesh = mikroplot::mesh::create(positions, 2, texCoords, 4);
		}

		const auto& layers = m_map->getLayers();
		// Create all rest textures for each layers
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

		// Create a drawable object for each layer
		for(auto i = 0u; i < layers.size(); ++i) {
			const auto layerType = layers[i]->getType();
			if(layerType == tmx::Layer::Type::Tile) {
				m_tileLayers.push_back(std::make_shared<TileLayer>(*m_map, i, m_tilesetTextures));
			} else if(layerType == tmx::Layer::Type::Group) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				m_bgLayers.push_back(std::make_shared<BackgroundLayer>(*m_map, i, m_imageTextures));
			} else if(layerType == tmx::Layer::Type::Object) {
				util::WARNING("Object layers are not supported in tmx-maps");
			} else {
				util::ERROR("Unknown layer type in tmx-map!");
			}
		}
	}


	void TileMap::render(const std::vector<float>& projection, const glm::vec2& cameraDelta) const {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);

		for(auto& layer : m_bgLayers) {
			m_imageShader->use([&,this](){
				m_imageShader->setUniformm("P", &projection[0], false);
				m_imageShader->setUniform("offset", 0, 0);
				layer->render(m_imageShader.get(), cameraDelta);
			});
		}

		for(auto& layer : m_tileLayers) {
			m_tileShader->use([&,this](){
				m_tileShader->setUniformm("P", &projection[0], false);
				m_tileShader->setUniform("offset", 0, 0);
				layer->render(m_tileShader.get(), m_mapMesh.get());
			});
		}


	}

	size2d_t TileMap::getTileSize() const {
		auto& s = m_map->getTileSize();
		return {s.x, s.y};
	}

	size2d_t TileMap::getMapSize() const {
		auto& s = m_map->getTileCount();
		return {s.x, s.y};
	}

	const size_t TileMap::getNumLayers() const {
		return m_tileLayers.size();
	}

	const int TileMap::getTileId(size_t layer, size_t x, size_t y) const {
		return m_tileLayers[layer]->tileIds[y][x];
	}

}
