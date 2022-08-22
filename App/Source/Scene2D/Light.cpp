#include "Light.h"

Light::Light()
{
	position = glm::vec3(0, 0, 0);
	color = glm::vec4(1, 1, 1, 1);
	power = kC = kL = kQ = 0;
}
