#pragma once
#include <cstdint>
#include <cmath>

class Vector2 {
public:
	float x, y;
	Vector2() : x{ 0 }, y{ 0 } { }
	Vector2(const float& x, const float& y) { this->x = x; this->y = y; }

	Vector2 operator+(const Vector2& other) noexcept
	{
		return Vector2(this->x + other.x,
			this->y + other.y);
	}

	Vector2 operator-(const Vector2& other) noexcept
	{
		return Vector2(this->x - other.x,
			this->y - other.y);
	}

	Vector2 operator*(const Vector2& other) noexcept
	{
		return Vector2(this->x * other.x,
			this->y * other.y);
	}

	Vector2 operator/(const Vector2& other) noexcept
	{
		return Vector2(this->x / other.x,
			this->y / other.y);
	}

	Vector2 operator+(const float& other) noexcept
	{
		return Vector2(this->x + other,
			this->y + other);
	}

	Vector2 operator-(const float& other) noexcept
	{
		return Vector2(this->x - other,
			this->y - other);
	}

	Vector2 operator*(const float& other) noexcept
	{
		return Vector2(this->x * other,
			this->y * other);
	}

	Vector2 operator/(const float& other) noexcept
	{
		return Vector2(this->x / other,
			this->y / other);
	}

};