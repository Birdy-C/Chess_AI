#include"Chess.h"

extern MyApp theApp;

void LTexture::renderTexture(unsigned short x, unsigned short y)
{
	SDL_Rect srcRect = { 0, 0, mWidth, mHeight };
	SDL_Rect destRect = { x, y, mWidth, mHeight };

	theApp.gRenderer.RenderCopy(mTexture, &srcRect, &destRect);
}

void LTexture::renderTexture(unsigned short x, unsigned short y, unsigned short Width, unsigned short Height)
{
	SDL_Rect srcRect = { 0, 0, mWidth, mHeight };
	SDL_Rect destRect = { x, y, Width, Height };

	theApp.gRenderer.RenderCopy(mTexture, &srcRect, &destRect);
}

void LTexture::load_image(char* path)
{
	SDL_Surface* loadedSurface = IMG_Load(path);

	if (loadedSurface == NULL)
		Error("LTexture.cpp", "void LTexture::load_image(char* path)", "Í¼Æ¬¼ÓÔØÊ§°Ü");

	else
	{
		SDL_SetColorKey(loadedSurface, SDL_TRUE, SDL_MapRGB(loadedSurface->format, 0, 0xFF, 0xFF));
		mTexture = theApp.gRenderer.CreateTextureFromSurface(loadedSurface);

		if (mTexture == NULL)
			Error("LTexture.cpp", "void LTexture::load_image(char* path)", "ÎÆÀíäÖÈ¾Ê§°Ü");
		else
		{
			mWidth = (unsigned short)loadedSurface->w;
			mHeight = (unsigned short)loadedSurface->h;
		}
		SDL_FreeSurface(loadedSurface);
	}
}

void LTexture::loadFromRenderedText(char *text, SDL_Color textColor, bool debug)
{
	static TTF_Font* gFont = TTF_OpenFont("./Media/myFont.ttf", 16);
	static TTF_Font* gFont1 = TTF_OpenFont("./Media/myFont.ttf", 12);
	SDL_Surface* textSurface;

	if (!debug)
		textSurface = TTF_RenderText_Blended(gFont, text, textColor);
	else
		textSurface = TTF_RenderText_Blended(gFont1, text, textColor);

	mTexture = theApp.gRenderer.CreateTextureFromSurface(textSurface);
	mWidth = textSurface->w;
	mHeight = textSurface->h;
	SDL_FreeSurface(textSurface);
	textSurface = NULL;
}

void LTexture::freeLTexture()
{
	if (mTexture != NULL)
	{
		SDL_DestroyTexture(mTexture);
		mWidth = 0;
		mHeight = 0;
		mTexture = NULL;
	}
}