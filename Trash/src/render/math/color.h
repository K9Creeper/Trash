#pragma once
#include <cstdint>
#include <cmath>

class Color {
private:
	uint8_t rdata[4] = { 0, 0, 0, 255 };
public:
	Color() {  }
	Color(float r, float g, float b, float a = 1.f) { rdata[0] = static_cast<int>(r * 255.f); rdata[1] = static_cast<int>(g * 255.f); rdata[2] = static_cast<int>(b * 255.f); rdata[3] = static_cast<int>(a * 255.f); }
	Color(int r, int g, int b, int a) { rdata[0] = r; rdata[1] = g; rdata[2] = b; rdata[3] = a; }
	~Color() {  }
	constexpr uint8_t& r() { return rdata[0]; }
	constexpr uint8_t& g() { return rdata[1]; }
	constexpr uint8_t& b() { return rdata[2]; }
	constexpr uint8_t& a() { return rdata[3]; }
	constexpr unsigned int ToU32()
	{
		return ((uint8_t(this->a()) << 24) + (uint8_t(this->r()) << 16) + (uint8_t(this->g()) << 8) + uint8_t(this->b()));
	}
	constexpr uint8_t* data() { return rdata; }
};