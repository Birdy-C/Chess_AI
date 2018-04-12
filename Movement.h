#pragma once

#include "BitBoard.h"
#include "Macro.h"

#define MOVE_NONE	0xFFFF

//最高2位存棋步类型，次高2位存升变的棋子类型，4位组成棋步的flag
enum 
{ 
	MOVE_NORMAL = 0, MOVE_CAST = 0xC000, MOVE_ENPASS = 0x8000, MOVE_PROMOTION = 0x4000,
	MOVE_FLAG = 0xC000, PROMOTION_PT = 0x3000
};

enum GenType
{
	GEN_PROMOTION, GEN_CAPTURE, GEN_QUIET, GEN_RID_CHECK
};

enum
{
	MAIN_MOVE = 0, PROMOTION_INIT = 1, PROMOTION = 2, CAPTURE_INIT = 3, GOOD_CAPTURE = 4, QUIET_INIT = 5, QUIETS = 6, BAD_CAPTURE = 7,
	RID_CHECK_INIT = 8, RID_CHECK = 9, RID_DOU_CHECK_INIT = 10, RID_DOU_CHECK = 11
};

class ChessBoard;

class Movement
{
private:
	uint16_t data;			//0~5bit代表orig，6~11bit代表dest，12~13bit代表升变的pattern-2，14~15bit代表特殊类型

public:
	Movement(){ Clear(); }
	Movement(const _ChessType_){ data = MOVE_NONE; }
	Movement(const _Pos_ &orig, const _Pos_ &dest, const _MoveType_ &movtp){ data = (orig | (dest << 6) | movtp); }
	void Clear(){ data = MOVE_NONE; }
	bool Exist()const{ return MOVE_NONE != data; }
	Movement& operator=(const Movement& father){ data = father.data; return *this; }
	_Pos_ Get_orig_pos()const{ return (_Pos_)(data & 63); }
	_Pos_ Get_dest_pos()const{ return (_Pos_)((data >> 6) & 63); }
	_MoveType_ Get_move_type()const{ return (_MoveType_)(data & MOVE_FLAG); }
	_ChessPattern_ Get_prom_pt()const{ return ((data & PROMOTION_PT) >> 12) + chess_N; }
	void SetValue(const _Pos_ &orig, const _Pos_ &dest, const _MoveType_ &movtp){ data = (orig | (dest << 6) | movtp); }
	void SetPromotionPattern(const _ChessPattern_ &pt){ data |= ((pt - 1) << 12); }
	void Print(const ChessBoard &bd) const;
	void FPrint(std::ostream &os, const ChessBoard &bd) const;
	bool operator==(const Movement &mov)const { return data == mov.data; }
	bool operator!=(const Movement &mov)const { return data != mov.data; }
};


typedef struct ExtMovement
{
	Movement move;
	_Score_  scr;

	bool operator>(const struct ExtMovement &p)const { return scr > p.scr; }
	bool operator<(const struct ExtMovement &p)const { return scr < p.scr; }
	struct ExtMovement& operator=(const Movement &mov){ move = mov; return *this; }

} ExtMove;

class MoveGenerator
{
private:
	const ChessBoard &Board;

	int	state;
	bool side;
	_Depth_ depth;

	_Pos_ check_pos;
	_Check_Type_ check_tp;

	Movement tt_Move;
	ExtMove  *cur, *bad_cap, *end;

	ExtMove* Search_RidCheck(ExtMove* move_List) const;
	ExtMove* Search_RidDoubleCheck(ExtMove* move_List) const;

public:
	MoveGenerator(const ChessBoard &Bd, const Movement tt_move, const bool sd, const _Depth_ dpt = MAX_DEPTH);

	Movement next_move();
	int get_state() const{ return state; }
	//template<GenType Type> void score();
	void score_cap();
};

extern BitBoard Between_bit[64][64];
extern BitBoard Line_by_2pos[64][64];


inline Movement make_move(const _Pos_ &orig_p, const _Pos_ &dest_p, const _MoveType_ &mt = MOVE_NORMAL)
{
	Movement mv(orig_p, dest_p, mt);
	return mv;
}

ExtMove* make_promotion(ExtMove *cur, const _Pos_ &p, int delta_pos);

//函数简介：生成当前盘面的所有兵的伪合法落子
template<GenType Type>
ExtMove* expand_pawn_move(ExtMove *cur, const ChessBoard &Board, bool side)
{
	const BitBoard rid_fileA = (BitBoard)0xFEFEFEFEFEFEFEFE, rid_fileH = (BitBoard)0x7F7F7F7F7F7F7F7F;
	BitBoard pawn_on_lifeline = Board.get_bb(side, chess_P) & life_line(side);

	//Promotion
	if (GEN_PROMOTION == Type && pawn_on_lifeline)
	{
		BitBoard lf = left_forward(pawn_on_lifeline & rid_fileA, side) & Board.get_bb(!side, chess_All);
		BitBoard rf = right_forward(pawn_on_lifeline & rid_fileH, side) & Board.get_bb(!side, chess_All);
		BitBoard f = forward(pawn_on_lifeline, side) & ~Board.get_bb_all();

		while (lf)
			cur = make_promotion(cur, pop_lsb(lf), -9 + (side << 4));
		while (rf)
			cur = make_promotion(cur, pop_lsb(rf), -7 + (side << 4));
		while (f)
			cur = make_promotion(cur, pop_lsb(f), -8 + (side << 4));
	}

	//Capture
	else if (GEN_CAPTURE == Type)
	{
		BitBoard lf = left_forward(Board.get_bb(side, chess_P) & rid_fileA & ~pawn_on_lifeline, side) & Board.get_bb(!side, chess_All);
		BitBoard rf = right_forward(Board.get_bb(side, chess_P) & rid_fileH & ~pawn_on_lifeline, side) & Board.get_bb(!side, chess_All);

		while (lf)
		{
			_Pos_ dest = pop_lsb(lf);
			*cur++ = make_move(dest - (-9 + (side << 4)), dest, chess_P);
		}
		while (rf)
		{
			_Pos_ dest = pop_lsb(rf);
			*cur++ = make_move(dest - (-7 + (side << 4)), dest, chess_P);
		}
		if (ENPASS_NONE != Board.En_passant())			//En_pass
		{
			BitBoard b = Board.get_bb(side, chess_P) & (side ? (eat_Pawn[WHITE_SIDE][Board.En_passant() | 32] << 8) : (eat_Pawn[BLACK_SIDE][Board.En_passant() | 24] >> 8));
			while (b)
				*cur++ = make_move(pop_lsb(b), (Board.En_passant() | 16) + 24 * side, MOVE_ENPASS);
		}
	}

	//Quiet
	else if (GEN_QUIET == Type)
	{
		BitBoard move1 = forward(Board.get_bb(side, chess_P) & ~pawn_on_lifeline, side) & ~Board.get_bb_all();
		BitBoard move2 = forward(move1, side) & ~Board.get_bb_all() & push_line(!side);

		while (move1)
		{
			_Pos_ dest = pop_lsb(move1);
			*cur++ = make_move(dest - (-8 + (side << 4)), dest, MOVE_NORMAL);
		}
		while (move2)
		{
			_Pos_ dest = pop_lsb(move2);
			*cur++ = make_move(dest - (-16 + (side << 5)), dest, MOVE_NORMAL);
		}
	}

	return cur;
}

//函数简介：生成当前盘面王的所有伪合法落子
template<GenType Type>
ExtMove* expand_king_move(ExtMove *cur, const ChessBoard &Board, const BitBoard &target, bool side)
{
	_Pos_ King_pos = Board.posK(side);
	BitBoard total_bit = Board.Piece_atk_area(King_pos) & target;

	while (total_bit)
	{
		_Pos_ dest = pop_lsb(total_bit);
		*cur++ = make_move(King_pos, dest);
	}

	if (GEN_QUIET == Type)
	{
		if (Board.cast_right(side) && !Board.His_attackers_to(King_pos))
		{
			if (Board.cast_right(side) & SHORT_CASTLING && CHESS_NONE == Board.chess_at(King_pos + 1) && CHESS_NONE == Board.chess_at(King_pos + 2)
				&& !Board.His_attackers_to(King_pos + 1) && !Board.His_attackers_to(King_pos + 2))
				*cur++ = make_move(King_pos, King_pos + 2, MOVE_CAST);
			if (Board.cast_right(side) & LONG_CASTLING && CHESS_NONE == Board.chess_at(King_pos - 1) && CHESS_NONE == Board.chess_at(King_pos - 2) && CHESS_NONE == Board.chess_at(King_pos - 3)
				&& !Board.His_attackers_to(King_pos - 1) && !Board.His_attackers_to(King_pos - 2))
				*cur++ = make_move(King_pos, King_pos - 2, MOVE_CAST);
		}
	}
	return cur;
}

//函数简介：生成当前盘面pt棋子的所有伪合法落子（马、象、车、后）
template<_ChessPattern_ pt>
ExtMove* expand_move(ExtMove *cur, const ChessBoard &Board, const BitBoard &target, const bool side)
{
	for (size_t id = 0; id < Board.chess_count((side * BLACK_CHESS_BIT) | pt); id++)
	{
		_Pos_ orig = Board.List[pt + side * BLACK_CHESS_BIT][id];
		BitBoard movbb = Board.Piece_atk_area(orig) & target;
		while (movbb)
		{
			_Pos_ dest = pop_lsb(movbb);
			*cur++ = make_move(orig, dest);
		}
	}
	return cur;
}

template<GenType Type>
ExtMove* expand_all(ExtMove *move_List, const ChessBoard &Board, const BitBoard &target)
{
	move_List = expand_pawn_move<Type>(move_List, Board, Board.side_toMove());
	move_List = expand_move<chess_N>(move_List, Board, target, Board.side_toMove());
	move_List = expand_move<chess_B>(move_List, Board, target, Board.side_toMove());
	move_List = expand_move<chess_R>(move_List, Board, target, Board.side_toMove());
	move_List = expand_move<chess_Q>(move_List, Board, target, Board.side_toMove());
	move_List = expand_king_move<Type>(move_List, Board, target, Board.side_toMove());

	return move_List;
}

//函数简介：生成当前局面下side方所有合法落子并存入new_Move数组，返回合法落子数
template<GenType Type>
ExtMove* expand(ExtMove *move_List, const ChessBoard &Board)
{
	assert(Type == GEN_PROMOTION || Type == GEN_CAPTURE || Type == GEN_QUIET);
	BitBoard target = Type == GEN_CAPTURE ? Board.get_bb(!Board.side_toMove(), chess_All) :
 		Type == GEN_QUIET ? ~Board.get_bb_all() : 0;

	if (GEN_PROMOTION == Type)
		return expand_pawn_move<GEN_PROMOTION>(move_List, Board, Board.side_toMove());
	else
		return expand_all<Type>(move_List, Board, target);
}