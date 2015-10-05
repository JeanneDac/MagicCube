#pragma once

#include <map>

extern map<char, CubeColor> ColorCharMap;

class Cube
{
public:
	Cube();
	~Cube();

	cube_t subCubes[3][3][3];
	//              x  y  z
	/*
	    y
	    |
	    |
	    |
	    |________x
	   /_/_/_/
	  /_/_/_/
	 /_/_/_/
	z 
	*/

	void Load(string);
	void SaveState();
	void DoMethod(CubeRotateMethod);
	void R();
	void Ri();
	void L();
	void Li();
	void B();
	void Bi();
	void D();
	void Di();
	void F();
	void Fi();
	void U();
	void Ui();
	void RotateLeft();
	void RotateRight();
	void RotateUp();
	void RotateDown();
	void RotateClk();
	void RotateCClk();
	bool CheckL();
	bool CheckR();
	bool CheckU();
	bool CheckD();
	bool CheckF();
	bool CheckB();
	bool Check();

private:
	cube_t oldSubCubes[3][3][3];
};

