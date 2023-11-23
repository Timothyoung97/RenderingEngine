#include "control.h"

namespace tre {

void Control::update(Input& input, Graphics& graphics, Scene& scene, Camera& cam, float deltaTime) {
	if (input._keyState[SDL_SCANCODE_W]) { // control camera movement
		cam.moveCamera(cam.directionV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._keyState[SDL_SCANCODE_S]) {
		cam.moveCamera(-cam.directionV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._keyState[SDL_SCANCODE_D]) {
		cam.moveCamera(-cam.camRightV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._keyState[SDL_SCANCODE_A]) {
		cam.moveCamera(cam.camRightV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._keyState[SDL_SCANCODE_Q]) {
		cam.moveCamera(cam.defaultUpV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._keyState[SDL_SCANCODE_E]) {
		cam.moveCamera(-cam.defaultUpV * deltaTime);
		scene._toRecalDistFromCam = true;
	}
	else if (input._mouseWheelScollY != 0) {
		cam.cameraMoveSpeed = SDL_clamp(cam.cameraMoveSpeed + input._mouseWheelScollY, .001f, .5f);
	}
	else if (input._mouseButtonState[MOUSE_BUTTON_IDX(SDL_BUTTON_RIGHT)]) { // control camera angle
		cam.turnCamera(input._deltaDisplacement.x, input._deltaDisplacement.y);
	}
	else if (input._keyState[SDL_SCANCODE_SPACE]) {
		for (int i = 0; i < 100; i++) {
			// Create new obj
			tre::Object* pNewObj = scene.addRandomObj();

			graphics.stats.totalMeshCount++;
			if ((pNewObj->pObjMeshes[0]->pMaterial->objTexture != nullptr && pNewObj->pObjMeshes[0]->pMaterial->objTexture->hasAlphaChannel)
				|| (pNewObj->pObjMeshes[0]->pMaterial->objTexture == nullptr && pNewObj->pObjMeshes[0]->pMaterial->baseColor.w < 1.0f)) {
				graphics.stats.transparentMeshCount++;
			}
			else {
				graphics.stats.opaqueMeshCount++;
			}
		}
	}
	else if (input._keyState[SDL_SCANCODE_K]) {
		toDumpFile = true;
	}

}
}