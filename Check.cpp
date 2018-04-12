#include "Board.h"
#include "AboutMove.h"
#include "Movement.h"


//������飺���side����posλ�õ������Ƿ����ڱ����Լ��Ĺ��������ܹ���
/*
BitBoard ChessBoard::Delta_ResistCheck(const bool &side, const _Pos_ &pos) const
{
	int line_tp = Line_by_2pos[pos][List[chess_K | (side * BLACK_CHESS_BIT)][0]];

	if (NONE_LINE != line_tp)
	{
		BitBoard line =
			RANK_LINE	   == line_tp ? rank_attacks[pos][(All.GetData() >> getline_shift[pos]) & 255] :
			FILE_LINE	   == line_tp ? file_attacks[pos][(All_trans.GetData() >> getline_shift_trans[pos]) & 255] :
			DIAG_A8H1_LINE == line_tp ? diag_A8H1_attacks[pos][(All_L45.GetData() >> start_pos_A8H1[pos]) & (shade_A8H1[pos])] :
										diag_H8A1_attacks[pos][(All_R45.GetData() >> start_pos_H8A1[pos]) & (shade_H8A1[pos])] ;
		
		if (line & BB[side][chess_K].GetData())
		{
			if (line_tp & STRAIGHT_LINE)
			{
				if (line & (BB[!side][chess_R].GetData() | BB[!side][chess_Q].GetData()))
					return line;
			}
			else if (line & (BB[!side][chess_B].GetData() | BB[!side][chess_Q].GetData()))		
				return line;
		}
	}
	return (BitBoard)0xFFFFFFFFFFFFFFFF;
}*/

//������飺���ڵ�ǰ����side���Ĺ����ܵ��Ľ�����Ϣ������⵽�����򽫽�������λ���뽫�����ͣ����������������������飬���������ܵ��Ľ���������0��1��2��
unsigned short ChessBoard::DeltaCheck(const bool &side, _Pos_ &check_pos, _Check_Type_ &checktype) const
{
	_Pos_ pos = List[chess_K | (side * BLACK_CHESS_BIT)][0];
	BitBoard temp_bit;
	unsigned short check_count = 0;

	//���б�ߣ���֤��б�߹���������������
	temp_bit = (this->*attack_area[chess_B])(pos) & (BB[!side][chess_B] | BB[!side][chess_Q]);
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Diag_Check;
	}
	//���ֱ�ߣ���֤��������ֱ�ߵĹ�������ͬʱ����
	temp_bit = (this->*attack_area[chess_R])(pos) & (BB[!side][chess_R] | BB[!side][chess_Q]);
	while (temp_bit)
	{
		check_count++;
		check_pos = pop_lsb(temp_bit);
		checktype = Straight_Check;
	}
	//�����
	temp_bit = (this->*attack_area[chess_N])(pos) & BB[!side][chess_N];
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Knight_Check;
	}
	//����
	temp_bit = eat_Pawn[side][pos] & BB[!side][chess_P];
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Pawn_Check;
	}
	
	return check_count;
}

//������飺����side���ڵ�ǰ���������а��ѵ������ŷ�������Ҫ���뽫������λ���뽫�����ͣ��Ϸ��ŷ���������new_Move���飬���������������ĺϷ��ŷ������ǵ��������µ��ý������
//�����㷨���ӳԵ������ߡ���ס�����ߡ��ƶ������������濼��
ExtMove* MoveGenerator::Search_RidCheck(ExtMove* move_List) const
{
	BitBoard dest_bit, total_bit, between_bit;

 	between_bit = (Line_Check & check_tp) ? Between_bit[check_pos][Board.List[(side * BLACK_CHESS_BIT) | chess_K][0]] & 0x7EFFFFFFFFFFFF7EULL : 0;

	//Pawn eat moves need extra discussion
	total_bit = eat_Pawn[!side][check_pos] & Board.get_bb(side, chess_P);
	while (total_bit)
	{
		_Pos_ orig = pop_lsb(total_bit);

		if (!(Board.pinners(side) & mask(orig)) || At_same_line(orig, check_pos, Board.List[(side * BLACK_CHESS_BIT) | chess_K][0]))
		{
			if (in_promotion_line(side, check_pos))
				move_List = make_promotion(move_List, check_pos, check_pos - orig);
			else
				*move_List++ = make_move(orig, check_pos);
		}
	}
	
	//Pawn obstruct moves
	total_bit = between_bit & forward(Board.get_bb(side, chess_P), side);
	while (total_bit)
	{
		_Pos_ dest = pop_lsb(total_bit), orig = dest + (side ? -8 : 8);

		if (!(Board.pinners(side) & mask(orig)) || At_same_line(orig, dest, Board.List[(side * BLACK_CHESS_BIT) | chess_K][0]))
		{
			if (in_promotion_line(side, check_pos))						//ֻ��check_pos�ڵ�����between_bit�Ż�������ߵĸ���
				move_List = make_promotion(move_List, dest, dest - orig);
			else
				*move_List++ = make_move(orig, dest);
		}
	}
	total_bit = between_bit & forward2(Board.get_bb(side, chess_P) & life_line(!side) & ~forward(Board.get_bb_all(), !side), side);
	while (total_bit)
	{
		_Pos_ dest = pop_lsb(total_bit), orig = dest + (side ? -16 : 16);

		if (!(Board.pinners(side) & mask(orig)) || At_same_line(orig, dest, Board.List[(side * BLACK_CHESS_BIT) | chess_K][0]))
			*move_List++ = make_move(orig, dest);
	}


	//Other moves

	dest_bit = mask(check_pos) | between_bit;

	while (dest_bit)
	{
		_Pos_ dest = pop_lsb(dest_bit);
		total_bit = Board.My_heavy_attackers_to(dest);
		while (total_bit)
		{
			_Pos_ orig = pop_lsb(total_bit);
			if (!(Board.pinners(side) & mask(orig)) || At_same_line(orig, dest, Board.List[(side * BLACK_CHESS_BIT) | chess_K][0]))
				*move_List++ = make_move(orig, dest);
		}
	}

	//En_pass estimate for Pawn_Check
	if (Pawn_Check == check_tp)
	{
		//�з������ı����ҷ�ͨ����·���Ե�
		if (Board.En_passant() == file_of(check_pos))
		{
			BitBoard b = Board.get_bb(side, chess_P) & (side ? (eat_Pawn[WHITE_SIDE][Board.En_passant() | 32] << 8) : (eat_Pawn[BLACK_SIDE][Board.En_passant() | 24] >> 8));
			while (b)
			{
				Movement mv = make_move(pop_lsb(b), (Board.En_passant() | 16) + 24 * side, MOVE_ENPASS);
				if (Board.Legal(mv))
					*move_List++ = mv;
			}
				
		}
	}

	return move_List;
}

//������飺����side���ڵ�ǰ���������а���˫�����ƶ������԰��ѽ��������ŷ����Ϸ��ŷ���������new_Move���飬���������������ĺϷ��ŷ������ǽ��������µ��ý������
//�����㷨��ö�������ŷ������������ƶ����λ���Ƿ���Ȼ������
ExtMove* MoveGenerator::Search_RidDoubleCheck(ExtMove* move_List) const
{
	BitBoard KingMove_bit;
	_Pos_ King_pos = Board.posK(side);

	//��ʱȥ��������������ô����Ӱ��attack_area������ȡbitmap
	ChessBoard temp = Board;
	temp.Eaten(King_pos);

	KingMove_bit = (Board.*attack_area[chess_K])(King_pos) & ~Board.get_bb(side, chess_All);
	while (KingMove_bit)
	{
		_Pos_ dest = pop_lsb(KingMove_bit);
		if (!temp.His_attackers_to(dest))
			*move_List++ = make_move(King_pos, dest);
	}

	//Board.Place(King_pos, (side * 8) | chess_K);

	return move_List;
}