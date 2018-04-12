#include "Evaluation.h"
#include "Macro.h"

//基础子力 
_Score_ psq[PIECE_NUM][SQ_COUNT];
extern const _Score_ Piece_Value[_PATTERN_COUNT_] = { S(171, 240), S(764, 848), S(826, 891), S(1282, 1373), S(2513, 2650), S(20000, 20000) };

const _Score_ Bonus[_PATTERN_COUNT_][_COORDINATE_COUNT_][_COORDINATE_COUNT_ / 2] =
{
	{ // Pawn
		{ S(  0, 0), S( 0, 0), S( 0, 0), S( 0, 0) },
		{ S(-11, 7), S( 6,-4), S( 7, 8), S( 3,-2) },
		{ S(-18,-4), S(-2,-5), S(19, 5), S(24, 4) },
		{ S(-17, 3), S(-9, 3), S(20,-8), S(35,-3) },
		{ S( -6, 8), S( 5, 9), S( 3, 7), S(21,-6) },
		{ S( -6, 8), S(-8,-5), S(-6, 2), S(-2, 4) },
		{ S( -4, 3), S(20,-9), S(-8, 1), S(-4,18) },
		{ S(  0, 0), S( 0, 0), S( 0, 0), S( 0, 0) }
	},
	{ // Knight
		{ S(-161,-105), S(-96,-82), S(-80,-46), S(-73,-14) },
		{ S( -83, -69), S(-43,-54), S(-21,-17), S(-10,  9) },
		{ S( -71, -50), S(-22,-39), S(  0, -7), S(  9, 28) },
		{ S( -25, -41), S( 18,-25), S( 43,  6), S( 47, 38) },
		{ S( -26, -46), S( 16,-25), S( 38,  3), S( 50, 40) },
		{ S( -11, -54), S( 37,-38), S( 56, -7), S( 65, 27) },
		{ S( -63, -65), S(-19,-50), S(  5,-24), S( 14, 13) },
		{ S(-195,-109), S(-67,-89), S(-42,-50), S(-29,-13) }
	},
	{ // Bishop
		{ S(-44,-58), S(-13,-31), S(-25,-37), S(-34,-19) },
		{ S(-20,-34), S( 20, -9), S( 12,-14), S(  1,  4) },
		{ S( -9,-23), S( 27,  0), S( 21, -3), S( 11, 16) },
		{ S(-11,-26), S( 28, -3), S( 21, -5), S( 10, 16) },
		{ S(-11,-26), S( 27, -4), S( 16, -7), S(  9, 14) },
		{ S(-17,-24), S( 16, -2), S( 12,  0), S(  2, 13) },
		{ S(-23,-34), S( 17,-10), S(  6,-12), S( -2,  6) },
		{ S(-35,-55), S(-11,-32), S(-19,-36), S(-29,-17) }
	},
	{ // Rook
		{ S(-25, 0), S(-16, 0), S(-16, 0), S(-9, 0) },
		{ S(-21, 0), S( -8, 0), S( -3, 0), S( 0, 0) },
		{ S(-21, 0), S( -9, 0), S( -4, 0), S( 2, 0) },
		{ S(-22, 0), S( -6, 0), S( -1, 0), S( 2, 0) },
		{ S(-22, 0), S( -7, 0), S(  0, 0), S( 1, 0) },
		{ S(-21, 0), S( -7, 0), S(  0, 0), S( 2, 0) },
		{ S(-12, 0), S(  4, 0), S(  8, 0), S(12, 0) },
		{ S(-23, 0), S(-15, 0), S(-11, 0), S(-5, 0) }
	},
	{ // Queen
		{ S( 0,-71), S(-4,-56), S(-3,-42), S(-1,-29) },
		{ S(-4,-56), S( 6,-30), S( 9,-21), S( 8, -5) },
		{ S(-2,-39), S( 6,-17), S( 9, -8), S( 9,  5) },
		{ S(-1,-29), S( 8, -5), S(10,  9), S( 7, 19) },
		{ S(-3,-27), S( 9, -5), S( 8, 10), S( 7, 21) },
		{ S(-2,-40), S( 6,-16), S( 8,-10), S(10,  3) },
		{ S(-2,-55), S( 7,-30), S( 7,-21), S( 6, -6) },
		{ S(-1,-74), S(-4,-55), S(-1,-43), S( 0,-30) }
	},
	{ // King
		{ S(267,  0), S(320, 48), S(270, 75), S(195, 84) },
		{ S(264, 43), S(304, 92), S(238,143), S(180,132) },
		{ S(200, 83), S(245,138), S(176,167), S(110,165) },
		{ S(177,106), S(185,169), S(148,169), S(110,179) },
		{ S(149,108), S(177,163), S(115,200), S( 66,203) },
		{ S(118, 95), S(159,155), S( 84,176), S( 41,174) },
		{ S( 87, 50), S(128, 99), S( 63,122), S( 20,139) },
		{ S( 63,  9), S( 88, 55), S( 47, 80), S(  0, 90) }
	}
};

#undef S


void psq_init() 
{
	//Piece_Value[_PATTERN_COUNT_] = { S(171,240), S(764,848),S(826,891), S(1282,1373), S(2513,2650),S(20000,20000) };
	//这俩用int好像有点奇怪otz
	for (int pc = WHITE_P; pc <= WHITE_K; ++pc)
	{
		_Score_ v = Piece_Value[pc];
		for (_Pos_ s = 0; s < 64; ++s)
		{
			_Coordinate_ f = file_of(s) < file_of(~s) ? file_of(s) : file_of(~s); // 小的列 用std:min会和windows的头文件冲突……
			psq[pc][s] = v + Bonus[pc][rank_of(s)][f];
			psq[pc | BLACK_CHESS_BIT][63 - s] = -psq[pc][s];
			//psq[~pc][~s] = -psq[pc][s];
			//两个不一样的定义……得看看后面怎么解释的……
		}
	}
}


// 子力价值
_Score_ ChessBoard::value_Material()
{
	//type.h & psqt.h
	_Score_ value = 0;
	for (_Pos_ p = 0; p < 64; p++)
	{
		_ChessType_ chess = chess_at(p);
		if (CHESS_NONE != chess)
			value += psq[chess][p];
	}
	/*
	value += (BitCount(BB[WHITE_SIDE][chess_P]) - BitCount(BB[BLACK_SIDE][chess_P])) * Piece_Value[chess_P];
	value += (BitCount(BB[WHITE_SIDE][chess_N]) - BitCount(BB[BLACK_SIDE][chess_N])) * Piece_Value[chess_N];
	value += (BitCount(BB[WHITE_SIDE][chess_B]) - BitCount(BB[BLACK_SIDE][chess_B])) * Piece_Value[chess_B];
	value += (BitCount(BB[WHITE_SIDE][chess_R]) - BitCount(BB[BLACK_SIDE][chess_R])) * Piece_Value[chess_R];
	value += (BitCount(BB[WHITE_SIDE][chess_Q]) - BitCount(BB[BLACK_SIDE][chess_Q])) * Piece_Value[chess_Q];
	*/


	/*	// https://chessprogramming.wikispaces.com/Bishop+Pair
	// 分值一半的兵
	if (2 == BitCount(BB[WHITE_SIDE][chess_B]))
	value += S(100, 100);

	if (2 == BitCount(BB[BLACK_SIDE][chess_B]))
	value -= S(100, 100);

	*/

	return value;
}
