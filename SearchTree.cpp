#include "Board.h"
#include "Search.h"
#include "Evaluation.h"

#define PV		true
#define NonPV	false

extern HashTable Hash_Table;						//��ϣ����

extern const _Value_ Contempt;				//��������
static uint16_t StepCount;

StateInfo st_inf[MAX_DEPTH + 1];
extern unsigned long long node_count, hit_count;
Movement debug_record[MAX_DEPTH];					//......................Debug

Movement ChessBoard::IterativeDeepening(const bool &side, const uint16_t &step)
{
	PVLine line;
	_Value_ val, alpha = -VALUE_INF, beta = VALUE_INF, delta;		//delta����������
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

//������飺���������ĺ�����depthΪ��ǰ���������ڼ��㣬����ʱӦ��ָ��depthΪ0��depth_limitΪ�����������ƣ�alpha��betaΪ��ǰ�ڵ��alpha��betaֵ
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
	if (depth_limit == depth)																//��������Ҷ�ڵ�
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

		//�������ȴ��ڵݹ飬����û�нڵ������PV�������ȴ�������ʧ��ʱ���½��г�������
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

	if (0 == branch_count)				//û�кϷ�����
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


//������飺������ϣ���ȡ��Ϣ
_Value_ HashTable::Probe_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &alpha, const _Value_ &beta, Movement &best_move)
{
	Hash* target = &data[zobrist.low_32 & HASHTABLE_SIZE];

	//Zobrist��ֵ�Ǻ��ҹ�ϣ���нڵ��������Ȳ�С�ڵ�ǰ�ڵ�Ҫ����������
	if (target->Get_zobrist_up() == zobrist.up_32 && target->Get_zobrist_low() == zobrist.low_8[3])
	{
		best_move = target->Get_nextBest();
		if (depth <= target->Get_depth())
		{
			//��targetΪPV��EVALUATE�ڵ����ϣ������
			if (target->Get_flag() & (HASH_PV | HASH_EVALUATE))
				return target->Get_evaluation();
			//��targetΪALPHA�ڵ㵫����С�ڵ�ǰalpha��ýڵ㲻��Ҫ�ٿ��ǣ�����ڵ���Ҫ��������
			if (target->Get_flag() & HASH_ALPHA && target->Get_evaluation() <= alpha)
				return target->Get_evaluation();
			//��targetΪBETA�ڵ㵫����ֵ���ڵ�ǰbeta��ýڵ�ĸ��ڵ���������������֧����Ҫ�ٿ��ǣ�����ڵ���Ҫ��������
			if (target->Get_flag() & HASH_BETA && target->Get_evaluation() >= beta)
				return target->Get_evaluation();
		}
	}
	return VALUE_UNKNOWN;
}

//������飺���ϣ��д�ڵ�����
void HashTable::Record_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &val, const uint16_t &flag, const Movement &best_move)
{
	Hash* target = &data[zobrist.low_32 & HASHTABLE_SIZE];

	//���ۺ�ʱ��������ͬһ�����ǳ����Ϣ���������Ϣ
	if (target->Get_zobrist_up() == zobrist.up_32)
	{
		if (target->Get_depth() > depth)
			return;
	}
	//��ͬZobrist��ֵ��������ͬһ����ڷ�����ͻ��PV�ڵ�ӵ��4�������Ȩ
	else if (target->Get_depth() + (target->Get_flag() & HASH_STEP_CT) + ((target->Get_flag() >> 15) << 2) > depth + (flag & HASH_STEP_CT) + ((flag >> 15) << 2))
		return;	

	target->Set_zobrist(zobrist);
	target->Set_Flag(flag);
	target->Set_evaluation(val);
	target->Set_depth(depth);
	target->Set_next_Best(best_move);
}

//������飺���ݹ�ϣ��ݹ������ȡ��ǰ�������Ҫ������ֱ����⵽�Ĺ�ϣ�������next_Best��Ϣ���������Ҷ�ڵ�
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