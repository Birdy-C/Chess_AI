#include "Chess.h"
#include "GUI.h"
#include <random>

MyApp theApp;
LARGE_INTEGER Tfre, Tstart, Tend;
double timing = 0;

int main(int agc, char *args[])
{
	theApp.InitApp();
	theApp.RunApp();

	return 0;
}

unsigned int WINAPI testDBG(LPVOID pt)
{
	theApp.MonteCarlo();

	return 0;
}

void MyApp::RunApp()
{
	bool quit = false, flag = true;
	SDL_Event event;

	//HANDLE mont = (HANDLE)_beginthreadex(NULL, 0, testDBG, NULL, 0, NULL);

	//MonteCarlo();
	//system("pause");

	//quit = true;

	Load_Record("./Media/record.txt");			//从文件中读取棋步记录到Move_record供调试，主程序中按P可根据记录下棋

	while (!quit)
	{
		/*
		if (side == BLACK_SIDE)
			flag = true;
		if (side == WHITE_SIDE && flag)
		{
			IterativeDeepening(side);
			flag = false;
		}*/
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				quit = TRUE;
			else if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
				quit = true;
			else if (event.type == SDL_MOUSEBUTTONDOWN || event.type == SDL_KEYDOWN || event.type == SDL_MOUSEMOTION)
				startgame(&event);
		}
		MyGUI.gRenderer.RenderApp();
	}
}

void MyApp::startgame(SDL_Event* pevent)
{
	void FILE_Print(const vector<Movement> &record);

	if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_r)
		MyGUI.reverseBoard();
	else if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_n)
		InitMatch();
	else if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_e)
		Board.evaluation(theApp.side, true);
	else if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_s)
		FILE_Print(Move_record);
	else if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_p)
	{	
		ChessBoard temp = Board;
		MoveChess(temp.IterativeDeepening(side, step_count));
	}
	else if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_m)
	{
		system("cls");
		MoveChess(Move_record[step_count]);
		Board.DEBUG_PRINT_EXPAND(side);
		printf("STEP %d\n", step_count);
	}
	else if (pevent->type == SDL_MOUSEBUTTONDOWN || pevent->type == SDL_MOUSEMOTION)
		handleMouseEvent(pevent);
}

MyApp::~MyApp()
{
	Mix_Quit();
	TTF_Quit();
	IMG_Quit();
	SDL_Quit();
}

GUI::~GUI()
{
	BoardBackground[0].freeLTexture();
	BoardBackground[1].freeLTexture();
	SelectedBackground.freeLTexture();
	CheckBackground.freeLTexture();
	for (int i = 0; i < 6; i++)
		Texture_White[i].freeLTexture();
	for (int i = 0; i < 6; i++)
		Texture_Black[i].freeLTexture();
	Promotion_White.freeLTexture();
	Promotion_Black.freeLTexture();

	Mix_FreeChunk(Move_sound);
	Mix_FreeChunk(Check_sound);
	Mix_FreeChunk(Mate_sound);
}

void Error(const char file[], const char function[], const char code[])
{
	cout << file << "-----" << function << "发生错误:" << endl << code << endl;
	system("pause");
	exit(1);
}
