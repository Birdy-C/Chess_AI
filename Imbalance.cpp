#include "Evaluation.h"
/*这个函数中还缺一个对残局的判断*/
const int PIECE_TYPE_NB = 8;

const int QuadraticOurs[][PIECE_TYPE_NB] = {
	//            OUR PIECES
	// pair pawn knight bishop rook queen
	{ 1667 }, // Bishop pair
	{ 40,    0 }, // Pawn
	{ 32,  255,  -3 }, // Knight      OUR PIECES
	{ 0,  104,   4,    0 }, // Bishop
	{ -26,   -2,  47,   105,  -149 }, // Rook
	{ -189,   24, 117,   133,  -134, -10 }  // Queen
};

const int QuadraticTheirs[][PIECE_TYPE_NB] = {
	//           THEIR PIECES
	// pair pawn knight bishop rook queen
	{ 0 }, // Bishop pair
	{ 36,    0 }, // Pawn
	{ 9,   63,   0 }, // Knight      OUR PIECES
	{ 59,   65,  42,     0 }, // Bishop
	{ 46,   39,  24,   -24,    0 }, // Rook
	{ 97,  100, -42,   137,  268,    0 }  // Queen
};
/*
// Endgame evaluation and scaling functions are accessed directly and not through
// the function maps because they correspond to more than one material hash key.
Endgame<KXK>    EvaluateKXK[] = { Endgame<KXK>(WHITE),    Endgame<KXK>(BLACK) };

Endgame<KBPsK>  ScaleKBPsK[] = { Endgame<KBPsK>(WHITE),  Endgame<KBPsK>(BLACK) };
Endgame<KQKRPs> ScaleKQKRPs[] = { Endgame<KQKRPs>(WHITE), Endgame<KQKRPs>(BLACK) };
Endgame<KPsK>   ScaleKPsK[] = { Endgame<KPsK>(WHITE),   Endgame<KPsK>(BLACK) };
Endgame<KPKP>   ScaleKPKP[] = { Endgame<KPKP>(WHITE),   Endgame<KPKP>(BLACK) };

// Helper used to detect a given material distribution
bool is_KXK(const Position& pos, Color us) {
	return  !more_than_one(pos.pieces(~us))
		&& pos.non_pawn_material(us) >= RookValueMg;
}

bool is_KBPsKs(const Position& pos, Color us) {
	return   pos.non_pawn_material(us) == BishopValueMg
		&& pos.count<BISHOP>(us) == 1
		&& pos.count<PAWN  >(us) >= 1;
}

bool is_KQKRPs(const Position& pos, Color us) {
	return  !pos.count<PAWN>(us)
		&& pos.non_pawn_material(us) == QueenValueMg
		&& pos.count<QUEEN>(us) == 1
		&& pos.count<ROOK>(~us) == 1
		&& pos.count<PAWN>(~us) >= 1;
}
*/
/// imbalance() calculates the imbalance by comparing the piece count of each
/// piece type for both colors.
template<bool Us>
_Value_ imbalance(const int pieceCount[][PIECE_TYPE_NB]) {

	const bool Them = (Us == WHITE_SIDE ? BLACK_SIDE : WHITE_SIDE);

	int bonus = 0;

	// Second-degree polynomial material imbalance, by Tord Romstad
	for (int pt1 = 0; pt1 <= 5; ++pt1)
	{
		if (!pieceCount[Us][pt1])
			continue;

		int v = 0;

		for (int pt2 = 0; pt2 <= pt1; ++pt2)
			v += QuadraticOurs[pt1][pt2] * pieceCount[Us][pt2]
			+ QuadraticTheirs[pt1][pt2] * pieceCount[Them][pt2];

		bonus += pieceCount[Us][pt1] * v;
	}

	return bonus;
}

 // namespace


	/// Material::probe() looks up the current position's material configuration in
	/// the material hash table. It returns a pointer to the Entry if the position
	/// is found. Otherwise a new Entry is computed and stored there, so we don't
	/// have to recompute all when the same material configuration occurs again.

_Score_ ChessBoard::value_Imbalance() {

	/*
		Value npm_w = pos.non_pawn_material(WHITE);
		Value npm_b = pos.non_pawn_material(BLACK);
		// Map total non-pawn material into [PHASE_ENDGAME, PHASE_MIDGAME]
		e->gamePhase = Phase(((npm - EndgameLimit) * PHASE_MIDGAME) / (MidgameLimit - EndgameLimit));

		// Let's look if we have a specialized evaluation function for this particular
		// material configuration. Firstly we look for a fixed configuration one, then
		// for a generic one if the previous search failed.
		if ((e->evaluationFunction = pos.this_thread()->endgames.probe<Value>(key)) != nullptr)
			return e;
			*/
	/*
		if (npm_w + npm_b == VALUE_ZERO && pos.pieces(PAWN)) // Only pawns on the board
		{
			if (!pos.count<PAWN>(BLACK))
			{
				assert(pos.count<PAWN>(WHITE) >= 2);

				e->scalingFunction[WHITE] = &ScaleKPsK[WHITE];
			}
			else if (!pos.count<PAWN>(WHITE))
			{
				assert(pos.count<PAWN>(BLACK) >= 2);

				e->scalingFunction[BLACK] = &ScaleKPsK[BLACK];
			}
			else if (pos.count<PAWN>(WHITE) == 1 && pos.count<PAWN>(BLACK) == 1)
			{
				// This is a special case because we set scaling functions
				// for both colors instead of only one.
				e->scalingFunction[WHITE] = &ScaleKPKP[WHITE];
				e->scalingFunction[BLACK] = &ScaleKPKP[BLACK];
			}
		}

		// Zero or just one pawn makes it difficult to win, even with a small material
		// advantage. This catches some trivial draws like KK, KBK and KNK and gives a
		// drawish scale factor for cases such as KRKBP and KmmKm (except for KBBKN).
		if (!pos.count<PAWN>(WHITE) && npm_w - npm_b <= BishopValueMg)
			e->factor[WHITE] = uint8_t(npm_w <  RookValueMg ? SCALE_FACTOR_DRAW :
				npm_b <= BishopValueMg ? 4 : 14);

		if (!pos.count<PAWN>(BLACK) && npm_b - npm_w <= BishopValueMg)
			e->factor[BLACK] = uint8_t(npm_b <  RookValueMg ? SCALE_FACTOR_DRAW :
				npm_w <= BishopValueMg ? 4 : 14);

		if (pos.count<PAWN>(WHITE) == 1 && npm_w - npm_b <= BishopValueMg)
			e->factor[WHITE] = (uint8_t)SCALE_FACTOR_ONEPAWN;

		if (pos.count<PAWN>(BLACK) == 1 && npm_b - npm_w <= BishopValueMg)
			e->factor[BLACK] = (uint8_t)SCALE_FACTOR_ONEPAWN;
			*/
		// Evaluate the material imbalance. We use PIECE_TYPE_NONE as a place holder
		// for the bishop pair "extended piece", which allows us to be more flexible
		// in defining bishop pair bonuses.
	const int PieceCount[_SIDE_COUNT_][PIECE_TYPE_NB] = {
		{ BitCount(BB[WHITE_SIDE][chess_B]) > 1, BitCount(BB[WHITE_SIDE][chess_P]),BitCount(BB[WHITE_SIDE][chess_N]),
		BitCount(BB[WHITE_SIDE][chess_B])    , BitCount(BB[WHITE_SIDE][chess_R]),BitCount(BB[WHITE_SIDE][chess_Q]) },
		{ BitCount(BB[BLACK_SIDE][chess_B]) > 1, BitCount(BB[BLACK_SIDE][chess_P]),BitCount(BB[BLACK_SIDE][chess_N]),
		BitCount(BB[BLACK_SIDE][chess_B])    , BitCount(BB[BLACK_SIDE][chess_R]),BitCount(BB[BLACK_SIDE][chess_Q]) } };

	int value = int16_t((imbalance<WHITE_SIDE>(PieceCount) - imbalance<BLACK_SIDE>(PieceCount)) / 16);
	return make_score(value, value);
	}
