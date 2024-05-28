#define _SILENCE_CXX17_STRSTREAM_DEPRECATION_WARNING
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics3D.h"
#include "olcPGEX_TransformedView.h"
#include <strstream>
#include <algorithm>

struct mesh {
	std::vector<olc::GFX3D::triangle> tris;
	
	bool LoadFromObjectFile(std::string sFilename)
	{
		std::ifstream f(sFilename);
		if (!f.is_open())
			return false;

		// Local cache of verts
		std::vector<olc::GFX3D::vec3d> verts;

		while (!f.eof())
		{
			char line[128];
			f.getline(line, 128);

			std::stringstream s;
			s << line;

			char junk;

			if (line[0] == 'v')
			{
				olc::GFX3D::vec3d v;
				s >> junk >> v.x >> v.y >> v.z;
				verts.push_back(v);
			}
			if (line[0] == 'f')
			{
				int f[3];
				s >> junk >> f[0] >> f[1] >> f[2];
				tris.push_back({ verts[f[0] - 1], verts[f[1] - 1], verts[f[2] - 1] });
			}
		}
		return true;
	}
};

class Example : public olc::PixelGameEngine
{
public:
	Example()
	{
		// Name your application
		sAppName = "Example";
	}

public:
	olc::Sprite* spr_logo = nullptr;
	olc::Decal* dec_logo = nullptr;
	mesh meshBoard, meshCube;
	olc::GFX3D::mat4x4 matProj;
	olc::GFX3D::PipeLine pipeRender;
	//olc::GFX3D::mat4x4 matProj;
	float fYaw;		// FPS Camera rotation in XZ plane
	float fTheta;
	olc::GFX3D::vec3d vCamera;
	olc::GFX3D::vec3d vUp = { 0,1,0 };
	olc::GFX3D::vec3d vEye = { 0,0,1 };
	olc::GFX3D::vec3d vLookDir = { 0,0,1 };
	float fCameraX = 0.0f;
	float fCameraY = 0.0f;
	float fCameraZ = 0.0f;
	olc::GFX3D::vec3d vTarget = olc::GFX3D::Math::Vec_Add(vCamera, vLookDir);

	bool OnUserCreate() override
	{
		spr_logo = new olc::Sprite("pge2_logo.png");
		dec_logo = new olc::Decal(spr_logo);
		// Called once at the start, so create things here


		//q1111.LoadFromObjectFile("1111.obj");
		//q1110.LoadFromObjectFile("1110.obj");
		//q1101.LoadFromObjectFile("1101.obj");
		//meshBoard.LoadFromObjectFile("board.obj");
		meshCube.LoadFromObjectFile("teapot.obj");


		matProj = olc::GFX3D::Math::Mat_MakeProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f);
		return true;
	}
	//bool OnUserUpdate() over

	bool OnUserUpdate(float fElapsedTime) override
	{
		//Clear(olc::DARK_BLUE);

		// Called once per frame, draws random coloured pixels
		vEye = { fCameraX,fCameraY,fCameraZ };
		olc::GFX3D::vec3d vLookTarget = olc::GFX3D::Math::Vec_Add(vEye, vLookDir);

		// Setup the camera properties for the pipeline - aka "view" transform
		pipeRender.SetCamera(vEye, vLookTarget, vUp);

		if (GetKey(olc::Key::UP).bHeld)
			vCamera.y += 8.0f * fElapsedTime;	// Travel Upwards

		if (GetKey(olc::Key::DOWN).bHeld)
			vCamera.y -= 8.0f * fElapsedTime;	// Travel Downwards


		// Dont use these two in FPS mode, it is confusing :P
		if (GetKey(olc::Key::LEFT).bHeld)
			vCamera.x -= 8.0f * fElapsedTime;	// Travel Along X-Axis

		if (GetKey(olc::Key::RIGHT).bHeld)
			vCamera.x += 8.0f * fElapsedTime;	// Travel Along X-Axis

		olc::GFX3D::vec3d vForward = olc::GFX3D::Math::Vec_Mul(vLookDir, 8.0f * fElapsedTime);

		// Set up "World Tranmsform" though not updating theta 
		// makes this a bit redundant
		olc::GFX3D::mat4x4 matRotZ, matRotX;
		//fTheta += 1.0f * fElapsedTime; // Uncomment to spin me right round baby right round
		matRotZ = olc::GFX3D::Math::Mat_MakeRotationZ(fTheta * 0.5f);
		matRotX = olc::GFX3D::Math::Mat_MakeRotationX(fTheta);

		olc::GFX3D::mat4x4 matTrans;
		matTrans = olc::GFX3D::Math::Mat_MakeTranslation(0.0f, 0.0f, 5.0f);

		olc::GFX3D::mat4x4 matWorld;
		matWorld = olc::GFX3D::Math::Mat_MakeIdentity();	// Form World Matrix
		matWorld = olc::GFX3D::Math::Mat_MultiplyMatrix(matRotZ, matRotX); // Transform by rotation
		matWorld = olc::GFX3D::Math::Mat_MultiplyMatrix(matWorld, matTrans); // Transform by translation

		// Create "Point At" Matrix for camera
		olc::GFX3D::vec3d vUp = { 0,1,0 };
		olc::GFX3D::vec3d vTarget = { 0,0,1 };
		olc::GFX3D::mat4x4 matCameraRot = olc::GFX3D::Math::Mat_MakeRotationY(fYaw);

		vLookDir = olc::GFX3D::Math::Mat_MultiplyVector(matCameraRot, vTarget);

		vTarget = olc::GFX3D::Math::Vec_Add(vCamera, vLookDir);
		olc::GFX3D::mat4x4 matCamera = olc::GFX3D::Math::Mat_PointAt(vCamera, vTarget, vUp);

		// Make view matrix from camera
		olc::GFX3D::mat4x4 matView = olc::GFX3D::Math::Mat_QuickInverse(matCamera);

		// Store triagles for rastering later
		std::vector<olc::GFX3D::triangle> vecTrianglesToRaster;

		// Draw Triangles
		for (auto tri : meshCube.tris)
		{
			DrawRectDecal({20,20}, { 20,20 });
			olc::GFX3D::triangle triProjected, triTransformed, triViewed;

			triTransformed.p[0] = olc::GFX3D::Math::Mat_MultiplyVector(matWorld, tri.p[0]);
			triTransformed.p[1] = olc::GFX3D::Math::Mat_MultiplyVector(matWorld, tri.p[1]);
			triTransformed.p[2] = olc::GFX3D::Math::Mat_MultiplyVector(matWorld, tri.p[2]);

			//Calculate triangle Normal
			olc::GFX3D::vec3d normal, line1, line2;

			//gets lines either side of triange
			line1 = olc::GFX3D::Math::Vec_Sub(triTransformed.p[1], triTransformed.p[0]);
			line2 = olc::GFX3D::Math::Vec_Sub(triTransformed.p[1], triTransformed.p[0]);

			// Take cross product ofl ines to get normal to triangle surface
			normal = olc::GFX3D::Math::Vec_CrossProduct(line1, line2);

			// You normally need to normalise a normal!
			normal = olc::GFX3D::Math::Vec_Normalise(normal);

			// Get ray from triangle to camera
			olc::GFX3D::vec3d vCameraRay = olc::GFX3D::Math::Vec_Sub(triTransformed.p[0], vCamera);

			// If ray is aligned with normal, then triangle is visible
			if (olc::GFX3D::Math::Vec_DotProduct(normal, vCameraRay) < 0.0f)
			{
				// Illumination
				olc::GFX3D::vec3d light_direction = { 0.0f, 1.0f, 1.0f };
				light_direction = olc::GFX3D::Math::Vec_Normalise(light_direction);

				float dp = std::max(0.1f, olc::GFX3D::Math::Vec_DotProduct(light_direction, normal));

				// Project triangles from 3D --> 2D
				triProjected.p[0] = olc::GFX3D::Math::Mat_MultiplyVector(matProj, triTransformed.p[0]);
				triProjected.p[1] = olc::GFX3D::Math::Mat_MultiplyVector(matProj, triTransformed.p[1]);
				triProjected.p[2] = olc::GFX3D::Math::Mat_MultiplyVector(matProj, triTransformed.p[2]);

				//Normalise manualy
				triProjected.p[0] = olc::GFX3D::Math::Vec_Div(triProjected.p[0], triProjected.p[0].w);
				triProjected.p[1] = olc::GFX3D::Math::Vec_Div(triProjected.p[1], triProjected.p[1].w);
				triProjected.p[2] = olc::GFX3D::Math::Vec_Div(triProjected.p[2], triProjected.p[2].w);


				// Scale into view
				olc::GFX3D::vec3d vOffsetView = { 1,1,0 };
				triProjected.p[0] = olc::GFX3D::Math::Vec_Add(triProjected.p[0], vOffsetView);
				triProjected.p[1] = olc::GFX3D::Math::Vec_Add(triProjected.p[1], vOffsetView);
				triProjected.p[2] = olc::GFX3D::Math::Vec_Add(triProjected.p[2], vOffsetView);
				triProjected.p[0].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[0].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[1].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[1].y *= 0.5f * (float)ScreenHeight();
				triProjected.p[2].x *= 0.5f * (float)ScreenWidth();
				triProjected.p[2].y *= 0.5f * (float)ScreenHeight();

				// Store triangle for sorting
				vecTrianglesToRaster.push_back(triProjected);
			}
			// Sort triangles from back to front
			sort(vecTrianglesToRaster.begin(), vecTrianglesToRaster.end(), [](olc::GFX3D::triangle& t1, olc::GFX3D::triangle& t2)
				{
					float z1 = (t1.p[0].z + t1.p[1].z + t1.p[2].z) / 3.0f;
					float z2 = (t2.p[0].z + t2.p[1].z + t2.p[2].z) / 3.0f;
					return z1 > z2;
				});

			for (auto& triProjected : vecTrianglesToRaster)
			{
				// Rasterize triangle
				FillTriangle(triProjected.p[0].x, triProjected.p[0].y,
					triProjected.p[1].x, triProjected.p[1].y,
					triProjected.p[2].x, triProjected.p[2].y,
					olc::Pixel(olc::BLACK));
				// Mesh	
				DrawTriangle(triProjected.p[0].x, triProjected.p[0].y,
					triProjected.p[1].x, triProjected.p[1].y,
					triProjected.p[2].x, triProjected.p[2].y,
					olc::Pixel(olc::GREY));
			}

			// OLC logo for copyright
			DrawDecal({ 0.0f, 0.0f }, dec_logo, { 0.25f, 0.25f });
			
			return true;
		}
	}
};

int main()
{
	Example demo;
	if (demo.Construct(800, 600, 1, 1))
		demo.Start();
	return 0;
}