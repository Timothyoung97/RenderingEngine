#include "imguihelper.h"

ImguiHelper::ImguiHelper(ID3D11Device* device, ID3D11DeviceContext* context, tre::Window* window, tre::Scene* scene, tre::RendererSetting* renSetting, tre::RendererStats* renStats, tre::Camera* cam, tre::Object* debugObj) {
	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplSDL2_InitForD3D(window->window);
	ImGui_ImplDX11_Init(device, context);

	// assign pointers
	pScene = scene;
	pRendererSetting = renSetting;
	pRendererStats = renStats;
	pCam = cam;
	pDebugModel = debugObj;
}

void ImguiHelper::render() {
	ImGuiIO& io = ImGui::GetIO();

	// Start the Dear ImGui frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplSDL2_NewFrame();
	ImGui::NewFrame();

	if (show_demo_window)
		ImGui::ShowDemoWindow(&show_demo_window);

	ImGui::Begin("Debug");

	ImGui::Checkbox("Demo Window", &show_demo_window);

	{	// SSAO
		ImGui::SeparatorText("SSAO");
		ImGui::Checkbox("SSAO Switch", &pRendererSetting->ssaoSwitch);
		ImGui::SliderFloat("SSAO Sample Radius", &pRendererSetting->ssaoSampleRadius, .000f, 2.f, "%.6f");
	}

	{	// Control for import models

		ImGui::SeparatorText("Test Object Control");

		float translation[3] = { pDebugModel->objPos.x, pDebugModel->objPos.y,  pDebugModel->objPos.z };
		ImGui::SliderFloat3("Translation", translation, .0f, 20.f);
		pDebugModel->objPos = XMFLOAT3(translation);

		float rotationXYZ[3] = { pDebugModel->objRotation.x, pDebugModel->objRotation.y, pDebugModel->objRotation.z };
		ImGui::SliderFloat3("Rotation", rotationXYZ, .0f, 360.f);
		pDebugModel->objRotation = XMFLOAT3(rotationXYZ);

		float scaleXYZ = pDebugModel->objScale.x;
		ImGui::SliderFloat("Scale", &scaleXYZ, .1f, 3.f);
		pDebugModel->objScale = XMFLOAT3(scaleXYZ, scaleXYZ, scaleXYZ);
		pDebugModel->_transformationFinal = tre::Maths::createTransformationMatrix(pDebugModel->objScale, pDebugModel->objRotation, pDebugModel->objPos);
	}

	{	// Camera Setting
		ImGui::SeparatorText("Camera");
		ImGui::SliderFloat("Camera FOV Y", &pCam->fovY, 1.0f, 179.0f);
		ImGui::SliderFloat("Camera Speed", &pCam->cameraMoveSpeed, .0f, 1.0f);
	}

	{	// Bounding Type Selection
		ImGui::SeparatorText("Bounding Volume");
		ImGui::Checkbox("Show Bounding Volume", &pRendererSetting->showBoundingVolume);
		static int selectedIdx = 0;
		const char* names[] = { "AABB", "Ritter Sphere", "Naive Sphere" };

		if (ImGui::Button("Bounding Volume Type")) {
			ImGui::OpenPopup("boundVType");
		}
		ImGui::SameLine();
		ImGui::TextUnformatted(names[selectedIdx]);
		if (ImGui::BeginPopup("boundVType")) {
			ImGui::SeparatorText("Bounding Type");
			for (int i = 0; i < IM_ARRAYSIZE(names); i++)
				if (ImGui::Selectable(names[i])) {
					selectedIdx = i;
					switch (selectedIdx) {
					case 0:
						pRendererSetting->typeOfBound = tre::AABBBoundingBox;
						pRendererSetting->meshIdx = 0;
						break;
					case 1:
						pRendererSetting->typeOfBound = tre::RitterBoundingSphere;
						pRendererSetting->meshIdx = 1;
						break;
					case 2:
						pRendererSetting->typeOfBound = tre::NaiveBoundingSphere;
						pRendererSetting->meshIdx = 1;
						break;
					}
				}
			ImGui::EndPopup();
		}
	}

	{	// light 
		ImGui::SeparatorText("Lights");
		ImGui::Checkbox("Pause Light", &pRendererSetting->pauseLight);
		if (pScene->lightResc.numOfLights < pScene->lightResc.maxPointLightNum) {
			if (ImGui::Button("Add Pt Light")) {
				pScene->lightResc.addRandPointLight();
			}
			ImGui::SameLine();
			ImGui::Text("Current Light Count: %d/%d", pScene->lightResc.numOfLights, pScene->lightResc.maxPointLightNum);
		}
		else {
			ImGui::Text("Max Light Count: %d/%d", pScene->lightResc.numOfLights, pScene->lightResc.maxPointLightNum);
		}

		ImGui::BulletText("Dir Light");
		ImGui::SliderFloat("Yaw", &pScene->dirlightYaw, .0f, 360.f);
		ImGui::SliderFloat("Pitch", &pScene->dirlightPitch, .0f, 89.f);
	}

	{	// farplane intervals
		ImGui::SeparatorText("Cascaded Shadow");
		ImGui::Text("Total Opaque Mesh: %d; Total Transparent Mesh: %d; All Mesh: %d;", pRendererStats->opaqueMeshCount, pRendererStats->transparentMeshCount, pRendererStats->totalMeshCount);
		ImGui::Text("");
		ImGui::Checkbox("CSM Debug", &pRendererSetting->csmDebugSwitch);
		ImGui::Text("Opaque Draw in Shadow Cascade 0: %d / %d", pRendererStats->shadowCascadeOpaqueObjs[0], pRendererStats->opaqueMeshCount);
		ImGui::SliderFloat("Far Plane 0", &pRendererSetting->csmPlaneIntervalsF.x, pRendererSetting->csmPlaneIntervals[0], pRendererSetting->csmPlaneIntervals[2]);
		ImGui::Text("Opaque Draw in Shadow Cascade 1: %d / %d", pRendererStats->shadowCascadeOpaqueObjs[1], pRendererStats->opaqueMeshCount);
		ImGui::SliderFloat("Far Plane 1", &pRendererSetting->csmPlaneIntervalsF.y, pRendererSetting->csmPlaneIntervals[1], pRendererSetting->csmPlaneIntervals[3]);
		ImGui::Text("Opaque Draw in Shadow Cascade 2: %d / %d", pRendererStats->shadowCascadeOpaqueObjs[2], pRendererStats->opaqueMeshCount);
		ImGui::SliderFloat("Far Plane 2", &pRendererSetting->csmPlaneIntervalsF.z, pRendererSetting->csmPlaneIntervals[2], pRendererSetting->csmPlaneIntervals[4]);
		ImGui::Text("Opaque Draw in Shadow Cascade 3: %d / %d", pRendererStats->shadowCascadeOpaqueObjs[3], pRendererStats->opaqueMeshCount);
		ImGui::SliderFloat("Far Plane 3", &pRendererSetting->csmPlaneIntervalsF.w, pRendererSetting->csmPlaneIntervals[3], 1000.0f);

		pRendererSetting->csmPlaneIntervals[1] = pRendererSetting->csmPlaneIntervalsF.x, pRendererSetting->csmPlaneIntervals[2] = pRendererSetting->csmPlaneIntervalsF.y, pRendererSetting->csmPlaneIntervals[3] = pRendererSetting->csmPlaneIntervalsF.z, pRendererSetting->csmPlaneIntervals[4] = pRendererSetting->csmPlaneIntervalsF.w;
	}

	{	// Stats
		ImGui::SeparatorText("Debug Info");
		ImGui::Text("Within Frustcum/Total: %d / %d", pScene->_culledOpaqueObjQ.size() + pScene->_culledTransparentObjQ.size(), pScene->_pObjQ.size());
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
	}

	ImGui::End();
	ImGui::Render();
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

void ImguiHelper::cleanup() {
	//Cleanup
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplSDL2_Shutdown();
	ImGui::DestroyContext();
}
