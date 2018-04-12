#include "Chess.h"
#include "Board.h"
#include "Hash.h"

//���ڿ���32λ���������
static unsigned long x, y, z, w;

//�ĸ�ά�ȷֱ�Ϊ��������ɫ���������͡���������������������
_Zobrist64_ zobrist[PIECE_NUM][SQ_COUNT];
//���ڼ�¼��ɫflag����·��flag��������λflag��50������Ʋ�
_Zobrist64_ side_zobrist, En_passent_zobrist[_COORDINATE_COUNT_ + 1], castling_zobrist[_SIDE_COUNT_][3];
//��ϣ��
HashTable Hash_Table(HASHTABLE_SIZE);

//������飺��������½��ڳ������п�ʼʱִ��һ�Σ����ڳ�ʼ��ZobristValue��ֵ���ڵ��Ի�ͳ��ʱ���ظ�����
void MyApp::Zobrist_Init()
{
	int pattern, side, pos;

	random_device rd;
	mt19937_64 gen(rd());		//����������ת64λ�����������

	uniform_int_distribution<_Zobrist64_> dis;

	Hash_Table.Clear();			//��չ�ϣ��

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

	//���ڿ������������
	x = (unsigned long)(dis(gen) >> 32);
	y = (unsigned long)(dis(gen) >> 32);
	z = (unsigned long)(dis(gen) >> 32);
	w = (unsigned long)(dis(gen) >> 32);
}

//������飺���¼��㵱ǰ�����µ�Zobrist��ֵ
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

//������飺����32λ�����������
unsigned long MyApp::rand32() 
{
	unsigned long t;

	t = x ^ (x << 11);
	x = y; y = z; z = w;
	w = w ^ (w >> 19) ^ (t ^ (t >> 8));

	return w;
}