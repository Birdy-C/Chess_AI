#include"Chess.h"

extern MyApp theApp;

void MyApp::InitMatch()
{
	GameOver = false;
	side = WHITE_SIDE;
	PromotionRank = NONE;
	Move_record.clear();
	Checker_pos[0] = Checker_pos[1] = NONE;
	
	MyGUI.selectedX = MyGUI.selectedY = NONE;
	MyGUI.selectedPromotion = NONE;

	Board.Reset();
}

void GUI::reverseBoard()
{
	Reverse_Board = !Reverse_Board;
}

bool MyApp::MoveChess(const _Coordinate_ &orig_x, const _Coordinate_ &orig_y, const _Coordinate_ &x, const _Coordinate_ &y, 
	const _ChessPattern_ &chesstype, const _ChessType_ &desttype, const bool &side)
{
	ChessBoard temp = Board;
	Movement new_Move((orig_x << 3) | orig_y, (x << 3) | y, chesstype | (side << 3));

	Board.Move((orig_x << 3) | orig_y, (x << 3) | y, chesstype, side);
	Move_record.push_back(new_Move);

	//过路兵
	if (chess_P == chesstype)
	{
		if ((orig_x & 1) ^ (x & 1))				
		{										//这一步吃了过路兵需要把吃掉的兵去掉并复位标志En_passant
			if (Board.En_passant == y && (3 << side) - 1 == x)
				Board.Eaten(((3 + side) << 3) | y, chess_P, !side);
			Board.En_passant = NONE;
		}
		else									//兵走两格时写标志
			Board.En_passant = y;									
	}
	else
	{
		Board.En_passant = NONE;

		//王车易位
		if (chesstype == chess_K)
		{
			//王车易位时需要改变车的位子
			if (4 == orig_y)
			{
				if (y == 2)
					Board.Move((side ? 0 : 7) << 3, ((side ? 0 : 7) << 3) | 3, chess_R, side);
				else if (y == 6)
					Board.Move(((side ? 0 : 7) << 3) | 7, ((side ? 0 : 7) << 3) | 5, chess_R, side);
			}
			Board.Castling[side] = 0;
		}
		else if (chess_R == chesstype)
		{
			if ((Board.Castling[side] & LONG_CASTLING) && 0 == orig_y)
				Board.Castling[side] &= ~LONG_CASTLING;
			else if ((Board.Castling[side] & SHORT_CASTLING) && 7 == orig_y)
				Board.Castling[side] &= ~SHORT_CASTLING;
		}
	}	

	//以下为吃子响应，即落之后的点的desttype不为空时
	if (NONE != desttype)
	{
		Board.Eaten((x << 3) | y, desttype & 7, !side);

		if (chess_R == (desttype & 7) && Board.Castling[!side])
		{
			if (0 == y)
				Board.Castling[!side] &= ~LONG_CASTLING;
			else if (7 == y)
				Board.Castling[!side] &= ~SHORT_CASTLING;
		}
	}

	if (Board.DeltaAttack(side, Board.King_pos[side]))			//己方被将，棋步无效
	{
		Board = temp;
		Move_record.erase(Move_record.end() - 1);
		return false;
	}

	CheckControl(NONE != desttype);

	return true;
}

void MyApp::CheckControl(const bool &eat)
{
	if (theApp.PromotionRank == NONE)
	{
		vector<Movement> Legal_Movement;
		_Pos_ check_pos[2];
		enum check_type checktype[2];
		unsigned short check_count = Board.DeltaCheck(!side, check_pos, checktype);

		if (!check_count)
		{
			if (eat)
				Mix_PlayChannel(-1, theApp.MyGUI.Eat_sound, 0);
			else
				Mix_PlayChannel(-1, theApp.MyGUI.Move_sound, 0);
			theApp.Board.Check = false;
		}
		else
		{
			if (eat)
				Mix_PlayChannel(-1, theApp.MyGUI.Eat_sound, 0);
			Mix_PlayChannel(-1, theApp.MyGUI.Check_sound, 0);
			theApp.Board.Check = true;
			theApp.Checker_pos[0] = check_pos[0];
			if (check_count & 2)
			{
				theApp.Checker_pos[1] = check_pos[1];
				theApp.Board.Search_RidDoubleCheck(!side, Legal_Movement);
				if (Legal_Movement.size() == 0)
				{
					Mix_PlayChannel(-1, theApp.MyGUI.Mate_sound, 0);
					theApp.GameOver = true;
				}
			}
			else
			{
				theApp.Checker_pos[1] = NONE;
				theApp.Board.Search_RidCheck(!side, check_pos[0], checktype[0], Legal_Movement);
				if (Legal_Movement.size() == 0)
				{
					Mix_PlayChannel(-1, theApp.MyGUI.Mate_sound, 0);
					theApp.GameOver = true;
				}
			}
		}
	}
}