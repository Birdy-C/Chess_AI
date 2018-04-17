#pragma once

#define MAX_COUNT_PER_PATTERN	8

#include "BitBoard.h"
#include "AboutMove.h"
#include "Movement.h"
#include "Hash.h"

enum { Pawn_Check = 1, Knight_Check = 2, Straight_Check = 4, Diag_Check = 8, Line_Check = 12 };

enum
{
	NONE_LINE = 0, 
	RANK_LINE = 1, FILE_LINE = 2, STRAIGHT_LINE = 3,
	DIAG_A8H1_LINE = 4, DIAG_H8A1_LINE = 8, DIAG_LINE = 12
};

enum Phase 
{
	PHASE_ENDGAME,
	PHASE_MIDGAME = 128,
	MG = 0, EG = 1, PHASE_NB = 2
};

extern _Zobrist64_ zobrist[PIECE_NUM][SQ_COUNT];
extern _Zobrist64_ side_zobrist, En_passent_zobrist[_COORDINATE_COUNT_ + 1], castling_zobrist[_SIDE_COUNT_][3];

class _Board_Flag_
{
public:
	_Coordinate_	En_pass;
	uint8_t			DrawIn_50step;
	uint8_t			Castling[_SIDE_COUNT_];
	_ChessType_		last_capture;
	Zobrist			zobrist_value;
	BitBoard		Pinned[_SIDE_COUNT_];			//被对方牵制的棋子，挪开后己方的王将会受到直接将军

	_Board_Flag_(){ En_pass = ENPASS_NONE; DrawIn_50step = 0; Castling[WHITE_SIDE] = Castling[BLACK_SIDE] = LONG_CASTLING | SHORT_CASTLING; zobrist_value.zobrist64 = 0; }
	_Board_Flag_& operator=(const _Board_Flag_ &fl){ memcpy(&En_pass, &(fl.En_pass), sizeof(_Board_Flag_)); return *this; }
	bool operator!=(const _Board_Flag_ &fl){ return (zobrist_value.zobrist64 != fl.zobrist_value.zobrist64 || 
		En_pass != fl.En_pass || DrawIn_50step != fl.DrawIn_50step || Castling[0] != fl.Castling[0] || Castling[1] != fl.Castling[1]); }
};

class ChessBoard
{
	friend class MyApp;
public:
	_Pos_			List[PIECE_NUM][MAX_COUNT_PER_PATTERN];
	size_t			index_InList[SQ_COUNT];

private:
	BitBoard		BB[_SIDE_COUNT_][_BB_COUNT_];
	BitBoard		All;
	BitBoard		All_trans;
	BitBoard		All_L45;
	BitBoard		All_R45;
	_ChessType_		ChessArray[SQ_COUNT];
	BitBoard		AttackRange[SQ_COUNT + 1];
	//BitBoard		Range_by_Piece[_SIDE_COUNT_][_BB_COUNT_];

	bool			side;
	_Board_Flag_	flag;

	uint8_t			counter[_SIDE_COUNT_][_BB_COUNT_];

public:
	ChessBoard(){ Reset(); }
	ChessBoard(const ChessBoard &father){ *this = father; }
	bool operator==(ChessBoard& pBoard);
	_ChessType_ chess_at(const _Pos_ p) const{ return ChessArray[p]; }
	_ChessPattern_ pattern_at(const _Pos_ &p) const{ return ChessArray[p] & CHESS_PATTERN; };
	BitBoard Piece_atk_area(const _Pos_ &p) const{ return AttackRange[p]; }
	//BitBoard AttackedBy(const bool &side, const _ChessPattern_ &pt){ return Range_by_Piece[side][pt]; }
	void Set_atk_area(const _Pos_ &p);					//仅用于UI的升变相应等待
	_Pos_ posK(const bool &side) const{ return List[(side * 8) | chess_K][0]; }
	BitBoard get_bb(const bool &side, const _ChessPattern_ &pt) const{ return BB[side][pt]; }
	BitBoard get_bb_all() const{ return All; }
	BitBoard get_bb_all_t() const{ return All_trans; }
	BitBoard get_bb_all_l45() const{ return All_L45; }
	BitBoard get_bb_all_r45() const{ return All_R45; }
	BitBoard pinners(const bool &side) const{ return flag.Pinned[side]; }
	bool side_toMove() const{ return side; }
	_ChessType_ move_capture() const{ return flag.last_capture; }
	_Coordinate_ En_passant() const{ return flag.En_pass; }
	uint8_t cast_right(const bool &side) const{ return flag.Castling[side]; }
	template<uint8_t right> void revoke_cast_right(const bool &side) { flag.Castling[side] ^= right; }
	Zobrist get_zobrist() const{ return flag.zobrist_value; }
	void update_zobrist(const _Zobrist64_ &key) { flag.zobrist_value.zobrist64 ^= key; }
	unsigned short draw50() const{ return flag.DrawIn_50step; }
	unsigned dec_chess_count(const _ChessType_ &tp)  { return --counter[side_of(tp)][pattern_of(tp)]; }
	unsigned inc_chess_count(const _ChessType_ &tp)  { return   counter[side_of(tp)][pattern_of(tp)]++; }
	unsigned chess_count(const _ChessType_ &tp) const{ return   counter[side_of(tp)][pattern_of(tp)]; }
	unsigned chess_count(const bool &side, const _ChessPattern_ &pt) const{ return counter[side][pt]; }
	bool count_IsNormal() const{ 
		return 0 == ((0x0000111112121818 - *((uint64_t*)counter[WHITE_SIDE])) & 
					 (0x0000111112121818 - *((uint64_t*)counter[BLACK_SIDE])) & 0x0000001010100000 ^ 0x0000001010100000); }

	//board funcion
	void Reset();
	void MoveChess(const Movement &mv, const _Depth_ d = MAX_DEPTH);
	void undo_MoveChess(const Movement &mv, const _Depth_ d = MAX_DEPTH);
	void Move(const _Pos_ orig, const _Pos_ dest);
	void Eaten(const _Pos_ pos);
	void Place(const _Pos_ pos, const _ChessType_ tp);

	//attack information function
	inline BitBoard _attack_wp(const _Pos_ &p) const;
	inline BitBoard _attack_bp(const _Pos_ &p) const;
	inline BitBoard _attack_n(const _Pos_ &p) const;
	inline BitBoard _attack_b(const _Pos_ &p) const;
	inline BitBoard _attack_r(const _Pos_ &p) const;
	inline BitBoard _attack_q(const _Pos_ &p) const;
	inline BitBoard _attack_k(const _Pos_ &p) const;
	BitBoard _attack_err(const _Pos_ &p) const{ assert(false); return 0; }
	BitBoard My_attackers_to(const _Pos_ &p) const;
	BitBoard His_attackers_to(const _Pos_ &p) const;
	BitBoard My_heavy_attackers_to(const _Pos_ &p) const;
	BitBoard His_heavy_attackers_to(const _Pos_ &p) const;

	//expand function
	bool pseudo_Legal(const Movement &mv) const;
	bool Legal(const Movement &mv) const;
	BitBoard movement_step(const _Pos_ &pos, bool TEST_CHECK = false);
	unsigned short strict_expand(const bool &side, Movement mvtb[MAX_BRANCH_COUNT]);
	void update_pinners(const bool sd);
	void update_attack_range(const _Pos_ orig, const _Pos_ dest);
	_MoveType_ Delta_MoveType(const _Pos_ orig, const _Pos_ dest) const;

	//search tree
	Movement IterativeDeepening(const bool &side, const uint16_t &step);
	template<bool PvNode> _Value_ AlphaBeta(const _Depth_ &depth, const _Depth_ &depth_limit, const bool &side, _Value_ alpha, _Value_ beta);
	void DEBUG_PRINT_EXPAND(const bool &side);

	//check-test
	//BitBoard Delta_ResistCheck(const bool &side, const _Pos_ &pos) const;
	unsigned short DeltaCheck(const bool &side, _Pos_ &check_pos, _Check_Type_ &checktype)  const;

	//table init
	void Init_BasicTable();
	void Init_MoveTable_KNP();
	void Init_MoveTableStraight();
	void Init_MoveTableDiag();
	void Init_BetweenTable();

	//hash funcion
	void Calculate_BoardHash();

	//evaluate function
	void init_Eval();
	void init_attackArea();
	_Score_ evaluate_pieces();
	_Score_ evaluate_threats(bool Us);
	int init_phase(Phase & gamePhase, _Score_ * non_pawn_material);

	_Value_ evaluation(const bool &side);
	_Score_ value_Material();
	_Score_ value_Space(BitBoard * attackAreaWhite, BitBoard * attackAreaBlack);
	//_Score_ value_Mobility();
	_Score_ value_Imbalance();
	_Score_ value_Pawn();
	template<bool Us> _Score_ pawn_evaluate();
	//template<Color Us, PieceType Pt>
	//_Score_ evaluate_pieces(bool color, int chesstype);
	//_Value_ value_Pieces();
	//template<Color Us, PieceType Pt>
	//_Value_ evaluate_pieces();
};

class PVLine
{
public:
	_Depth_ PVnodes_count;
	Movement PVmovement[MAX_DEPTH];

	PVLine(){ PVnodes_count = 0; }
	void Get_PVLine(ChessBoard Board, bool side);
};

typedef BitBoard(ChessBoard::*FP)(const _Pos_&) const;

extern FP attack_area[];

void BSFTable_Init();



inline BitBoard ChessBoard::_attack_wp(const _Pos_ &p) const
{
	return eat_Pawn[WHITE_SIDE][p];
}

inline BitBoard ChessBoard::_attack_bp(const _Pos_ &p) const
{
	return eat_Pawn[BLACK_SIDE][p];
}

inline BitBoard ChessBoard::_attack_n(const _Pos_ &p) const
{
	return move_Knight[p];
}

inline BitBoard ChessBoard::_attack_b(const _Pos_ &p) const
{
	return (diag_A8H1_attacks[p][(uint8_t)(All_L45 >> start_pos_A8H1[p])] |			//只取最低8位，在斜线长度不到8时高位为无效位，表格建立时已经考虑到这点
		diag_H8A1_attacks[p][(uint8_t)(All_R45 >> start_pos_H8A1[p])]);
}

inline BitBoard ChessBoard::_attack_r(const _Pos_ &p) const
{
	return (rank_attacks[p][(uint8_t)(All >> (p & 0xF8))] |
		file_attacks[p][(uint8_t)(All_trans >> (file_of(p) << 3))]);
}

inline BitBoard ChessBoard::_attack_q(const _Pos_ &p) const
{
	return (this->*attack_area[chess_B])(p) | (this->*attack_area[chess_R])(p);
}

inline BitBoard ChessBoard::_attack_k(const _Pos_ &p) const
{
	return move_King[p];
}