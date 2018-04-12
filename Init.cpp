#include "Chess.h"

void MyApp::InitApp()
{
	MyGUI.Init();
	MoveTable_Init();
	Zobrist_Init();
	InitMatch();
	Board.init_Eval();
}

void MyApp::MoveTable_Init()
{
	Board.Init_BasicTable();

	Board.Init_MoveTable_KNP();
	Board.Init_MoveTableStraight();
	Board.Init_MoveTableDiag();

	Board.Init_BetweenTable();
}

void GUI::Init()
{
	SCREEN_WIDTH = 800;
	SCREEN_HEIGHT = 800;

	if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0)
		Error("Main.cpp", "void MyApp::Init()", "SDL¼ÓÔØÊ§°Ü");

	gWindow = SDL_CreateWindow("FlySky", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
	if (gWindow == NULL)
		Error("Main.cpp", "void MyApp::Init()", "´°¿Ú´´½¨Ê§°Ü");

	gRenderer.CreateRenderer(gWindow);

	if (!(IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG) & (IMG_INIT_PNG | IMG_INIT_JPG)))
		Error("Main.cpp", "void MyApp::Init()", "SDL¼ÓÔØÊ§°Ü");

	if (TTF_Init() == -1)
		Error("Main.cpp", "void MyApp::Init()", "SDL¼ÓÔØÊ§°Ü");

	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0)
		Error("Main.cpp", "void MyApp::Init()", "SDL¼ÓÔØÊ§°Ü");

	Reverse_Board = false;
	LoadMedia();
}

void GUI::LoadMedia()
{
	BoardBackground[0].load_image("./Media/Board.png");
	BoardBackground[1].load_image("./Media/Board_r.png");
	SelectedBackground.load_image("./Media/SelectedBackground.png");
	CheckBackground.load_image("./Media/CheckBackground.png");
	Texture_White[chess_P].load_image("./Media/WP.png");
	Texture_White[chess_N].load_image("./Media/WN.png");
	Texture_White[chess_B].load_image("./Media/WB.png");
	Texture_White[chess_R].load_image("./Media/WR.png");
	Texture_White[chess_Q].load_image("./Media/WQ.png");
	Texture_White[chess_K].load_image("./Media/WK.png");
	Texture_Black[chess_P].load_image("./Media/BP.png");
	Texture_Black[chess_N].load_image("./Media/BN.png");
	Texture_Black[chess_B].load_image("./Media/BB.png");
	Texture_Black[chess_R].load_image("./Media/BR.png");
	Texture_Black[chess_Q].load_image("./Media/BQ.png");
	Texture_Black[chess_K].load_image("./Media/BK.png");
	Promotion_White.load_image("./Media/Promotion_White.png");
	Promotion_Black.load_image("./Media/Promotion_Black.png");

	Move_sound = Mix_LoadWAV("./Media/Move_sound.wav");
	Eat_sound = Mix_LoadWAV("./Media/Eat_sound.wav");
	Check_sound = Mix_LoadWAV("./Media/Check_sound.wav");
	Mate_sound = Mix_LoadWAV("./Media/Mate_sound.wav");
	Illegal_sound = Mix_LoadWAV("./Media/Illegal_sound.wav");
}