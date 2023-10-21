#pragma once

#include <d3d11.h>

#include "imgui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx11.h"

#include "camera.h"
#include "renderer.h"
#include "window.h"
#include "scene.h"
#include "object.h"

class ImguiHelper {
public:
	
	ImguiHelper(ID3D11Device* device, ID3D11DeviceContext* context, tre::Window* window, tre::Scene* scene, tre::RendererSetting* renSetting, tre::RendererStats* renStats, tre::Camera* cam, tre::Object* debugObj);

	// Setting
	bool show_demo_window = false;
	tre::RendererSetting* pRendererSetting;
	tre::RendererStats* pRendererStats;
	tre::Scene* pScene;
	tre::Camera* pCam;
	tre::Object* pDebugModel;

	void render();
	void cleanup();
};