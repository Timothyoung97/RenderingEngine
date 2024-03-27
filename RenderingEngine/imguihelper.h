#pragma once

#include <d3d11.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"

#include "camera.h"
#include "graphics.h"
#include "window.h"
#include "scene.h"
#include "object.h"
#include "utility.h"
#include "engine.h"

extern std::shared_ptr<tre::Engine> pEngine;

namespace tre {

class ImguiHelper {
public:
	
	ImguiHelper();

	// Setting
	bool show_demo_window = false;
	tre::GraphicsSetting* pRendererSetting;
	tre::GraphicsStats* pRendererStats;
	tre::Scene* pScene;
	tre::Camera* pCam;

	void init();

	void render();
	void cleanup();
};

}