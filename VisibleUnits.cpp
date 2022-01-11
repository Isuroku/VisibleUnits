#include <iostream>
#include <vector>
#include <map>
#include <chrono>
#include "SerializationUtils.h"
#include "CellKey.h"

using namespace std;

//Files for input and output
const string UnitsFileName("Units.txt");
const string UnitsOutputFileName("UnitOutput.txt");

//Math constants
const float SQRT_2(sqrtf(2));
const float PI = 3.14159265358979323846264f;
const float D2R(PI / 180); //for transform dergee to radians

//Map grid type
//Use tree map (not hash map), because we need in sort cells from left to right and bottom to top
//key - coords of cell; value - vector of unit indexes in unit vector
typedef map<SCellKey, vector<size_t>, less<SCellKey>> CUnitMap;

//Collect all units to cells in grid
void CreateMap(float inViewDist, const Vector2f& inMinCorner, vector<SUnit>& inAllUnits, CUnitMap& outCells);

//Units of every cell check its visibility
void IteratingMapCells(const CUnitMap& inCells, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits);

//inUnit check visibility
void CheckUnitVisibility(SUnit& inUnit, float inViewDistSq, float inCosHalfSector, const vector<size_t>& inIndexes, size_t inStartIndex, vector<SUnit>& inAllUnits);

//Units check visibility to each other
void CheckUnitsVisibilityEachOther(SUnit& inUnit1, SUnit& inUnit2, float inViewDistSq, float inCosHalfSector);

int main()
{
	SGameParams game_params;
	vector<SUnit> units;

	auto start_time = chrono::high_resolution_clock::now();

	//read data file
	if (!ReadUnits(UnitsFileName, game_params, units))
		return 1;

	auto stop_time = chrono::high_resolution_clock::now();
	auto read_milliseconds = chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	start_time = chrono::high_resolution_clock::now();

	//Set all units in cells in map grid
	CUnitMap cells;
	CreateMap(game_params.ViewDist, game_params.MinCorner, units, cells);

	float view_dist_sq = game_params.ViewDist * game_params.ViewDist;
	float angle_cos = cos(game_params.ViewSector / 2 * D2R);
	//iterating all cells and units into
	IteratingMapCells(cells, view_dist_sq, angle_cos, units);

	stop_time = chrono::high_resolution_clock::now();
	auto calc_milliseconds = chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	start_time = chrono::high_resolution_clock::now();
	//write results
	WriteUnitNeatCounts(UnitsOutputFileName, units);

	stop_time = chrono::high_resolution_clock::now();
	auto write_milliseconds = chrono::duration_cast<std::chrono::milliseconds>(stop_time - start_time);

	std::cout << "read " << read_milliseconds.count() << " msec" << endl;
	std::cout << "calc " << calc_milliseconds.count() << " msec" << endl;
	std::cout << "write " << write_milliseconds.count() << " msec" << endl;

	return 0;
}

//Collect all units to cells in grid
void CreateMap(float inViewDist, const Vector2f& inMinCorner, vector<SUnit>& inAllUnits, CUnitMap& outCells)
{
	//Square cell diagonal - viewing distance
	//so all units in one cell can see to each other by distance
	//may be we could optimize check distance (unfortunately not)

	float cell_width = inViewDist / SQRT_2;

	for (size_t i = 0; i < inAllUnits.size(); i++)
	{
		const SUnit& u = inAllUnits[i];

		//changing in position (use new coord center) allows us use only positive cell coords
		Vector2f rel_pos = u.Pos - inMinCorner;

		SCellKey key;
		key.X = (unsigned int)(rel_pos.X / cell_width);
		key.Y = (unsigned int)(rel_pos.Y / cell_width);

		//may be cell present already
		const auto it = outCells.find(key);
		if (it == outCells.end())
		{
			//create cell and set index of current unit
			vector<size_t> vec(1);
			vec[0] = i;
			outCells.insert(make_pair(key, vec));
		}
		else
		{
			//add neighbour unit in cell
			vector<size_t>& vec = it->second;
			vec.push_back(i);
		}
	}
}

//Units of every cell check its visibility
void IteratingMapCells(const CUnitMap& inCells, float inViewDistSq, float inCosHalfSector, vector<SUnit>& inAllUnits)
{
	for (CUnitMap::const_iterator it = inCells.begin(); it != inCells.end(); it++)
	{
		const vector<size_t>& vec = it->second;

		SCellKey find_key = it->first;

		//around current cell eight neighbours cells (except border cells)
		//other cells are too far
		//we need check current cell and four next
		//previous four cells checked us already

		CUnitMap::const_iterator neib_it_right = inCells.find(SCellKey(find_key.X + 1, find_key.Y));
		CUnitMap::const_iterator neib_it_left_up = find_key.X > 0 ? inCells.find(SCellKey(find_key.X - 1, find_key.Y + 1)) : inCells.end();
		CUnitMap::const_iterator neib_it_up = inCells.find(SCellKey(find_key.X, find_key.Y + 1));
		CUnitMap::const_iterator neib_it_right_up = inCells.find(SCellKey(find_key.X + 1, find_key.Y + 1));

		for (size_t i = 0; i < vec.size(); i++)
		{
			size_t index1 = vec[i];
			SUnit& u1 = inAllUnits[index1];

			//current unit check next units in cell: i + 1
			//prev units checked current unit already
			CheckUnitVisibility(u1, inViewDistSq, inCosHalfSector, vec, i + 1, inAllUnits);

			//check all units in four neighbours celss
			if (neib_it_right != inCells.end())
			{
				const vector<size_t>& vec2 = neib_it_right->second;
				CheckUnitVisibility(u1, inViewDistSq, inCosHalfSector, vec2, 0, inAllUnits);
			}

			if (neib_it_left_up != inCells.end())
			{
				const vector<size_t>& vec2 = neib_it_left_up->second;
				CheckUnitVisibility(u1, inViewDistSq, inCosHalfSector, vec2, 0, inAllUnits);
			}

			if (neib_it_up != inCells.end())
			{
				const vector<size_t>& vec2 = neib_it_up->second;
				CheckUnitVisibility(u1, inViewDistSq, inCosHalfSector, vec2, 0, inAllUnits);
			}

			if (neib_it_right_up != inCells.end())
			{
				const vector<size_t>& vec2 = neib_it_right_up->second;
				CheckUnitVisibility(u1, inViewDistSq, inCosHalfSector, vec2, 0, inAllUnits);
			}
		}
	}
}

//inUnit check visibility
//inStartIndex - for check inUnit's cell
void CheckUnitVisibility(SUnit& inUnit, float inViewDistSq, float inCosHalfSector, const vector<size_t>& inIndexes, size_t inStartIndex, vector<SUnit>& inAllUnits)
{
	for (size_t i = inStartIndex; i < inIndexes.size(); i++)
	{
		size_t index = inIndexes[i];
		SUnit& u2 = inAllUnits[index];
		CheckUnitsVisibilityEachOther(inUnit, u2, inViewDistSq, inCosHalfSector);
	}
}

//Units check visibility to each other
void CheckUnitsVisibilityEachOther(SUnit& inUnit1, SUnit& inUnit2, float inViewDistSq, float inCosHalfSector)
{
	float d2 = Vector2f::GetDistanceSq(inUnit1.Pos, inUnit2.Pos);
	if (d2 > inViewDistSq)
		return; //too far away

	float dist = sqrt(d2);
	if (dist <= Vector2f::MIN_LENGTH)
	{
		//units has equal position
		//we think that they see each other
		inUnit1.NearCount++;
		inUnit2.NearCount++;
		return;
	}

	Vector2f r1 = inUnit2.Pos - inUnit1.Pos;
	//Normalization - we already have distance, not need it again
	r1.X /= dist;
	r1.Y /= dist;

	//both vectors are unit (len 1)
	//inUnit1.Dir was normalized on load
	//in this case: dot product == cos of angle between vectors
	float acos = Vector2f::GetDotProduct(r1, inUnit1.Dir);
	if (acos > inCosHalfSector)
	{
		inUnit1.NearCount++;
	}

	Vector2f r2 = -1 * r1;
	acos = Vector2f::GetDotProduct(r2, inUnit2.Dir);
	if (acos > inCosHalfSector)
	{
		inUnit2.NearCount++;
	}
}