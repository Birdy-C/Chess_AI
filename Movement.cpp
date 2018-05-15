#include "Movement.h"
#include "Board.h"
#include "AboutMove.h"
#include "Search.h"


MoveGenerator::MoveGenerator(ChessBoard &Bd, const Movement tt_move, const bool sd, const _Depth_ dpt, const bool qsearch)
	: Board(Bd), side(sd), depth(dpt), tt_Move(tt_move)
{
	end = cur = bad_cap = st_inf[dpt].mvtb;

	if (qsearch)
		state = QSEARCH_INIT;
	else
	{
		switch (Board.DeltaCheck(side, check_pos, check_tp))
		{
		case 0:
			state = MAIN_MOVE; break;
		case 1:
			state = RID_CHECK_INIT; break;
		case 2:
			state = RID_DOU_CHECK_INIT; break;
		default:
			assert(false);
		}
	}
}


//函数简介：寻找出目标队列中评分最高的棋步，将它放至队首并返回
ExtMove pick_best(ExtMove* begin, ExtMove* end)
{
	std::swap(*begin, *std::max_element(begin, end));
	return *begin;
}

//函数简介：根据state的不同返回下一着法
Movement MoveGenerator::next_move()
{
	ExtMove new_Move;

	switch (state)
	{
	case MAIN_MOVE:
		++state;
		if (tt_Move.Exist())
			return tt_Move;

	case PROMOTION_INIT:
		if (Board.get_bb(side, chess_P) & life_line(side))
			end = expand<GEN_PROMOTION>(cur, Board);
		++state;

	case PROMOTION:
		while (cur < end)
		{
			new_Move = *cur++;
			if (new_Move.move != tt_Move)
				return new_Move.move;
		}
		++state;

	case CAPTURE_INIT:
		end = expand<GEN_CAPTURE>(cur, Board);
		score_cap();
		if (CAPTURE_INIT == state)
			++state;
		else
		{
			state = QSEARCH;
			if (tt_Move.Exist())
				return tt_Move;
		}		

	case GOOD_CAPTURE:
        while (cur < end)
		{
			new_Move = pick_best(cur++, end);
			if (new_Move.move != tt_Move)
			{
				if (Board.see_ge(new_Move.move, 0))
					return new_Move.move;

				*bad_cap++ = new_Move;
			}
		}
		++state;

	case QUIET_INIT:
		cur = bad_cap;
    		end = expand<GEN_QUIET>(cur, Board);
		++state;

	case QUIETS:
		while (cur < end)
		{       
			new_Move = *cur++;
			if (new_Move.move != tt_Move)
				return new_Move.move;
		}
		cur = st_inf[depth].mvtb;
		++state;

	case BAD_CAPTURE:
		if (cur < bad_cap)
			return (*cur++).move;
		break;

	case RID_CHECK_INIT:
		end = Search_RidCheck(cur);
		++state;
		if (tt_Move.Exist())
			return tt_Move;

	case RID_CHECK:
		while (cur < end)
		{
			new_Move = *cur++;
			if (new_Move.move != tt_Move)
				return new_Move.move;
		}
		++state;

	case RID_DOU_CHECK_INIT:
		end = Search_RidDoubleCheck(cur);
		++state;
		if (tt_Move.Exist())
			return tt_Move;

	case RID_DOU_CHECK:
		while (cur < end)
		{
			new_Move = *cur++;
			if (new_Move.move != tt_Move)
				return new_Move.move;
		}
		break;

	case QSEARCH_INIT:
		end = expand<GEN_CAPTURE>(cur, Board);
		score_cap();
		++state;
		if (tt_Move.Exist())
			return tt_Move;

	case QSEARCH:
		while (cur < end)
		{
			new_Move = *cur++;
			if (Board.see_ge(new_Move.move, 0) && new_Move.move != tt_Move)
				return new_Move.move;
		}
		break;

	default:
		assert(false);
	}

	Movement empty_mv;
	return empty_mv;
}


//函数简介：从落子后己方是否被将军来判断棋步的合法性，rid_check的棋步在生成时就进行合法性判断，不应调用该函数
bool ChessBoard::Legal(const Movement &mv) const
{
	_Pos_ orig = mv.Get_orig_pos(), dest = mv.Get_dest_pos();

	if (MOVE_ENPASS == mv.Get_move_type())
	{
		ChessBoard temp = *this;
		temp.Move(orig, dest);
		temp.Eaten(forward(dest, !side));
		return 0 == temp.His_attackers_to(temp.posK(side));
	}
	else if (chess_K == (chess_at(orig) & CHESS_PATTERN))
		return (MOVE_CAST == mv.Get_move_type()) || !His_attackers_to(dest);
	else
		return !(pinners(side) & mask(orig)) || At_same_line(orig, dest, posK(side));
}