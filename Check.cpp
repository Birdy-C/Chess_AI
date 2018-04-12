#include "Board.h"
#include "AboutMove.h"
#include "Movement.h"


//函数简介：检测side方在pos位置的棋子是否正在保护自己的国王不遭受攻击
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

//函数简介：用于当前局面side方的国王受到的将军信息，若检测到将军则将将军方的位置与将军类型（马、兵、长兵器）存入数组，函数返回受到的将军数量（0、1、2）
unsigned short ChessBoard::DeltaCheck(const bool &side, _Pos_ &check_pos, _Check_Type_ &checktype) const
{
	_Pos_ pos = List[chess_K | (side * BLACK_CHESS_BIT)][0];
	BitBoard temp_bit;
	unsigned short check_count = 0;

	//检测斜线，可证明斜线攻击不可能有两个
	temp_bit = (this->*attack_area[chess_B])(pos) & (BB[!side][chess_B] | BB[!side][chess_Q]);
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Diag_Check;
	}
	//检测直线，可证明横线与直线的攻击可能同时出现
	temp_bit = (this->*attack_area[chess_R])(pos) & (BB[!side][chess_R] | BB[!side][chess_Q]);
	while (temp_bit)
	{
		check_count++;
		check_pos = pop_lsb(temp_bit);
		checktype = Straight_Check;
	}
	//检测马
	temp_bit = (this->*attack_area[chess_N])(pos) & BB[!side][chess_N];
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Knight_Check;
	}
	//检测兵
	temp_bit = eat_Pawn[side][pos] & BB[!side][chess_P];
	if (temp_bit)
	{
		check_count++;
		check_pos = Lsb64(temp_bit);
		checktype = Pawn_Check;
	}
	
	return check_count;
}

//函数简介：搜索side方在当前局面下所有摆脱单将的着法，函数要求传入将军方的位置与将军类型，合法着法将被存入new_Move数组，函数返回搜索到的合法着法数；非单将局面下调用将会出错
//函数算法：从吃掉将军者、挡住将军者、移动国王三个方面考虑
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
			if (in_promotion_line(side, check_pos))						//只有check_pos在底线是between_bit才会包含底线的格子
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
		//敌方将军的兵被我方通过过路兵吃掉
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

//函数简介：搜索side方在当前局面下所有摆脱双将（移动国王以摆脱将军）的着法，合法着法将被存入new_Move数组，函数返回搜索到的合法着法数；非将军局面下调用将会出错
//函数算法：枚举王的着法，逐个检测王移动后的位置是否依然被攻击
ExtMove* MoveGenerator::Search_RidDoubleCheck(ExtMove* move_List) const
{
	BitBoard KingMove_bit;
	_Pos_ King_pos = Board.posK(side);

	//暂时去除国王，若不这么做会影响attack_area函数读取bitmap
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