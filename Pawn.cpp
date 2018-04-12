#include "Pawn.h"
//
#define V int
#define S(mg, eg) make_score(mg, eg)

// Isolated pawn penalty
const _Score_ Isolated = S(13, 18);

// Backward pawn penalty
const _Score_ Backward = S(24, 12);

// Connected pawn bonus by opposed, phalanx, #support and rank
_Score_ Connected[2][2][3][8];

// Doubled pawn penalty
const _Score_ Doubled = S(18, 38);

// Weakness of our pawn shelter in front of the king by [isKingFile][distance from edge][rank].
// RANK_1 = 0 is used for files where we have no pawns or our pawn is behind our king.
const V ShelterWeakness[][4][8] = 
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

void pawn_init() {

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
_Score_ ChessBoard::pawn_evaluate() {

	const bool     Them = (Us == WHITE_SIDE ? BLACK_SIDE : WHITE_SIDE);
	const Direction Up = (Us == WHITE_SIDE ? NORTH : SOUTH);
	const Direction Right = (Us == WHITE_SIDE ? NORTH_EAST : SOUTH_WEST);
	const Direction Left = (Us == WHITE_SIDE ? NORTH_WEST : SOUTH_EAST);

	BitBoard b, neighbours, stoppers, doubled, supported, phalanx;
	BitBoard lever, leverPush;
	bool opposed, backward;
	_Value_ score = 0;

	BitBoard ourPawns = BB[Us][chess_P];
	BitBoard theirPawns = BB[Them][chess_P];
	BitBoard tempPawn = ourPawns;
	BitBoard choosenPawn;
	// Loop through all pawns of the current color and score each pawn
	while (tempPawn)
	{
		choosenPawn = pop_lsb(tempPawn);
		BitBoard s = (unsigned long long)1 << choosenPawn;
		//看不懂……
		assert(s & ourPawns);//选出来的是P
		//(0x0101010101010101 >> (rank_of(pos) << 3)) << (rank_of(pos) << 3);

		opposed = theirPawns & (Us == WHITE_SIDE ? ((0x0101010101010101 >> (rank_of(s) << 3)) << (rank_of(s) << 3)) << file_of(s) :
			((0x0101010101010101 << (rank_of(s) << 3)) >> (rank_of(s) << 3)) << file_of(s));
		//opposed = theirPawns & forward_file_bb(Us, s);
		doubled = ourPawns   & (Us == WHITE_SIDE ? s << 8 : s >> 8);//后面一行
		neighbours = get_rank(ourPawns & 0xFEFEFEFEFEFEFEFEULL, s << 1) || get_rank(ourPawns & 0x7F7F7F7F7F7F7F7FULL, s >> 1);
		//neighbours = ourPawns   & adjacent_files_bb(f);//左右列
		phalanx = get_file(neighbours, s & 7);
		//phalanx = neighbours & rank_bb(s);//本行
		supported = get_file(neighbours, (Us != WHITE_SIDE ? s << 8 : s >> 8) & 7);
		//supported = neighbours & rank_bb(s - Up);//前一行


		if (supported | phalanx)
			score += Connected[opposed][bool(phalanx)][BitCount(~supported)][(rank_of(s) ^ (Us * 7)) & 7];

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
	int value = pawn_evaluate<WHITE_SIDE>() - pawn_evaluate<BLACK_SIDE>();
	return make_score(value, value);
}
