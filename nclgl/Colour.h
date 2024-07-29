#pragma once
#include "Vector4.h"

class Colour
{
public:
	static const Vector4 White;
	static const Vector4 Red;
	static const Vector4 Green;
	static const Vector4 Blue;
	static const Vector4 Black;
};

const Vector4 Colour::White(1.0f, 1.0f, 1.0f, 1.0f);
const Vector4 Colour::Red(1.0f, 0.0f, 0.0f, 1.0f);
const Vector4 Colour::Green(0.0f, 1.0f, 0.0f, 1.0f);
const Vector4 Colour::Blue(0.0f, 0.0f, 1.0f, 1.0f);
const Vector4 Colour::Black(0.0f, 0.0f, 0.0f, 1.0f);