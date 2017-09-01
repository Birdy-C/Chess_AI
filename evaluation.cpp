//Birdy

//�����÷� ��900����500����325����300����100
#define score_P_basic 1000
#define score_N_basic 3000
#define score_B_basic 3250
#define score_R_basic 5000
#define score_Q_basic 9000
#define score_K_basic 100000
#define SPACE_VALUE1 10
#define SPACE_VALUE2 10
#define PAWN_VALUE1 12 //�±�
#define PAWN_VALUE2 13 //����
#define PAWN_VALUE3 27 //ͨ·��
#define KNIGHT_VALUE 7
#define ROOK_VALUE 7 //ռ��ͨ·�ĳ�
#include"Chess.h"
#include"AboutMove.h"

extern _Bit64_ mask[64];


int line2count[256] = {0};
int chessValue[6] = { score_P_basic, score_N_basic, score_B_basic, score_R_basic, score_Q_basic, score_K_basic };
int flexValue[6] = { 0,5,5,5,5,5 };	//�����ԵĶ��壨����������ԵĻ���
int Value2[6] = { 0,7,6,5,5,5 };	//�������ӿ������� 
int pawnValue[8] = { 0,0,0,0,100,1500,2000,0 };	//���ļ�ֵ
//====================================���ܺ���====================================//

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

//������λ�ķ���λ����һλ��û�з��ظ���
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

//�����м������ӡ��Ա�/��Ŀ��ĸ�����ķ���flag = 1
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


 //��ʼ��������Χ	
 //������Χ��ʼ����ʱ��֤ÿ������ֻ��һ�����Ӹ���

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


//====================================�����ֵ====================================//

//�ܵ�����
int ChessBoard::evaluation()
{
	//�׷�Ϊ�ӽ� �����￪�߸����һ����¼�ܵ�
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


//0������ֵ1
int ChessBoard::value_chess1()
{
	/*
	����Ҿ��ÿ��Ժ�������һ����������
	Ȼ����ڲ���������ֻ������һ���ı任�����Ļ������Ҿ����ǿ��Եġ���
	����������Ӱ��Ļ�,Ҳ���������Ӻ;��Ƹı�Ļ�������������һ������֮�����¼���
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


//1�����ֵ2
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
	//��һ���ұ������ã����ӵ�λ������ʾ�����м�Ļ���ֵ�ߣ��ڿ������߼�ֵ������
	//Ȼ��͵��д���������ӵġ���Ӧ�õø�
	for (int chess = 1; chess < 5; chess++)
	{
		temp1 = attackAreaWhite[chess];
		temp2 = attackAreaBlack[chess];
		value += (countChess(temp1, 1) - countChess(temp2, 1)) * flexValue[chess];
	}


	return value;
}

//2�ۺϼ�ֵ3
int ChessBoard::value_chess3()
{
	int value = 0;

	//P
	//����ͳ�ƣ��±���������ͨ·��
	int number_white = 0, number_black = 0;
	{
		int temp;
		_Bit64_ Pawn_mask = 0x0101010101010101;
		int mask = 1;
		int flagwhite = 0, flagblack = 0;
		for (int i = 0; i < 9; i++)//9��ʱ������ٽ�
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
				//�±�
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
				//�±�
				if (1 == flagblack)
					value += PAWN_VALUE1;
				flagblack = 0;
			}

			Pawn_mask = Pawn_mask << 1;
			mask = mask << 1;
		}
		//ͨ·������ֱ�Ӱ�����������û�����Ӽ����ˡ��� ������ǰ������bug�Ŀ���
		//���þ�������������ͨ·��������һ���ļӷ�Ҳ��ࣿ
		temp = number_black | number_black << 1 | number_black >> 1;
		value += countChess(number_white & ~temp, 0)* PAWN_VALUE3;
		temp = number_white | number_white << 1 | number_white >> 1;
		value -= countChess(number_black & ~temp, 0)* PAWN_VALUE3;
	}
	//N
	//B
	//R
	//�������ͨ·�ĳ�
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

//3����ռ�ļ�ֵ4
int ChessBoard::value_space(_Bit64_ attackAreaWhiteAll, _Bit64_ attackAreaBlackAll)
{

	int value = 0;

	//�ռ�Ĺ�ֵ
	value += (countChess(attackAreaWhiteAll, 1) - countChess(attackAreaBlackAll, 1)) * SPACE_VALUE1;


	//����Χ��25������
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
	//�����Ҿ��û����ԼӸ��������������������ƶ���Χ��С�Ļ���������


	return value;
}


//4������ 15000 ����5(��������)
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

//5��չ��ֵ 6
int ChessBoard::value_develop(_Bit64_ *attackAreaWhite, _Bit64_ *attackAreaBlack)
{
	//����Ҫ���ǣ����ߵ���˭��
	//Ȼ��������д������Ҫ�ߵ��ǰ�
	//if (WHITE_SIDE == side)


	int value = 0, value_temp = 0;
	_Bit64_ board_temp;
    //���ǰ�ֱ�ӿ��ԳԵ���
	board_temp = attackAreaWhite[6] & ~attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			value_temp = chessValue[DeltaChess(1 << pos) & 7];//�����ӵ�����
			if (value_temp > value)
				value = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}
	}

	//�������ǰ׿��Խ�������(�ڵ������б���)
	board_temp = attackAreaWhite[6] & attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			_Bit64_ chess_place = 1 << pos;
			value_temp = chessValue[DeltaChess(chess_place) & 7];//�����ӵ�����

			// ��һ���ڳ�ʼ��������Χ��ʱ�����˳�㿪��64�����鴦��
			int chess_type;  //Ȼ�������Ҫ����ɵ��������һ�°���ʲô����
			for (chess_type = 0; chess_type < 6; chess_type++)
			{
				if (chess_place & attackAreaWhite[chess_type])
					break;
			}
			value_temp -= chessValue[chess_type];//�����ӵ�����

			if (value_temp > value)
				value = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}

	}

	//�������Ǻ�ɫ˫צ����ͳһ�� 2000���� ��ǰ��Ĵ���һ��
	//Ȼ����ߵ��ж���ʱ��flag��- -��֪�������Ĵ���᲻��boom
	int flag = 0;
	int value_indanger = 0;
	//���Ǻ�ֱ�ӿ��ԳԵ���
	board_temp = attackAreaWhite[6] & ~attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);			
			value_temp = chessValue[DeltaChess(1 << pos) & 7];//�����ӵ�����
			if (value_temp > value_indanger)
				value_indanger = value_temp;
			
			//���ﲻ֪Ҫ����������ô��
			flag++;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}
	}

	//�������ǰ׿��Խ�������(�ڵ������б���)
	board_temp = attackAreaWhite[6] & attackAreaBlack[6] & Black_All.GetData();
	if (board_temp)
	{
		while (board_temp)
		{
			int pos = Lsb64(board_temp);
			_Bit64_ chess_place = 1 << pos;
			value_temp = chessValue[DeltaChess(chess_place) & 7];//�����ӵ�����

															 // ��һ���ڳ�ʼ��������Χ��ʱ�����˳�㿪��64�����鴦��
			int chess_type;  //Ȼ�������Ҫ����ɵ��������һ�°���ʲô����
			for (chess_type = 0; chess_type < 6; chess_type++)
			{
				if (chess_place & attackAreaWhite[chess_type])
					break;
			}
			value_temp -= chessValue[chess_type];//�����ӵ�����
			if (value_temp > 500)				//���������ж�Ϊ����
				flag++;
			if (value_temp > value_indanger)
				value_indanger = value_temp;
			board_temp &= 0xFFFFFFFFFFFFFFFE << pos;
		}

	}

	if (flag >= 2)
	{
		//˫ץ
			value -= 2000;
	}
	else 
	{
		//���������˫������
		if (value > value_indanger)
			value -= value_indanger;
	}
	return value;
}
