#pragma once
#ifndef MATERIAL_H
#define MATERIAL_H

struct Component
{
	float r, g, b;
	Component(float r = 0.1f, float g = 0.1f, float b = 0.1f)
	{
		Set(r, g, b);
	}
	void Set(float r, float g, float b)
	{
		this->r = r; this->g = g; this->b = b;
	}

	Component& operator=(const Component& rhs)
	{
		r = rhs.r;
		g = rhs.g;
		b = rhs.b;
		return *this;
	}

};
struct Material
{
	Component kAmbient = (0.1f, 0.1f, 0.1f);
	Component kDiffuse = (0.6f, 0.6f, 0.6f);
	Component kSpecular = (0.3f, 0.3f, 0.3f);
	float kShininess = 1.f;
	unsigned size = 0; //remember to initialize to 0
	//to do: add a constructor


	Material& operator=(const Material& rhs)
	{
		kAmbient = rhs.kAmbient;
		kDiffuse = rhs.kDiffuse;
		kSpecular = rhs.kSpecular;
		kShininess = rhs.kShininess;
		size = rhs.size;
		return *this;
	}
	Material();
};

#endif