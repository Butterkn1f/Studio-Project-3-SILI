#include "Material.h"

Material::Material()
{
	kAmbient.Set(0.1f, 0.1f, 0.1f);
	kDiffuse.Set(0.6f, 0.6f, 0.6f);
	kSpecular.Set(0.3f, 0.3f, 0.3f);
	kShininess = 1.f;
}
