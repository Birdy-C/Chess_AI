#include "Chess.h"

extern MyApp theApp;

void Renderer::RenderApp()
{
	SDL_RenderClear(gRenderer);

	DrawBackground();
	DrawChessmen();

	SDL_RenderPresent(gRenderer);
}

void Renderer::DrawBackground()
{
	BitBoard chess;
	unsigned short x, y;
	_Pos_ p;

	theApp.MyGUI.BoardBackground[theApp.MyGUI.Reverse_Board].renderTexture(0, 0);
	if (~(theApp.MyGUI.selectedX & theApp.MyGUI.selectedY))
	{
		if (theApp.MyGUI.Reverse_Board)
			theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * (7 - theApp.MyGUI.selectedY), 33 + 92 * (7 - theApp.MyGUI.selectedX));
		else
			theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * theApp.MyGUI.selectedY, 33 + 92 * theApp.MyGUI.selectedX);
	}

	//highlight
	chess = theApp.HighLight;
	while (chess)
	{
		p = pop_lsb(chess);
		x = rank_of(p);
		y = file_of(p);
		if (theApp.MyGUI.Reverse_Board)
			x = 7 - x, y = 7 - y;
		theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * y, 33 + 92 * x);
	}
	//check
	chess = theApp.PromotionRank == NONE ? theApp.Board.His_attackers_to(theApp.Board.posK(theApp.side)) : 0;
	if (chess)
		chess |= theApp.Board.get_bb(theApp.side, chess_K);
	while (chess)
	{		
		p = pop_lsb(chess);
		x = rank_of(p);
		y = file_of(p);
		if (theApp.MyGUI.Reverse_Board)
			x = 7 - x, y = 7 - y;
		theApp.MyGUI.CheckBackground.renderTexture(35 + 92 * y, 32 + 92 * x);
	}
}

void Renderer::DrawChessmen()
{
	unsigned long long chess, p;
	unsigned short x, y;

	for (_ChessPattern_ i = chess_P; i < _PATTERN_COUNT_; i++)
	{
		chess = theApp.Board.get_bb(WHITE_SIDE, i);
		while (chess)
		{
			x = y = 0;
			p = chess & (~chess + 1);
			chess &= ~p;
			while (p >> 8)
				p >>= 8, x++;
			while (p >>= 1)
				y++;
			if (theApp.MyGUI.Reverse_Board)
				x = 7 - x, y = 7 - y;
			theApp.MyGUI.Texture_White[i].renderTexture(35 + 92 * y, 30 + 92 * x);
		}
	}

	for (int i = 0; i < 6; i++)
	{
		chess = theApp.Board.get_bb(BLACK_SIDE, i);
		while (chess)
		{
			x = y = 0;
			p = chess & (~chess + 1);
			chess &= ~p;
			while (p >> 8)
				p >>= 8, x++;
			while (p >>= 1)
				y++;
			if (theApp.MyGUI.Reverse_Board)
				x = 7 - x, y = 7 - y;
			theApp.MyGUI.Texture_Black[i].renderTexture(35 + 92 * y, 30 + 92 * x);
		}
	}
	if (theApp.PromotionRank != NONE)
	{
		if (theApp.side == WHITE_SIDE)
			theApp.MyGUI.Promotion_White.renderTexture(209, 310);
		else
			theApp.MyGUI.Promotion_Black.renderTexture(209, 310);
		if (theApp.MyGUI.selectedPromotion != NONE)
			theApp.MyGUI.SelectedBackground.renderTexture(252 + theApp.MyGUI.selectedPromotion * 79, 391, 58, 58);
	}
}