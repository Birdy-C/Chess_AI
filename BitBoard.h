#pragma once

#include <iostream>
#include <Windows.h>
#include <ctime>
#include <assert.h>
#include <stdint.h>

#define _COORDINATE_COUNT_  8
#define ENPASS_NONE			8

typedef unsigned long long	BitBoard;
typedef unsigned char		_Pos_;
typedef unsigned char		_Coordinate_;

typedef unsigned char		_ChessPattern_;
typedef unsigned char		_ChessType_;

enum
{
	WHITE_SIDE = 0, BLACK_SIDE = 1, _SIDE_COUNT_ = 2
};

enum
{
	chess_P = 0, chess_N = 1, chess_B = 2, chess_R = 3, chess_Q = 4, chess_K = 5, chess_All = 6,
	_PATTERN_COUNT_ = 6, _BB_COUNT_ = 8
};

enum
{
	WHITE_P =  0, WHITE_N =  1, WHITE_B =  2, WHITE_R =  3, WHITE_Q =  4, WHITE_K =  5, WHITE_All =  6,
	BLACK_P =  8, BLACK_N =  9, BLACK_B = 10, BLACK_R = 11, BLACK_Q = 12, BLACK_K = 13, BLACK_All = 14,
	BLACK_CHESS_BIT = 8,		CHESS_PATTERN = 7,
	PIECE_NUM		= 16,
	CHESS_NONE		= 255
};

enum
{
	SQ_A8 =  0, SQ_B8 =  1, SQ_C8 =  2, SQ_D8 =  3, SQ_E8 =  4, SQ_F8 =  5, SQ_G8 =  6, SQ_H8 =  7,
	SQ_A7 =  8, SQ_B7 =  9, SQ_C7 = 10, SQ_D7 = 11, SQ_E7 = 12, SQ_F7 = 13, SQ_G7 = 14, SQ_H7 = 15,
	SQ_A6 = 16, SQ_B6 = 17, SQ_C6 = 18, SQ_D6 = 19, SQ_E6 = 20, SQ_F6 = 21, SQ_G6 = 22, SQ_H6 = 23,
	SQ_A5 = 24, SQ_B5 = 25, SQ_C5 = 26, SQ_D5 = 27, SQ_E5 = 28, SQ_F5 = 29, SQ_G5 = 30, SQ_H5 = 31,
	SQ_A4 = 32, SQ_B4 = 33, SQ_C4 = 34, SQ_D4 = 35, SQ_E4 = 36, SQ_F4 = 37, SQ_G4 = 38, SQ_H4 = 39,
	SQ_A3 = 40, SQ_B3 = 41, SQ_C3 = 42, SQ_D3 = 43, SQ_E3 = 44, SQ_F3 = 45, SQ_G3 = 46, SQ_H3 = 47,
	SQ_A2 = 48, SQ_B2 = 49, SQ_C2 = 50, SQ_D2 = 51, SQ_E2 = 52, SQ_F2 = 53, SQ_G2 = 54, SQ_H2 = 55,
	SQ_A1 = 56, SQ_B1 = 57, SQ_C1 = 58, SQ_D1 = 59, SQ_E1 = 60, SQ_F1 = 61, SQ_G1 = 62, SQ_H1 = 63,
	SQ_COUNT = 64,
	SQ_NONE  = 64
};


inline uint64_t mask(const _Pos_ &p)
{
	return 1ULL << p;
}

inline void Clear(BitBoard &Board)
{
	Board = 0;
}

inline void Update(BitBoard &Board, const _Pos_ &pos64)
{
	Board ^= mask(pos64);
}

inline void Update(BitBoard &Board, const _Pos_ &orig_pos64, const _Pos_ &dest_pos64)
{
	Board ^= mask(orig_pos64) | mask(dest_pos64);
}

inline void Update(BitBoard &Board, const BitBoard &change_bit)
{
	Board ^= change_bit;
}


unsigned short BitCount(BitBoard b);
unsigned short BitCount(BitBoard b, const BitBoard shade);
void BSFTable_Init();
_Pos_ Lsb64(BitBoard &b);
_Pos_ pop_lsb(BitBoard &b);


inline _Coordinate_ rank_of(const _Pos_ &p)
{
	return p >> 3;
}

inline _Coordinate_ relative_rank_of(const bool &side, const _Pos_ &p)
{
	return rank_of(p) ^ (side * 7);
}

inline _Coordinate_ file_of(const _Pos_ &p)
{
	return p & 7;
}

inline bool side_of(const _ChessType_ &tp)
{
	return BLACK_CHESS_BIT == (tp & BLACK_CHESS_BIT);
}

inline _ChessPattern_ pattern_of(const _ChessType_ &tp)
{
	return tp & CHESS_PATTERN;
}

inline BitBoard get_rank(const BitBoard &b, const _Coordinate_ &rank)
{
	return (b >> (rank << 3)) & 0xFF;
}

inline BitBoard get_file(const BitBoard &b, const _Coordinate_ &file)
{
	return b & (((BitBoard)0x0101010101010101) << file);
}

inline BitBoard push_line(const bool &side)				//推进线，对方领地的第一行，对拜访来说是第五行
{
	return (BitBoard)0x00000000FF000000 << (side * 8);
}

inline BitBoard life_line(const bool &side)				//生命线，对白方来说是第七行
{
	return (BitBoard)0x000000000000FF00 << (side * 40);
}

inline BitBoard dest_line(const bool &side)				//底线，升变线，对白方来说是第七行
{
	return (BitBoard)0x00000000000000FF << (side * 56);
}

inline bool in_promotion_line(const bool &side, const _Pos_ &p)
{
	return side ? (p >= 56) : (p < 8);
}

inline BitBoard forward(const BitBoard &b, const bool &side)
{
	return side ? (b << 8) : (b >> 8);
}

inline _Pos_ forward(const _Pos_ &p, const bool &side)
{
	return side ? p + 8 : p - 8;
}

inline BitBoard forward2(const BitBoard &b, const bool &side)
{
	return side ? (b << 16) : (b >> 16);
}

inline _Pos_ forward2(const _Pos_ &p, const bool &side)
{
	return side ? p + 16 : p - 16;
}

inline BitBoard left_forward(const BitBoard &b, const bool &side)
{
	return side ? (b << 7) : (b >> 9);
}

inline BitBoard right_forward(const BitBoard &b, const bool &side)
{
	return side ? (b << 9) : (b >> 7);
}

