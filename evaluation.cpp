//Birdy

//基础得分 后900，车500，象325，马300，兵100
#define score_P_basic 1000
#define score_N_basic 3000
#define score_B_basic 3250
#define score_R_basic 5000
#define score_Q_basic 9000
#define score_K_basic 100000
#define SPACE_VALUE1 10
#define SPACE_VALUE2 10
#include"Chess.h"

extern _Bit64_ mask[64];

extern _Bit64_ move_Pawn[2][64];
extern _Bit64_ move_Knight[64];
extern _Bit64_ move_King[64];
extern  _Bit64_ rank_attacks[64][256] = { 0 };	//行
extern  _Bit64_ file_attacks[64][256] = { 0 };	//列
extern  _Bit64_ diag_A8H1_attacks[64][256] = { 0 };
extern  _Bit64_ diag_H8A1_attacks[64][256] = { 0 };

extern const short getline_shift[64] = { 0, 0, 0, 0, 0, 0, 0, 0, 8, 8, 8, 8, 8, 8, 8, 8, 16, 16, 16, 16, 16, 16, 16, 16, 24, 24, 24, 24, 24, 24, 24, 24, 32, 32, 32, 32, 32, 32, 32, 32, 40, 40, 40, 40, 40, 40, 40, 40, 48, 48, 48, 48, 48, 48, 48, 48, 56, 56, 56, 56, 56, 56, 56, 56 };
extern const short getline_shift_trans[64] = { 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56, 0, 8, 16, 24, 32, 40, 48, 56 };
extern const short length_A8H1_diag[64] = { 8, 7, 6, 5, 4, 3, 2, 1, 7, 8, 7, 6, 5, 4, 3, 2, 6, 7, 8, 7, 6, 5, 4, 3, 5, 6, 7, 8, 7, 6, 5, 4, 4, 5, 6, 7, 8, 7, 6, 5, 3, 4, 5, 6, 7, 8, 7, 6, 2, 3, 4, 5, 6, 7, 8, 7, 1, 2, 3, 4, 5, 6, 7, 8 };
extern const short length_H8A1_diag[64] = { 1, 2, 3, 4, 5, 6, 7, 8, 2, 3, 4, 5, 6, 7, 8, 7, 3, 4, 5, 6, 7, 8, 7, 6, 4, 5, 6, 7, 8, 7, 6, 5, 5, 6, 7, 8, 7, 6, 5, 4, 6, 7, 8, 7, 6, 5, 4, 3, 7, 8, 7, 6, 5, 4, 3, 2, 8, 7, 6, 5, 4, 3, 2, 1 };
extern const short shade_A8H1[64] = { 255, 127, 63, 31, 15, 7, 3, 1, 127, 255, 127, 63, 31, 15, 7, 3, 63, 127, 255, 127, 63, 31, 15, 7, 31, 63, 127, 255, 127, 63, 31, 15, 15, 31, 63, 127, 255, 127, 63, 31, 7, 15, 31, 63, 127, 255, 127, 63, 3, 7, 15, 31, 63, 127, 255, 127, 1, 3, 7, 15, 31, 63, 127, 255 };
extern const short shade_H8A1[64] = { 1, 3, 7, 15, 31, 63, 127, 255, 3, 7, 15, 31, 63, 127, 255, 127, 7, 15, 31, 63, 127, 255, 127, 63, 15, 31, 63, 127, 255, 127, 63, 31, 31, 63, 127, 255, 127, 63, 31, 15, 63, 127, 255, 127, 63, 31, 15, 7, 127, 255, 127, 63, 31, 15, 7, 3, 255, 127, 63, 31, 15, 7, 3, 1 };
extern const _Pos_ start_pos_A8H1[64] = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 28, 21, 15, 10, 6, 3, 1, 43, 36, 28, 21, 15, 10, 6, 3, 49, 43, 36, 28, 21, 15, 10, 6, 54, 49, 43, 36, 28, 21, 15, 10, 58, 54, 49, 43, 36, 28, 21, 15, 61, 58, 54, 49, 43, 36, 28, 21, 63, 61, 58, 54, 49, 43, 36, 28 };
extern const _Pos_ start_pos_H8A1[64] = { 0, 1, 3, 6, 10, 15, 21, 28, 1, 3, 6, 10, 15, 21, 28, 36, 3, 6, 10, 15, 21, 28, 36, 43, 6, 10, 15, 21, 28, 36, 43, 49, 10, 15, 21, 28, 36, 43, 49, 54, 15, 21, 28, 36, 43, 49, 54, 58, 21, 28, 36, 43, 49, 54, 58, 61, 28, 36, 43, 49, 54, 58, 61, 63 };

int line2count[256] = {0};


//====================================功能函数========================================//

void MyApp::eval_Init()
{
	for (int i = 0; i < 256; i++)
	{
		int temp = i;
		for (int j = 0; j < 8; j++)
		{
			if (temp & 1) 
			{
				line2count[i]++;
			}
			temp = temp >> 1;
		}
	}
}

//求出最低位的非零位是哪一位、没有返回负数
int ChessBoard::Lsb64(_Bit64_ Arg) {
	if (!Arg)
		return -1;
	int RetVal = 63;
	if (Arg & 0x00000000ffffffff) { RetVal -= 32; Arg &= 0x00000000ffffffff; }
	if (Arg & 0x0000ffff0000ffff) { RetVal -= 16; Arg &= 0x0000ffff0000ffff; }
	if (Arg & 0x00ff00ff00ff00ff) { RetVal -= 8; Arg &= 0x00ff00ff00ff00ff; }
	if (Arg & 0x0f0f0f0f0f0f0f0f) { RetVal -= 4; Arg &= 0x0f0f0f0f0f0f0f0f; }
	if (Arg & 0x3333333333333333) { RetVal -= 2; Arg &= 0x3333333333333333; }
	if (Arg & 0x5555555555555555) RetVal -= 1;
	return RetVal;
}

//返回有几个棋子、对兵/数目多的给特殊的符号flag = 1
int ChessBoard::countChess(_Bit64_ board, bool flag)
{
	int count = 0;
	if (1 == flag)
	{
		for (int i = 0; i < 8; i++)
		{
			count += line2count[board & 0xFF];
			board = board >> 8;
		}
	}
	else
	{
		if (!board)
			return count;
		if (!(board & 0x0000000FFFFFFFF))
		{
			for (int i = 0; i < 4; i++)
			{
				count += line2count[board & 0xFF];
				board = board >> 8;
			}
		}
		else
		{
			board = board >> 32;
		}
		if (!board)
			return count;

		for (int i = 0; i < 4; i++)
		{
			count += line2count[board & 0xFF];
			board = board >> 8;
		}
	}
	return count;
}


 //初始化攻击范围	
 //攻击范围初始化的时候保证每个棋子只由一颗棋子负责

int ChessBoard::init_attackArea(_ChessPattern_* attackAreaWhite, _ChessPattern_* attackAreaBlack)
{
	_Bit64_ temp;
	//P

	attackAreaWhite[chess_P] = (White[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) >> 7 | (White[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) >> 9;
	attackAreaBlack[chess_P] = (Black[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) << 7 | (Black[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) << 9;
	attackAreaWhite[6] |= attackAreaWhite[chess_P];
	attackAreaBlack[6] |= attackAreaBlack[chess_P];
	//N
	temp = White[chess_N].GetData();
	cout << endl << temp << "  " << White[chess_N].GetData() << endl;
	while(temp)
	{
		int pos = Lsb64(temp);
		attackAreaWhite[chess_N] |= move_Knight[pos];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}
	temp = Black[chess_N].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaBlack[chess_N] |= move_Knight[pos];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}
	attackAreaWhite[chess_N] &= !attackAreaWhite[6];
	attackAreaBlack[chess_N] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_N];
	attackAreaBlack[6] |= attackAreaBlack[chess_N];

	//B
	temp = White[chess_B].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaWhite[chess_B] |= diag_A8H1_attacks[pos][((White_All_L45.GetData() | Black_All_L45.GetData()) >> start_pos_A8H1[pos]) & (shade_A8H1[pos])];
		attackAreaWhite[chess_B] |= diag_H8A1_attacks[pos][((White_All_R45.GetData() | Black_All_R45.GetData()) >> start_pos_H8A1[pos]) & (shade_H8A1[pos])];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}

	temp = Black[chess_B].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaBlack[chess_B] |= diag_A8H1_attacks[pos][((White_All_L45.GetData() | Black_All_L45.GetData()) >> start_pos_A8H1[pos]) & (shade_A8H1[pos])];
		attackAreaBlack[chess_B] |= diag_H8A1_attacks[pos][((White_All_R45.GetData() | Black_All_R45.GetData()) >> start_pos_H8A1[pos]) & (shade_H8A1[pos])];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}

	attackAreaWhite[chess_B] &= !attackAreaWhite[6];
	attackAreaBlack[chess_B] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_B];
	attackAreaBlack[6] |= attackAreaBlack[chess_B];

	//R
	temp = White[chess_R].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaWhite[chess_R] |= rank_attacks[pos][((White_All.GetData() | Black_All.GetData()) >> getline_shift[pos]) & 255];
		attackAreaWhite[chess_R] |= file_attacks[pos][((White_All_trans.GetData() | Black_All_trans.GetData()) >> getline_shift_trans[pos]) & 255];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}

	temp = Black[chess_R].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaBlack[chess_R] |= rank_attacks[pos][((White_All.GetData() | Black_All.GetData()) >> getline_shift[pos]) & 255];
		attackAreaBlack[chess_R] |= file_attacks[pos][((White_All_trans.GetData() | Black_All_trans.GetData()) >> getline_shift_trans[pos]) & 255];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}

	attackAreaWhite[chess_R] &= !attackAreaWhite[6];
	attackAreaBlack[chess_R] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_R];
	attackAreaBlack[6] |= attackAreaBlack[chess_R];
	//Q
	temp = White[chess_Q].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaWhite[chess_Q] |= diag_A8H1_attacks[pos][((White_All_L45.GetData() | Black_All_L45.GetData()) >> start_pos_A8H1[pos]) & (shade_A8H1[pos])];
		attackAreaWhite[chess_Q] |= diag_H8A1_attacks[pos][((White_All_R45.GetData() | Black_All_R45.GetData()) >> start_pos_H8A1[pos]) & (shade_H8A1[pos])];
		attackAreaWhite[chess_Q] |= rank_attacks[pos][((White_All.GetData() | Black_All.GetData()) >> getline_shift[pos]) & 255];
		attackAreaWhite[chess_Q] |= file_attacks[pos][((White_All_trans.GetData() | Black_All_trans.GetData()) >> getline_shift_trans[pos]) & 255];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}
	temp = Black[chess_Q].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaBlack[chess_Q] |= diag_A8H1_attacks[pos][((White_All_L45.GetData() | Black_All_L45.GetData()) >> start_pos_A8H1[pos]) & (shade_A8H1[pos])];
		attackAreaBlack[chess_Q] |= diag_H8A1_attacks[pos][((White_All_R45.GetData() | Black_All_R45.GetData()) >> start_pos_H8A1[pos]) & (shade_H8A1[pos])];
		attackAreaBlack[chess_Q] |= rank_attacks[pos][((White_All.GetData() | Black_All.GetData()) >> getline_shift[pos]) & 255];
		attackAreaBlack[chess_Q] |= file_attacks[pos][((White_All_trans.GetData() | Black_All_trans.GetData()) >> getline_shift_trans[pos]) & 255];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}
	attackAreaWhite[chess_Q] &= !attackAreaWhite[6];
	attackAreaBlack[chess_Q] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_Q];
	attackAreaBlack[6] |= attackAreaBlack[chess_Q];

	//K
	temp = White[chess_K].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaWhite[chess_K] |= move_King[pos];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}
	temp = Black[chess_K].GetData();
	while (temp)
	{
		int pos = Lsb64(temp);
		attackAreaBlack[chess_K] |= move_King[pos];
		temp &= 0xFFFFFFFFFFFFFFFE << pos;
	}

	attackAreaWhite[chess_K] &= !attackAreaWhite[6];
	attackAreaBlack[chess_K] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_K];
	attackAreaBlack[6] |= attackAreaBlack[chess_K];

	return 0;
}


//====================================具体估值========================================//

//总的评估
int ChessBoard::evaluation()
{
	//白方为视角 我这里开七个最后一个记录总的
	_ChessPattern_ attackAreaWhite[7] = { 0 };
	_ChessPattern_ attackAreaBlack[7] = { 0 };
	init_attackArea(attackAreaWhite, attackAreaBlack);

	value_space(attackAreaWhite[6], attackAreaBlack[6]);
	cout << "value1" << '\t' << endl;
	cout << value_chess1() << endl;

	return 0;
}


//基础价值
int ChessBoard::value_chess1()
{
	int value = 0;
	//P
	value += (countChess(White[chess_P].GetData(), 1) - countChess(Black[chess_P].GetData(), 1)) * score_P_basic;
	//N
	value += (countChess(White[chess_N].GetData(), 1) - countChess(Black[chess_N].GetData(), 1)) * score_N_basic;
	//B
	value += (countChess(White[chess_B].GetData(), 1) - countChess(Black[chess_B].GetData(), 1)) * score_B_basic;
	//R
	value += (countChess(White[chess_R].GetData(), 1) - countChess(Black[chess_R].GetData(), 1)) * score_R_basic;
	//Q
	value += (countChess(White[chess_Q].GetData(), 1) - countChess(Black[chess_Q].GetData(), 1)) * score_Q_basic;

	return value;
}

//具体价值
int ChessBoard::value_chess2()
{
	//P
	//N
	//B
	//R
	//Q
	//K
	return 0;
}

//综合价值
int ChessBoard::value_chess3()
{
	//P
	//N
	//B
	//R
	//Q
	//K
	return 0;
}


//计算空间的价值
int ChessBoard::value_space(_ChessPattern_ attackAreaWhiteAll, _ChessPattern_ attackAreaBlackAll)
{

	int value = 0;

	//空间的估值
	value += (countChess(attackAreaWhiteAll, 1) - countChess(attackAreaBlackAll, 1)) * SPACE_VALUE1;


	//王周围的25个格子
	_ChessPattern_ space_K_white = 0, space_K_black = 0;
	space_K_white = White[chess_K].GetData();
	space_K_black = Black[chess_K].GetData();

	space_K_white |= (space_K_white & 0xFEFEFEFEFEFEFEFE) << 1 | (space_K_white & 0x7F7F7F7F7F7F7F7F) >> 1;
	space_K_white |= (space_K_white & 0x00FFFFFFFFFFFFFF) << 8 | (space_K_white & 0xFFFFFFFFFFFFFF00) >> 8;
	space_K_white |= (space_K_white & 0xFEFEFEFEFEFEFEFE) << 1 | (space_K_white & 0x7F7F7F7F7F7F7F7F) >> 1;
	space_K_white |= (space_K_white & 0x00FFFFFFFFFFFFFF) << 8 | (space_K_white & 0xFFFFFFFFFFFFFF00) >> 8;

	space_K_black |= (space_K_black & 0xFEFEFEFEFEFEFEFE) << 1 | (space_K_black & 0x7F7F7F7F7F7F7F7F) >> 1;
	space_K_black |= (space_K_black & 0x00FFFFFFFFFFFFFF) << 8 | (space_K_black & 0xFFFFFFFFFFFFFF00) >> 8;
	space_K_black |= (space_K_black & 0xFEFEFEFEFEFEFEFE) << 1 | (space_K_black & 0x7F7F7F7F7F7F7F7F) >> 1;
	space_K_black |= (space_K_black & 0x00FFFFFFFFFFFFFF) << 8 | (space_K_black & 0xFFFFFFFFFFFFFF00) >> 8;

	value += (-countChess(space_K_white & attackAreaBlackAll, 1) + countChess(space_K_black & attackAreaWhiteAll, 1)) * SPACE_VALUE2;
	//这里我觉得还可以加个参数，就这个如果王可移动范围过小的话可以深搜


	return 0;
}

//机动性 15000
int ChessBoard::value_flexibility()
{
	return 0;
}

int ChessBoard::value_develop()
{
	return 0;
}

/*
//P
//N
//B
//R
//Q
//K
*/