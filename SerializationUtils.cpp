#include <fstream>
#include <iostream>
#include "SerializationUtils.h"

const char COMMENTS_CHAR('#');

void GenerateUnitFile(const string& inFileName, float inViewDist, float inSector, size_t inCount);

bool ReadUnits(const string& inFileName, SGameParams& outGameParams, vector<SUnit>& outUnits)
{
	char str_buf[128];
	ifstream fs(inFileName);
	if (!fs.good())
	{
		//generate file if absent
		GenerateUnitFile(inFileName, 100, 135, 10000);
		fs.open(inFileName);
	}

	bool prev_datas_read = false;

	while (!fs.eof())
	{
		fs.getline(str_buf, 128);
		//cout << str_buf << endl;
		if (str_buf[0] == COMMENTS_CHAR)
			continue;

		if (!prev_datas_read)
		{
			if (sscanf_s(str_buf, "%f %f", &outGameParams.ViewDist, &outGameParams.ViewSector) != 2)
			{
				std::cout << "view distance and sector reading error!" << endl;
				return false;
			}

			prev_datas_read = true;
			continue;
		}

		float xp, yp, xd, yd;
		if (sscanf_s(str_buf, "%f;%f %f;%f", &xp, &yp, &xd, &yd) != 4)
		{
			std::cout << "scanf unit error: " << str_buf << endl;
			continue;
		}

		SUnit unit(xp, yp, xd, yd);

		if (outUnits.empty())
		{
			outGameParams.MinCorner = unit.Pos;
		}
		else
		{
			outGameParams.MinCorner = Vector2f::GetMinCorner(outGameParams.MinCorner, unit.Pos);
		}

		outUnits.push_back(unit);

	}

	return true;
}

void WriteUnitNeatCounts(const string& inFileName, const vector<SUnit>& inUnits)
{
	ofstream fs(inFileName, ios::out);
	fs << "#sequence of visible count for sequence units from Units.txt" << endl;
	for (const SUnit& u : inUnits)
	{
		fs << u.NearCount << endl;
	}
}

float GetRand(float v1, float v2)
{
	float r = (float)rand() / RAND_MAX; //[0; 1]
	return v1 + (v2 - v1) * r;
}

void GenerateUnitFile(const string& inFileName, float inViewDist, float inSector, size_t inCount)
{
	ofstream fs(inFileName, ios::out);
	fs << "#visible_distance(float) space view_sector(float)" << endl;
	fs << inViewDist << " " << inSector << endl;
	fs << "#sequence of units" << endl;
	fs << "#2D_position(float;float) space 2D_direction(float;float)";

	srand(time(0));
	while (inCount > 0)
	{
		fs << endl;
		float x = GetRand(-1.f * inCount, 1.f * inCount);
		float y = GetRand(-1.f * inCount, 1.f * inCount);

		float dx = GetRand(-5, 5);
		float dy = GetRand(-5, 5);

		fs << x << ";" << y << " " << dx << ";" << dy;
		inCount--;
	}
}