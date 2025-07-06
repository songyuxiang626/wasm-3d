#include <iostream>
#include <vector>
#include <fstream>
#include <bgfx/bgfx.h>
#include "./Ray.cpp"

class Utils {
public:
	static std::vector<char> readFile(const std::string& filename) {
        std::ifstream file(filename, std::ios::ate | std::ios::binary);

        if (!file.is_open())
        {
            std::string error = "failed to open file: " + filename;
            throw std::runtime_error(error);
        }

        std::size_t fileSize = (std::size_t)file.tellg();
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();

        return buffer;
	}

    static bgfx::ProgramHandle loadProgram(const std::string& vs, const std::string& fs) {
        std::vector<char> vertexShaderCode = Utils::readFile(vs);
        bgfx::ShaderHandle vertexShader = bgfx::createShader(bgfx::makeRef(vertexShaderCode.data(), vertexShaderCode.size()));
        if (!bgfx::isValid(vertexShader))
        {
            throw std::runtime_error("Failed to create vertex shader");
        }
        else
        {
            std::cout << "Vertex shader load success!" << std::endl;
        }

        std::vector<char> fragmentShaderCode = Utils::readFile(fs);
        bgfx::ShaderHandle fragmentShader = bgfx::createShader(bgfx::makeRef(fragmentShaderCode.data(), fragmentShaderCode.size()));
        if (!bgfx::isValid(fragmentShader))
        {
            throw std::runtime_error("Failed to create fragment shader");
        }
        else
        {
            std::cout << "Fragment shader load success!" << std::endl;
        }

        bgfx::ProgramHandle program = bgfx::createProgram(vertexShader, fragmentShader, true);

        if (!bgfx::isValid(program))
        {
            throw std::runtime_error("Failed to create program");
        }
        else {
            std::cout << "Shader program create success!" << std::endl;
        }

        return program;
    }

	static constexpr bool rayIntersect(
		const glm::vec3& v1,
		const glm::vec3& v2,
		const glm::vec3& v3,
		const Ray& ray,
		float& u,
		float& v,
		float& t
	) {
		glm::vec3 edge1 = v2 - v1;
		glm::vec3 edge2 = v3 - v1;
		glm::vec3 pvec = glm::cross(ray.d, edge2);
		float det = glm::dot(edge1, pvec);

		if (det > -1e-8f && det < 1e-8f)
			return false;
		float inv_det = 1.0f / det;
		glm::vec3 tvec = ray.o - v1;
		u = glm::dot(tvec, pvec) * inv_det;
		if (u < 0.0 || u > 1.0)
			return false;
		glm::vec3 qvec = glm::cross(tvec, edge1);
		v = glm::dot(ray.d, qvec) * inv_det;
		if (v < 0.0 || u + v > 1.0)
			return false;
		t = glm::dot(edge2, qvec) * inv_det;

		return true;
	}
};