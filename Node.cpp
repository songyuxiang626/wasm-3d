#include <bgfx/bgfx.h>

class Node {
public:
	bgfx::VertexBufferHandle& vbh;
	bgfx::IndexBufferHandle& ibh;
	bgfx::ProgramHandle& program;
	bgfx::UniformHandle& u_texture;
	float model_matrix[16];
	float world_matrix[16];
	float matrix[16];
	int order = 0;

	Node(
		bgfx::VertexBufferHandle& vbh,
		bgfx::IndexBufferHandle& ibh,
		bgfx::ProgramHandle& program,
		bgfx::UniformHandle& u_texture)
		: vbh(vbh), ibh(ibh), program(program), u_texture(u_texture) {
		// 初始化矩阵为单位矩阵
		for (int i = 0; i < 16; i++) {
			model_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f; // 单位矩阵
			world_matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
			matrix[i] = (i % 5 == 0) ? 1.0f : 0.0f;
		}
	}

	bool operator<(const Node& other) const {
		return order > other.order;
	}
};