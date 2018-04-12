#pragma once

#include "BitBoard.h"
#include <SDL.h>
#include <SDL_ttf.h>
#include <SDL_image.h>
#include <SDL_mixer.h>

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
		gRenderer = SDL_CreateRenderer(gWindow, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
	}
	void RenderCopy(SDL_Texture *mTexture, SDL_Rect *srcRect, SDL_Rect *destRect){
		SDL_RenderCopy(gRenderer, mTexture, srcRect, destRect);
	}
	SDL_Texture *CreateTextureFromSurface(SDL_Surface* loadedSurface){
		return SDL_CreateTextureFromSurface(gRenderer, loadedSurface);
	}
	void RenderApp();
	void DrawBackground();
	void DrawChessmen();
};


class GUI
{
	friend class MyApp;
	friend class Renderer;
private:
	unsigned short	SCREEN_WIDTH;
	unsigned short	SCREEN_HEIGHT;

	SDL_Window     *gWindow;

	bool			Reverse_Board;
	_Coordinate_	selectedX;
	_Coordinate_	selectedY;
	unsigned char	selectedPromotion;

	LTexture		BoardBackground[2];
	LTexture		SelectedBackground;
	LTexture		CheckBackground;
	LTexture		Texture_White[6];
	LTexture		Texture_Black[6];
	LTexture		Promotion_White;
	LTexture		Promotion_Black;

	Mix_Chunk	   *Move_sound;
	Mix_Chunk	   *Eat_sound;
	Mix_Chunk	   *Check_sound;
	Mix_Chunk	   *Mate_sound;
	Mix_Chunk	   *Illegal_sound;

public:
	Renderer		gRenderer;

	GUI(){};
	~GUI();
	void Init();
	void LoadMedia();
	void RenderApp();
	void reverseBoard();
};