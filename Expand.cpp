#include "Board.h"
#include "Movement.h"
#include "AboutMove.h"
#include <assert.h>

//函数介绍：用于返回pos处类型为chess的棋子的伪合法着步（不检测将军）
BitBoard ChessBoard::movement_step(const _Pos_ &pos, bool TEST_CHECK)
{
	_ChessPattern_ pt = chess_at(pos) & CHESS_PATTERN;
	BitBoard final_position = 0;					//for return 

	switch (pt)
	{
	case chess_P:
	{
		if (WHITE_SIDE == side)
		{
			final_position = (~(BB[WHITE_SIDE][chess_All] | BB[BLACK_SIDE][chess_All])) & mask(pos - 8);
			if (pos >= 48 && final_position)
				final_position |= (~BB[BLACK_SIDE][chess_All]) & mask(pos - 16);
			final_position |= (eat_Pawn[WHITE_SIDE][pos] & BB[BLACK_SIDE][chess_All]);
			//过路兵判定
			if (En_passant() != ENPASS_NONE && rank_of(pos) == 3)
			{
				if (file_of(pos) == En_passant() - 1)
					final_position |= mask(pos - 7);
				else if (file_of(pos) == En_passant() + 1)
					final_position |= mask(pos - 9);
			}
		}
		else
		{
			final_position = (~(BB[WHITE_SIDE][chess_All] | BB[BLACK_SIDE][chess_All])) & mask(pos + 8);
			if (!(pos >> 4) && final_position)
				final_position |= (~BB[WHITE_SIDE][chess_All]) & mask(pos + 16);
			final_position |= (eat_Pawn[BLACK_SIDE][pos] & BB[WHITE_SIDE][chess_All]);
			//过路兵判定
			if (En_passant() != ENPASS_NONE && rank_of(pos) == 4)
			{
				if (file_of(pos) == En_passant() - 1)
					final_position |= mask(pos + 9);
				else if (file_of(pos) == En_passant() + 1)
					final_position |= mask(pos + 7);
			}
		}
		break;
	}
	case chess_N:
		final_position = (this->*attack_area[chess_N])(pos);
		break;
	case chess_Q:
		//利用switch语句的特点计算皇后的合法落子区
	case chess_R:
		final_position = (this->*attack_area[chess_R])(pos);
		if (chess_R == pt)
			break;
	case chess_B:
		final_position |= (this->*attack_area[chess_B])(pos);
		break;
	case chess_K:
		final_position = move_King[pos];
		_Pos_ King_pos = List[(side * BLACK_CHESS_BIT) | chess_K][0];

		//首先判断易位FLAG以节省开销		
		if (cast_right(side))
		{
			if (WHITE_SIDE == side)
			{
				//取出56~60位与10001比较并判将
				if ((cast_right(WHITE_SIDE) & LONG_CASTLING) && ((All >> 56) & 31) == 17)
					if (!TEST_CHECK || (!His_attackers_to(King_pos) && !His_attackers_to(King_pos - 1) && !His_attackers_to(King_pos - 2)))
						final_position |= mask(58);
				//取出60~63位与1001比较并判将
				if ((cast_right(WHITE_SIDE) & SHORT_CASTLING) && ((All >> 60) & 15) == 9)
					if (!TEST_CHECK || (!His_attackers_to(King_pos) && !His_attackers_to(King_pos + 1) && !His_attackers_to(King_pos + 2)))
						final_position |= mask(62);
			}
			else
			{
				if ((cast_right(BLACK_SIDE) & LONG_CASTLING) && (All & 31) == 17)
					if (!TEST_CHECK || (!His_attackers_to(King_pos) && !His_attackers_to(King_pos - 1) && !His_attackers_to(King_pos - 2)))
						final_position |= mask(2);
				if ((cast_right(BLACK_SIDE) & SHORT_CASTLING) && ((All >> 4) & 15) == 9)
					if (!TEST_CHECK || (!His_attackers_to(King_pos) && !His_attackers_to(King_pos + 1) && !His_attackers_to(King_pos + 2)))
						final_position |= mask(6);
			}
		}
		break;
	}
	//去除掉被己方棋子占领的区域
	final_position &= ~BB[side][chess_All];

	//检测将军，只在GUI调用中执行

	if (TEST_CHECK)
	{
		BitBoard total_bit = final_position;
		ChessBoard temp;
		Movement attemp_move;

		while (total_bit)
		{
			_Pos_ dest = pop_lsb(total_bit);
			temp = *this;
			attemp_move = make_move(pos, dest, temp.Delta_MoveType(pos, dest));
			temp.MoveChess(attemp_move);
			if (temp.My_attackers_to(temp.posK(side)))
				final_position ^= mask(dest);
		}
	}
	return final_position;
}

//函数简介：生成升变的若干落子方法，传入的p为dest_pos
ExtMove* make_promotion(ExtMove *cur, const _Pos_ &p, int delta_pos)
{
	*cur++ = make_move(p - delta_pos, p, MOVE_PROMOTION | ((chess_Q - 1) << 12));
	*cur++ = make_move(p - delta_pos, p, MOVE_PROMOTION | ((chess_N - 1) << 12));
	*cur++ = make_move(p - delta_pos, p, MOVE_PROMOTION | ((chess_R - 1) << 12));
	*cur++ = make_move(p - delta_pos, p, MOVE_PROMOTION | ((chess_B - 1) << 12));

	return cur;
}


//函数简介：为吃子着法进行简易评分从而优化排序
void MoveGenerator::score_cap()
{
	extern const _Score_ Piece_Value[_PATTERN_COUNT_];

	for (ExtMove* m = cur; m < end; m++)
		m->scr = (Piece_Value[Board.chess_at(m->move.Get_dest_pos())] & 0xFFFF) - 
			200 * relative_rank_of(Board.side_toMove(), m->move.Get_dest_pos());
}