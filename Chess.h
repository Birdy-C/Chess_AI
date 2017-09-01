#ifndef __CHESS_H_
#define __CHESS_H_

#pragma warning(disable:4996)

#include <vector>
#include <iostream>
#include <Windows.h>
#include <ctime>
#include <cmath>
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>  
#include <SDL_mixer.h>
#include "Macro.h"

using namespace std;

class BitBoard
{
//private:
public:
	_Bit64_ Board;
public:
	BitBoard(){ Board ^= Board; }
	_Bit64_ GetData(){ return Board; }
	void Init(const _Bit64_ &value){ Board = value; }
	void Move(const _Pos_ &orig_pos64, const _Pos_ &dest_pos64);
	void Eaten(const _Pos_ &pos64);
};

class Movement
{
private:
	_Pos_ orig_pos;
	_Pos_ dest_pos;
	_ChessType_ move_pattern;
public:
	Movement(){ orig_pos = dest_pos = NONE; move_pattern = NONE; }
	Movement(const _Pos_ &orig, const _Pos_ &dest, const _ChessPattern_ &pat){ orig_pos = orig; dest_pos = dest; move_pattern = pat; }
	void SetValue(const _Pos_ &orig, const _Pos_ &dest, const _ChessPattern_ &pat){ orig_pos = orig; dest_pos = dest; move_pattern = pat; }
};

class ChessBoard
{
public:
	BitBoard White[6];
	BitBoard White_All;
	BitBoard White_All_trans;
	BitBoard White_All_L45;
	BitBoard White_All_R45;
	BitBoard Black[6];
	BitBoard Black_All;
	BitBoard Black_All_trans;
	BitBoard Black_All_L45;
	BitBoard Black_All_R45;
	BitBoard Debug;					//Debug

	_Coordinate_ En_passant;
	_Pos_ King_pos[2];
	unsigned char Castling[2];
	bool Check;

	ChessBoard(){ Reset(); }
	ChessBoard(const ChessBoard &father){ memcpy(White, father.White, sizeof(ChessBoard)); }
	void Reset();
	void Move(const _Pos_ &orig, const _Pos_ &dest, const _ChessPattern_ &chesstype, const bool &side);
	void Eaten(const _Pos_ &pos, const _ChessPattern_ &chesstype, const bool &side);
	_Pos_ Trans_Bit64toPos(const _Bit64_ &bit);

	_Bit64_ movement_step(const _ChessType_ &chess, const _Pos_ &pos);
	vector <Movement> strict_expand(const bool &side);
	bool DeltaMoveLegality(const _Pos_ &orig, const _Pos_ &dest, const _ChessPattern_ &chesstype, const _ChessType_ &desttype, const bool &side);

	_ChessPattern_ DeltaChess(const _Coordinate_ &x, const _Coordinate_ &y);
	_ChessType_ DeltaChess(const _Bit64_ p);
	bool DeltaAttack(const bool &side, const _Pos_ &pos);
	unsigned short DeltaCheck(const bool &side, _Pos_ check_pos[2], enum check_type checktype[2]);
	void Search_RidCheck(const bool &side, const _Pos_ &check_pos, const enum check_type &LChecktype, vector<Movement> &result);
	void Search_RidDoubleCheck(const bool &side, vector<Movement> &result);



	//下为评估函数部分
	int Lsb64(_Bit64_ Arg);
	int countChess(_Bit64_ board, bool flag);
	int init_attackArea(_Bit64_ * attackAreaWhite, _Bit64_ * attackAreaBlack);
	int evaluation();
	int value_chess1();
	//int value_chess2();
	int value_chess2(_Bit64_ * attackAreaWhite, _Bit64_ * attackAreaBlack);
	int value_chess3();
	int value_space(_Bit64_ attackAreaWhiteAll, _Bit64_ attackAreaBlackAll);
	int value_flexibility(_Bit64_ * attackAreaWhiteAll, _Bit64_ * attackAreaBlackAll);
	int value_develop(_Bit64_ * attackAreaWhite, _Bit64_ * attackAreaBlack);
};

class LTexture
{
private:
	SDL_Texture* mTexture;
	unsigned short mWidth;
	unsigned short mHeight;
public:
	SDL_Texture* GetTexture(){ return mTexture; }
	void load_image(char* path);
	void renderTexture(unsigned short x, unsigned short y);
	void renderTexture(unsigned short x, unsigned short y, unsigned short Width, unsigned short Height);
	void loadFromRenderedText(char *text, SDL_Color textColor = { 60, 60, 20 }, bool debug = false);
	void freeLTexture();
};

class Renderer
{
private:
	SDL_Renderer *gRenderer;

public:
	void CreateRenderer(SDL_Window *gWindow){ 
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC); }
	void RenderCopy( SDL_Texture *mTexture, SDL_Rect *srcRect, SDL_Rect *destRect){ 
		SDL_RenderCopy(gRenderer, mTexture, srcRect, destRect); }
	SDL_Texture *CreateTextureFromSurface(SDL_Surface* loadedSurface){ 
		return SDL_CreateTextureFromSurface(gRenderer, loadedSurface); }
	void RenderApp();
	void DrawBackground();
	void DrawChessmen();
};

class GUI
{
	friend class MyApp;
	friend class Renderer;
private:
	unsigned short SCREEN_WIDTH;
	unsigned short SCREEN_HEIGHT;

	bool Reverse_Board;
	_Coordinate_ selectedX;
	_Coordinate_ selectedY;
	unsigned char selectedPromotion;

	LTexture BoardBackground[2];
	LTexture SelectedBackground;
	LTexture CheckBackground;
	LTexture Texture_White[6];
	LTexture Texture_Black[6];
	LTexture Promotion_White;
	LTexture Promotion_Black;

	Mix_Chunk *Move_sound;
	Mix_Chunk *Eat_sound;
	Mix_Chunk *Check_sound;
	Mix_Chunk *Mate_sound;

	GUI(){};
	~GUI();
	void LoadMedia();
	void RenderApp();
	void reverseBoard();
};

class MyApp
{
	friend class Renderer;
	friend class LTexture;
private:
	SDL_Window    *gWindow;
	Renderer       gRenderer;
	GUI  MyGUI;
	bool GameOver;
	bool side;
	_Coordinate_ PromotionRank;
	_Pos_ Checker_pos[2];
	vector <Movement> Move_record;

	void InitMatch();
	void startgame(SDL_Event* pevent);
	bool MoveChess(const _Coordinate_ &orig_x, const _Coordinate_ &orig_y, const _Coordinate_ &x, const _Coordinate_ &y,
		const _ChessPattern_ &chesstype, const _ChessType_ &desttype, const bool &side);
	void CheckControl(const bool &eat);
	///////////////////////////////////////////////////Move Table Init Function
	void MoveTable_Init();
	void MoveTable_Init_KingKnightPawn();
	void MoveTable_Init_Straight();
	void BetweenTable_Init();
	void MoveTable_Init_Diag();
	///////////////////////////////////////////////////Mouse Event Function
	void handleMouseEvent(const SDL_Event* e);
	void Mouse_ChessMotivation(int MouseX, int MouseY);
	void Mouse_Promotion(int MouseX, int MouseY, const SDL_Event* e);

	////////////////////////////////////////////////////Evaluation Function
	void eval_Init();
public:
	ChessBoard Board;

	MyApp(){};
	~MyApp();
	void InitApp();
	void RunApp();

	/*
	void handleButtonIn(int MouseX, int MouseY);
	void handleButtonDown(int MouseX, int MouseY);
	void handlePlaceChess(const unsigned short& x, const unsigned short& y);
	bool DeltaWin(const unsigned short& x, const unsigned short& y);
	void CallAI();*/
};

void Error(const char file[], const char function[], const char code[]);

#endif