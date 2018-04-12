#include"Chess.h"

extern MyApp theApp;

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
				x = 7 - x, y = 7 - y;

			_Pos_ old_pos = (MyGUI.selectedX << 3) | MyGUI.selectedY, new_pos = (x << 3) | y;

			if (old_pos == new_pos)
			{
				MyGUI.selectedX = MyGUI.selectedY = NONE;
				Clear(HighLight);
			}
			else if (SQ_COUNT <= old_pos)
			{
				if (CHESS_NONE != Board.chess_at(new_pos) && side_of(Board.chess_at(new_pos)) == side)
				{
					MyGUI.selectedX = x, MyGUI.selectedY = y;
					HighLight = Board.movement_step(new_pos, true);			//debug.......................
					//HighLight.Init(Board.AttackRange[(x << 3) | y]);
				}
			}
			else
			{
				assert(old_pos < 64 && new_pos < 64);
				_ChessType_ new_tp = Board.chess_at(new_pos);

				if (mask(new_pos) & Board.movement_step(old_pos))
				{
					if (mask(new_pos) & HighLight)						//真正下棋的分支
						MoveChess(MyGUI.selectedX, MyGUI.selectedY, x, y);
					else
						Mix_PlayChannel(-1, theApp.MyGUI.Illegal_sound, 0);

					MyGUI.selectedX = MyGUI.selectedY = NONE;
					Clear(HighLight);
				}
				
				else if (CHESS_NONE != new_tp && side_of(new_tp) == side)
				{
					MyGUI.selectedX = x, MyGUI.selectedY = y;
					HighLight = Board.movement_step(new_pos, true);		//debug.......................
					//HighLight.Init(Board.AttackRange[(x << 3) | y]);
				}
				else
				{
					MyGUI.selectedX = MyGUI.selectedY = NONE;
					Clear(HighLight);
				}
			}
		}
	}
}

void MyApp::Mouse_Promotion(int MouseX, int MouseY, const SDL_Event* e)
{
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
			MyGUI.selectedPromotion = NONE;
	}
	if (e->type == SDL_MOUSEBUTTONDOWN && MyGUI.selectedPromotion != NONE)
	{
		Board.Eaten(PromotionRank | (side * 56));

		switch (MyGUI.selectedPromotion)
		{
		case 0:
			Board.Place(PromotionRank | (side * 56), (side * BLACK_CHESS_BIT) | chess_Q);
			Move_record[step_count - 1].SetPromotionPattern(chess_Q);
			break;
		case 1:
			Board.Place(PromotionRank | (side * 56), (side * BLACK_CHESS_BIT) | chess_R);
			Move_record[step_count - 1].SetPromotionPattern(chess_R);
			break;
		case 2:
			Board.Place(PromotionRank | (side * 56), (side * BLACK_CHESS_BIT) | chess_B);
			Move_record[step_count - 1].SetPromotionPattern(chess_B); 
			break;
		case 3:
			Board.Place(PromotionRank | (side * 56), (side * BLACK_CHESS_BIT) | chess_N);
			Move_record[step_count - 1].SetPromotionPattern(chess_N); 
			break;
		}
		
		Board.Set_atk_area(PromotionRank | (side * 56));		//由于延迟了着步确定，需要在着步确定时更新升变位置AttackRange

		MyGUI.selectedPromotion = NONE;
		PromotionRank = NONE;

		side = !side;
		CheckControl(false);

		Board.DEBUG_PRINT_EXPAND(side);
	}
}