#include"Chess.h"

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
	theApp.MyGUI.BoardBackground[theApp.MyGUI.Reverse_Board].renderTexture(0, 0);
	if (~(theApp.MyGUI.selectedX & theApp.MyGUI.selectedY))
	{
		if (theApp.MyGUI.Reverse_Board)
			theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * theApp.MyGUI.selectedY, 33 + 92 * (7 - theApp.MyGUI.selectedX));
		else
			theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * theApp.MyGUI.selectedY, 33 + 92 * theApp.MyGUI.selectedX);
	}
	//debug
	unsigned long long chess, p;
	unsigned short x, y;

	chess = theApp.Board.Debug.GetData();
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
			x = 7 - x;
		theApp.MyGUI.SelectedBackground.renderTexture(36 + 92 * y, 33 + 92 * x);
	}
	//check
	if (theApp.Board.Check)
	{
		if (WHITE_SIDE == theApp.side)
			p = theApp.Board.White[chess_K].GetData();
		else
			p = theApp.Board.Black[chess_K].GetData();

		x = (theApp.Board.Trans_Bit64toPos(p) >> 3);
		y = (theApp.Board.Trans_Bit64toPos(p) & 7);
		if (theApp.MyGUI.Reverse_Board)
			x = 7 - x;
		theApp.MyGUI.CheckBackground.renderTexture(35 + 92 * y, 32 + 92 * x);

		x = theApp.Checker_pos[0] >> 3;
		y = theApp.Checker_pos[0] & 7;
		if (theApp.MyGUI.Reverse_Board)
			x = 7 - x;
		theApp.MyGUI.CheckBackground.renderTexture(35 + 92 * y, 32 + 92 * x);

		if (theApp.Checker_pos[1] != NONE)
		{
			x = theApp.Checker_pos[1] >> 3;
			y = theApp.Checker_pos[1] & 7;
			if (theApp.MyGUI.Reverse_Board)
				x = 7 - x;
			theApp.MyGUI.CheckBackground.renderTexture(35 + 92 * y, 32 + 92 * x);
		}
	}
}

void Renderer::DrawChessmen()
{
	unsigned long long chess, p;
	unsigned short x, y;

	for (int i = 0; i < 6; i++)
	{
		chess = theApp.Board.White[i].GetData();
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
				x = 7 - x;
			theApp.MyGUI.Texture_White[i].renderTexture(35 + 92 * y, 30 + 92 * x);
		}
	}

	for (int i = 0; i < 6; i++)
	{
		chess = theApp.Board.Black[i].GetData();
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
				x = 7 - x;
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