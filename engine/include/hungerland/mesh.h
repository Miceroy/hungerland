#pragma once
#include <hungerland/math.h>
#include <hungerland/shader.h>
#include <memory>
#include <string>
#include <vector>
#include <memory>

namespace hungerland {
namespace mesh {
	///
	/// \brief The Mesh class
	///
	struct Mesh {
		~Mesh() {
			release();
		}
		unsigned vao;
		unsigned vbos[2];

		void setVBOData(int index, const std::vector<float>& data, size_t numComponents);
		void setVBOData(int index, const std::vector<glm::vec2>& data);
		void release();
	};

	///
	/// \brief create
	/// \param positions
	/// \param textureCoords
	/// \return
	///
	std::shared_ptr<Mesh> create(const std::vector<glm::vec2>& positions, const std::vector<glm::vec2>& textureCoords);

	///
	/// \brief create
	/// \param positions
	/// \param numPositionComponents
	/// \param textureCoords
	/// \param numTexCoordComponents
	/// \return
	///
	std::shared_ptr<Mesh> create(const std::vector<float>& positions, size_t numPositionComponents, const std::vector<float>& textureCoords, size_t numTexCoordComponents);

	///
	/// \brief render
	/// \param mesh
	/// \param mode
	/// \param count
	///
	void render(const Mesh& mesh, int mode, unsigned count);

} // End - namespace mesh

namespace quad {
	///
	/// \brief create
	/// \return mesh::Mesh
	///
	std::shared_ptr<mesh::Mesh> create();

	///
	/// \brief render
	/// \param mesh
	///
	void render(const mesh::Mesh& mesh);

	///
	/// \brief setPositions
	/// \param mesh
	/// \param positions
	///
	void setPositions(mesh::Mesh& mesh, const std::vector<glm::vec2>& positions);
} // End - namespace quad

}
