#pragma once

#include "Board.h"

typedef struct
{
	_Board_Flag_ bd_flag;
	ExtMove	mvtb[MAX_BRANCH_COUNT];
} StateInfo;

extern StateInfo st_inf[MAX_DEPTH + 1];