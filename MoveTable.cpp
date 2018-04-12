#include "Movement.h"
#include "Board.h"

BitBoard eat_Pawn[2][64];
BitBoard move_Knight[64];
BitBoard move_King[64];
BitBoard rank_attacks[64][256] = { 0 };	//行
BitBoard file_attacks[64][256] = { 0 };	//列
BitBoard diag_A8H1_attacks[64][256] = { 0 };
BitBoard diag_H8A1_attacks[64][256] = { 0 };

BitBoard Between_bit[64][64];						//若两个棋子是否在同一条线上，返回它们之间（不包含它们本身）的位子，0x81000000000081标志直线类型
BitBoard Line_by_2pos[64][64];						//返回由两个棋子组成的直线，如果它们构成直线

extern const _Pos_ start_pos_A8H1[64] = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28 };
extern const _Pos_ start_pos_H8A1[64] = { 0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63 };


//将一行转换为一列，仅在表格生成时使用
static BitBoard transfer_rank2file(BitBoard data)
{
	BitBoard result = 0, b = 1;
	for (_Pos_ i = 0; i < 8; i++)
	{
		if (data & mask(i))
			result |= b << (i << 3);
	}
	return result;
}

// 基础表格的生成
void ChessBoard::Init_BasicTable()
{
	BSFTable_Init();
}

//王、马、兵的落子区域较为固定，可直接根据位置确定落子区域，对于兵的吃子与过路兵判断并不适合查表，故没有在表中体现
void ChessBoard::Init_MoveTable_KNP()
{
	int knightsq[KNIGHT_POSSIBLE_MOVE] = { -17, -15, -6, 10, 17, 15, 6, -10 };
	int kingsq[KING_POSSIBLE_MOVE] = { -9, -8, -7, -1, 1, 7, 8, 9 };
	BitBoard tempK, tempN, pos_bit;

	for (int pos = 0; pos < 64; pos++)
	{
		//Pawn
		pos_bit = mask(pos);

		eat_Pawn[WHITE_SIDE][pos] = (pos_bit & 0x7F7F7F7F7F7F7F7FULL) >> 7 | (pos_bit & 0xFEFEFEFEFEFEFEFEULL) >> 9;
		eat_Pawn[BLACK_SIDE][pos] = (pos_bit & 0xFEFEFEFEFEFEFEFEULL) << 7 | (pos_bit & 0x7F7F7F7F7F7F7F7FULL) << 9;

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
					tempN |= mask(new_pos);
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
					tempK |= mask(new_pos);
			}
		}
		move_King[pos] = tempK;
	}
}

//借助于转置Bitmap，依据直线上双方棋子的布局情况判断棋子对车或者后的阻挡情况并生成着法表，函数在发现
//区域冲突后首先将该点加入有效区域中，调用movement_step函数时在末位去除己方区域，即留下有效吃子区域
void ChessBoard::Init_MoveTableStraight()
{
	BitBoard pos_bit;

	for (int pos = 0; pos < 8; pos++)
	{
		for (int pattern = 0; pattern < 256; pattern++)
		{
			pos_bit = mask(pos);
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
			pos_bit = mask(pos);
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
			pos_bit = mask(pos << 3);
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
			pos_bit = mask(pos << 3);
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

//借助于旋转45度的Bitmap，依据斜线上双方棋子的布局情况判断棋子对象或者后的阻挡情况并生成着法表，函数在发现
//区域冲突后首先将该点加入有效区域中，调用movement_step函数时在末位去除己方区域，即留下有效吃子区域
void ChessBoard::Init_MoveTableDiag()
{
	const uint8_t length_A8H1_diag[64] = { 8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8 };
	const uint8_t length_H8A1_diag[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1 };

	BitBoard pos_bit;
	short tempx, tempy, pattern_bit;

	/* 象或后在A8-H1方向移动的预先计算 */
	for (int pos = 0; pos < 64; pos++)
	{
		//求出当前格子在所在斜线中是第几格
		const short index = (((pos >> 3) > (pos & 7)) ? (pos & 7) : (pos >> 3));

		//传入的pattern中的有效位数应不高于pos所在的斜线长度
		for (int pattern = 0; pattern < (1 << length_A8H1_diag[pos]); pattern++)
		{
			//计算从这个格子朝左上方走的着法
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask(pos);
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
			pos_bit = mask(pos);
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

			for (int ex_pattern = pattern + (1 << length_A8H1_diag[pos]); ex_pattern < 256; ex_pattern += (1 << length_A8H1_diag[pos]))
				diag_A8H1_attacks[pos][ex_pattern] = diag_A8H1_attacks[pos][pattern];
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
			pos_bit = mask(pos);
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
			pos_bit = mask(pos);
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

			for (int ex_pattern = pattern + (1 << length_H8A1_diag[pos]); ex_pattern < 256; ex_pattern += (1 << length_H8A1_diag[pos]))
				diag_H8A1_attacks[pos][ex_pattern] = diag_H8A1_attacks[pos][pattern];
		}
	}
}

void ChessBoard::Init_BetweenTable()
{
	memset(Between_bit, 0, 4096 * sizeof(BitBoard));

	for (_Pos_ pos_A = 0; pos_A < 64; pos_A++)
	{
		for (_Pos_ pos_B = 0; pos_B < 64; pos_B++)
		{
			if (pos_B == pos_A)
				continue;
			if (pos_B < pos_A)
			{
				Between_bit[pos_A][pos_B] = Between_bit[pos_B][pos_A];
				Line_by_2pos[pos_A][pos_B] = Line_by_2pos[pos_B][pos_A];
			}
			else
			{
				//两者处于同一横线
				if (rank_of(pos_A) == rank_of(pos_B))
				{
					Between_bit[pos_A][pos_B] = LINE_RANK;
					for (_Pos_ temp = pos_A + 1; temp < pos_B; temp++)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = rank_attacks[pos_A][0];
				}
				//两者处于同一竖线
				else if (file_of(pos_A) == file_of(pos_B))
				{
					Between_bit[pos_A][pos_B] = LINE_FILE;
					for (_Pos_ temp = pos_A + 8; temp < pos_B; temp += 8)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = file_attacks[pos_A][0];
				}
				//两者处于同一主对角线
				else if (start_pos_A8H1[pos_A] == start_pos_A8H1[pos_B])
				{
					Between_bit[pos_A][pos_B] = LINE_A8H1;
					for (_Pos_ temp = pos_A + 9; temp < pos_B; temp += 9)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = diag_A8H1_attacks[pos_A][0];
				}
				//两者处于同一副对角线
				else if (start_pos_H8A1[pos_A] == start_pos_H8A1[pos_B])
				{
					Between_bit[pos_A][pos_B] = LINE_H8A1;
					for (_Pos_ temp = pos_A + 7; temp < pos_B; temp += 7)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = diag_H8A1_attacks[pos_A][0];
				}
				else
					Line_by_2pos[pos_A][pos_B] = 0ULL;
			}
		}
	}
}