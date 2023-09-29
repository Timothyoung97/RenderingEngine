#include "object.h"

namespace tre {
	
XMMATRIX Object::makeLocalToWorldMatrix() {
	XMMATRIX localTransformation = Maths::createTransformationMatrix(this->objScale, this->objRotation, this->objPos);

	if (this->parent != nullptr) {
		XMMATRIX parentMatrix = this->parent->makeLocalToWorldMatrix();
		localTransformation = XMMatrixMultiply(parentMatrix, localTransformation);
	}

	return localTransformation;
}

}