#include "stdafx.h"
#include "utilities.h"

// http://stackoverflow.com/questions/236129/split-a-string-in-c
vector<string> &split(const string &s, char delim, vector<string> &elems)
{
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}


vector<string> split(const string &s, char delim)
{
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}

char toUpper(char c)
{
	return c & ~32;
}

char toLower(char c)
{
	return c | 32;
}

string toUpperString(string str)
{
	transform(str.begin(), str.end(), str.begin(), toUpper);
	return  str;
}

string toLowerString(string str)
{
	transform(str.begin(), str.end(), str.begin(), toLower);
	return  str;
}

string stepsToString(CubeSteps &steps)
{
	string r;
	ptrdiff_t size = steps.size();
	for (ptrdiff_t i = 0; i < size; ++i)
	{
		r += CubeRotateMethodName[steps[i]] + " ";
	}
	return r;
}

void printError(CubeError err)
{
	if (err.what == "")
	{
		fprintf(stderr, "CubeError\n");
	}
	else
	{
		fprintf(stderr, "CubeError: %s\n", err.what.c_str());
	}
}

void printError(SolverError err)
{
	if (err.what == "")
	{
		fprintf(stderr, "SolverError\n");
	}
	else
	{
		fprintf(stderr, "SolverError: %s\n", err.what.c_str());
	}
}

void randomCube(Cube &cube)
{
	for (int i = 0; i < rand() % 1000 + 1; ++i)
	{
		CubeRotateMethod method = (CubeRotateMethod)((rand() % 19) + 1);
		cube.DoMethod(method);
	}
}

string randomCube()
{
	Cube cube;
	randomCube(cube);
	return cube.Save();
}

CubeRotateMethod inverse(CubeRotateMethod m)
{
	if (m < ROTATE_NONEi)
	{
		return (CubeRotateMethod)(m + (ROTATE_NONEi - ROTATE_NONE)); //inverse
	}
	else
	{
		return (CubeRotateMethod)(m - (ROTATE_NONEi - ROTATE_NONE)); //inverse
	}

}

void copySteps(CubeSteps &src, CubeSteps &dest)
{
	size_t size = src.size();
	for (ptrdiff_t i = 0; i < (ptrdiff_t)size; ++i)
	{
		if (src[i] != ROTATE_NONE && src[i] != ROTATE_NONEi)
			dest.push_back(src[i]);
	}
}

bool isWholeRotate(CubeRotateMethod m)
{
	return (m >= ROTATE_WHOLEX && m <= ROTATE_WHOLEZ) || (m >= ROTATE_WHOLEXi && m <= ROTATE_WHOLEZi);
}