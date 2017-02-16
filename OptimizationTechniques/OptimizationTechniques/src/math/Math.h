#ifndef AGARZON_MATH_H
#define AGARZON_MATH_H

#include <cmath>

namespace agarzon
{
	struct Vec3
	{
		float x = 0.0f;
		float y = 0.0f;
		float z = 0.0f;

		Vec3() {}
		Vec3(float x_, float y_, float z_) : x(x_), y(y_), z(z_) {}

		
		Vec3& operator=(const Vec3& rhs)
		{
			x = rhs.x;
			y = rhs.y;
			z = rhs.z;

			return (*this);
		}

		Vec3 operator-(const Vec3& rhs) const
		{
			return Vec3(x - rhs.x, y - rhs.y, z - rhs.z);
		}

		Vec3 operator+(const Vec3& rhs) const
		{
			return Vec3(x + rhs.x, y + rhs.y, z + rhs.z);
		}

		float Magnitude() const
		{
			return std::sqrt(x*x + y*y + z*z);
		}

		// Note: Add more vector functionality as needed
	};

	float Distance(const Vec3& a, const Vec3& b)
	{
		return (b - a).Magnitude();
	}
}

#endif
