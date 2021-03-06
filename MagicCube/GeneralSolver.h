#pragma once

#include <assert.h>
#include "CubeSolver.h"

class GeneralSolver :
	public CubeSolver
{
public:
	GeneralSolver(Cube &cube) : CubeSolver(cube)
	{}
	~GeneralSolver();

	void Solve();

	void MoveToUp(CubeColor);
	void MoveToDown(CubeColor);
	void MoveToLeft(CubeColor);
	void MoveToRight(CubeColor);
	void MoveToFront(CubeColor);
	void MoveToBack(CubeColor);

	void Stage1(); //pdf stage2: solve the white cross
	void Stage2(); //pdf stage3: solve the white corners
	void Stage3(); //pdf stage4: solve the middle layer
	void Stage4(); //pdf stage5: solve the top layer: get yellow cross
	void Stage5(); //pdf stage5: solve the top layer: get all the yellow on top
	void Stage6(); //pdf stage6: position the yellow corners correctly
	void Stage7(); //pdf stage6: position yellow edges correctly

private:
	bool CheckStage3();
	bool CheckStage4State1(); // +
	bool CheckStage4State3();
	bool CheckStage4State4(); // -
	bool CheckStage4State4i(); // |
	void CheckStage6ABCD(bool*, bool*, bool*, bool*);
	void CheckStage7EFGH(bool*, bool*, bool*, bool*);
};

