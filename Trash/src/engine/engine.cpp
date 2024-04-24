#include "engine.h"
#include "render/render.h"
#include "render/floodgui/flood_gui.h"

#include <iostream>

Engine::Engine() {
	render = new Render();
}


void MultiplyMatrixVector(Vector3& i, Vector3& o, Matrix4x4& m)
{
	o.x = i.x * m.m[0][0] + i.y * m.m[1][0] + i.z * m.m[2][0] + m.m[3][0];
	o.y = i.x * m.m[0][1] + i.y * m.m[1][1] + i.z * m.m[2][1] + m.m[3][1];
	o.z = i.x * m.m[0][2] + i.y * m.m[1][2] + i.z * m.m[2][2] + m.m[3][2];
	float w = i.x * m.m[0][3] + i.y * m.m[1][3] + i.z * m.m[2][3] + m.m[3][3];

	if (w != 0.0f)
	{
		o.x /= w; o.y /= w; o.z /= w;
	}
}

void Engine::Start() {

	meshCube.triangles = {
		// SOUTH
		Triangle{ Vector3{ 0.0f, 0.0f, 0.0f},    {0.0f, 1.0f, 0.0f},   { 1.0f, 1.0f, 0.0f }},
		Triangle{ Vector3{0.0f, 0.0f, 0.0f},   { 1.0f, 1.0f, 0.0f},   { 1.0f, 0.0f, 0.0f} },

		// EAST                                                      
		Triangle{ Vector3{1.0f, 0.0f, 0.0f},    {1.0f, 1.0f, 0.0f},    {1.0f, 1.0f, 1.0f} },
		Triangle{ Vector3{1.0f, 0.0f, 0.0f},    {1.0f, 1.0f, 1.0f},    {1.0f, 0.0f, 1.0f }},

		// NORTH                                                     
		Triangle{ Vector3{1.0f, 0.0f, 1.0f},    {1.0f, 1.0f, 1.0f},    {0.0f, 1.0f, 1.0f} },
		Triangle{ Vector3{1.0f, 0.0f, 1.0f},    {0.0f, 1.0f, 1.0f},    {0.0f, 0.0f, 1.0f} },

		// WEST                                                      
		Triangle{ Vector3{0.0f, 0.0f, 1.0f},    {0.0f, 1.0f, 1.0f},   { 0.0f, 1.0f, 0.0f }},
		Triangle{ Vector3{0.0f, 0.0f, 1.0f},    {0.0f, 1.0f, 0.0f},    {0.0f, 0.0f, 0.0f }},

		// TOP                                                       
		Triangle{ Vector3{0.0f, 1.0f, 0.0f},    {0.0f, 1.0f, 1.0f},    {1.0f, 1.0f, 1.0f }},
		Triangle{ Vector3{0.0f, 1.0f, 0.0f},    {1.0f, 1.0f, 1.0f},   { 1.0f, 1.0f, 0.0f }},

		// BOTTOM                                                    
		Triangle{ Vector3{1.0f, 0.0f, 1.0f},    {0.0f, 0.0f, 1.0f},   { 0.0f, 0.0f, 0.0f }},
		Triangle{Vector3 {1.0f, 0.0f, 1.0f},    {0.0f, 0.0f, 0.0f},    {1.0f, 0.0f, 0.0f }},
	};

	render->Init();

	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	// Projection Matrix
	float fNear = 0.1f;
	float fFar = 1000.0f;
	float fFov = 90.0f;
	float fAspectRatio = (float)height / (float)width;
	float fFovRad = 1.0f / tanf(fFov * 0.5f / 180.0f * 3.14159f);

	matProj.m[0][0] = fAspectRatio * fFovRad;
	matProj.m[1][1] = fFovRad;
	matProj.m[2][2] = fFar / (fFar - fNear);
	matProj.m[3][2] = (-fFar * fNear) / (fFar - fNear);
	matProj.m[2][3] = 1.0f;
	matProj.m[3][3] = 0.0f;

	render->Begin([&]() {
		OnRender();
	});
}

void Engine::OnRender() {
	const float& height = FloodGui::Context.Display.DisplaySize.y;
	const float& width = FloodGui::Context.Display.DisplaySize.x;

	Matrix4x4 matRotZ, matRotX;
	fTheta += 0.05f;

	// Rotation Z
	matRotZ.m[0][0] = cosf(fTheta);
	matRotZ.m[0][1] = sinf(fTheta);
	matRotZ.m[1][0] = -sinf(fTheta);
	matRotZ.m[1][1] = cosf(fTheta);
	matRotZ.m[2][2] = 1;
	matRotZ.m[3][3] = 1;

	// Rotation X
	matRotX.m[0][0] = 1;
	matRotX.m[1][1] = cosf(fTheta * 0.5f);
	matRotX.m[1][2] = sinf(fTheta * 0.5f);
	matRotX.m[2][1] = -sinf(fTheta * 0.5f);
	matRotX.m[2][2] = cosf(fTheta * 0.5f);
	matRotX.m[3][3] = 1;

	for (Triangle& tri : meshCube.triangles)
	{
		Triangle triProjected, triTranslated, triRotatedZ, triRotatedZX;

		// Rotate in Z-Axis
		MultiplyMatrixVector(tri.p[0], triRotatedZ.p[0], matRotZ);
		MultiplyMatrixVector(tri.p[1], triRotatedZ.p[1], matRotZ);
		MultiplyMatrixVector(tri.p[2], triRotatedZ.p[2], matRotZ);

		// Rotate in X-Axis
		MultiplyMatrixVector(triRotatedZ.p[0], triRotatedZX.p[0], matRotX);
		MultiplyMatrixVector(triRotatedZ.p[1], triRotatedZX.p[1], matRotX);
		MultiplyMatrixVector(triRotatedZ.p[2], triRotatedZX.p[2], matRotX);

		// Offset into the screen
		triTranslated = triRotatedZX;
		triTranslated.p[0].z = triRotatedZX.p[0].z + 3.0f;
		triTranslated.p[1].z = triRotatedZX.p[1].z + 3.0f;
		triTranslated.p[2].z = triRotatedZX.p[2].z + 3.0f;

		// Project triangles from 3D --> 2D
		MultiplyMatrixVector(triTranslated.p[0], triProjected.p[0], matProj);
		MultiplyMatrixVector(triTranslated.p[1], triProjected.p[1], matProj);
		MultiplyMatrixVector(triTranslated.p[2], triProjected.p[2], matProj);

		// Scale into view
		triProjected.p[0].x += 1.0f; triProjected.p[0].y += 1.0f;
		triProjected.p[1].x += 1.0f; triProjected.p[1].y += 1.0f;
		triProjected.p[2].x += 1.0f; triProjected.p[2].y += 1.0f;
		triProjected.p[0].x *= 0.5f * (float)width;
		triProjected.p[0].y *= 0.5f * (float)height;
		triProjected.p[1].x *= 0.5f * (float)width;
		triProjected.p[1].y *= 0.5f * (float)height;
		triProjected.p[2].x *= 0.5f * (float)width;
		triProjected.p[2].y *= 0.5f * (float)height;

		// Rasterize triangle
		render->DrawTri(triProjected.p[0].x, triProjected.p[0].y,
			triProjected.p[1].x, triProjected.p[1].y,
			triProjected.p[2].x, triProjected.p[2].y);
	}
}