#include"Chess.h"

MyApp theApp;
_Bit64_ mask[64];

int main(int agc, char *args[])
{
	theApp.InitApp();
	theApp.RunApp();
	return 0;
}

void MyApp::RunApp()
{
	bool quit = false;
	SDL_Event event;

	while (!quit)
	{
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
				quit = TRUE;
			else
				startgame(&event);
		}
		gRenderer.RenderApp();
	}
}

void MyApp::startgame(SDL_Event* pevent)
{
	if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_r)
		MyGUI.reverseBoard();
	if (pevent->type == SDL_KEYDOWN && pevent->key.keysym.sym == SDLK_n)
		InitMatch();
	if (pevent->type == SDL_MOUSEBUTTONDOWN || pevent->type == SDL_MOUSEMOTION)
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
	cout << file << "-----" << function << "·¢Éú´íÎó:" << endl << code << endl;
	system("pause");
	exit(1);
}