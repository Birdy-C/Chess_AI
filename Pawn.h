#pragma once
#include "Evaluation.h"

BitBoard passedPawns[_SIDE_COUNT_];
BitBoard pawnAttacks[_SIDE_COUNT_];
BitBoard pawnAttacksSpan[_SIDE_COUNT_];
//Square kingSquares[_SIDE_COUNT_];
//Score kingSafety[_SIDE_COUNT_];
int weakUnopposed[_SIDE_COUNT_];
int castlingRights[_SIDE_COUNT_];
int semiopenFiles[_SIDE_COUNT_];
int pawnsOnSquares[_SIDE_COUNT_][_SIDE_COUNT_]; // [color][light/dark squares]
int asymmetry;
int openFiles;
