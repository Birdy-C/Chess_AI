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
#define PAWN_VALUE1 12 //孤兵
#define PAWN_VALUE2 13 //叠兵
#define PAWN_VALUE3 27 //通路兵
#define KNIGHT_VALUE 7
#define ROOK_VALUE 7 //占据通路的车
#include"Chess.h"
#include"AboutMove.h"

extern _Bit64_ mask[64];


int line2count[256] = {0};
int chessValue[6] = { score_P_basic, score_N_basic, score_B_basic, score_R_basic, score_Q_basic, score_K_basic };
int flexValue[6] = { 0,5,5,5,5,5 };	//机动性的定义（如果他是线性的话）
int Value2[6] = { 0,7,6,5,5,5 };	//单个棋子控制区域 
int pawnValue[8] = { 0,0,0,0,100,1500,2000,0 };	//兵的价值
//====================================功能函数====================================//

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
		if (board & 0x0000000FFFFFFFF)
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

int ChessBoard::init_attackArea(_Bit64_* attackAreaWhite, _Bit64_* attackAreaBlack)
{
	_Bit64_ temp;

	//P
	attackAreaWhite[chess_P] = (White[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) >> 7 | (White[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) >> 9;
	attackAreaBlack[chess_P] = (Black[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) << 7 | (Black[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) << 9;
	attackAreaWhite[6] |= attackAreaWhite[chess_P];
	attackAreaBlack[6] |= attackAreaBlack[chess_P];

	//N66
	//cout << temp << endl;
	temp = White[chess_N].GetData();
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
	//attackAreaWhite[chess_N] &= !attackAreaWhite[6];
	//attackAreaBlack[chess_N] &= !attackAreaBlack[6];
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

	//attackAreaWhite[chess_B] &= !attackAreaWhite[6];
	//attackAreaBlack[chess_B] &= !attackAreaBlack[6];
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

	//attackAreaWhite[chess_R] &= !attackAreaWhite[6];
	//attackAreaBlack[chess_R] &= !attackAreaBlack[6];
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
	//attackAreaWhite[chess_Q] &= !attackAreaWhite[6];
	//attackAreaBlack[chess_Q] &= !attackAreaBlack[6];
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

	//attackAreaWhite[chess_K] &= !attackAreaWhite[6];
	//attackAreaBlack[chess_K] &= !attackAreaBlack[6];
	attackAreaWhite[6] |= attackAreaWhite[chess_K];
	attackAreaBlack[6] |= attackAreaBlack[chess_K];

	return 0;
}


//====================================具体估值====================================//

//总的评估
int ChessBoard::evaluation()
{
	//白方为视角 我这里开七个最后一个记录总的
	_Bit64_ attackAreaWhite[7] = { 0 };
	_Bit64_ attackAreaBlack[7] = { 0 };
	init_attackArea(attackAreaWhite, attackAreaBlack);
	int value[6];
	int result = 0;;

	value[0] = value_chess1();
	value[1] = value_chess2(attackAreaWhite, attackAreaBlack);
	value[2] = value_chess3();
	value[3] = value_space(attackAreaWhite[6], attackAreaBlack[6]);
	value[4] = value_flexibility(attackAreaWhite, attackAreaBlack);
	value[5] = value_develop(attackAreaWhite, attackAreaBlack);

	cout << "value1" << '\t' << "value2" << '\t' << "value3" << '\t'; 
	cout << "value_space" << '\t' << "value_flexibility" << '\t' << "value_develop" << '\t' << endl;
	
	for (int i = 0; i < 6; i++)
	{
		cout << value[i] << '\t';
		result += value[i];
	}
	cout << endl;

	return result;
}


//0基础价值1
int ChessBoard::value_chess1()
{
	/*
	这个我觉得可以和延拓在一起这样……
	然后对于不计算子力只用走了一步的变换来看的话……我觉得是可以的。、
	对于子力的影响的话,也是藐视因子和局势改变的话，可以在下了一定步数之后重新计算
	*/
	int value = 0;
	//P
	value += (countChess(White[chess_P].GetData(), 1) - countChess(Black[chess_P].GetData(), 1)) * score_P_basic;
	//N
	value += (countChess(White[chess_N].GetData(), 0) - countChess(Black[chess_N].GetData(), 0)) * score_N_basic;
	//B
	value += (countChess(White[chess_B].GetData(), 0) - countChess(Black[chess_B].GetData(), 0)) * score_B_basic;
	//R
	value += (countChess(White[chess_R].GetData(), 0) - countChess(Black[chess_R].GetData(), 0)) * score_R_basic;
	//Q
	value += (countChess(White[chess_Q].GetData(), 0) - countChess(Black[chess_Q].GetData(), 0)) * score_Q_basic;

	return value;
}


//1具体价值2
int ChessBoard::value_chess2(_Bit64_ *attackAreaWhite, _Bit64_ *attackAreaBlack)
{
	int value = 0;
	//P
	{
		_Bit64_ Pawn_mask = 0xFF;
		for (int i = 0; i < 8; i++)
		{
			value += countChess(White[chess_P].GetData()&Pawn_mask, 0)* pawnValue[7 - i];
			value -= countChess(Black[chess_P].GetData()&Pawn_mask, 0)* pawnValue[i];
			Pawn_mask = Pawn_mask << 8;
		}
	}
	//N
	_Bit64_ temp1 = 0, temp2 = 0;
	//这一块我本意是用，棋子的位置来表示，在中间的话价值高，在靠近边线价值底这样
	//然后偷懒写成了这样子的……应该得改
	for (int chess = 1; chess < 5; chess++)
	{
		temp1 = attackAreaWhite[chess];
		temp2 = attackAreaBlack[chess];
		value += (countChess(temp1, 1) - countChess(temp2, 1)) * flexValue[chess];
	}


	return value;
}

//2综合价值3
int ChessBoard::value_chess3()
{
	int value = 0;

	//P
	//纵向统计，孤兵、叠兵、通路兵
	int number_white = 0, number_black = 0;
	{
		int temp;
		_Bit64_ Pawn_mask = 0x0101010101010101;
		int mask = 1;
		int flagwhite = 0, flagblack = 0;
		for (int i = 0; i < 9; i++)//9的时候计算临界
		{
			if (8 == i)
				temp = 0;
			else
				temp = countChess(White[chess_P].GetData()&Pawn_mask, 0);

			if (temp > 0)
			{
				if (temp > 1)
					value -= (temp - 1) * PAWN_VALUE2;
				number_white |= mask;
				flagwhite++;

			}
			else
			{
				//孤兵
				if (1 == flagwhite)
					value -= PAWN_VALUE1;
				flagwhite = 0;
			}
			if (8 == i)
				temp = 0;
			else
				temp = countChess(Black[chess_P].GetData()&Pawn_mask, 0);

			if (temp > 0)
			{
				if (temp > 1)
					value += (temp - 1) * PAWN_VALUE2;
				number_black |= mask;
				flagblack++;

			}
			else
			{
				//孤兵
				if (1 == flagblack)
					value += PAWN_VALUE1;
				flagblack = 0;
			}

			Pawn_mask = Pawn_mask << 1;
			mask = mask << 1;
		}
		//通路兵这里直接按它附近两排没有棋子计算了…… 不计算前后还是有bug的可能
		//觉得就算两个叠兵是通路兵……按一个的加分也差不多？
		temp = number_black | number_black << 1 | number_black >> 1;
		value += countChess(number_white & ~temp, 0)* PAWN_VALUE3;
		temp = number_white | number_white << 1 | number_white >> 1;
		value -= countChess(number_black & ~temp, 0)* PAWN_VALUE3;
	}
	//N
	//B
	//R
	//这里计算通路的车
	_Bit64_ mask = 0, line;
	line = (number_white | number_black) & 0xFF;
	for (int i = 0; i < 8; i++) 
	{
		mask |= line;
		line = line << 8;
	}
	mask = ~mask;
	value += (countChess(mask&White[chess_R].GetData(), 0) - countChess(mask&Black[chess_R].GetData(), 0)) * ROOK_VALUE;

	//Q
	//K

	return value;
}

//3计算空间的价值4
int ChessBoard::value_space(_Bit64_ attackAreaWhiteAll, _Bit64_ attackAreaBlackAll)
{

	int value = 0;

	//空间的估值
	value += (countChess(attackAreaWhiteAll, 1) - countChess(attackAreaBlackAll, 1)) * SPACE_VALUE1;


	//王周围的25个格子
	_Bit64_ space_K_white = 0, space_K_black = 0;
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

	value += (- countChess(space_K_white & attackAreaBlackAll, 1) + countChess(space_K_black & attackAreaWhiteAll, 1)) * SPACE_VALUE2;
	//这里我觉得还可以加个参数，就这个如果王可移动范围过小的话可以深搜


	return value;
}


//4机动性 15000 以下5(可落子数)
int ChessBoard::value_flexibility(_Bit64_ *attackAreaWhite, _Bit64_ *attackAreaBlack)
{
	int value = 0;
	_Bit64_ temp1 = 0, temp2 = 0;
	for (int chess = 0; chess < 6; chess++)
	{
		temp1 = attackAreaWhite[chess] & ~attackAreaBlack[chess_P] & ~White_All.GetData();
		temp2 = attackAreaBlack[chess] & ~attackAreaWhite[chess_P] & ~Black_All.GetData();
		value += (countChess(temp1, 1) - countChess(temp2, 1)) * flexValue[chess];
	}
	return value;
}

//5发展价值 6
int ChessBoard::value_develop(_Bit64_ *attackAreaWhite, _Bit64_ *attackAreaBlack)
{
	//这里要考虑，先走的是谁。
	//然后我这里写接下来要走的是白
	//if (WHITE_SIDE == side)


	int value = 0, value_temp = 0;
	_Bit64_ board_temp;
    //先是白直接可以吃的子
	board_temp = attackAreaWhite[6] & ~attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			value_temp = chessValue[DeltaChess(1 << pos) & 7];//求棋子的类型
			if (value_temp > value)
				value = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}
	}

	//接下来是白可以交换的子(黑的棋子有保护)
	board_temp = attackAreaWhite[6] & attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			_Bit64_ chess_place = 1 << pos;
			value_temp = chessValue[DeltaChess(chess_place) & 7];//求棋子的类型

			// 这一块在初始化攻击范围的时候可以顺便开个64的数组处理
			int chess_type;  //然后就这里要重新傻乎乎的求一下白是什么棋子
			for (chess_type = 0; chess_type < 6; chess_type++)
			{
				if (chess_place & attackAreaWhite[chess_type])
					break;
			}
			value_temp -= chessValue[chess_type];//求棋子的类型

			if (value_temp > value)
				value = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}

	}

	//接下来是黑色双爪、就统一扣 2000左右 和前面的处理一样
	//然后这边的判断暂时靠flag了- -不知道对王的处理会不会boom
	int flag = 0;
	int value_indanger = 0;
	//先是黑直接可以吃的子
	board_temp = attackAreaWhite[6] & ~attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);			
			value_temp = chessValue[DeltaChess(1 << pos) & 7];//求棋子的类型
			if (value_temp > value_indanger)
				value_indanger = value_temp;
			
			//这里不知要是是王会怎么样
			flag++;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}
	}

	//接下来是白可以交换的子(黑的棋子有保护)
	board_temp = attackAreaWhite[6] & attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			_Bit64_ chess_place = 1 << pos;
			value_temp = chessValue[DeltaChess(chess_place) & 7];//求棋子的类型

															 // 这一块在初始化攻击范围的时候可以顺便开个64的数组处理
			int chess_type;  //然后就这里要重新傻乎乎的求一下白是什么棋子
			for (chess_type = 0; chess_type < 6; chess_type++)
			{
				if (chess_place & attackAreaWhite[chess_type])
					break;
			}
			value_temp -= chessValue[chess_type];//求棋子的类型
			if (value_temp > 500)				//这里象换马不判断为交换
				flag++;
			if (value_temp > value_indanger)
				value_indanger = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}

	}

	if (flag >= 2)
	{
		//双抓
			value -= 2000;
	}
	else 
	{
		//这里是如果双方交换
		if (value > value_indanger)
			value -= value_indanger;
	}
	return value;
}
