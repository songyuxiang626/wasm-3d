#include <bgfx/bgfx.h>

class MeshBasicMaterial {
public:
	static bgfx::ProgramHandle program;
	static bgfx::UniformHandle u_texture;
};

bgfx::ProgramHandle MeshBasicMaterial::program = BGFX_INVALID_HANDLE;
bgfx::UniformHandle MeshBasicMaterial::u_texture = BGFX_INVALID_HANDLE;