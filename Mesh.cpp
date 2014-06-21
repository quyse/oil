#include "Mesh.hpp"
#include "../inanity/graphics/VertexBuffer.hpp"
#include "../inanity/graphics/IndexBuffer.hpp"

BEGIN_INANITY_OIL

Mesh::Mesh(ptr<Graphics::VertexBuffer> vertexBuffer, ptr<Graphics::IndexBuffer> indexBuffer)
: vertexBuffer(vertexBuffer), indexBuffer(indexBuffer) {}

Mesh::~Mesh() {}

ptr<Graphics::VertexBuffer> Mesh::GetVertexBuffer() const
{
	return vertexBuffer;
}

ptr<Graphics::IndexBuffer> Mesh::GetIndexBuffer() const
{
	return indexBuffer;
}

END_INANITY_OIL
