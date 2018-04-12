#pragma once
#pragma warning(disable:4996)

#include "GUI.h"
#include "Macro.h"
#include "Movement.h"
#include "Board.h"
#include <vector>
#include <iostream>
#include <Windows.h>
#include <assert.h>
#include <algorithm>
#include <ctime>
#include <cmath>

using namespace std;

class MyApp
{
	friend class Renderer;
	friend class LTexture;
	friend class ChessBoard;
private:
	GUI  MyGUI;
	bool GameOver;
	_Coordinate_ PromotionRank;
	vector <Movement> Move_record;
	uint16_t step_count;

	void startgame(SDL_Event* pevent);
	void InitMatch();
	void Zobrist_Init();
	void MoveTable_Init();
	void MoveChess(const _Coordinate_ &orig_x, const _Coordinate_ &orig_y, const _Coordinate_ &x, const _Coordinate_ &y);
	void MoveChess(const Movement new_Move);
	void CheckControl(const bool &eat);
	///////////////////////////////////////////////////Mouse Event Function
	void handleMouseEvent(const SDL_Event* e);
	void Mouse_ChessMotivation(int MouseX, int MouseY);
	void Mouse_Promotion(int MouseX, int MouseY, const SDL_Event* e);
	////////////////////////////////////////////////////_DEBUG_ONLY_
	//void MonteCarlo();
	void Load_Record(char path[80] = "./Media/record.txt");

public:
	ChessBoard Board;
	BitBoard HighLight;
	bool side;

	MyApp(){};
	~MyApp();
	void InitApp();
	void RunApp();
	unsigned long rand32();

	void MonteCarlo();
	/*
	void handleButtonIn(int MouseX, int MouseY);
	void handleButtonDown(int MouseX, int MouseY);
	void handlePlaceChess(const unsigned short& x, const unsigned short& y);
	*/
};

void Error(const char file[], const char function[], const char code[]);