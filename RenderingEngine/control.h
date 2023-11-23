#pragma once

#include "scene.h"
#include "camera.h"
#include "graphics.h"
#include "input.h"

namespace tre {

class Control {
public:

	Control() = default;

	bool toDumpFile = false;

	void update(Input& input, Graphics& graphics, Scene& scene, Camera& cam, float deltaTime);
};

}