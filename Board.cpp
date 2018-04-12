#include "Board.h"
#include "AboutMove.h"
#include "Search.h"

static const _Pos_ r45L_map[64]  = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 29, 22, 16, 11, 7, 4, 2, 43, 37, 30, 23, 17, 12, 8, 5, 49, 44, 38, 31, 24, 18, 13, 9, 54, 50, 45, 39, 32, 25, 19, 14, 58, 55, 51, 46, 40, 33, 26, 20, 61, 59, 56, 52, 47, 41, 34, 27, 63, 62, 60, 57, 53, 48, 42, 35 };
static const _Pos_ r45R_map[64]  = { 0, 1, 3, 6, 10, 15, 21, 28, 2, 4, 7, 11, 16, 22, 29, 36, 5, 8, 12, 17, 23, 30, 37, 43, 9, 13, 18, 24, 31, 38, 44, 49, 14, 19, 25, 32, 39, 45, 50, 54, 20, 26, 33, 40, 46, 51, 55, 58, 27, 34, 41, 47, 52, 56, 59, 61, 35, 42, 48, 53, 57, 60, 62, 63 };
static const _Pos_ trans_map[64] = { 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63 };

extern FP attack_area[] =
{
	&ChessBoard::_attack_wp,
	&ChessBoard::_attack_n,
	&ChessBoard::_attack_b,
	&ChessBoard::_attack_r,
	&ChessBoard::_attack_q,
	&ChessBoard::_attack_k,
	&ChessBoard::_attack_err,
	&ChessBoard::_attack_err,
	&ChessBoard::_attack_bp,
	&ChessBoard::_attack_n,
	&ChessBoard::_attack_b,
	&ChessBoard::_attack_r,
	&ChessBoard::_attack_q,
	&ChessBoard::_attack_k,
	&ChessBoard::_attack_err,
	&ChessBoard::_attack_err,
};

//函数简介：重置棋盘至初始状态
void ChessBoard::Reset()
{
	BB[WHITE_SIDE][chess_P] = 0x00FF000000000000ULL;
	BB[WHITE_SIDE][chess_N] = 0x4200000000000000ULL;
	BB[WHITE_SIDE][chess_B] = 0x2400000000000000ULL;
	BB[WHITE_SIDE][chess_R] = 0x8100000000000000ULL;
	BB[WHITE_SIDE][chess_Q] = 0x0800000000000000ULL;
	BB[WHITE_SIDE][chess_K] = 0x1000000000000000ULL;
	BB[WHITE_SIDE][chess_All] = 0xFFFF000000000000ULL;

	BB[BLACK_SIDE][chess_P] = 0x000000000000FF00ULL;
	BB[BLACK_SIDE][chess_N] = 0x0000000000000042ULL;
	BB[BLACK_SIDE][chess_B] = 0x0000000000000024ULL;
	BB[BLACK_SIDE][chess_R] = 0x0000000000000081ULL;
	BB[BLACK_SIDE][chess_Q] = 0x0000000000000008ULL;
	BB[BLACK_SIDE][chess_K] = 0x0000000000000010ULL;
	BB[BLACK_SIDE][chess_All] = 0x000000000000FFFFULL;

	All = 0xFFFF00000000FFFFULL;
	All_trans = 0xC3C3C3C3C3C3C3C3ULL;
	All_L45 = 0xFB31861C38618CDFULL;
	All_R45 = 0xFB31861C38618CDFULL;

	memset(ChessArray, CHESS_NONE, _COORDINATE_COUNT_ * _COORDINATE_COUNT_);
	for (_Pos_ p = SQ_A2; p <= SQ_H2; p++)
		ChessArray[p] = WHITE_P;
	ChessArray[SQ_A1] = ChessArray[SQ_H1] = WHITE_R;
	ChessArray[SQ_B1] = ChessArray[SQ_G1] = WHITE_N;
	ChessArray[SQ_C1] = ChessArray[SQ_F1] = WHITE_B;
	ChessArray[SQ_D1] = WHITE_Q; ChessArray[SQ_E1] = WHITE_K;
	for (_Pos_ p = SQ_A7; p <= SQ_H7; p++)
		ChessArray[p] = BLACK_P;
	ChessArray[SQ_A8] = ChessArray[SQ_H8] = BLACK_R;
	ChessArray[SQ_B8] = ChessArray[SQ_G8] = BLACK_N;
	ChessArray[SQ_C8] = ChessArray[SQ_F8] = BLACK_B;
	ChessArray[SQ_D8] = BLACK_Q; ChessArray[SQ_E8] = BLACK_K;

	memset(AttackRange, 0, sizeof(BitBoard) * (SQ_COUNT + 1));
	for (_Pos_ p = SQ_A2; p <= SQ_H2; p++)
		AttackRange[p] = eat_Pawn[WHITE_SIDE][p];
	AttackRange[SQ_A1] = (this->*attack_area[chess_R])(SQ_A1); AttackRange[SQ_H1] = (this->*attack_area[chess_R])(SQ_H1);
	AttackRange[SQ_B1] = (this->*attack_area[chess_N])(SQ_B1); AttackRange[SQ_G1] = (this->*attack_area[chess_N])(SQ_G1);
	AttackRange[SQ_C1] = (this->*attack_area[chess_B])(SQ_C1); AttackRange[SQ_F1] = (this->*attack_area[chess_B])(SQ_F1);
	AttackRange[SQ_D1] = (this->*attack_area[chess_Q])(SQ_D1); AttackRange[SQ_E1] = (this->*attack_area[chess_K])(SQ_E1);
	for (_Pos_ p = SQ_A7; p <= SQ_H7; p++)
		AttackRange[p] = eat_Pawn[BLACK_SIDE][p];
	AttackRange[SQ_A8] = (this->*attack_area[chess_R])(SQ_A8); AttackRange[SQ_H8] = (this->*attack_area[chess_R])(SQ_H8);
	AttackRange[SQ_B8] = (this->*attack_area[chess_N])(SQ_B8); AttackRange[SQ_G8] = (this->*attack_area[chess_N])(SQ_G8);
	AttackRange[SQ_C8] = (this->*attack_area[chess_B])(SQ_C8); AttackRange[SQ_F8] = (this->*attack_area[chess_B])(SQ_F8);
	AttackRange[SQ_D8] = (this->*attack_area[chess_Q])(SQ_D8); AttackRange[SQ_E8] = (this->*attack_area[chess_K])(SQ_E8);

	/*
	Range_by_Piece[WHITE_SIDE][chess_P] = 0x0000FF0000000000ULL;
	Range_by_Piece[WHITE_SIDE][chess_N] = AttackRange[SQ_B1] | AttackRange[SQ_G1];
	Range_by_Piece[WHITE_SIDE][chess_B] = AttackRange[SQ_C1] | AttackRange[SQ_F1];
	Range_by_Piece[WHITE_SIDE][chess_R] = AttackRange[SQ_A1] | AttackRange[SQ_H1];
	Range_by_Piece[WHITE_SIDE][chess_Q] = AttackRange[SQ_D1];
	Range_by_Piece[WHITE_SIDE][chess_K] = AttackRange[SQ_E1];
	Range_by_Piece[WHITE_SIDE][chess_P] = 0x0000000000FF0000ULL;
	Range_by_Piece[WHITE_SIDE][chess_N] = AttackRange[SQ_B8] | AttackRange[SQ_G8];
	Range_by_Piece[WHITE_SIDE][chess_B] = AttackRange[SQ_C8] | AttackRange[SQ_F8];
	Range_by_Piece[WHITE_SIDE][chess_R] = AttackRange[SQ_A8] | AttackRange[SQ_H8];
	Range_by_Piece[WHITE_SIDE][chess_Q] = AttackRange[SQ_D8];
	Range_by_Piece[WHITE_SIDE][chess_K] = AttackRange[SQ_E8];*/

	memset(List, SQ_NONE, sizeof(_Pos_) * PIECE_NUM * MAX_COUNT_PER_PATTERN);
	for (size_t ct = 0; ct < 8; ct++)
		List[WHITE_P][ct] = (_Pos_)(SQ_A2 + ct);
	List[WHITE_N][0] = SQ_B1;	List[WHITE_N][1] = SQ_G1;
	List[WHITE_B][0] = SQ_C1;	List[WHITE_B][1] = SQ_F1;
	List[WHITE_R][0] = SQ_A1;	List[WHITE_R][1] = SQ_H1;
	List[WHITE_Q][0] = SQ_D1;
	List[WHITE_K][0] = SQ_E1;
	for (size_t ct = 0; ct < 8; ct++)
		List[BLACK_P][ct] = (_Pos_)(SQ_A7 + ct);
	List[BLACK_N][0] = SQ_B8;	List[BLACK_N][1] = SQ_G8;
	List[BLACK_B][0] = SQ_C8;	List[BLACK_B][1] = SQ_F8;
	List[BLACK_R][0] = SQ_A8;	List[BLACK_R][1] = SQ_H8;
	List[BLACK_Q][0] = SQ_D8;
	List[BLACK_K][0] = SQ_E8;

	memset(index_InList, NONE, sizeof(int) * SQ_COUNT);
	for (size_t ct = 0; ct < 8; ct++)
		index_InList[(_Pos_)(SQ_A2 + ct)] = ct;
	index_InList[SQ_B1] = 0; index_InList[SQ_G1] = 1;
	index_InList[SQ_C1] = 0; index_InList[SQ_F1] = 1;
	index_InList[SQ_A1] = 0; index_InList[SQ_H1] = 1;
	index_InList[SQ_D1] = 0; 
	index_InList[SQ_E1] = 0;
	for (size_t ct = 0; ct < 8; ct++)
		index_InList[(_Pos_)(SQ_A7 + ct)] = ct;
	index_InList[SQ_B8] = 0; index_InList[SQ_G8] = 1;
	index_InList[SQ_C8] = 0; index_InList[SQ_F8] = 1;
	index_InList[SQ_A8] = 0; index_InList[SQ_H8] = 1;
	index_InList[SQ_D8] = 0;
	index_InList[SQ_E8] = 0;

	memset(counter, 0, 16);
	counter[WHITE_SIDE][chess_P] = counter[BLACK_SIDE][chess_P] = 8;
	counter[WHITE_SIDE][chess_N] = counter[BLACK_SIDE][chess_N] = 2;
	counter[WHITE_SIDE][chess_B] = counter[BLACK_SIDE][chess_B] = 2;
	counter[WHITE_SIDE][chess_R] = counter[BLACK_SIDE][chess_R] = 2;
	counter[WHITE_SIDE][chess_Q] = counter[BLACK_SIDE][chess_Q] = 1;
	counter[WHITE_SIDE][chess_K] = counter[BLACK_SIDE][chess_K] = 1;

	side = WHITE_SIDE;

	flag.En_pass = ENPASS_NONE;
	flag.DrawIn_50step = 0;
	flag.Castling[WHITE_SIDE] = flag.Castling[BLACK_SIDE] = (SHORT_CASTLING | LONG_CASTLING);
	flag.last_capture = CHESS_NONE;
	flag.Pinned[WHITE_SIDE] = flag.Pinned[BLACK_SIDE] = 0;

	Calculate_BoardHash();
}


//函数简介：仅用于AI调用的落子函数，不包含任何UI响应
void ChessBoard::MoveChess(const Movement &mv, const _Depth_ d)
{
	_Pos_ orig = mv.Get_orig_pos(), dest = mv.Get_dest_pos();
	_MoveType_ tp = mv.Get_move_type();

	st_inf[d].bd_flag = flag;

	flag.DrawIn_50step++;
	flag.last_capture = CHESS_NONE;

	//吃子处理，当车被吃时更新王车移位right
	if (CHESS_NONE != chess_at(dest))
	{
		flag.last_capture = chess_at(dest);
		flag.DrawIn_50step = 0;

		if (cast_right(!side) && chess_R == (chess_at(dest) & CHESS_PATTERN))
		{
			if (0 == file_of(dest) && cast_right(!side) & LONG_CASTLING)
			{
				revoke_cast_right<LONG_CASTLING>(!side);
				update_zobrist(castling_zobrist[!side][LONG_CASTLING]);
			}
			else if (7 == file_of(dest) && cast_right(!side) & SHORT_CASTLING)
			{
				revoke_cast_right<SHORT_CASTLING>(!side);
				update_zobrist(castling_zobrist[!side][SHORT_CASTLING]);
			}
		}
		Eaten(dest);
	}

	//基本移动处理
	{
		//更新过路兵flag与相应Zobrist键值
		update_zobrist(En_passent_zobrist[flag.En_pass]);
		flag.En_pass = ENPASS_NONE;
		if (chess_P == (chess_at(orig) & CHESS_PATTERN))
		{
			flag.DrawIn_50step = 0;
			if (((rank_of(orig) ^ rank_of(dest)) & 1) == 0)
				flag.En_pass = file_of(orig);
		}
		update_zobrist(En_passent_zobrist[flag.En_pass]);

		//更新王车易位flag与相应Zobrist键值，王车易位棋步的更新将在特殊棋步处理中完成
		if (cast_right(side))
		{
			if (chess_R == (chess_at(orig) & CHESS_PATTERN))
			{
				if (0 == file_of(orig) && cast_right(side) & LONG_CASTLING)
				{
					revoke_cast_right<LONG_CASTLING>(side);
					update_zobrist(castling_zobrist[side][LONG_CASTLING]);
				}
				else if (7 == file_of(orig) && cast_right(side) & SHORT_CASTLING)
				{
					revoke_cast_right<SHORT_CASTLING>(side);
					update_zobrist(castling_zobrist[side][SHORT_CASTLING]);
				}
			}
			else if (chess_K == (chess_at(orig) & CHESS_PATTERN))
			{
				if (cast_right(side) & LONG_CASTLING)
				{
					revoke_cast_right<LONG_CASTLING>(side);
					update_zobrist(castling_zobrist[side][LONG_CASTLING]);
				}
				if (cast_right(side) & SHORT_CASTLING)
				{
					revoke_cast_right<SHORT_CASTLING>(side);
					update_zobrist(castling_zobrist[side][SHORT_CASTLING]);
				}
			}
		}
		Move(orig, dest);							//必须在最后移动，否则会造成之前flag的判断错误，orig位置为空
	}

	//特殊棋步类型的处理
	if (tp & MOVE_FLAG)
	{
		if (MOVE_CAST == tp)
		{
			//移动车至相应位置
			if (6 == file_of(dest))
			{
				Move(dest + 1, dest - 1);
				AttackRange[dest + 1] = 0;
			}
			else
			{
				Move(dest - 2, dest + 1);
				AttackRange[dest - 2] = 0;
			}

			flag.Castling[side] = 0;
		}
		else if (MOVE_PROMOTION == tp)
		{
			Eaten(dest);
			Place(dest, (_ChessType_)((side * BLACK_CHESS_BIT) | mv.Get_prom_pt()));
		}
		else //if (MOVE_ENPASS == tp)
		{
			Eaten(forward(dest, !side));
			AttackRange[forward(dest, !side)] = 0;
		}
	}
	
	update_pinners(side);
	update_pinners(!side);

	update_attack_range(orig, dest);

	side = !side;
	update_zobrist(side_zobrist);
}

//函数简介：仅用于AI调用的撤销落子函数，不包含任何UI响应
void ChessBoard::undo_MoveChess(const Movement &mv, const _Depth_ d)
{
	_Pos_ orig = mv.Get_orig_pos(), dest = mv.Get_dest_pos();
	_MoveType_ tp = mv.Get_move_type();

	side = !side;

	if (tp & MOVE_FLAG)
	{
		if (MOVE_CAST == tp)
		{
			if (6 == file_of(dest))
			{
				Move(dest - 1, dest + 1);
				AttackRange[dest - 1] = 0;
			}
			else
			{
				Move(dest + 1, dest - 2);
				AttackRange[dest + 1] = 0;
			}
		}
		else if (MOVE_PROMOTION == tp)
		{
			Eaten(dest);
			Place(dest, (_ChessType_)((side * BLACK_CHESS_BIT) | chess_P));
			AttackRange[dest] = 0;
		}
		else //if(MOVE_ENPASS == tp)
		{
			Place(forward(dest, !side), (_ChessType_)(((!side) * BLACK_CHESS_BIT) | chess_P));
			AttackRange[forward(dest, !side)] = (this->*attack_area[(_ChessType_)(((!side) * BLACK_CHESS_BIT) | chess_P)])(forward(dest, !side));
		}
	}

	Move(dest, orig);

	if (CHESS_NONE != move_capture())
		Place(dest, move_capture());

	update_attack_range(dest, orig);
	
	if (CHESS_NONE != move_capture())
		AttackRange[dest] = (this->*attack_area[move_capture()])(dest);

	flag = st_inf[d].bd_flag;
}

//函数简介：用于将棋盘上side方chesstype类型的棋子从orig位置移动到dest位置
void ChessBoard::Move(const _Pos_ orig, const _Pos_ dest)
{
	assert(CHESS_NONE == chess_at(dest));
	
	_ChessType_ tp = ChessArray[orig];

	Update(BB[side_of(tp)][pattern_of(tp)], orig, dest);
	Update(BB[side_of(tp)][chess_All], orig, dest);

	Update(All, orig, dest);
	Update(All_trans, trans_map[orig], trans_map[dest]);
	Update(All_L45, r45L_map[orig], r45L_map[dest]);
	Update(All_R45, r45R_map[orig], r45R_map[dest]);
	
	ChessArray[dest] = tp;
	ChessArray[orig] = CHESS_NONE;

	List[tp][index_InList[orig]] = dest;
	index_InList[dest] = index_InList[orig];
	index_InList[orig] = NONE;

	update_zobrist(zobrist[tp][orig]);
	update_zobrist(zobrist[tp][dest]);
}

//函数简介：用于吃掉棋盘上side方在pos位置上的棋子；鉴于算法采用“异或”而不是“与”，错误的调用将会出错
void ChessBoard::Eaten(const _Pos_ pos)
{
	assert(CHESS_NONE != chess_at(pos));

	_ChessType_ tp = ChessArray[pos];

	Update(BB[side_of(tp)][pattern_of(tp)], pos);
	Update(BB[side_of(tp)][chess_All], pos);

	Update(All, pos);
	Update(All_trans, trans_map[pos]);
	Update(All_L45, r45L_map[pos]);
	Update(All_R45, r45R_map[pos]);

	ChessArray[pos] = CHESS_NONE;

	List[tp][index_InList[pos]] = List[tp][dec_chess_count(tp)];
	index_InList[List[tp][chess_count(tp)]] = index_InList[pos];
	List[tp][chess_count(tp)] = SQ_NONE;
	index_InList[pos] = NONE;

	update_zobrist(zobrist[tp][pos]);
}

//函数简介：用于添加棋盘上side方在pos位置上chesstype类型的棋子，用于兵的升变时新棋子的生成与搜索树unmake_move的时候补偿被吃掉的子
void ChessBoard::Place(const _Pos_ pos, const _ChessType_ tp)
{
	Update(BB[side_of(tp)][pattern_of(tp)], pos);
	Update(BB[side_of(tp)][chess_All], pos);

	Update(All, pos);
	Update(All_trans, trans_map[pos]);
	Update(All_L45, r45L_map[pos]);
	Update(All_R45, r45R_map[pos]);

	ChessArray[pos] = tp;

	List[tp][chess_count(tp)] = pos;
	index_InList[pos] = inc_chess_count(tp);

	update_zobrist(zobrist[tp][pos]);
}

void ChessBoard::Set_atk_area(const _Pos_ &p)
{
	AttackRange[p] = (this->*attack_area[chess_at(p)])(p);
}

//函数简介：返回所有攻击p位置的棋子的bitboard
BitBoard ChessBoard::My_attackers_to(const _Pos_ &p) const
{
	return
		eat_Pawn[!side][p] & get_bb(side, chess_P) |
		_attack_n(p) & get_bb(side, chess_N) |
		_attack_b(p) & (get_bb(side, chess_B) | get_bb(side, chess_Q)) |
		_attack_r(p) & (get_bb(side, chess_R) | get_bb(side, chess_Q)) |
		_attack_k(p) & get_bb(side, chess_K);
}

//函数简介：返回所有攻击p位置的棋子的bitboard
BitBoard ChessBoard::His_attackers_to(const _Pos_ &p) const
{
	return
		eat_Pawn[side][p] & get_bb(!side, chess_P) |
		_attack_n(p)& get_bb(!side, chess_N) |
		_attack_b(p)& (get_bb(!side, chess_B) | get_bb(!side, chess_Q)) |
		_attack_r(p)& (get_bb(!side, chess_R) | get_bb(!side, chess_Q)) |
		_attack_k(p)& get_bb(!side, chess_K);
}

//函数简介：返回所有攻击p位置的马象车后的bitboard（不包含兵和王）
BitBoard ChessBoard::My_heavy_attackers_to(const _Pos_ &p) const
{
	return
		_attack_n(p)& get_bb(side, chess_N) |
		_attack_b(p)& (get_bb(side, chess_B) | get_bb(side, chess_Q)) |
		_attack_r(p)& (get_bb(side, chess_R) | get_bb(side, chess_Q));
}

//函数简介：返回所有攻击p位置的马象车后的bitboard（不包含兵和王）
BitBoard ChessBoard::His_heavy_attackers_to(const _Pos_ &p) const
{
	return
		_attack_n(p)& get_bb(!side, chess_N) |
		_attack_b(p)& (get_bb(!side, chess_B) | get_bb(!side, chess_Q)) |
		_attack_r(p)& (get_bb(!side, chess_R) | get_bb(!side, chess_Q));
}

//函数简介：更新pinners牵制者bitboard
void ChessBoard::update_pinners(const bool sd)
{
	BitBoard total_bit, enemy_RQ = get_bb(!sd, chess_R) | get_bb(!sd, chess_Q), enemy_BQ = get_bb(!sd, chess_B) | get_bb(!sd, chess_Q);
	_Pos_ King_pos = posK(sd);

	flag.Pinned[sd] = 0;

	if (0 != (0xFFULL << (rank_of(King_pos) << 3) & enemy_RQ))
	{
		total_bit = rank_attacks[King_pos][(uint8_t)(All >> (King_pos & 0xF8))] & get_bb(sd, chess_All);
		while (total_bit)
		{
			_Pos_ pinned_pos = pop_lsb(total_bit);
			if (rank_attacks[pinned_pos][(uint8_t)(All >> (pinned_pos & 0xF8))] & enemy_RQ)
				flag.Pinned[sd] |= mask(pinned_pos);
		}
	}

	if (0 != (0x0101010101010101ULL << file_of(King_pos) & enemy_RQ))
	{
		total_bit = file_attacks[King_pos][(uint8_t)(All_trans >> (file_of(King_pos) << 3))] & get_bb(sd, chess_All);
		while (total_bit)
		{
			_Pos_ pinned_pos = pop_lsb(total_bit);
			if (file_attacks[pinned_pos][(uint8_t)(All_trans >> (file_of(pinned_pos) << 3))] & enemy_RQ)
				flag.Pinned[sd] |= mask(pinned_pos);
		}
	}

	if (0 != (diag_A8H1_attacks[King_pos][0] & enemy_BQ))
	{
		total_bit = diag_A8H1_attacks[King_pos][(uint8_t)(All_L45 >> start_pos_A8H1[King_pos])] & get_bb(sd, chess_All);
		while (total_bit)
		{
			_Pos_ pinned_pos = pop_lsb(total_bit);
			if (diag_A8H1_attacks[pinned_pos][(uint8_t)(All_L45 >> start_pos_A8H1[pinned_pos])] & enemy_BQ)
				flag.Pinned[sd] |= mask(pinned_pos);
		}
	}

	if (0 != (diag_H8A1_attacks[King_pos][0] & enemy_BQ))
	{
		total_bit = diag_H8A1_attacks[King_pos][(uint8_t)(All_R45 >> start_pos_H8A1[King_pos])] & get_bb(sd, chess_All);
		while (total_bit)
		{
			_Pos_ pinned_pos = pop_lsb(total_bit);
			if (diag_H8A1_attacks[pinned_pos][(uint8_t)(All_R45 >> start_pos_H8A1[pinned_pos])] & enemy_BQ)
				flag.Pinned[sd] |= mask(pinned_pos);
		}
	}
}

void ChessBoard::update_attack_range(const _Pos_ orig, const _Pos_ dest)
{
	static const uint32_t magic = 0x92FFEFF;

	AttackRange[orig] = 0;
	AttackRange[dest] = (this->*attack_area[chess_at(dest)])(dest);

	if (count_IsNormal())
	{
		uint32_t ct;

		ct = (*(uint32_t*)(counter[WHITE_SIDE] + 2));

		if (ct & 0x00010000)
			AttackRange[List[chess_Q][0]] = _attack_q(List[chess_Q][0]);

		switch ((ct * magic) >> 28)
		{
		case 0:
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]); break;
		case 1:
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]);
			AttackRange[List[chess_B][1]] = _attack_b(List[chess_B][1]); break;
		case 2:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]); break;
		case 3:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]);
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]); break;
		case 4:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]);
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]);
			AttackRange[List[chess_B][1]] = _attack_b(List[chess_B][1]); break;
		case 5:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]);
			AttackRange[List[chess_R][1]] = _attack_r(List[chess_R][1]); break;
		case 6:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]);
			AttackRange[List[chess_R][1]] = _attack_r(List[chess_R][1]);
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]); break;
		case 7:
			AttackRange[List[chess_R][0]] = _attack_r(List[chess_R][0]);
			AttackRange[List[chess_R][1]] = _attack_r(List[chess_R][1]);
			AttackRange[List[chess_B][0]] = _attack_b(List[chess_B][0]);
			AttackRange[List[chess_B][1]] = _attack_b(List[chess_B][1]); break;
		default: break;				//case 15
		}

		ct = (*(uint32_t*)(counter[BLACK_SIDE] + 2));

		if (ct & 0x00010000)
			AttackRange[List[chess_Q | BLACK_CHESS_BIT][0]] = _attack_q(List[chess_Q | BLACK_CHESS_BIT][0]);

		switch ((ct * magic) >> 28)
		{
		case 0:
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]); break;
		case 1:
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][1]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][1]); break;
		case 2:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]); break;
		case 3:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]); break;
		case 4:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][1]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][1]); break;
		case 5:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_R | BLACK_CHESS_BIT][1]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][1]); break;
		case 6:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_R | BLACK_CHESS_BIT][1]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][1]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]); break;
		case 7:
			AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_R | BLACK_CHESS_BIT][1]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][1]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][0]);
			AttackRange[List[chess_B | BLACK_CHESS_BIT][1]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][1]); break;
		default: break;				//case 15
		}
	}
	else
	{
		for (size_t pt_id = 0; pt_id < chess_count(WHITE_SIDE, chess_B); pt_id++)
			AttackRange[List[chess_B][pt_id]] = _attack_b(List[chess_B][pt_id]);
		for (size_t pt_id = 0; pt_id < chess_count(WHITE_SIDE, chess_R); pt_id++)
			AttackRange[List[chess_R][pt_id]] = _attack_r(List[chess_R][pt_id]);
		for (size_t pt_id = 0; pt_id < chess_count(WHITE_SIDE, chess_Q); pt_id++)
			AttackRange[List[chess_Q][pt_id]] = _attack_q(List[chess_Q][pt_id]);

		for (size_t pt_id = 0; pt_id < chess_count(BLACK_SIDE, chess_B); pt_id++)
			AttackRange[List[chess_B | BLACK_CHESS_BIT][pt_id]] = _attack_b(List[chess_B | BLACK_CHESS_BIT][pt_id]);
		for (size_t pt_id = 0; pt_id < chess_count(BLACK_SIDE, chess_R); pt_id++)
			AttackRange[List[chess_R | BLACK_CHESS_BIT][pt_id]] = _attack_r(List[chess_R | BLACK_CHESS_BIT][pt_id]);
		for (size_t pt_id = 0; pt_id < chess_count(BLACK_SIDE, chess_Q); pt_id++)
			AttackRange[List[chess_Q | BLACK_CHESS_BIT][pt_id]] = _attack_q(List[chess_Q | BLACK_CHESS_BIT][pt_id]);
	}
}