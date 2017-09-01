#include"Chess.h"
#include"AboutMove.h"

extern _Bit64_ mask[64];

_Bit64_ move_Pawn[2][64];
_Bit64_ move_Knight[64];
_Bit64_ move_King[64];
 _Bit64_ rank_attacks[64][256] = { 0 };	//行
 _Bit64_ file_attacks[64][256] = { 0 };	//列
 _Bit64_ diag_A8H1_attacks[64][256] = { 0 };
 _Bit64_ diag_H8A1_attacks[64][256] = { 0 };

static const short getline_shift[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16, 16, 16, 24, 24, 24, 24, 24, 24, 24, 24, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40, 40, 40, 40, 40, 40, 48, 48, 48, 48, 48, 48, 48, 48, 56, 56, 56, 56, 56, 56, 56, 56 };
static const short getline_shift_trans[64] = { 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56 };
static const short length_A8H1_diag[64] = { 8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8 };
static const short length_H8A1_diag[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1 };
static const short shade_A8H1[64] = { 255, 127, 63, 31, 15, 7, 3, 1, 127, 255, 127, 63, 31, 15, 7, 3, 63, 127, 255, 127, 63, 31, 15, 7, 31, 63, 127, 255, 127, 63, 31, 15, 15, 31, 63, 127, 255, 127, 63, 31, 7, 15, 31, 63, 127, 255, 127, 63, 3, 7, 15, 31, 63, 127, 255, 127, 1, 3, 7, 15, 31, 63, 127, 255 };
static const short shade_H8A1[64] = { 1, 3, 7, 15, 31, 63, 127, 255, 3, 7, 15, 31, 63, 127, 255, 127, 7, 15, 31, 63, 127, 255, 127, 63, 15, 31, 63, 127, 255, 127, 63, 31, 31, 63, 127, 255, 127, 63, 31, 15, 63, 127, 255, 127, 63, 31, 15, 7, 127, 255, 127, 63, 31, 15, 7, 3, 255, 127, 63, 31, 15, 7, 3, 1 };
static const _Pos_ start_pos_A8H1[64] = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28 };
static const _Pos_ start_pos_H8A1[64] = { 0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63 };

_Bit64_ Between_bit[64][64];//判断两个棋子是不是在同一条线上

static _Bit64_ transfer_rank2file(_Bit64_ data)
{
	_Bit64_ result = 0, b = 1;
	for (short i = 0; i < 8; i++)
	{
		if (data & mask[i])
			result |= b << (i << 3);
	}
	return result;
}

//王、马、兵的落子区域较为固定，可直接根据位置确定落子区域，对于兵的吃子与过路兵判断并不适合查表，故没有在表中体现
void MyApp::MoveTable_Init_KingKnightPawn()
{
	int knightsq[KNIGHT_POSSIBLE_MOVE] = { -17, -15, -6, 10, 17, 15, 6, -10 };
	int kingsq[KING_POSSIBLE_MOVE] = { -9, -8, -7, -1, 1, 7, 8, 9 };
	_Bit64_ tempK, tempN, pos_bit;

	for (int pos = 0; pos < 64; pos++)
	{
		//Pawn
		pos_bit = mask[pos];

		move_Pawn[WHITE_SIDE][pos] = ((pos_bit & 0x00FFFFFFFFFFFF00) >> 8) | ((pos_bit & 0x00FF000000000000) >> 16);
		move_Pawn[BLACK_SIDE][pos] = ((pos_bit & 0x00FFFFFFFFFFFF00) << 8) | ((pos_bit & 0x000000000000FF00) << 16);

		tempN = 0;
		int new_pos;
		for (int k = 0; k < KNIGHT_POSSIBLE_MOVE; k++)
		{
			new_pos = pos + knightsq[k];
			if (new_pos >= 0 && new_pos < 64)
			{
				/* 马所在的格子的行数/列数与它下一步可以走的格子的行数/列数之间的差须小于3 */
				if ((((pos >> 3) - (new_pos >> 3))*((pos >> 3) - (new_pos >> 3)) +
					((pos & 7) - (new_pos & 7))*((pos & 7) - (new_pos & 7))) == 5)
					tempN |= mask[new_pos];
			}
		}
		move_Knight[pos] = tempN;

		tempK = 0;
		for (int k = 0; k < KING_POSSIBLE_MOVE; k++)
		{
			new_pos = pos + kingsq[k];
			if (new_pos >= 0 && new_pos < 64)
			{
				if (((((pos >> 3) - (new_pos >> 3))*((pos >> 3) - (new_pos >> 3))) <= 1) &&
					((((pos & 7) - (new_pos & 7))*((pos & 7) - (new_pos & 7))) <= 1))
					tempK |= mask[new_pos];
			}
		}
		move_King[pos] = tempK;
	}
}

//直线与斜线的有效区域判定类似，分别借助于转置以及旋转45的Bitmap实现，传入的pattern应包含White_All与Black_All，函数在发现
//区域冲突后首先将该点加入有效区域中，调用movement_step函数时在末位去除己方区域，即留下有效吃子区域
void MyApp::MoveTable_Init_Straight()
{
	_Bit64_ pos_bit;

	for (int pos = 0; pos < 8; pos++)
	{
		for (int pattern = 0; pattern < 256; pattern++)
		{
			pos_bit = mask[pos];
			//计算当前位置向左的合法落子区
			while (1)
			{
				pos_bit = pos_bit >> 1;
				if (pos_bit)
					rank_attacks[pos][pattern] |= pos_bit;
				else					//位置越界
					break;
				if (pos_bit & pattern)	//与棋子冲突
					break;
			}
			//计算当前位置向右的合法落子区
			pos_bit = mask[pos];
			while (1)
			{
				pos_bit = pos_bit << 1;
				if (pos_bit < 0x100)
					rank_attacks[pos][pattern] |= pos_bit;
				else
					break;
				if (pos_bit & pattern)
					break;
			}
			//计算当前位置向上的合法落子区
			pos_bit = mask[pos << 3];
			while (1)
			{
				pos_bit = pos_bit >> 8;
				if (pos_bit)
					file_attacks[pos << 3][pattern] |= pos_bit;
				else
					break;
				if (pos_bit & transfer_rank2file(pattern))
					break;
			}
			//计算当前位置向下的合法落子区
			pos_bit = mask[pos << 3];
			while (1)
			{
				pos_bit = pos_bit << 8;
				if (pos_bit)
					file_attacks[pos << 3][pattern] |= pos_bit;
				else
					break;
				if (pos_bit & transfer_rank2file(pattern))
					break;
			}
		}
	}
	//先前只计算了第一行的向左向右区域与第一列的向上向下区域，利用循环补完表
	for (int pos = 8; pos < 64; pos++)
		for (int pattern = 0; pattern < 256; pattern++)
			rank_attacks[pos][pattern] = rank_attacks[pos & 7][pattern] << (pos & 0xFFF8);
	for (int pos = 1; pos < 64; pos++)
	{
		if (~(pos & 7) == 0)
			continue;
		for (int pattern = 0; pattern < 256; pattern++)
			file_attacks[pos][pattern] = file_attacks[pos & 0xFFF8][pattern] << (pos & 7);
	}
}

void MyApp::MoveTable_Init_Diag()
{
	_Bit64_ pos_bit;
	short tempx, tempy, pattern_bit;

	/* 象或后在A8-H1方向移动的预先计算 */
	for (int pos = 0; pos < 64; pos++)
	{
		//求出当前格子在所在斜线中是第几格
		const short index = (((pos >> 3) >(pos & 7)) ? (pos & 7) : (pos >> 3));

		//传入的pattern中的有效位数应不高于pos所在的斜线长度
		for (int pattern = 0; pattern < (1 << length_A8H1_diag[pos]); pattern++)
		{
			//计算从这个格子朝左上方走的着法
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask[pos];
			pattern_bit = 1 << index;			//获取当前位置在棋形pattern中的位，通过移动pattern_bit判断斜线中是否与棋子冲突
			while (1)
			{
				pos_bit = pos_bit >> 9;
				pattern_bit = pattern_bit >> 1;

				if (--tempx < 0 || --tempy < 0)
					break;
				else							//未越界
					diag_A8H1_attacks[pos][pattern] |= pos_bit;
				if (pattern_bit & pattern)		//与棋子冲突
					break;
			}

			//计算从这个格子朝右下方走的着法
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask[pos];
			pattern_bit = 1 << index;
			while (1)
			{
				pos_bit = pos_bit << 9;
				pattern_bit = pattern_bit << 1;

				if (++tempx >= 8 || ++tempy >= 8)
					break;
				else
					diag_A8H1_attacks[pos][pattern] |= pos_bit;
				if (pattern_bit & pattern)
					break;
			}
		}
	}

	/* 象或后在H8-A1方向移动的预先计算 */
	for (int pos = 0; pos < 64; pos++)
	{
		const short index = ((pos >> 3) > 7 - (pos & 7) ? 7 - (pos & 7) : (pos >> 3));

		for (int pattern = 0; pattern < (1 << length_H8A1_diag[pos]); pattern++)
		{
			//计算从这个格子朝右上方走的着法
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask[pos];
			pattern_bit = 1 << index;
			while (1)
			{
				pos_bit = pos_bit >> 7;
				pattern_bit = pattern_bit >> 1;

				if (--tempx < 0 || ++tempy >= 8)
					break;
				else
					diag_H8A1_attacks[pos][pattern] |= pos_bit;
				if (pattern_bit & pattern)
					break;
			}

			//计算从这个格子朝左下方走的着法
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask[pos];
			pattern_bit = 1 << index;
			while (1)
			{
				pos_bit = pos_bit << 7;
				pattern_bit = pattern_bit << 1;

				if (++tempx >= 8 || --tempy < 0)
					break;
				else
					diag_H8A1_attacks[pos][pattern] |= pos_bit;
				if (pattern_bit & pattern)
					break;
			}
		}
	}
}

void MyApp::BetweenTable_Init()
{
	memset(Between_bit, 0, 4096 * sizeof(_Bit64_));

	for (_Pos_ pos_A = 0; pos_A < 64; pos_A++)
	{
		for (_Pos_ pos_B = 0; pos_B < 64; pos_B++)
		{
			if (pos_B == pos_A)
				continue;
			if (pos_B < pos_A)
				Between_bit[pos_A][pos_B] = Between_bit[pos_B][pos_A];
			else
			{
				//两者处于同一横线
				if (pos_A >> 3 == pos_B >> 3)
					for (_Pos_ temp = pos_A + 1; temp < pos_B; temp++)
						Between_bit[pos_A][pos_B] |= mask[temp];
				//两者处于同一竖线
				else if ((pos_A & 7) == (pos_B & 7))
					for (_Pos_ temp = pos_A + 8; temp < pos_B; temp += 8)
						Between_bit[pos_A][pos_B] |= mask[temp];
				//两者处于同一主对角线
				else if (start_pos_A8H1[pos_A] == start_pos_A8H1[pos_B])
					for (_Pos_ temp = pos_A + 9; temp < pos_B; temp += 9)
						Between_bit[pos_A][pos_B] |= mask[temp];
				else if (start_pos_H8A1[pos_A] == start_pos_H8A1[pos_B])
					for (_Pos_ temp = pos_A + 7; temp < pos_B; temp += 7)
						Between_bit[pos_A][pos_B] |= mask[temp];
			}
		}
	}
}

void MyApp::MoveTable_Init()
{
	MoveTable_Init_KingKnightPawn();

	MoveTable_Init_Straight();

	MoveTable_Init_Diag();

	BetweenTable_Init();
}

_Bit64_ ChessBoard::movement_step(const _ChessType_ &chess, const _Pos_ &pos)
{
	if (NONE == chess)
		return 0;

	bool color = chess & BLACK_CHESS_BIT;
	_ChessPattern_ chess_type = chess & 7;
	_Bit64_ final_position = 0;		//for return 
	_Bit64_ posbit = mask[pos];

	switch (chess_type)
	{
	case chess_P:
		if (WHITE_SIDE == color)
		{
			final_position = move_Pawn[WHITE_SIDE][pos];
			//去除掉前方一格被挡仍允许走两格的情况
			final_position &= (~(((White_All.GetData() | Black_All.GetData()) & mask[pos - 8]) >> 8));
			//去除掉前方被敌方棋子阻挡仍允许直线吃子的情况
			final_position &= ~(Black_All.GetData());
			//斜向吃子，排除<<7引起的同行A列H列互吃的BUG
			final_position |= ((posbit & (_Bit64_)0x7F7F7F7F7F7F7F7F) >> 7 | (posbit & (_Bit64_)0xFEFEFEFEFEFEFEFE) >> 9) & Black_All.GetData();
			//过路兵判定
			if (En_passant != NONE && pos >> 3 == 3)
			{
				if ((pos & 7) == En_passant - 1)
					final_position |= mask[pos] >> 7;
				else if ((pos & 7) == En_passant + 1)
					final_position |= mask[pos] >> 9;
			}
		}
		else
		{
			final_position = move_Pawn[BLACK_SIDE][pos];
			final_position &= (~(((White_All.GetData() | Black_All.GetData()) & mask[pos + 8]) << 8));
			final_position &= ~(White_All.GetData());
			final_position |= ((posbit & (_Bit64_)0xFEFEFEFEFEFEFEFE) << 7 | (posbit & (_Bit64_)0x7F7F7F7F7F7F7F7F) << 9) & White_All.GetData();
			if (En_passant != NONE && pos >> 3 == 4)
			{
				if ((pos & 7) == En_passant - 1)
					final_position |= mask[pos] << 9;
				else if ((pos & 7) == En_passant + 1)
					final_position |= mask[pos] << 7;
			}
		}
		break;
	case chess_N:
		final_position = move_Knight[pos];
		break;
	case chess_Q:
		//利用switch语句的特点计算皇后的合法落子区
	case chess_R:
		final_position |= rank_attacks[pos][((White_All.GetData() | Black_All.GetData()) >> getline_shift[pos]) & 255];
		final_position |= file_attacks[pos][((White_All_trans.GetData() | Black_All_trans.GetData()) >> getline_shift_trans[pos]) & 255];
		if (chess_R == chess_type)
			break;
	case chess_B:
		final_position |= diag_A8H1_attacks[pos][((White_All_L45.GetData() | Black_All_L45.GetData()) >> start_pos_A8H1[pos]) & (shade_A8H1[pos])];
		final_position |= diag_H8A1_attacks[pos][((White_All_R45.GetData() | Black_All_R45.GetData()) >> start_pos_H8A1[pos]) & (shade_H8A1[pos])];
		break;
	case chess_K:
		final_position = move_King[pos];
		if (WHITE_SIDE == color)
		{
			//首先判断易位FLAG以节省开销
			if (Castling[WHITE_SIDE] && !Check)
			{
				//取出56~60位与10001比较并判将
				if ((Castling[WHITE_SIDE] & LONG_CASTLING) && (((White_All.GetData() | Black_All.GetData()) >> 56) & 31) == 17 &&
					!DeltaAttack(WHITE_SIDE, King_pos[WHITE_SIDE] - 1) && !DeltaAttack(WHITE_SIDE, King_pos[WHITE_SIDE] - 2))
					final_position |= mask[58];
				//取出60~63位与1001比较并判将
				if ((Castling[WHITE_SIDE] & SHORT_CASTLING) && (((White_All.GetData() | Black_All.GetData()) >> 60) & 15) == 9 && 
					!DeltaAttack(WHITE_SIDE, King_pos[WHITE_SIDE] + 1) && !DeltaAttack(WHITE_SIDE, King_pos[WHITE_SIDE] + 2))
					final_position |= mask[62];
			}
		}	
		else if (Castling[BLACK_SIDE] && !Check)
		{
			if ((Castling[BLACK_SIDE] & LONG_CASTLING) && ((White_All.GetData() | Black_All.GetData()) & 31) == 17 &&
				!DeltaAttack(BLACK_SIDE, King_pos[BLACK_SIDE] - 1) && !DeltaAttack(BLACK_SIDE, King_pos[BLACK_SIDE] - 2))
				final_position |= mask[2];
			if ((Castling[BLACK_SIDE] & SHORT_CASTLING) &&(((White_All.GetData() | Black_All.GetData()) >> 4) & 15) == 9 &&
				!DeltaAttack(BLACK_SIDE, King_pos[BLACK_SIDE] + 1) && !DeltaAttack(BLACK_SIDE, King_pos[BLACK_SIDE] + 2))
				final_position |= mask[6];
		}
		break;
	}
	//去除掉被己方棋子占领的区域
	if (WHITE_SIDE == color)
		final_position &= ~White_All.GetData();
	else
		final_position &= ~Black_All.GetData();

	return final_position;
}