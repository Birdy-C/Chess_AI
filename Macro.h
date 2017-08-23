#ifndef __GUI_H_
#define __GUI_H_

#pragma warning(disable:4996)

typedef unsigned long long _Bit64_;
typedef unsigned char _Pos_;
typedef unsigned char _Coordinate_;

typedef unsigned char _ChessPattern_;
typedef unsigned char _ChessType_;

enum check_type{ Pawn_Check = 0, Knight_Check, Diag_Check, Straight_Check };

#define chess_P 0
#define chess_N 1
#define chess_B 2
#define chess_R 3
#define chess_Q 4
#define chess_K 5
#define BLACK_CHESS_BIT 8

#define CHESSMEN_SIZE 90

#define NONE 255

#define WHITE_SIDE false
#define BLACK_SIDE true
#define SEARCH_FAIL false
#define SEARCH_SUCCESS true

#define KNIGHT_POSSIBLE_MOVE 8
#define KING_POSSIBLE_MOVE 8

#define LONG_CASTLING  1		//рвн╩
#define SHORT_CASTLING 2

#define CHECK_ARRAY_SIZE 30

#endif