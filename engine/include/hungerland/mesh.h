#pragma once
#include <hungerland/math.h>
#include <hungerland/shader.h>
#include <memory>
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
#include <string>
#include <vector>
#include <memory>

namespace hungerland {
namespace mesh {
	///
	/// \brief The Mesh class
	///
	/// @ingroup hungerland::mesh
	/// @author Mikko Romppainen (kajakbros@gmail.com)
	///
	class Mesh {
	public:
		typedef std::shared_ptr<Mesh> Ref;
		~Mesh();
		unsigned vao;
		unsigned vbos[2];

		void setVBOData(int index, const std::vector<float>& data, size_t numComponents, bool dynamic = false);
		void setVBOData(int index, const std::vector<glm::vec2>& data, bool dynamic = false);
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
	/// \brief setPositions
	/// \param mesh
	/// \param positions
	///
	void setPositions(mesh::Mesh& mesh, const std::vector<glm::vec2>& positions);

	///
	/// \brief setTexCoords
	/// \param mesh
	/// \param texCoords
	///
	void setTexCoords(mesh::Mesh& mesh, const std::vector<glm::vec2>& texCoords);

	///
	/// \brief render
	/// \param mesh
	/// \param mode
	/// \param count
	///
	void draw(const Mesh& mesh, int mode, unsigned count);

} // End - namespace mesh

namespace quad {
	///
	/// \brief createSprite
	/// \return mesh::Mesh
	///
	std::shared_ptr<mesh::Mesh> createSprite(float halfSizeX, float halfSizeY);

	///
	/// \brief createImage
	/// \return mesh::Mesh
	///
	std::shared_ptr<mesh::Mesh> createImage(float originX, float originY, float sizeX, float sizeY, float texScaleX=1, float texScaleY=1);

	///
	/// \brief createScreenSizeQuad
	/// \param left
	/// \param right
	/// \param bottom
	/// \param top
	/// \return
	///
	std::shared_ptr<mesh::Mesh> createScreenSizeQuad(float left, float right, float bottom, float top);

	///
	/// \brief draw
	/// \param mesh
	///
	void draw(const mesh::Mesh& mesh);

	///
	/// \brief drawImage
	/// \param mesh
	///
	void drawImage(const mesh::Mesh& mesh);

} // End - namespace quad

}
