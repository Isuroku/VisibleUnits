#pragma once

#include "Vector2f.h"

//Class stored info about unit
struct SUnit
{
public:

	SUnit(float inXPos, float inYPos, float inXDir, float inYDir): Pos(inXPos, inYPos), Dir(inXDir, inYDir), NearCount(0)
	{ 
		Dir = Dir.GetNormalized();
	}

	//Position
	Vector2f Pos;
	//Direction
	Vector2f Dir;
	//Count of other units, which will be visible
	unsigned int NearCount;
};
