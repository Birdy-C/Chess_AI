#include "Chess.h"
#include "Board.h"
#include "Hash.h"

//用于快速32位随机数生成
static unsigned long x, y, z, w;

//四个维度分别为：棋子颜色、棋子类型、横坐标计数、纵坐标计数
_Zobrist64_ zobrist[PIECE_NUM][SQ_COUNT];
//用于记录颜色flag、过路兵flag、王车易位flag、50步和棋计步
_Zobrist64_ side_zobrist, En_passent_zobrist[_COORDINATE_COUNT_ + 1], castling_zobrist[_SIDE_COUNT_][3];
//哈希表
HashTable Hash_Table(HASHTABLE_SIZE);

//函数简介：正常情况下仅在程序运行开始时执行一次，用于初始化ZobristValue的值；在调试或统计时可重复调用
void MyApp::Zobrist_Init()
{
	int pattern, side, pos;

	random_device rd;
	mt19937_64 gen(rd());		//马特赛特旋转64位随机数发生器

	uniform_int_distribution<_Zobrist64_> dis;

	Hash_Table.Clear();			//清空哈希表

	for (side = 0; side < _SIDE_COUNT_; side++)
		for (pattern = 0; pattern < _PATTERN_COUNT_; pattern++)
			for (pos = 0; pos < SQ_COUNT; pos++)
				zobrist[side * BLACK_CHESS_BIT + pattern][pos] = dis(gen);

	side_zobrist = dis(gen);

	for (x = 0; x <= _COORDINATE_COUNT_; x++)
		En_passent_zobrist[x] = dis(gen);

	castling_zobrist[WHITE_SIDE][LONG_CASTLING] = dis(gen);
	castling_zobrist[WHITE_SIDE][SHORT_CASTLING] = dis(gen);
	castling_zobrist[BLACK_SIDE][LONG_CASTLING] = dis(gen);
	castling_zobrist[BLACK_SIDE][SHORT_CASTLING] = dis(gen);

	//用于快速随机数发生
	x = (unsigned long)(dis(gen) >> 32);
	y = (unsigned long)(dis(gen) >> 32);
	z = (unsigned long)(dis(gen) >> 32);
	w = (unsigned long)(dis(gen) >> 32);
}

//函数简介：重新计算当前盘面下的Zobrist键值
void ChessBoard::Calculate_BoardHash()
{
	extern MyApp theApp;
	_ChessType_ ptype;
	
	flag.zobrist_value.zobrist64 = 0;

	for (int x = 0; x < _COORDINATE_COUNT_; x++)
	{
		for (int y = 0; y < _COORDINATE_COUNT_; y++)
		{
			ptype = chess_at((x << 3) | y);
			if (ptype != NONE)
				update_zobrist(zobrist[side_of(ptype) * BLACK_CHESS_BIT + pattern_of(ptype)][x << 3 | y]);
		}
	}
	if (theApp.side)
		update_zobrist(side_zobrist);
	update_zobrist(En_passent_zobrist[En_passant()]);
	if (flag.Castling[WHITE_SIDE] | flag.Castling[BLACK_SIDE])
	{
		if (flag.Castling[WHITE_SIDE] & LONG_CASTLING)
			update_zobrist(castling_zobrist[WHITE_SIDE][LONG_CASTLING]);
		if (flag.Castling[WHITE_SIDE] & SHORT_CASTLING)
			update_zobrist(castling_zobrist[WHITE_SIDE][SHORT_CASTLING]);
		if (flag.Castling[BLACK_SIDE] & LONG_CASTLING)
			update_zobrist(castling_zobrist[BLACK_SIDE][LONG_CASTLING]);
		if (flag.Castling[BLACK_SIDE] & SHORT_CASTLING)
			update_zobrist(castling_zobrist[BLACK_SIDE][SHORT_CASTLING]);
	}
}

//函数简介：快速32位随机数发生器
unsigned long MyApp::rand32() 
{
	unsigned long t;

	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ (t ^ (t >> 8));

	return w;
}