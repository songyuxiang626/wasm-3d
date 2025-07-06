#include <glm.hpp>
#pragma once

class Ray {
public:
	glm::vec3 o;
	glm::vec3 d;
	Ray(const glm::vec3& origin = glm::vec3(0.0f), const glm::vec3& direction = glm::vec3(0.0f, 0.0f, -1.0f))
		: o(origin), d(direction) {
	}
};