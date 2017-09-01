#include"Chess.h"

extern _Bit64_ mask[64];
static const _Pos_ r45L_map[64] = { 28, 21, 15, 10, 6, 3, 1, 0, 36, 29, 22, 16, 11, 7, 4, 2, 43, 37, 30, 23, 17, 12, 8, 5, 49, 44, 38, 31, 24, 18, 13, 9, 54, 50, 45, 39, 32, 25, 19, 14, 58, 55, 51, 46, 40, 33, 26, 20, 61, 59, 56, 52, 47, 41, 34, 27, 63, 62, 60, 57, 53, 48, 42, 35 };
static const _Pos_ r45R_map[64] = { 0, 1, 3, 6, 10, 15, 21, 28, 2, 4, 7, 11, 16, 22, 29, 36, 5, 8, 12, 17, 23, 30, 37, 43, 9, 13, 18, 24, 31, 38, 44, 49, 14, 19, 25, 32, 39, 45, 50, 54, 20, 26, 33, 40, 46, 51, 55, 58, 27, 34, 41, 47, 52, 56, 59, 61, 35, 42, 48, 53, 57, 60, 62, 63 };
static const _Pos_ trans_map[64] = { 0, 8, 16, 24, 32, 40, 48, 56, 1, 9, 17, 25, 33, 41, 49, 57, 2, 10, 18, 26, 34, 42, 50, 58, 3, 11, 19, 27, 35, 43, 51, 59, 4, 12, 20, 28, 36, 44, 52, 60, 5, 13, 21, 29, 37, 45, 53, 61, 6, 14, 22, 30, 38, 46, 54, 62, 7, 15, 23, 31, 39, 47, 55, 63 };

void ChessBoard::Reset()
{
	White[chess_P].Init((_Bit64_)0x00FF000000000000);
	White[chess_N].Init((_Bit64_)0x4200000000000000);
	White[chess_B].Init((_Bit64_)0x2400000000000000);
	White[chess_R].Init((_Bit64_)0x8100000000000000);
	White[chess_Q].Init((_Bit64_)0x0800000000000000);
	White[chess_K].Init((_Bit64_)0x1000000000000000);
	White_All.Init((_Bit64_)0xFFFF000000000000);
	White_All_trans.Init((_Bit64_)0xC0C0C0C0C0C0C0C0);
	White_All_L45.Init((_Bit64_)0xFB31860C08000000);
	White_All_R45.Init((_Bit64_)0xFB31860C08000000);

	Black[chess_P].Init((_Bit64_)0x000000000000FF00);
	Black[chess_N].Init((_Bit64_)0x0000000000000042);
	Black[chess_B].Init((_Bit64_)0x0000000000000024);
	Black[chess_R].Init((_Bit64_)0x0000000000000081);
	Black[chess_Q].Init((_Bit64_)0x0000000000000008);
	Black[chess_K].Init((_Bit64_)0x0000000000000010);
	Black_All.Init((_Bit64_)0x000000000000FFFF);
	Black_All_trans.Init((_Bit64_)0x0303030303030303);
	Black_All_L45.Init((_Bit64_)0x0000001030618CDF);
	Black_All_R45.Init((_Bit64_)0x0000001030618CDF);

	En_passant = NONE; 
	Castling[WHITE_SIDE] = Castling[BLACK_SIDE] = LONG_CASTLING | SHORT_CASTLING;
	King_pos[WHITE_SIDE] = 60;
	King_pos[BLACK_SIDE] = 4;
	Check = false;
}

//返回是什么类型的棋子
_ChessType_ ChessBoard::DeltaChess(const _Coordinate_ &x, const _Coordinate_ &y)
{
	_Bit64_ p = (_Bit64_)1 << ((x << 3) + y);
	
	if (!(p & White_All.GetData() || p & Black_All.GetData()))
		return NONE;
	if (p & White_All.GetData())
	{
		for (int i = 0; i < 6; i++)
			if (p & White[i].GetData())
				return i;
	}
	else
	{
		for (int i = 0; i < 6; i++)
			if (p & Black[i].GetData())
				return i | BLACK_CHESS_BIT;
	}
	return NONE;
}

_ChessType_ ChessBoard::DeltaChess(const _Bit64_ p)
{
	
	if (!(p & White_All.GetData() || p & Black_All.GetData()))
		return NONE;
	if (p & White_All.GetData())
	{
		for (int i = 0; i < 6; i++)
			if (p & White[i].GetData())
				return i;
	}
	else
	{
		for (int i = 0; i < 6; i++)
			if (p & Black[i].GetData())
				return i | BLACK_CHESS_BIT;
	}
	return NONE;
}


void ChessBoard::Move(const _Pos_ &orig, const _Pos_ &dest, const _ChessPattern_ &chesstype, const bool &side)
{
	if (WHITE_SIDE == side)
	{
		White[chesstype].Move(orig, dest);
		White_All.Move(orig, dest);
		White_All_trans.Move(trans_map[orig], trans_map[dest]);
		White_All_L45.Move(r45L_map[orig], r45L_map[dest]);
		White_All_R45.Move(r45R_map[orig], r45R_map[dest]);
	}
	else
	{
		Black[chesstype].Move(orig, dest);
		Black_All.Move(orig, dest);
		Black_All_trans.Move(trans_map[orig], trans_map[dest]);
		Black_All_L45.Move(r45L_map[orig], r45L_map[dest]);
		Black_All_R45.Move(r45R_map[orig], r45R_map[dest]);
	}
	if (chesstype == chess_K)
		King_pos[side] = dest;
}

void ChessBoard::Eaten(const _Pos_ &pos, const _ChessPattern_ &chesstype, const bool &side)
{
	if (WHITE_SIDE == side)
	{
		White[chesstype].Eaten(pos);
		White_All.Eaten(pos);
		White_All_trans.Eaten(trans_map[pos]);
		White_All_L45.Eaten(r45L_map[pos]);
		White_All_R45.Eaten(r45R_map[pos]);
	}
	else
	{
		Black[chesstype].Eaten(pos);
		Black_All.Eaten(pos);
		Black_All_trans.Eaten(trans_map[pos]);
		Black_All_L45.Eaten(r45L_map[pos]);
		Black_All_R45.Eaten(r45R_map[pos]);
	}
}

_Pos_ ChessBoard::Trans_Bit64toPos(const _Bit64_ &bit)
{
	_Pos_ p = 0;
	unsigned long bit_temp;

	__asm
	{
		mov         esi, dword ptr[bit]
		mov         eax, dword ptr[esi+4]
		test        eax, eax
		je          load_low
		mov         p, 20h
		jmp         mov_bt
		load_low:
		mov			eax, dword ptr[esi]
		mov_bt:
		mov			dword ptr[bit_temp], eax
	}
	/*
	if (bit >> 32)
		p |= 32, bit_temp = bit >> 32;
	else
		bit_temp = bit & 0xFFFFFFFF;*/

	if (bit_temp & 0xFFFF0000)
		p |= 16, bit_temp >>= 16;
	if (bit_temp & 0x0000FF00)
		p |= 8, bit_temp >>= 8;
	while (bit_temp >>= 1)
		p++;

	return p;
}

void BitBoard::Move(const _Pos_ &orig_pos64, const _Pos_ &dest_pos64)
{
	Board ^= mask[orig_pos64] | mask[dest_pos64];
}

void BitBoard::Eaten(const _Pos_ &pos64)
{
	Board ^= mask[pos64];
}
