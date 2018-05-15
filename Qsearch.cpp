#include "Evaluation.h"
#include "BitBoard.h"

extern HashTable Hash_Table;

//==========================================
// min_attacker() is a helper function used by see_ge() to locate the least
// valuable attacker for the side to move, remove the attacker we just found
// from the bitboards and scand for new X-ray attacks behind it.

_ChessPattern_ ChessBoard::min_attacker(_ChessPattern_ Pt, const BitBoard* bb, _Pos_ to, BitBoard stmAttackers, _Pos_& from, BitBoard& attackers) 
{
	if (chess_K == Pt)
		return chess_K;

	BitBoard b = stmAttackers & bb[Pt];
	if (!b)
		return min_attacker(Pt + 1, bb, to, stmAttackers, from, attackers);

	// 移掉这个棋子
	
	from = Lsb64(b);
	BitmapUpdate(from);

	// 更新attackers
	if (Pt == chess_P || Pt == chess_B || Pt == chess_Q)
		attackers |= _attack_b(to) & (bb[chess_B] | bb[chess_Q]);

	if (Pt == chess_R || Pt == chess_Q)
		attackers |= _attack_r(to) & (bb[chess_R] | bb[chess_Q]);

	attackers &= get_bb_all(); // After X-ray that may add already processed pieces
	return Pt;
}



static const _Value_ PieceValueMG[8] = { 171, 764, 826, 1282, 2513, 20000, 0, 0 };

/// Position::see_ge (Static Exchange Evaluation Greater or Equal) tests if the
/// SEE value of move is greater or equal to the given threshold. We'll use an
/// algorithm similar to alpha-beta pruning with a null window.
bool ChessBoard::see_ge(const Movement m, _Value_ threshold)
{
	bool Us = side_toMove();

	// Only deal with normal moves, assume others pass a simple see
	if (m.Get_move_type() != MOVE_NORMAL || CHESS_NONE == m.Get_dest_pos())
		return 0 >= threshold;

	_Pos_ from = m.Get_orig_pos(), to = m.Get_dest_pos();

	//PieceType nextVictim = type_of(piece_on(from));
	_ChessPattern_ nextVictim = chess_at(from) & CHESS_PATTERN;

	bool stm = !Us;
	_Value_ balance;			// Values of the pieces taken by us minus opponent's ones
	BitBoard stmAttackers;

	// The opponent may be able to recapture so this is the best result
	// we can hope for.
	//balance = PieceValue[MG][piece_on(to)] - threshold;
	balance = PieceValueMG[chess_at(to) & CHESS_PATTERN] - threshold;

	if (balance < 0)
		return false;

	// Now assume the worst possible result: that the opponent can
	// capture our piece for free.
	balance -= PieceValueMG[nextVictim];

	if (balance >= 0) // Always true if nextVictim == KING
		return true;

	bool opponentToMove = true;

	static _Pos_ updated[16];
	size_t index = 0;

	updated[index++] = from;
	BitmapUpdate(from);

	// Find all attackers to the destination square, with the moving piece removed,
	// but possibly an X-ray attacker added behind it.
	//Bitboard attackers = attackers_to(to, occupied) & occupied;
	BitBoard attackers = (My_attackers_to(to) | His_attackers_to(to));

	while (true)
	{
		// The balance is negative only because we assumed we could win
		// the last piece for free. We are truly winning only if we can
		// win the last piece _cheaply enough_. Test if we can actually
		// do this otherwise "give up".
		assert(balance < 0);

		stmAttackers = attackers & get_bb(stm, chess_All) & ~pinners(stm);

		// Don't allow pinned pieces to attack pieces except the king as long all
		// pinners are on their original square.
		
		stmAttackers |= pinners(stm) & Between_bit[posK(stm)][to];

		// If we have no more attackers we must give up
		if (!stmAttackers)
			break;

		// Locate and remove the next least valuable attacker
		_Pos_ target = SQ_NONE;
		nextVictim = min_attacker(chess_P, BB[stm], to, stmAttackers, target, attackers);
		if (SQ_NONE != target)
			updated[index++] = target;

		if (nextVictim == chess_K)
		{
			// Our only attacker is the king. If the opponent still has
			// attackers we must give up. Otherwise we make the move and
			// (having no more attackers) the opponent must give up.
			if (!(attackers & get_bb(!stm, chess_All)))
				opponentToMove = !opponentToMove;
			break;
		}

		// Assume the opponent can win the next piece for free and switch sides
		balance += PieceValueMG[nextVictim];
		opponentToMove = !opponentToMove;

		// If balance is negative after receiving a free piece then give up
		if (balance < 0)
			break;

		// Complete the process of switching sides. The first line swaps
		// all negative numbers with non-negative numbers. The compiler
		// probably knows that it is just the bitwise negation ~balance.
		balance = -balance - 1;
		stm = !stm;
	}

	for (size_t i = 0; i < index; ++i)
		BitmapUpdate(updated[i]);

	// If the opponent gave up we win, otherwise we lose.
	return opponentToMove;
}

_Value_ ChessBoard::QSearch(_Value_ alpha, _Value_ beta, Movement ttMove, _Depth_ d)
{
	MoveGenerator mv_gen(*this, ttMove, side, d, true);
	Movement new_Move;
	_Value_ val, current = -VALUE_INF;
	int branch_count = 0;
	extern Movement debug_record[MAX_DEPTH];

	//if ((val = Hash_Table.Probe_HashTable(get_zobrist(), 0, alpha, beta, ttMove)) != VALUE_UNKNOWN)
		//return val;

	while ((new_Move = mv_gen.next_move()).Exist())
	{
		if (!Legal(new_Move))
			continue;

		++branch_count;
		debug_record[d] = new_Move;
		MoveChess(new_Move, d);

		ChessBoard temp = *this;
		val = -QSearch(-beta, -alpha, ttMove, d + 1);
		if (!(temp == *this))
		{
			*this = temp;
			assert(false);
		}

		undo_MoveChess(new_Move, d);

		if (val > beta)
			return val;

		if (val > current)
		{
			current = val;
			if (val > alpha)
				alpha = val;
		}
	}

	return branch_count ? current : evaluation(side, false);
}