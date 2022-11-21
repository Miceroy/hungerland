#pragma once
#include <mikroplot/window.h>
#include <mikroplot/texture.h>
#include <memory>
#include <tmxlite/Map.hpp>
#include <tmxlite/TileLayer.hpp>
/*namespace tmx {
	class TileLayer;
	class Map;
}*/

namespace hungerland {

class MapLayer {
public:
	struct Subset {
		std::shared_ptr<mikroplot::Texture> lookup;
		std::shared_ptr<mikroplot::Texture> texture;
		std::array<float,2> tileSize = {96.0f, 96.0f};
		std::array<float,2> tilesetCount = {5.0f, 3.0f};
		float opacity = 1.0f;
	};

	MapLayer(const tmx::Map& map, const tmx::TileLayer& layer, const std::vector< std::shared_ptr<mikroplot::Texture> >& mapTextures);
	~MapLayer();

	std::vector<Subset>	m_subsets;
	mikroplot::Grid m_tiles;
};


class MapLayers {
public:
	MapLayers() {}
	MapLayers(const tmx::Map& map, const std::vector< std::shared_ptr<mikroplot::Texture> >& mapTextures);
	~MapLayers() {}
	void render(const std::vector<float>& projectionMatrix) const;

private:
	std::shared_ptr<mikroplot::Shader>					m_shader;
	std::vector< std::shared_ptr<mikroplot::Texture> >	m_mapTextures;
	std::shared_ptr<mikroplot::mesh::Mesh>				m_mesh;
	std::vector<MapLayer>								m_layers;
};

}
