#pragma once

#include "object.h"

namespace tre {
class Scene {
public:

	tre::Object floor;

	void createFloor(Mesh* mesh, Texture* texture);
};
}