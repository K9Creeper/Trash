#pragma once
#include "../floodgui/flood_gui_math.h"
#include <iostream>

using Vector3 = FloodVector3;

inline float Vector_DotProduct(const Vector3& v1, const Vector3& v2)
{
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

inline float Vector_Length(const Vector3& v)
{
	return sqrtf(Vector_DotProduct(v, v));
}

inline Vector3 Vector_Normalise(const Vector3& v)
{
	float l = Vector_Length(v);
	return { v.x / l, v.y / l, v.z / l };
}

inline Vector3 Vector_CrossProduct(const Vector3& v1, const Vector3& v2)
{
	Vector3 v;
	v.x = v1.y * v2.z - v1.z * v2.y;
	v.y = v1.z * v2.x - v1.x * v2.z;
	v.z = v1.x * v2.y - v1.y * v2.x;
	return v;
}

struct Matrix4x4 {
private:
	
public:
	float m[4][4] = {0};
	
	void QuickInverse(const Matrix4x4& m1) // Only for Rotation/Translation Matrices
	{
		m[0][0] = m1.m[0][0]; m[0][1] = m1.m[1][0]; m[0][2] = m1.m[2][0]; m[0][3] = 0.0f;
		m[1][0] = m1.m[0][1]; m[1][1] = m1.m[1][1]; m[1][2] = m1.m[2][1]; m[1][3] = 0.0f;
		m[2][0] = m1.m[0][2]; m[2][1] = m1.m[1][2]; m[2][2] = m1.m[2][2]; m[2][3] = 0.0f;
		m[3][0] = -(m1.m[3][0] * m[0][0] + m1.m[3][1] * m[1][0] + m1.m[3][2] * m[2][0]);
		m[3][1] = -(m1.m[3][0] * m[0][1] + m1.m[3][1] * m[1][1] + m1.m[3][2] * m[2][1]);
		m[3][2] = -(m1.m[3][0] * m[0][2] + m1.m[3][1] * m[1][2] + m1.m[3][2] * m[2][2]);
		m[3][3] = 1.0f;
	}
	inline void MultiplyMatrix(const Matrix4x4& m1, const Matrix4x4& m2)
	{
		for (int c = 0; c < 4; c++)
			for (int r = 0; r < 4; r++)
				m[r][c] = m1.m[r][0] * m2.m[0][c] + m1.m[r][1] * m2.m[1][c] + m1.m[r][2] * m2.m[2][c] + m1.m[r][3] * m2.m[3][c];
	}

	inline void MakeIdentity()
	{
		m[0][0] = 1.0f;
		m[1][1] = 1.0f;
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;
	}

	inline void MakeRotationX(const float& fAngleRad)
	{
		m[0][0] = 1.0f;
		m[1][1] = cosf(fAngleRad);
		m[1][2] = sinf(fAngleRad);
		m[2][1] = -sinf(fAngleRad);
		m[2][2] = cosf(fAngleRad);
		m[3][3] = 1.0f;
	}

	inline void MakeRotationY(const float& fAngleRad)
	{
		m[0][0] = cosf(fAngleRad);
		m[0][2] = sinf(fAngleRad);
		m[2][0] = -sinf(fAngleRad);
		m[1][1] = 1.0f;
		m[2][2] = cosf(fAngleRad);
		m[3][3] = 1.0f;
	}

	inline void MakeRotationZ(const float& fAngleRad)
	{
		m[0][0] = cosf(fAngleRad);
		m[0][1] = sinf(fAngleRad);
		m[1][0] = -sinf(fAngleRad);
		m[1][1] = cosf(fAngleRad);
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;
	}

	inline FloodVector3 MultiplyVector(const Vector3& i, float& w)
	{
		Vector3 o;
		o.x = i.x * m[0][0] + i.y *m[1][0] + i.z * m[2][0] + m[3][0];
		o.y = i.x * m[0][1] + i.y * m[1][1] + i.z * m[2][1] + m[3][1];
		o.z = i.x * m[0][2] + i.y * m[1][2] + i.z * m[2][2] + m[3][2];
		w = i.x * m[0][3] + i.y * m[1][3] + i.z * m[2][3] + m[3][3];

		if (w != 0.0f)
		{
			o.x /= w; o.y /= w; o.z /= w;
		}
		return o;
	}

	inline void MakeTranslation(const float& x, const float& y, const float& z)
	{
		m[0][0] = 1.0f;
		m[1][1] = 1.0f;
		m[2][2] = 1.0f;
		m[3][3] = 1.0f;
		m[3][0] = x;
		m[3][1] = y;
		m[3][2] = z;
	}

	inline void PointAt(Vector3& pos, Vector3& target, Vector3& up)
	{
		// Calculate new forward direction
		Vector3 newForward = (target - pos);
		newForward = Vector_Normalise(newForward);

		// Calculate new Up direction
		Vector3 a = (newForward * Vector_DotProduct(up, newForward));
		Vector3 newUp = (up - a);
		newUp = Vector_Normalise(newUp);

		// New Right direction is easy, its just cross product
		Vector3 newRight = Vector_CrossProduct(newUp, newForward);

		// Construct Dimensioning and Translation Matrix	
		m[0][0] = newRight.x;	m[0][1] = newRight.y;	m[0][2] = newRight.z;	m[0][3] = 0.0f;
		m[1][0] = newUp.x;		m[1][1] = newUp.y;		m[1][2] = newUp.z;		m[1][3] = 0.0f;
		m[2][0] = newForward.x;	m[2][1] = newForward.y;	m[2][2] = newForward.z;	m[2][3] = 0.0f;
		m[3][0] = pos.x;			m[3][1] = pos.y;			m[3][2] = pos.z;			m[3][3] = 1.0f;
	}


};

