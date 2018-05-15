#include"Chess.h"

extern MyApp theApp;
extern _Zobrist64_ side_zobrist, En_passent_zobrist[_COORDINATE_COUNT_ + 1], castling_zobrist[_SIDE_COUNT_][3];

//函数简介：初始化比赛，除了初始化棋盘之外还需要初始化flag
void MyApp::InitMatch()
{
	GameOver = false;
	side = WHITE_SIDE;
	PromotionRank = NONE;
	Move_record.clear();
	step_count = 0;
	
	MyGUI.selectedX = MyGUI.selectedY = NONE;
	MyGUI.selectedPromotion = NONE;

	Board.Reset();
}

void GUI::reverseBoard()
{
	Reverse_Board = !Reverse_Board;
}

//函数简介：仅用于UI中鼠标操作的调用进行一步落子，函数将识别各种flag、王车易位、过路兵、升变等操作并作出响应，若用户的落子不合法将被拒绝落子
void MyApp::MoveChess(const _Coordinate_ &orig_x, const _Coordinate_ &orig_y, const _Coordinate_ &x, const _Coordinate_ &y)
{
	//ChessBoard temp = Board;
	_Pos_ orig = (orig_x << 3) | orig_y, dest = (x << 3) | y;
	_MoveType_ mvtp = Board.Delta_MoveType(orig, dest);

	if (MOVE_PROMOTION == mvtp)
	{
		mvtp = MOVE_NORMAL;
		PromotionRank = y;
	}
	else
		PromotionRank = NONE;


	Movement new_Move(orig, dest, mvtp);
	Move_record.push_back(new_Move);
	Board.MoveChess(new_Move);

	if (NONE == PromotionRank)
		side = !side;
	step_count++;

	//system("cls");
	CheckControl(NONE != Board.move_capture());
	//Board.DEBUG_PRINT_EXPAND(side);
}

//函数简介：主要用于AI搜索时进行一步落子，函数将识别各种flag、王车易位、过路兵、升变等操作并作出响应，函数将默认落子是合法的，若传入不合法的Movement将会造成错误
void MyApp::MoveChess(const Movement new_Move)
{
	Move_record.push_back(new_Move);
	Board.MoveChess(new_Move);
	
	side = !side;
	step_count++;

	CheckControl(NONE != Board.move_capture());						//disable this sentence when debugging
}

//函数简介：仅用于UI的调用，针对落子后是否有吃子、是否将军、是否和棋或将死并播放音效
void MyApp::CheckControl(const bool &eat)
{
	if (NONE == theApp.PromotionRank)
	{
		if (50 == Board.draw50())
		{
			GameOver = true;
			MyGUI.gRenderer.RenderApp();
			MessageBox(NULL, TEXT("Draw-in!"), TEXT("Fly sky"), MB_OK);
			return;
		}

		Movement mv;
		MoveGenerator mv_gen(Board, mv, side);

		if (MAIN_MOVE == mv_gen.get_state())
		{
			if (eat)
				Mix_PlayChannel(-1, theApp.MyGUI.Eat_sound, 0);
			else
				Mix_PlayChannel(-1, theApp.MyGUI.Move_sound, 0);

			if (!mv_gen.next_move().Exist())
			{
				theApp.GameOver = true;
				MyGUI.gRenderer.RenderApp();
				MessageBox(NULL, TEXT("Draw-in!"), TEXT("Fly sky"), MB_OK);
			}
		}
		else
		{
			if (eat)
				Mix_PlayChannel(-1, theApp.MyGUI.Eat_sound, 0);
			Mix_PlayChannel(-1, theApp.MyGUI.Check_sound, 0);

			if (!mv_gen.next_move().Exist())
			{
				Mix_PlayChannel(-1, theApp.MyGUI.Mate_sound, 0);
				theApp.GameOver = true;
				MyGUI.gRenderer.RenderApp();
				MessageBox(NULL, TEXT("Check-Mate!"), TEXT("Fly sky"), MB_OK);
			}
		}
	}
	else
	{
		if (eat)
			Mix_PlayChannel(-1, theApp.MyGUI.Eat_sound, 0);
		else
			Mix_PlayChannel(-1, theApp.MyGUI.Move_sound, 0);
	}
}

//函数简介：仅用于UI调用，用于识别一个从orig_pos到dest_pos的棋步是什么类型
_MoveType_ ChessBoard::Delta_MoveType(const _Pos_ orig, const _Pos_ dest) const
{
	if (chess_K == (chess_at(orig) & CHESS_PATTERN) && (orig == dest + 2 || orig == dest - 2))
		return MOVE_CAST;

	if (chess_P == (chess_at(orig) & CHESS_PATTERN))
	{
		if (mask(orig) & life_line(side))
			return MOVE_PROMOTION;
		if (En_passant() == file_of(dest) && mask(orig) & push_line(side))
			return MOVE_ENPASS;
	}
	return MOVE_NORMAL;
}