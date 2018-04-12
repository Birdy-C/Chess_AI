#include "Evaluation.h"

extern const _Score_ Piece_Value[_PATTERN_COUNT_];
extern _Score_ psq[PIECE_NUM][SQ_COUNT];

Phase gamePhase;
_Score_ non_pawn_material[_SIDE_COUNT_];

//
// Data members
//const BitBoard& pos;

//BitBoard mobilityArea[_SIDE_COUNT_];
//_Value_ mobility[_SIDE_COUNT_] = { 0, 0 };

// attackedBy[color][piece type] is a bitboard representing all squares
// attacked by a given color and piece type. Special "piece types" which are
// also calculated are QUEEN_DIAGONAL and ALL_PIECES.
//BitBoard attackedBy[_SIDE_COUNT_][CHESS_PATTERN];
BitBoard attackAreaWhite[CHESS_PATTERN] = { 0 };
BitBoard attackAreaBlack[CHESS_PATTERN] = { 0 };
// attackedBy2[color] are the squares attacked by 2 pieces of a given color,
// possibly via x-ray or by one pawn and one piece. Diagonal x-ray through
// pawn or squares attacked by 2 pawns are not explicitly added.
BitBoard attackedBy2[_SIDE_COUNT_];

// kingRing[color] is the zone around the king which is considered
// by the king safety evaluation. This consists of the squares directly
// adjacent to the king, and (only for a king on its first rank) the
// squares two ranks in front of the king. For instance, if black's king
// is on g8, kingRing[BLACK] is a bitboard containing the squares f8, h8,
// f7, g7, h7, f6, g6 and h6.
BitBoard kingRing[_SIDE_COUNT_];

// kingAttackersCount[color] is the number of pieces of the given color
// which attack a square in the kingRing of the enemy king.
int kingAttackersCount[_SIDE_COUNT_];

// kingAttackersWeight[color] is the sum of the "weights" of the pieces of the
// given color which attack a square in the kingRing of the enemy king. The
// weights of the individual piece types are given by the elements in the
// KingAttackWeights array.
int kingAttackersWeight[_SIDE_COUNT_];

// kingAdjacentZoneAttacksCount[color] is the number of attacks by the given
// color to squares directly adjacent to the enemy king. Pieces which attack
// more than one square are counted multiple times. For instance, if there is
// a white knight on g5 and black's king is on g8, this white knight adds 2
// to kingAdjacentZoneAttacksCount[WHITE].
int kingAdjacentZoneAttacksCount[_SIDE_COUNT_];

//====================================功能函数====================================//

_Score_ make_score(int mg, int eg)
{
	return (_Score_)(((uint32_t)eg << 16) | (uint32_t)mg);
}


void ChessBoard::init_Eval()
{
	psq_init();
	pawn_init();
}


 //函数简介：初始化攻击范围，己方棋子也会被算进攻击范围	
void ChessBoard::init_attackArea()
{
	//P
	attackAreaWhite[chess_P] = (BB[WHITE_SIDE][chess_P] & 0x7F7F7F7F7F7F7F7FULL) >> 7 | (BB[WHITE_SIDE][chess_P] & 0xFEFEFEFEFEFEFEFEULL) >> 9;
	attackAreaBlack[chess_P] = (BB[BLACK_SIDE][chess_P] & 0xFEFEFEFEFEFEFEFEULL) << 7 | (BB[BLACK_SIDE][chess_P] & 0x7F7F7F7F7F7F7F7FULL) << 9;
	attackAreaWhite[chess_K] = AttackRange[List[chess_K][0]];
	attackAreaBlack[chess_K] = AttackRange[List[chess_K | BLACK_CHESS_BIT][0]];
	if (count_IsNormal())
	{
		attackAreaWhite[chess_N] = AttackRange[List[chess_N][0]] | AttackRange[List[chess_N][1]];
		attackAreaWhite[chess_B] = AttackRange[List[chess_B][0]] | AttackRange[List[chess_B][1]];
		attackAreaWhite[chess_R] = AttackRange[List[chess_R][0]] | AttackRange[List[chess_R][1]];
		attackAreaWhite[chess_Q] = AttackRange[List[chess_Q][0]];	
		attackAreaBlack[chess_N] = AttackRange[List[chess_N | BLACK_CHESS_BIT][0]] | AttackRange[List[chess_N | BLACK_CHESS_BIT][1]];
		attackAreaBlack[chess_B] = AttackRange[List[chess_B | BLACK_CHESS_BIT][0]] | AttackRange[List[chess_B | BLACK_CHESS_BIT][1]];
		attackAreaBlack[chess_R] = AttackRange[List[chess_R | BLACK_CHESS_BIT][0]] | AttackRange[List[chess_R | BLACK_CHESS_BIT][1]];
		attackAreaBlack[chess_Q] = AttackRange[List[chess_Q | BLACK_CHESS_BIT][0]];
	}
	else
	{
		for (_ChessPattern_ pt = chess_N; pt <= chess_Q; ++pt)
		{
			attackAreaWhite[pt] = 0ULL;
			for (size_t pt_id = 0; pt_id < chess_count(WHITE_SIDE, pt); pt_id++)
				attackAreaWhite[pt] |= AttackRange[List[pt][pt_id]];
			attackAreaBlack[pt] = 0ULL;
			for (size_t pt_id = 0; pt_id < chess_count(BLACK_SIDE, pt); pt_id++)
				attackAreaBlack[pt] |= AttackRange[List[pt | BLACK_CHESS_BIT][pt_id]];
		}
	}
	attackAreaWhite[chess_All] = attackAreaWhite[chess_P] | attackAreaWhite[chess_N] | attackAreaWhite[chess_B] | attackAreaWhite[chess_R] | attackAreaWhite[chess_Q] | attackAreaWhite[chess_K];
	attackAreaBlack[chess_All] = attackAreaBlack[chess_P] | attackAreaBlack[chess_N] | attackAreaBlack[chess_B] | attackAreaBlack[chess_R] | attackAreaBlack[chess_Q] | attackAreaBlack[chess_K];

	attackAreaWhite[chess_All] = attackAreaWhite[chess_K];
	attackedBy2[WHITE_SIDE] = attackAreaWhite[chess_All] & attackAreaWhite[chess_P];
	attackAreaWhite[chess_All] = attackAreaWhite[chess_All] | attackAreaWhite[chess_P];

	attackAreaBlack[chess_All] = attackAreaBlack[chess_K];
	attackedBy2[BLACK_SIDE] = attackAreaBlack[chess_All] & attackAreaBlack[chess_P];
	attackAreaBlack[chess_All] = attackAreaBlack[chess_All] | attackAreaBlack[chess_P];

}

template<Tracing T>  template<Color Us, PieceType Pt>
Score Evaluation<T>::evaluate_pieces() {

	const Color Them = (Us == WHITE ? BLACK : WHITE);
	const Bitboard OutpostRanks = (Us == WHITE ? Rank4BB | Rank5BB | Rank6BB
		: Rank5BB | Rank4BB | Rank3BB);
	const Square* pl = pos.squares<Pt>(Us);

	Bitboard b, bb;
	Square s;
	Score score = SCORE_ZERO;

	attackedBy[Us][Pt] = 0;

	if (Pt == QUEEN)
		attackedBy[Us][QUEEN_DIAGONAL] = 0;

	while ((s = *pl++) != SQ_NONE)
	{
		// Find attacked squares, including x-ray attacks for bishops and rooks
		b = Pt == BISHOP ? attacks_bb<BISHOP>(s, pos.pieces() ^ pos.pieces(QUEEN))
			: Pt == ROOK ? attacks_bb<  ROOK>(s, pos.pieces() ^ pos.pieces(QUEEN) ^ pos.pieces(Us, ROOK))
			: pos.attacks_from<Pt>(s);

		if (pos.pinned_pieces(Us) & s)
			b &= LineBB[pos.square<KING>(Us)][s];

		attackedBy2[Us] |= attackedBy[Us][ALL_PIECES] & b;
		attackedBy[Us][ALL_PIECES] |= attackedBy[Us][Pt] |= b;

		if (Pt == QUEEN)
			attackedBy[Us][QUEEN_DIAGONAL] |= b & PseudoAttacks[BISHOP][s];

		if (b & kingRing[Them])
		{
			kingAttackersCount[Us]++;
			kingAttackersWeight[Us] += KingAttackWeights[Pt];
			kingAdjacentZoneAttacksCount[Us] += popcount(b & attackedBy[Them][KING]);
		}

		int mob = popcount(b & mobilityArea[Us]);

		mobility[Us] += MobilityBonus[Pt - 2][mob];

		// Bonus for this piece as a king protector
		score += KingProtector[Pt - 2] * distance(s, pos.square<KING>(Us));

		if (Pt == BISHOP || Pt == KNIGHT)
		{
			// Bonus for outpost squares
			bb = OutpostRanks & ~pe->pawn_attacks_span(Them);
			if (bb & s)
				score += Outpost[Pt == BISHOP][bool(attackedBy[Us][PAWN] & s)] * 2;
			else
			{
				bb &= b & ~pos.pieces(Us);
				if (bb)
					score += Outpost[Pt == BISHOP][bool(attackedBy[Us][PAWN] & bb)];
			}

			// Bonus when behind a pawn
			if (relative_rank(Us, s) < RANK_5
				&& (pos.pieces(PAWN) & (s + pawn_push(Us))))
				score += MinorBehindPawn;

			if (Pt == BISHOP)
			{
				// Penalty for pawns on the same color square as the bishop
				score -= BishopPawns * pe->pawns_on_same_color_squares(Us, s);

				// Bonus for bishop on a long diagonal which can "see" both center squares
				if (more_than_one(Center & (attacks_bb<BISHOP>(s, pos.pieces(PAWN)) | s)))
					score += LongRangedBishop;
			}

			// An important Chess960 pattern: A cornered bishop blocked by a friendly
			// pawn diagonally in front of it is a very serious problem, especially
			// when that pawn is also blocked.
			if (Pt == BISHOP
				&& pos.is_chess960()
				&& (s == relative_square(Us, SQ_A1) || s == relative_square(Us, SQ_H1)))
			{
				Direction d = pawn_push(Us) + (file_of(s) == FILE_A ? EAST : WEST);
				if (pos.piece_on(s + d) == make_piece(Us, PAWN))
					score -= !pos.empty(s + d + pawn_push(Us)) ? TrappedBishopA1H1 * 4
					: pos.piece_on(s + d + d) == make_piece(Us, PAWN) ? TrappedBishopA1H1 * 2
					: TrappedBishopA1H1;
			}
		}

		if (Pt == ROOK)
		{
			// Bonus for aligning with enemy pawns on the same rank/file
			if (relative_rank(Us, s) >= RANK_5)
				score += RookOnPawn * popcount(pos.pieces(Them, PAWN) & PseudoAttacks[ROOK][s]);

			// Bonus when on an open or semi-open file
			if (pe->semiopen_file(Us, file_of(s)))
				score += RookOnFile[bool(pe->semiopen_file(Them, file_of(s)))];

			// Penalty when trapped by the king, even more if the king cannot castle
			else if (mob <= 3)
			{
				Square ksq = pos.square<KING>(Us);

				if (((file_of(ksq) < FILE_E) == (file_of(s) < file_of(ksq)))
					&& !pe->semiopen_side(Us, file_of(ksq), file_of(s) < file_of(ksq)))
					score -= (TrappedRook - make_score(mob * 22, 0)) * (1 + !pos.can_castle(Us));
			}
		}

		if (Pt == QUEEN)
		{
			// Penalty if any relative pin or discovered attack against the queen
			Bitboard pinners;
			if (pos.slider_blockers(pos.pieces(Them, ROOK, BISHOP), s, pinners))
				score -= WeakQueen;
		}
	}

	if (T)
		Trace::add(Pt, Us, score);

	return score;
}

//函数简介：计算现在所处的状态 0 代表开局 255 残局
//参考 https://chessprogramming.wikispaces.com/Tapered+Eval
//然后上面这一种和咸鱼不一样……
int ChessBoard::init_phase(Phase& gamePhase,_Score_* non_pawn_material)
{
	non_pawn_material[WHITE_SIDE] = 0;
	non_pawn_material[BLACK_SIDE] = 0;

	for (int chess = chess_N; chess <= chess_Q; chess++)
	{
		non_pawn_material[WHITE_SIDE] += BitCount(BB[WHITE_SIDE][chess]) * Piece_Value[chess];
		non_pawn_material[BLACK_SIDE] += BitCount(BB[BLACK_SIDE][chess]) * Piece_Value[chess];
	}

	_Score_ npm_w = non_pawn_material[WHITE_SIDE];
	_Score_ npm_b = non_pawn_material[BLACK_SIDE];

	//_Score_ npm = std::max(EndgameLimit, std::min(npm_w + npm_b, MidgameLimit));
	_Score_ npm;
	npm = npm_w + npm_b < MidgameLimit ? npm_w + npm_b: MidgameLimit;
	npm = EndgameLimit > npm ? EndgameLimit : npm;
	// Map total non-pawn material into [PHASE_ENDGAME, PHASE_MIDGAME]
	gamePhase = Phase(((npm - EndgameLimit) * PHASE_MIDGAME) / (MidgameLimit - EndgameLimit));
	return 0;
}


//====================================具体估值====================================//

//总的评估
_Value_ ChessBoard::evaluation(const bool &side)
{
	//白方为视角 这里开七个最后一个记录总的

	bool test_mode = false;

	_Score_ result = 0;
	_Score_ score = 0;
	init_attackArea();
	init_phase(gamePhase, non_pawn_material);

	// Material
	result = value_Material();
	score += result;
	if(test_mode)
		std::cout <<" value_Material \t "<< (int16_t)(result & 0xffff) << '\t' << (int16_t)(result >> 16) << std::endl;


	// Imbalance 主要是棋子数量的影响 残局的判断还没加
	result = value_Imbalance();
	score += result;
	if (test_mode)
		std::cout << " value_Imbalance \t " << (int16_t)(result & 0xffff) << '\t' << (int16_t)(result >> 16) << std::endl;

	// 兵
	result = value_Pawn();
	score += result;
	if (test_mode)
		std::cout << " value_Pawn \t " << (int16_t)(result & 0xffff) << '\t' << (int16_t)(result >> 16) << std::endl;


	// Early exit if score is high
	int score1;
	score1 = (((int16_t)(score & 0xffff)) + ((int16_t)(score >> 16))) / 2;
	if (abs(score1)+ Contempt > LazyThreshold)
		return side ? score1 : -score1;




	// Space 这个主要是在开局的时候加快出子
	if (non_pawn_material[WHITE_SIDE] + non_pawn_material[BLACK_SIDE] >= SpaceThreshold)
	{
		result = value_Space(attackAreaWhite, attackAreaBlack);
		score += result;
		if (test_mode)
			std::cout << "value_Space \t " << (int16_t)(result & 0xffff) << '\t' << (int16_t)(result >> 16) << std::endl;
	}



	// Interpolate between a middlegame and a (scaled by 'sf') endgame score
	int sf = 64;//这个可以根据mgvalue和egvalue求出来 然而我看不懂……直接带了normal 在evaluate主函数结尾
	int v = ((int16_t)(score >> 16)) * int(gamePhase)
		+ ((int16_t)(score & 0xffff)) * int(PHASE_MIDGAME - gamePhase) * sf / 64;

	v /= int(PHASE_MIDGAME);

	//std::cout << score1 << std::endl;
	return side ? v : -v;
}



// 这个空间的函数在咸鱼里是黑白共用一个的
// evaluate_space() computes the space evaluation for a given side. The
// space evaluation is a simple bonus based on the number of safe squares
// available for minor pieces on the central four files on ranks 2--4. Safe
// squares one, two or three squares behind a friendly pawn are counted
// twice. Finally, the space bonus is multiplied by a weight. The aim is to
// improve play on game opening.
_Score_ ChessBoard::value_Space(BitBoard *attackAreaWhite, BitBoard *attackAreaBlack)
{
	 BitBoard SpaceMask;
	 BitBoard safe;
	 BitBoard behind;
	 int bonus;
	 int weight;
	 int open_files = 0; // 通路数量
	 int score;

	 BitBoard Pawn = BB[WHITE_SIDE][chess_P] | BB[BLACK_SIDE][chess_P];
	 if (Pawn & FileABB)	 open_files++;
	 if (Pawn & FileBBB)	 open_files++;
	 if (Pawn & FileCBB)	 open_files++;
	 if (Pawn & FileDBB)	 open_files++;
	 if (Pawn & FileEBB)	 open_files++;
	 if (Pawn & FileFBB)	 open_files++;
	 if (Pawn & FileGBB)	 open_files++;
	 if (Pawn & FileHBB)	 open_files++;

	 //白色
	SpaceMask = CenterFiles & (Rank2BB | Rank3BB | Rank4BB);
	

	// Find the safe squares for our pieces inside the area defined by
	// SpaceMask. A square is unsafe if it is attacked by an enemy
	// pawn, or if it is undefended and attacked by an enemy piece.
	safe = SpaceMask
		& ~BB[WHITE_SIDE][chess_P]
		& ~attackAreaBlack[chess_P]
		& (attackAreaWhite[chess_All] | ~attackAreaBlack[chess_All]);

	// Find all squares which are at most three squares behind some friendly pawn
	behind = BB[WHITE_SIDE][chess_P];
	behind |= behind >> 8;
	behind |= behind >> 16;


	// ...count safe + (behind & safe) with a single popcount. non-zero bits in a bitboard
	bonus = BitCount((safe << 32) | (behind & safe));
	weight = BitCount(BB[WHITE_SIDE][chess_All]) - 2 * open_files;

	score = bonus * weight * weight / 16;

	//黑色
	
	SpaceMask= CenterFiles & (Rank7BB | Rank6BB | Rank5BB);

	// Find the safe squares for our pieces inside the area defined by
	// SpaceMask. A square is unsafe if it is attacked by an enemy
	// pawn, or if it is undefended and attacked by an enemy piece.
	safe = SpaceMask
		& ~BB[BLACK_SIDE][chess_P]
		& ~attackAreaWhite[chess_P]
		& (attackAreaBlack[chess_All] | ~attackAreaWhite[chess_All]);

	// Find all squares which are at most three squares behind some friendly pawn
	behind = BB[BLACK_SIDE][chess_P];
	behind |= behind << 8;
	behind |= behind << 16;
	// ...count safe + (behind & safe) with a single popcount. non-zero bits in a bitboard
	bonus = BitCount((safe >> 32) | (behind & safe));
	weight = BitCount(BB[BLACK_SIDE][chess_All]) - 2 * open_files;

	score -= bonus * weight * weight / 16;

	return S(score / 16, 0);
}



// 这个需要每个拆开来
// 需要知道每个棋子的可移动位置数量 
_Score_ ChessBoard::value_Mobility()
{  
	// MobilityBonus[PieceType][attacked] contains bonuses for middle and end game,
	// indexed by piece type and number of attacked squares in the mobility area.
	const _Score_ MobilityBonus[][32] = 
	{
		{
			S(0, 0)
		},
		{					// Knights	
			S(-75, -76), S(-57, -54), S(-9, -28), S(-2, -10), S(6, 5), S(14, 12), S(22, 26), S(29, 29), S(36, 29) 
		},
		{					// Bishops
			S(-48, -59), S(-20, -23), S(16, -3), S(26, 13), S(38, 24), S(51, 42), S(55, 54),							
			S( 63,  57), S( 63,  65), S(68, 73), S(81, 78), S(81, 86), S(91, 88), S(98, 97) 
		},
		{					// Rooks
			S(-58, -76), S(-27,-18), S(-15, 28), S(-10, 55), S(-5, 69), S(-2, 82), S( 9,112), S(16, 118),
			S( 30, 132), S( 29,142), S( 32,155), S( 38,165), S(46,166), S(48,169), S(58,171) 
		},
		{					// Queens
			S(-39,-36), S(-21,-15), S(  3,  8), S(  3, 18), S( 14, 34), S( 22, 54), S( 28, 61), S( 41, 73), S(43, 79), S(48, 92), 
			S( 56, 94), S( 60,104), S( 60,113), S( 66,120), S( 67,123), S( 70,126), S( 71,133), S( 73,136), S(79,140), S(88,143), 
			S( 88,148), S( 99,166), S(102,170), S(102,175),	S(106,184), S(109,191), S(113,206), S(116,212) 
		}
	};

	// 单个棋子的范围 - 除去自己的棋子的位置  和  被对方比自己小的棋子攻击的位置

	return 0;
}


/*
_Value_ ChessBoard::value_Pieces()
{
	_Value_ score = 0;
	score += evaluate_pieces<((bool)WHITE_SIDE), (int)chess_K>() - evaluate_pieces<(bool)BLACK_SIDE, (int)chess_K>();
	score += evaluate_pieces<WHITE_SIDE, BISHOP>() - evaluate_pieces<BLACK_SIDE, BISHOP>();
	score += evaluate_pieces<WHITE_SIDE, ROOK  >() - evaluate_pieces<BLACK_SIDE, ROOK  >();
	score += evaluate_pieces<WHITE_SIDE, QUEEN >() - evaluate_pieces<BLACK_SIDE, QUEEN >();
	return score;

}


template<Tracing T>  template<bool Us, int Pt>
_Value_ ChessBoard<T>::evaluate_pieces() {

	const Color Them = (Us == WHITE ? BLACK : WHITE);
	const Bitboard OutpostRanks = (Us == WHITE ? Rank4BB | Rank5BB | Rank6BB
		: Rank5BB | Rank4BB | Rank3BB);
	const Square* pl = pos.squares<Pt>(Us);

	Bitboard b, bb;
	Square s;
	Score score = SCORE_ZERO;

	attackedBy[Us][Pt] = 0;

	if (Pt == QUEEN)
		attackedBy[Us][QUEEN_DIAGONAL] = 0;

	while ((s = *pl++) != SQ_NONE)
	{
		// Find attacked squares, including x-ray attacks for bishops and rooks
		b = Pt == BISHOP ? attacks_bb<BISHOP>(s, pos.pieces() ^ pos.pieces(QUEEN))
			: Pt == ROOK ? attacks_bb<  ROOK>(s, pos.pieces() ^ pos.pieces(QUEEN) ^ pos.pieces(Us, ROOK))
			: pos.attacks_from<Pt>(s);

		if (pos.pinned_pieces(Us) & s)
			b &= LineBB[pos.square<KING>(Us)][s];

		attackedBy2[Us] |= attackedBy[Us][ALL_PIECES] & b;
		attackedBy[Us][ALL_PIECES] |= attackedBy[Us][Pt] |= b;

		if (Pt == QUEEN)
			attackedBy[Us][QUEEN_DIAGONAL] |= b & PseudoAttacks[BISHOP][s];

		if (b & kingRing[Them])
		{
			kingAttackersCount[Us]++;
			kingAttackersWeight[Us] += KingAttackWeights[Pt];
			kingAdjacentZoneAttacksCount[Us] += popcount(b & attackedBy[Them][KING]);
		}

		int mob = popcount(b & mobilityArea[Us]);

		mobility[Us] += MobilityBonus[Pt - 2][mob];

		// Bonus for this piece as a king protector
		score += KingProtector[Pt - 2] * distance(s, pos.square<KING>(Us));

		if (Pt == BISHOP || Pt == chess_K)
		{
			// Bonus for outpost squares
			bb = OutpostRanks & ~pe->pawn_attacks_span(Them);
			if (bb & s)
				score += Outpost[Pt == BISHOP][bool(attackedBy[Us][PAWN] & s)] * 2;
			else
			{
				bb &= b & ~pos.pieces(Us);
				if (bb)
					score += Outpost[Pt == BISHOP][bool(attackedBy[Us][PAWN] & bb)];
			}

			// Bonus when behind a pawn
			if (relative_rank(Us, s) < RANK_5
				&& (pos.pieces(PAWN) & (s + pawn_push(Us))))
				score += MinorBehindPawn;

			if (Pt == BISHOP)
			{
				// Penalty for pawns on the same color square as the bishop
				score -= BishopPawns * pe->pawns_on_same_color_squares(Us, s);

				// Bonus for bishop on a long diagonal which can "see" both center squares
				if (more_than_one(Center & (attacks_bb<BISHOP>(s, pos.pieces(PAWN)) | s)))
					score += LongRangedBishop;
			}

			// An important Chess960 pattern: A cornered bishop blocked by a friendly
			// pawn diagonally in front of it is a very serious problem, especially
			// when that pawn is also blocked.
			if (Pt == BISHOP
				&& pos.is_chess960()
				&& (s == relative_square(Us, SQ_A1) || s == relative_square(Us, SQ_H1)))
			{
				Direction d = pawn_push(Us) + (file_of(s) == FILE_A ? EAST : WEST);
				if (pos.piece_on(s + d) == make_piece(Us, PAWN))
					score -= !pos.empty(s + d + pawn_push(Us)) ? TrappedBishopA1H1 * 4
					: pos.piece_on(s + d + d) == make_piece(Us, PAWN) ? TrappedBishopA1H1 * 2
					: TrappedBishopA1H1;
			}
		}

		if (Pt == ROOK)
		{
			// Bonus for aligning with enemy pawns on the same rank/file
			if (relative_rank(Us, s) >= RANK_5)
				score += RookOnPawn * popcount(pos.pieces(Them, PAWN) & PseudoAttacks[ROOK][s]);

			// Bonus when on an open or semi-open file
			if (pe->semiopen_file(Us, file_of(s)))
				score += RookOnFile[bool(pe->semiopen_file(Them, file_of(s)))];

			// Penalty when trapped by the king, even more if the king cannot castle
			else if (mob <= 3)
			{
				Square ksq = pos.square<KING>(Us);

				if (((file_of(ksq) < FILE_E) == (file_of(s) < file_of(ksq)))
					&& !pe->semiopen_side(Us, file_of(ksq), file_of(s) < file_of(ksq)))
					score -= (TrappedRook - make_score(mob * 22, 0)) * (1 + !pos.can_castle(Us));
			}
		}

		if (Pt == QUEEN)
		{
			// Penalty if any relative pin or discovered attack against the queen
			Bitboard pinners;
			if (pos.slider_blockers(pos.pieces(Them, ROOK, BISHOP), s, pinners))
				score -= WeakQueen;
		}
	}

	if (T)
		Trace::add(Pt, Us, score);

	return score;
}
*/

