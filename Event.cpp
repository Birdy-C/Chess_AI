#include"Chess.h"

extern MyApp theApp;
extern _Zobrist64_ side_zobrist, En_passent_zobrist[_COORDINATE_COUNT_ + 1], castling_zobrist[_SIDE_COUNT_][3];

//������飺��ʼ�����������˳�ʼ������֮�⻹��Ҫ��ʼ��flag
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

//������飺������UI���������ĵ��ý���һ�����ӣ�������ʶ�����flag��������λ����·��������Ȳ�����������Ӧ�����û������Ӳ��Ϸ������ܾ�����
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

//������飺��Ҫ����AI����ʱ����һ�����ӣ�������ʶ�����flag��������λ����·��������Ȳ�����������Ӧ��������Ĭ�������ǺϷ��ģ������벻�Ϸ���Movement������ɴ���
void MyApp::MoveChess(const Movement new_Move)
{
	Move_record.push_back(new_Move);
	Board.MoveChess(new_Move);
	
	side = !side;
	step_count++;

	CheckControl(NONE != Board.move_capture());						//disable this sentence when debugging
}

//������飺������UI�ĵ��ã�������Ӻ��Ƿ��г��ӡ��Ƿ񽫾����Ƿ���������������Ч
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

//������飺������UI���ã�����ʶ��һ����orig_pos��dest_pos���岽��ʲô����
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