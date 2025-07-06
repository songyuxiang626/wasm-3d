#include <bgfx/bgfx.h>

class PlaneGeometry {
public:
	const static float PosTexCoord0Vertex[20];

	const static uint16_t indices[6];

	const static bgfx::VertexLayout vertexLayout;

	static bgfx::VertexBufferHandle vbh;
	static bgfx::IndexBufferHandle ibh;
};

const float PlaneGeometry::PosTexCoord0Vertex[20] = {
		-5.f,  5.f, 0.0f, 0.f, 1.f,
		 5.f,  5.f, 0.0f, 1.f, 1.f,
		-5.f, -5.f, 0.0f, 0.f, 0.f,
		 5.f, -5.f, 0.0f, 1.f, 0.f
};

const uint16_t PlaneGeometry::indices[6] = {
		0, 2, 1,
		1, 2, 3,
};

const bgfx::VertexLayout PlaneGeometry::vertexLayout = []() {
	bgfx::VertexLayout s_vertexLayout;
	s_vertexLayout
	.begin()
	.add(bgfx::Attrib::Position, 3, bgfx::AttribType::Float)
	.add(bgfx::Attrib::TexCoord0, 2, bgfx::AttribType::Float)
	.end();
	return s_vertexLayout;
}();

bgfx::VertexBufferHandle PlaneGeometry::vbh = BGFX_INVALID_HANDLE;
bgfx::IndexBufferHandle PlaneGeometry::ibh = BGFX_INVALID_HANDLE;