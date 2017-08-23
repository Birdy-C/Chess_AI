#include"Chess.h"

extern MyApp theApp;
extern _Bit64_ mask[64];

void MyApp::handleMouseEvent(const SDL_Event* e)
{
	int MouseX, MouseY;

	SDL_GetMouseState(&MouseX, &MouseY);
	
	if (PromotionRank != NONE)
		Mouse_Promotion(MouseX, MouseY, e);
	else if (!GameOver && e->type == SDL_MOUSEBUTTONDOWN)
		Mouse_ChessMotivation(MouseX, MouseY);
}

void MyApp::Mouse_ChessMotivation(int MouseX, int MouseY)
{
	_Coordinate_ x, y;

	if (MouseX > 35 && MouseY > 30 && MouseX < 770 && MouseY < 765)
	{
		MouseX -= 35, MouseY -= 30;
		x = MouseY / 92;
		y = MouseX / 92;

		if ((x * 92 + 5 < MouseY) && (x * 92 + 87 > MouseY) && (y * 92 + 5 < MouseX) && (y * 92 + 87 > MouseX))
		{
			if (MyGUI.Reverse_Board)
				x = 7 - x;

			if (MyGUI.selectedX == x && MyGUI.selectedY == y)
			{
				MyGUI.selectedX = MyGUI.selectedY = NONE;
				Board.Debug.Board = 0;
			}
			else if (!(~(MyGUI.selectedX & MyGUI.selectedY)) && (Board.DeltaChess(x, y) >> 3) == side)
			{
				MyGUI.selectedX = x, MyGUI.selectedY = y;
				//debug
				if (Board.DeltaChess(x, y) != NONE)
					Board.Debug.Board = Board.movement_step(Board.DeltaChess(MyGUI.selectedX, MyGUI.selectedY), (MyGUI.selectedX << 3) + MyGUI.selectedY);
			}
			else
			{
				_ChessPattern_ chesstype = Board.DeltaChess(MyGUI.selectedX, MyGUI.selectedY);
				_ChessPattern_ desttype = Board.DeltaChess(x, y);

				if (chesstype == NONE && desttype >> 3 == side)
				{
					MyGUI.selectedX = x, MyGUI.selectedY = y;
					//debug
					if (Board.DeltaChess(x, y) != NONE)
						Board.Debug.Board = Board.movement_step(Board.DeltaChess(MyGUI.selectedX, MyGUI.selectedY), (MyGUI.selectedX << 3) + MyGUI.selectedY);
					else
						Board.Debug.Board = 0;
				}
				else if (side == WHITE_SIDE && !(chesstype & 8) && (desttype & 8) ||
					side == BLACK_SIDE && chesstype & 8 && (desttype == NONE || !(desttype & 8)))
				{
					if (mask[(x << 3) | y] & Board.movement_step(chesstype, (MyGUI.selectedX << 3) | MyGUI.selectedY))
					{																					//真正下棋的分支
						bool success = MoveChess(MyGUI.selectedX, MyGUI.selectedY, x, y, chesstype & 7, desttype, side);

						if (success)
						{
							if (side == WHITE_SIDE && chesstype == 0 && x == 0 || side == BLACK_SIDE && chesstype == 8 && x == 7)
								PromotionRank = y;
							else
								side = !side;
						}
					}
					MyGUI.selectedX = MyGUI.selectedY = NONE;
					Board.Debug.Board = 0;
					if (GameOver)
					{
						gRenderer.RenderApp();
						MessageBox(NULL, TEXT("Check-Mate!"), TEXT("Fly sky"), MB_OK);
					}
				}
				else
				{
					//debug
					if (Board.DeltaChess(x, y) != NONE)
					{
						MyGUI.selectedX = x, MyGUI.selectedY = y;
						Board.Debug.Board = Board.movement_step(Board.DeltaChess(MyGUI.selectedX, MyGUI.selectedY), (MyGUI.selectedX << 3) + MyGUI.selectedY);
					}
					else
					{
						MyGUI.selectedX = MyGUI.selectedY = NONE;
						Board.Debug.Board = 0;
					}						
				}
			}
		}
	}
}

void MyApp::Mouse_Promotion(int MouseX, int MouseY, const SDL_Event* e)
{
	_ChessType_ chesstype;
	_Coordinate_ x;

	if (e->type == SDL_MOUSEMOTION && MouseY >= 395 && MouseY <= 445)
	{
		if (MouseX >= 256 && MouseX <= 306)
			MyGUI.selectedPromotion = 0;
		else if (MouseX >= 335 && MouseX <= 385)
			MyGUI.selectedPromotion = 1;
		else if (MouseX >= 414 && MouseX <= 464)
			MyGUI.selectedPromotion = 2;
		else if (MouseX >= 493 && MouseX <= 543)
			MyGUI.selectedPromotion = 3;
		else
			MyGUI.selectedPromotion = -1;
	}
	if (e->type == SDL_MOUSEBUTTONDOWN && MyGUI.selectedPromotion != -1)
	{
		if (side == WHITE_SIDE)
		{
			x = 0;
			switch (MyGUI.selectedPromotion)
			{
			case 0:
				Board.White[chess_Q].Eaten(PromotionRank); 
				chesstype = chess_Q; break;
			case 1:
				Board.White[chess_R].Eaten(PromotionRank); 
				chesstype = chess_R; break;
			case 2:
				Board.White[chess_B].Eaten(PromotionRank); 
				chesstype = chess_B; break;
			case 3:
				Board.White[chess_N].Eaten(PromotionRank); 
				chesstype = chess_N; break;
			}
			Board.White[chess_P].Eaten(PromotionRank);
		}
		else
		{
			x = 7;
			switch (MyGUI.selectedPromotion)
			{
			case 0:
				Board.Black[chess_Q].Eaten(56 | PromotionRank); 
				chesstype = chess_Q | BLACK_CHESS_BIT; break;
			case 1:
				Board.Black[chess_R].Eaten(56 | PromotionRank); 
				chesstype = chess_R | BLACK_CHESS_BIT; break;
			case 2:
				Board.Black[chess_B].Eaten(56 | PromotionRank); 
				chesstype = chess_B | BLACK_CHESS_BIT; break;
			case 3:
				Board.Black[chess_N].Eaten(56 | PromotionRank); 
				chesstype = chess_N | BLACK_CHESS_BIT; break;
			}
			Board.Black[chess_P].Eaten(56 | PromotionRank);
		}

		PromotionRank = NONE;
		CheckControl(false);

		MyGUI.selectedPromotion = NONE;
		side = !side;
	}
}