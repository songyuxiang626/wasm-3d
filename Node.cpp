#include <glm/glm.hpp>
#include <bgfx/bgfx.h>

class Node {
public:
	bgfx::VertexBufferHandle& vbh;
	bgfx::IndexBufferHandle& ibh;
	bgfx::ProgramHandle& program;
	bgfx::UniformHandle& u_texture;
	glm::mat4 model_matrix;
	glm::mat4 world_matrix;
	glm::mat4 matrix;
	int order = 0;

	Node(
		bgfx::VertexBufferHandle& vbh,
		bgfx::IndexBufferHandle& ibh,
		bgfx::ProgramHandle& program,
		bgfx::UniformHandle& u_texture)
		: vbh(vbh), ibh(ibh), program(program), u_texture(u_texture), model_matrix(1.0f), world_matrix(1.0f), matrix(1.0f) {
	}

	bool operator<(const Node& other) const {
		return order > other.order;
	}
};