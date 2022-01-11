#pragma once

#include "Vector2f.h"

//Class stored view distance, sector and origin of map grid (MinCorner)
struct SGameParams
{
public:
	SGameParams() : ViewDist(0), ViewSector(0) { }
	SGameParams(const SGameParams& inValue) : ViewDist(inValue.ViewDist), ViewSector(inValue.ViewSector), MinCorner(inValue.MinCorner) { }

	float ViewDist;
	float ViewSector;

	Vector2f MinCorner;
};