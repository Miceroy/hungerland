#include <hungerland/map.h>

#include <mikroplot/graphics.h>
#include <mikroplot/shader.h>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
#include <hungerland/util.h>
#include <glm/gtc/matrix_transform.hpp>
#include "gl_check.h"
#include "shader.inl"

namespace hungerland {
	MapLayer::MapLayer(const tmx::Map& map, const tmx::TileLayer& layer, const std::vector<std::shared_ptr<mikroplot::Texture> >& mapTextures) {
		const auto& mapSize = map.getTileCount();
		const auto& tilesets = map.getTilesets();
		m_tiles = mikroplot::gridNM(layer.getSize().x,layer.getSize().y, 0);
		for(auto i = 0u; i < tilesets.size(); ++i) {
			// Check each tile ID to see if it falls in the current tile set
			const auto& tileSet = tilesets[i];
			const auto& tiles = layer.getTiles();
			std::vector<float> pixels;
			bool tsUsed = false;
			for(auto y = 0u; y < mapSize.y; ++y) {
				for(auto x = 0u; x < mapSize.x; ++x) {
					auto tileIndex = y * mapSize.x + x;
					if(tileIndex < tiles.size()
						&& tiles[tileIndex].ID >= tileSet.getFirstGID()
						&& tiles[tileIndex].ID < (tileSet.getFirstGID() + tileSet.getTileCount())) {
						auto tileId = tiles[tileIndex].ID - tileSet.getFirstGID() + 1;
						m_tiles[m_tiles.size()-y-1][x] = tileId;
						// Red channel: making sure to index relative to the tileset
						pixels.push_back(tileId);
						// Green channel: tile flips are performed on the shader
						pixels.push_back(tiles[tileIndex].flipFlags);
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
				assert(i < mapTextures.size());
				Subset subset;
				subset.texture = mapTextures[i];
				subset.lookup = std::make_shared<mikroplot::Texture>(mapSize.x, mapSize.y, 4, &pixels[0]);
				subset.tileSize[0] =  tileSet.getTileSize().x;
				subset.tileSize[1] = tileSet.getTileSize().y;
				subset.tilesetCount[0] = tileSet.getColumnCount();
				assert(tileSet.getTileCount() % tileSet.getColumnCount() == 0);
				subset.tilesetCount[1] = tileSet.getTileCount()/tileSet.getColumnCount();
				subset.opacity = layer.getOpacity();
				m_subsets.push_back(subset);
			}
		}
	}


	MapLayer::~MapLayer() {

	}

	MapLayers::MapLayers(const tmx::Map& map, const std::vector< std::shared_ptr<mikroplot::Texture> >& mapTextures)
		: m_shader(std::make_shared<mikroplot::Shader>(tilemapVSSource(),tilemapFSSource()))
		, m_mapTextures(mapTextures) {
		auto bounds = map.getBounds();
		std::vector<float> positions = {
			bounds.left,				bounds.top,
			bounds.left + bounds.width, bounds.top,
			bounds.left,				bounds.top + bounds.height,
			bounds.left + bounds.width,	bounds.top + bounds.height,
		};
		std::vector<float> texCoords = {
			0.f, 0.f,
			1.f, 0.f,
			0.f, 1.f,
			1.f, 1.f
		};

		m_mesh = mikroplot::mesh::create(positions, 2, texCoords, 2);

		// Create a drawable object for each tile layer
		const auto& layers = map.getLayers();
		for(auto i = 0u; i < layers.size(); ++i) {
			const auto layerType = layers[i]->getType();
			if(layerType == tmx::Layer::Type::Tile) {
				const tmx::TileLayer& layer = *dynamic_cast<const tmx::TileLayer*>(map.getLayers()[i].get());
				m_layers.push_back(MapLayer(map, layer, mapTextures));
			} else if(layerType == tmx::Layer::Type::Group) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Image) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else if(layerType == tmx::Layer::Type::Object) {
				util::WARNING("Group layers are not supported in tmx-maps");
			} else {
				util::ERROR("Unknown layer type in tmx-map!");
			}
		}


	}


	void MapLayers::render(const std::vector<float>& projection) const {
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		m_shader->use([&,this](){
			m_shader->setUniformm("P", &projection[0], false);
			m_shader->setUniform("tileMap", 0);
			m_shader->setUniform("lookupMap", 1);
			for(auto& layer : m_layers) {
				for(const auto& ss : layer.m_subsets)	{
					// Set map properties
					m_shader->setUniform("u_tileSize", ss.tileSize[0], ss.tileSize[1]);
					m_shader->setUniform("u_tilesetCount", ss.tilesetCount[0], ss.tilesetCount[1]);
					m_shader->setUniform("u_opacity", ss.opacity);

					glActiveTexture(GL_TEXTURE0);
					mikroplot::checkGLError();
					glBindTexture(GL_TEXTURE_2D, ss.texture->getTextureId());
					mikroplot::checkGLError();

					glActiveTexture(GL_TEXTURE1);
					mikroplot::checkGLError();
					glBindTexture(GL_TEXTURE_2D, ss.lookup->getTextureId());
					mikroplot::checkGLError();

					mikroplot::mesh::render(*m_mesh, GL_TRIANGLE_STRIP, 4);
				}
			}
		});
	}
}
