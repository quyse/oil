#ifndef ___INANITY_OIL_MESH_HPP___
#define ___INANITY_OIL_MESH_HPP___

#include "oil.hpp"
#include "../inanity/graphics/graphics.hpp"

BEGIN_INANITY_GRAPHICS

class VertexBuffer;
class IndexBuffer;

END_INANITY_GRAPHICS

BEGIN_INANITY_OIL

class MeshInstance;

/// Abstract class representing mesh.
class Mesh : public Object
{
private:
	ptr<Graphics::VertexBuffer> vertexBuffer;
	ptr<Graphics::IndexBuffer> indexBuffer;

public:
	Mesh(ptr<Graphics::VertexBuffer> vertexBuffer, ptr<Graphics::IndexBuffer> indexBuffer);
	~Mesh();

	ptr<Graphics::VertexBuffer> GetVertexBuffer() const;
	ptr<Graphics::IndexBuffer> GetIndexBuffer() const;
};

END_INANITY_OIL

#endif
