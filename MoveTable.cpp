#include "Movement.h"
#include "Board.h"

BitBoard eat_Pawn[2][64];
BitBoard move_Knight[64];
BitBoard move_King[64];
BitBoard rank_attacks[64][256] = { 0 };	//��
BitBoard file_attacks[64][256] = { 0 };	//��
BitBoard diag_A8H1_attacks[64][256] = { 0 };
BitBoard diag_H8A1_attacks[64][256] = { 0 };

BitBoard Between_bit[64][64];						//�����������Ƿ���ͬһ�����ϣ���������֮�䣨���������Ǳ�����λ�ӣ�0x81000000000081��־ֱ������
BitBoard Line_by_2pos[64][64];						//����������������ɵ�ֱ�ߣ�������ǹ���ֱ��

extern const _Pos_ start_pos_A8H1[64] = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28 };
extern const _Pos_ start_pos_H8A1[64] = { 0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63 };


//��һ��ת��Ϊһ�У����ڱ������ʱʹ��
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

// ������������
void ChessBoard::Init_BasicTable()
{
	BSFTable_Init();
}

//�������������������Ϊ�̶�����ֱ�Ӹ���λ��ȷ���������򣬶��ڱ��ĳ������·���жϲ����ʺϲ����û���ڱ�������
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
				/* �����ڵĸ��ӵ�����/����������һ�������ߵĸ��ӵ�����/����֮��Ĳ���С��3 */
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

//������ת��Bitmap������ֱ����˫�����ӵĲ�������ж����ӶԳ����ߺ���赲����������ŷ��������ڷ���
//�����ͻ�����Ƚ��õ������Ч�����У�����movement_step����ʱ��ĩλȥ���������򣬼�������Ч��������
void ChessBoard::Init_MoveTableStraight()
{
	BitBoard pos_bit;

	for (int pos = 0; pos < 8; pos++)
	{
		for (int pattern = 0; pattern < 256; pattern++)
		{
			pos_bit = mask(pos);
			//���㵱ǰλ������ĺϷ�������
			while (1)
			{
				pos_bit = pos_bit >> 1;
				if (pos_bit)
					rank_attacks[pos][pattern] |= pos_bit;
				else					//λ��Խ��
					break;
				if (pos_bit & pattern)	//�����ӳ�ͻ
					break;
			}
			//���㵱ǰλ�����ҵĺϷ�������
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
			//���㵱ǰλ�����ϵĺϷ�������
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
			//���㵱ǰλ�����µĺϷ�������
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
	//��ǰֻ�����˵�һ�е����������������һ�е�����������������ѭ�������
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

//��������ת45�ȵ�Bitmap������б����˫�����ӵĲ�������ж����Ӷ�����ߺ���赲����������ŷ��������ڷ���
//�����ͻ�����Ƚ��õ������Ч�����У�����movement_step����ʱ��ĩλȥ���������򣬼�������Ч��������
void ChessBoard::Init_MoveTableDiag()
{
	const uint8_t length_A8H1_diag[64] = { 8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8 };
	const uint8_t length_H8A1_diag[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1 };

	BitBoard pos_bit;
	short tempx, tempy, pattern_bit;

	/* ������A8-H1�����ƶ���Ԥ�ȼ��� */
	for (int pos = 0; pos < 64; pos++)
	{
		//�����ǰ����������б�����ǵڼ���
		const short index = (((pos >> 3) > (pos & 7)) ? (pos & 7) : (pos >> 3));

		//�����pattern�е���Чλ��Ӧ������pos���ڵ�б�߳���
		for (int pattern = 0; pattern < (1 << length_A8H1_diag[pos]); pattern++)
		{
			//�����������ӳ����Ϸ��ߵ��ŷ�
			tempx = pos >> 3;
			tempy = pos & 7;
			pos_bit = mask(pos);
			pattern_bit = 1 << index;			//��ȡ��ǰλ��������pattern�е�λ��ͨ���ƶ�pattern_bit�ж�б�����Ƿ������ӳ�ͻ
			while (1)
			{
				pos_bit = pos_bit >> 9;
				pattern_bit = pattern_bit >> 1;

				if (--tempx < 0 || --tempy < 0)
					break;
				else							//δԽ��
					diag_A8H1_attacks[pos][pattern] |= pos_bit;
				if (pattern_bit & pattern)		//�����ӳ�ͻ
					break;
			}

			//�����������ӳ����·��ߵ��ŷ�
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

	/* ������H8-A1�����ƶ���Ԥ�ȼ��� */
	for (int pos = 0; pos < 64; pos++)
	{
		const short index = ((pos >> 3) > 7 - (pos & 7) ? 7 - (pos & 7) : (pos >> 3));

		for (int pattern = 0; pattern < (1 << length_H8A1_diag[pos]); pattern++)
		{
			//�����������ӳ����Ϸ��ߵ��ŷ�
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

			//�����������ӳ����·��ߵ��ŷ�
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
				//���ߴ���ͬһ����
				if (rank_of(pos_A) == rank_of(pos_B))
				{
					Between_bit[pos_A][pos_B] = LINE_RANK;
					for (_Pos_ temp = pos_A + 1; temp < pos_B; temp++)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = rank_attacks[pos_A][0];
				}
				//���ߴ���ͬһ����
				else if (file_of(pos_A) == file_of(pos_B))
				{
					Between_bit[pos_A][pos_B] = LINE_FILE;
					for (_Pos_ temp = pos_A + 8; temp < pos_B; temp += 8)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = file_attacks[pos_A][0];
				}
				//���ߴ���ͬһ���Խ���
				else if (start_pos_A8H1[pos_A] == start_pos_A8H1[pos_B])
				{
					Between_bit[pos_A][pos_B] = LINE_A8H1;
					for (_Pos_ temp = pos_A + 9; temp < pos_B; temp += 9)
						Between_bit[pos_A][pos_B] |= mask(temp);
					Line_by_2pos[pos_A][pos_B] = diag_A8H1_attacks[pos_A][0];
				}
				//���ߴ���ͬһ���Խ���
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