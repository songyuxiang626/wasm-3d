#include <bx/math.h>
#pragma once

class Ray {
public:
	bx::Vec3 o;
	bx::Vec3 d;
	Ray(const bx::Vec3& origin = {0.0f, 0.0f, 0.0f}, const bx::Vec3& direction = {0.0f, 0.0f, -1.0f})
		: o(origin), d(direction) {
	}
};