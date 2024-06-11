#define _SILENCE_CXX17_STRSTREAM_DEPRECATION_WARNING
#define OLC_PGE_APPLICATION
#include "olcPixelGameEngine.h"
#include "olcPGEX_Graphics3D.h"
#include "olcPGEX_TransformedView.h"
#include <strstream>
#include <algorithm>

#if defined(__EMSCRIPTEN__)
#include <emscripten.h>
#define FILE_RESOLVE(url, file) emscripten_wget(url, file); emscripten_sleep(0)
#else
#define FILE_RESOLVE(url, file)
#endif
#define OLC_PGEX_GRAPHICS3D

using namespace olc;

class Graphics3DPGEX : public olc::PixelGameEngine
{
	GFX3D::mesh board;
	GFX3D::mesh tile1111;
	GFX3D::PipeLine renderer;

	GFX3D::vec3d vUp = { 0,-1,0 };
	GFX3D::vec3d vEye = { 0, 3, -5 };
	GFX3D::vec3d vLookDir = { 0,0,5 };

	float fTheta = 0;

	struct sCell {
		bool empty = true;
		int sTile = 0;
	};

	int boardWidth = 4;
	int boardLength = 4;
	sCell* pBoard;

	Sprite* boardTex;

public:
	Graphics3DPGEX()
	{
		sAppName = "Quarto graphic game";
	}

public:
	bool OnUserCreate() override
	{
		board.LoadFromObjectFile("board.obj");
		tile1111.LoadFromObjectFile("1111.obj");
		olc::GFX3D::ConfigureDisplay();

		boardTex = new Sprite("dirtblock.png");

		renderer.SetProjection(90.0f, (float)ScreenHeight() / (float)ScreenWidth(), 0.1f, 1000.0f, 0.0f, 0.0f, ScreenWidth(), ScreenHeight());
		
		pBoard = new sCell[boardWidth * boardLength];

		for (int x = 0; x < boardWidth; x++) {
			for (int y = 0; y < boardLength; y++) {
				pBoard[y * boardWidth + x].empty = false;
				pBoard[y * boardWidth + x].sTile = 0;
			}
		}
		
		return true;
	}

	void CameraControls()
	{
		if (GetKey(Key::UP).bHeld) vEye.y -= 8.f * GetElapsedTime();
		if (GetKey(Key::DOWN).bHeld) vEye.y += 8.f * GetElapsedTime();
		if (GetKey(Key::RIGHT).bHeld) vEye.x += 8.f * GetElapsedTime();
		if (GetKey(Key::LEFT).bHeld) vEye.x -= 8.f * GetElapsedTime();
		if (GetMouse(olc::Mouse::LEFT).bHeld) vEye.z += 8.f * GetElapsedTime();
		if (GetMouse(olc::Mouse::RIGHT).bHeld) vEye.z -= 8.f * GetElapsedTime();
	}

	bool OnUserUpdate(float fElapsedTime) override
	{
		fTheta += fElapsedTime;

		CameraControls();

		Clear(VERY_DARK_BLUE);

		GFX3D::ClearDepth();

		GFX3D::vec3d vLookTarget = GFX3D::Math::Vec_Add(vEye, vLookDir);
		
		olc::GFX3D::mat4x4 matWorld;

		renderer.SetCamera(vEye, vLookTarget, vUp);

		int nStartX = 0;
		int nEndX = boardWidth;
		int nStartY = 0;
		int nEndY = boardLength;

		renderer.SetTexture(boardTex);
		renderer.Render(board.tris);

		for (int x = nStartX; x < nEndX; x++) {
			for (int y = nStartY; y < nEndY; y++) {
				if (pBoard[y * boardWidth + x].empty) {

				}
				else {
					if (pBoard[y * boardWidth + x].sTile == 0) {
						matWorld = olc::GFX3D::Math::Mat_MakeTranslation(x, 0.0f, y);
						matWorld = GFX3D::Math::Mat_MakeRotationY(fTheta);
						renderer.SetTransform(matWorld);
						renderer.Render(tile1111.tris, GFX3D::RENDER_WIRE);	
					}
					
				}
			}
		}

		//GFX3D::mat4x4 matRotateX = GFX3D::Math::Mat_MakeRotationX(0);
		//GFX3D::mat4x4 matRotateZ = GFX3D::Math::Mat_MakeRotationY(fTheta);//fTheta / 3.0f);
		//GFX3D::mat4x4 matWorld = GFX3D::Math::Mat_MultiplyMatrix(matRotateX, matRotateZ);
		matWorld = GFX3D::Math::Mat_MakeRotationY(fTheta);//fTheta / 3.0f);
		renderer.SetTransform(matWorld);

		//renderer.Render(board.tris);

		return true;
	}

	bool OnUserDestroy()override {
		return true;
	}
};

int main()
{
	Graphics3DPGEX Quarto;
	if (Quarto.Construct(768, 480, 1, 1))
		Quarto.Start();

	return 0;
}