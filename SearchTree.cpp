#include "Board.h"
#include "Search.h"
#include "Evaluation.h"

#define PV		true
#define NonPV	false

extern HashTable Hash_Table;						//哈希表类

extern const _Value_ Contempt;				//藐视因子
static uint16_t StepCount;

StateInfo st_inf[MAX_DEPTH + 1];
extern unsigned long long node_count, hit_count;
Movement debug_record[MAX_DEPTH];					//......................Debug

Movement ChessBoard::IterativeDeepening(const bool &side, const uint16_t &step)
{
	PVLine line;
	_Value_ val, alpha = -VALUE_INF, beta = VALUE_INF, delta;		//delta即期望窗口
	bool FailLow, FailHigh;
	int attempt;

	assert(step < 1024);
	StepCount = step;

	//system("cls");
	node_count = 0;
	for (_Depth_ depth = BASIC_SEARCH_DEPTH; depth <= SEARCH_DEPTH; depth++)
	{
		attempt = 1;
		if (depth == BASIC_SEARCH_DEPTH)
			val = AlphaBeta<PV>(0, depth, side, alpha, beta);
		else
		{
			delta = 50;
			FailLow = FailHigh = false;
			alpha = val - delta;
			beta = val + delta;
			while (true)
			{
				std::cout << "Alpha = " << alpha << "\tBeta = " << beta << std::endl;
				val = AlphaBeta<PV>(0, depth, side, alpha, beta);
				if (val < alpha && !FailHigh)
				{
					alpha = max(val - delta, -VALUE_INF);
					beta  = (alpha + beta) >> 1;
					FailLow = true;
				}
				else if (val > beta && !FailLow)
				{
					alpha = (alpha + beta) >> 1;
					beta = min(val + delta, VALUE_INF);
					FailHigh = true;
				}
				else
					break;

				delta += ((delta >> 2) + 15);
				attempt++;
			}
		}

		std::cout << "depth " << (short)depth << ": " << val << "\tAtt: " << attempt << "\tNode: " << node_count << std::endl;
		line.Get_PVLine(*this, side);
			
		std::cout << std::endl << std::endl;
		if (abs(val) > VALUE_INF - 100)
			break;
	}
	std::cout << std::endl << std::endl;

	return line.PVmovement[0];
}

//函数简介：搜索树核心函数，depth为当前已搜索至第几层，调用时应该指定depth为0；depth_limit为搜索层数限制；alpha与beta为当前节点的alpha与beta值
template<bool PvNode>
_Value_ ChessBoard::AlphaBeta(const _Depth_ &depth, const _Depth_ &depth_limit, const bool &side, _Value_ alpha, _Value_ beta)
{
	_Value_ val;
	Movement tt_Move;

	node_count++;

	if ((val = Hash_Table.Probe_HashTable(get_zobrist(), depth_limit - depth, alpha, beta, tt_Move)) != VALUE_UNKNOWN)
	{
		hit_count++;
		return val;
	}
	if (depth_limit == depth)																//搜索到达叶节点
	{
		val = evaluation(0 == (depth & 1));
		//Hash_Table.Record_HashTable(zobrist_value, 0, val, HASH_EVALUATE | StepCount, NONE);
		return val;
	}
	

	MoveGenerator mv_gen(*this, tt_Move, side, depth);
	Movement new_Move;
	_Value_ current = -VALUE_INF;
	int branch_count = 0;
	bool RootNode = depth == 0;
	uint16_t HASH_FLAG = HASH_ALPHA;


	while ((new_Move = mv_gen.next_move()).Exist())
	{
		assert(CHESS_NONE != chess_at(new_Move.Get_orig_pos()));
		assert(side == side_of(chess_at(new_Move.Get_orig_pos())));

		if (!Legal(new_Move))
			continue;

		branch_count++;
		debug_record[depth] = new_Move;
		MoveChess(new_Move, depth);
		if (chess_count(chess_K) != 1 || chess_count(chess_K | BLACK_CHESS_BIT) != 1)
			assert(false);

		//利用零宽度窗口递归，断言没有节点会优于PV，当零宽度窗口搜索失败时重新进行常规搜索
		val = -AlphaBeta<NonPV>(depth + 1, depth_limit, !side, -alpha - 1, -alpha);

		if (PvNode || val > alpha && depth_limit - depth > 1 && branch_count == 1)
			val = -AlphaBeta<PV>(depth + 1, depth_limit, !side, -beta, -alpha);

		undo_MoveChess(new_Move, depth);

		if (val >= beta)
		{
			Hash_Table.Record_HashTable(get_zobrist(), depth_limit - depth, val, HASH_BETA | StepCount, new_Move);
			return val;
		}
		if (val > current)		
		{
			current = val;
			if (val > alpha)
			{
				alpha = val;
				HASH_FLAG = HASH_PV;
			}
			tt_Move = new_Move;
		}
	}

	if (0 == branch_count)				//没有合法落子
	{
		if (His_attackers_to(posK(side)))
			return -VALUE_INF + depth;
		else
			return Contempt;
	}

	assert(CHESS_NONE != chess_at(tt_Move.Get_orig_pos()));
	Hash_Table.Record_HashTable(get_zobrist(), depth_limit - depth, current, HASH_FLAG | StepCount, tt_Move);

	return current;
}


//函数简介：检索哈希表获取信息
_Value_ HashTable::Probe_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &alpha, const _Value_ &beta, Movement &best_move)
{
	Hash* target = &data[zobrist.low_32 & HASHTABLE_SIZE];

	//Zobrist键值吻合且哈希表中节点的搜索深度不小于当前节点要求的搜索深度
	if (target->Get_zobrist_up() == zobrist.up_32 && target->Get_zobrist_low() == zobrist.low_8[3])
	{
		best_move = target->Get_nextBest();
		if (depth <= target->Get_depth())
		{
			//若target为PV或EVALUATE节点则哈希表命中
			if (target->Get_flag() & (HASH_PV | HASH_EVALUATE))
				return target->Get_evaluation();
			//若target为ALPHA节点但评分小于当前alpha则该节点不需要再考虑，否则节点需要继续搜索
			if (target->Get_flag() & HASH_ALPHA && target->Get_evaluation() <= alpha)
				return target->Get_evaluation();
			//若target为BETA节点但评分值大于当前beta则该节点的父节点所包含的整个分支不需要再考虑，否则节点需要继续搜索
			if (target->Get_flag() & HASH_BETA && target->Get_evaluation() >= beta)
				return target->Get_evaluation();
		}
	}
	return VALUE_UNKNOWN;
}

//函数简介：向哈希表写节点数据
void HashTable::Record_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &val, const uint16_t &flag, const Movement &best_move)
{
	Hash* target = &data[zobrist.low_32 & HASHTABLE_SIZE];

	//不论何时都不允许同一盘面的浅层信息覆盖深层信息
	if (target->Get_zobrist_up() == zobrist.up_32)
	{
		if (target->Get_depth() > depth)
			return;
	}
	//不同Zobrist键值的盘面在同一个入口发生冲突，PV节点拥有4层的优先权
	else if (target->Get_depth() + (target->Get_flag() & HASH_STEP_CT) + ((target->Get_flag() >> 15) << 2) > depth + (flag & HASH_STEP_CT) + ((flag >> 15) << 2))
		return;	

	target->Set_zobrist(zobrist);
	target->Set_Flag(flag);
	target->Set_evaluation(val);
	target->Set_depth(depth);
	target->Set_next_Best(best_move);
}

//函数简介：根据哈希表递归检索获取当前盘面的主要变例，直到检测到的哈希表项不存在next_Best信息则检索到达叶节点
void PVLine::Get_PVLine(ChessBoard Board, bool side)
{
	extern MyApp theApp;
	ChessBoard temp = Board;
	Hash* target = Hash_Table.Get_HashNode(temp.get_zobrist().low_32);

	PVnodes_count = 0;
	while (target->Exist_nextBest() && temp.get_zobrist().up_32 == target->Get_zobrist_up() && temp.get_zobrist().low_8[3] == target->Get_zobrist_low() && PVnodes_count < MAX_DEPTH)
	{
		target->Get_nextBest().Print(temp);
		PVmovement[PVnodes_count++] = target->Get_nextBest();
		temp.MoveChess(target->Get_nextBest());
		side = !side;
		target = Hash_Table.Get_HashNode(temp.get_zobrist().low_32);
	}
}