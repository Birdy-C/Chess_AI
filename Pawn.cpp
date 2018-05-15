#include "Pawn.h"
#include "BitBoard.h"

//
#define V int
#define S(mg, eg) make_score(mg, eg)

// Isolated pawn penalty
static const _Score_ Isolated = S(13, 18);

// Backward pawn penalty
static const _Score_ Backward = S(24, 12);

// Connected pawn bonus by opposed, phalanx, #support and rank
static _Score_ Connected[2][2][3][8];

// Doubled pawn penalty
static const _Score_ Doubled = S(18, 38);

// Weakness of our pawn shelter in front of the king by [isKingFile][distance from edge][rank].
// RANK_1 = 0 is used for files where we have no pawns or our pawn is behind our king.
static const V ShelterWeakness[2][4][8] = 
{
	{ 
		{ V( 97), V(17), V( 9), V(44), V( 84), V( 87), V( 99) },		// Not On King file
		{ V(106), V( 6), V(33), V(86), V( 87), V(104), V(112) },
		{ V(101), V( 2), V(65), V(98), V( 58), V( 89), V(115) },
		{ V( 73), V( 7), V(54), V(73), V( 84), V( 83), V(111) } 
	},
	{ 
		{ V(104), V(20), V( 6), V(27), V( 86), V( 93), V( 82) },		// On King file
		{ V(123), V( 9), V(34), V(96), V(112), V( 88), V( 75) },
		{ V(120), V(25), V(65), V(91), V( 66), V( 78), V(117) },
		{ V( 81), V( 2), V(47), V(63), V( 94), V( 93), V(104) } 
	}
};

// Danger of enemy pawns moving toward our king by [type][distance from edge][rank].
// For the unopposed and unblocked cases, RANK_1 = 0 is used when opponent has
// no pawn on the given file, or their pawn is behind our king.
const V StormDanger[][4][8] = 
{
	{ 
		{ V(0),  V(-290), V(-274), V(57), V(41) },			// BlockedByKing
		{ V(0),  V(  60), V( 144), V(39), V(13) },
		{ V(0),  V(  65), V( 141), V(41), V(34) },
		{ V(0),  V(  53), V( 127), V(56), V(14) } 
	},
	{ 
		{ V(4),  V(73), V(132), V(46), V(31) },				// Unopposed
		{ V(1),  V(64), V(143), V(26), V(13) },
		{ V(1),  V(47), V(110), V(44), V(24) },
		{ V(0),  V(72), V(127), V(50), V(31) } 
	},
	{ 
		{ V(0),  V(0), V( 79), V(23), V( 1) },				// BlockedByPawn
		{ V(0),  V(0), V(148), V(27), V( 2) },
		{ V(0),  V(0), V(161), V(16), V( 1) },
		{ V(0),  V(0), V(171), V(22), V(15) } 
	},
	{ 
		{ V(22),  V(45), V(104), V(62), V( 6) },			// Unblocked
		{ V(31),  V(30), V( 99), V(39), V(19) },
		{ V(23),  V(29), V( 96), V(41), V(15) },
		{ V(21),  V(23), V(116), V(41), V(15) } 
	}
};

void pawn_init() 
{
	static const int Seed[8] = { 0, 13, 24, 18, 76, 100, 175, 330 };

	for (int opposed = 0; opposed <= 1; ++opposed)
		for (int phalanx = 0; phalanx <= 1; ++phalanx)
			for (int support = 0; support <= 2; ++support)
				for (int r = 1; r < 7; ++r)
				{
					int v = 17 * support;
					v += (Seed[r] + (phalanx ? (Seed[r + 1] - Seed[r]) / 2 : 0)) >> opposed;

					Connected[opposed][phalanx][support][r] = make_score(v, v * (r - 2) / 4);
				}
}

template<bool Us>
_Score_ ChessBoard::pawn_evaluate() 
{
	const bool     Them = (Us == WHITE_SIDE ? BLACK_SIDE : WHITE_SIDE);

	BitBoard b, neighbours, stoppers, supported;
	BitBoard lever, leverPush;
	bool opposed, phalanx, doubled;
	//bool backward;								这里backward和求一个棋子后面这个格子的函数名字冲突了
	_Score_ score = 0;

	BitBoard ourPawns = BB[Us][chess_P];
	BitBoard theirPawns = BB[Them][chess_P];
	BitBoard tempPawn = ourPawns;
	_Pos_ choosenPawn;

	// Loop through all pawns of the current color and score each pawn
	while (tempPawn)
	{
		choosenPawn = pop_lsb(tempPawn);

		//opppsed：有没有被敌兵阻挡
		opposed = theirPawns & ((get_file(file_of(choosenPawn)) >> (relative_rank_of(Us, choosenPawn) << 3)) << (Us * (rank_of(choosenPawn) << 3)));

		//doubled：叠兵
		doubled = ourPawns & backward(mask(choosenPawn), Us);

		//neighbours：左右列自己的兵
		neighbours = ourPawns & (
			get_file(ourPawns & 0xFEFEFEFEFEFEFEFEULL, file_of(choosenPawn + 1)) |
			get_file(ourPawns & 0x7F7F7F7F7F7F7F7FULL, file_of(choosenPawn - 1)) );
		
		//phalanx：本行中自己相邻的兵
		phalanx = get_rank(neighbours, rank_of(choosenPawn));
		
		//supported：前一行中被当前的兵所支援的兵
		supported = get_rank(neighbours, rank_of(choosenPawn) + (Us == WHITE_SIDE ? -1 : 1));

		if (supported || phalanx)
			score += Connected[opposed][phalanx][BitCount(supported)][relative_rank_of(Us, choosenPawn)];

		else if (!neighbours)
			score -= Isolated;

		//else if (backward)
		//	score -= Backward;

		if (doubled && !supported)
			score -= Doubled;
	}
	return score;
}

_Score_ ChessBoard::value_Pawn()
{
	//int value = pawn_evaluate<WHITE_SIDE>() - pawn_evaluate<BLACK_SIDE>();
	//return make_score(value, value);
	return pawn_evaluate<WHITE_SIDE>() - pawn_evaluate<BLACK_SIDE>();
}
