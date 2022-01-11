#pragma once
#include <string>
#include <vector>
#include "Unit.h"
#include "GameParams.h"

using namespace std;

//Read visible distance, sector (outGameParams)
//Find minimum corner of map grid
//Read all units: position, direction (normalize direction at once)
bool ReadUnits(const string& inFileName, SGameParams& outGameParams, vector<SUnit>& outUnits);

//Write all visible counts
void WriteUnitNeatCounts(const string& inFileName, const vector<SUnit>& inUnits);