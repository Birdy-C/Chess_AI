#pragma once

#define CHESSMEN_SIZE			90

#define NONE					255
#define INVALID				   -1

#define KNIGHT_POSSIBLE_MOVE	8
#define KING_POSSIBLE_MOVE		8

#define LONG_CASTLING			1
#define SHORT_CASTLING			2

#define MAX_BRANCH_COUNT		128
#define MAX_DEPTH				24

#define K						*(1024)
#define M						*(1048576)

#define BASIC_SEARCH_DEPTH		1
#define SEARCH_DEPTH			8

#define IS_NOT_LINE		0ULL
#define IS_LINE			0x8100000000000081ULL
#define LINE_RANK		0x0000000000000001ULL
#define LINE_FILE		0x0000000000000080ULL
#define LINE_A8H1		0x0100000000000000ULL
#define LINE_H8A1		0x8000000000000000ULL

typedef unsigned short	_MoveType_;
typedef unsigned char	_Check_Type_;
typedef long			_Score_;
typedef short			_Value_;
typedef char			_Depth_;