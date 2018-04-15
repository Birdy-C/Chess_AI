#pragma once
//Birdy 18.2.22
#include "Board.h"
#include "AboutMove.h"

#define S(mg, eg) make_score(mg, eg)

//============================���̵Ķ���===================================
enum Direction : int {
	NORTH = 8,
	EAST = 1,
	SOUTH = -NORTH,
	WEST = -EAST,

	NORTH_EAST = NORTH + EAST,
	SOUTH_EAST = SOUTH + EAST,
	SOUTH_WEST = SOUTH + WEST,
	NORTH_WEST = NORTH + WEST
};

const BitBoard FileABB = 0x0101010101010101ULL;
const BitBoard FileBBB = FileABB << 1;
const BitBoard FileCBB = FileABB << 2;
const BitBoard FileDBB = FileABB << 3;
const BitBoard FileEBB = FileABB << 4;
const BitBoard FileFBB = FileABB << 5;
const BitBoard FileGBB = FileABB << 6;
const BitBoard FileHBB = FileABB << 7;

// Rank �ļ�¼�������෴ ��Ϊ�Ǵ����Ͽ�ʼ��
const BitBoard Rank1BB = 0xFF;
const BitBoard Rank2BB = Rank1BB << (8 * 1);
const BitBoard Rank3BB = Rank1BB << (8 * 2);
const BitBoard Rank4BB = Rank1BB << (8 * 3);
const BitBoard Rank5BB = Rank1BB << (8 * 4);
const BitBoard Rank6BB = Rank1BB << (8 * 5);
const BitBoard Rank7BB = Rank1BB << (8 * 6);
const BitBoard Rank8BB = Rank1BB << (8 * 7);


const BitBoard Center = (FileDBB | FileEBB) & (Rank4BB | Rank5BB);
const BitBoard QueenSide = FileABB | FileBBB | FileCBB | FileDBB;
const BitBoard CenterFiles = FileCBB | FileDBB | FileEBB | FileFBB;
const BitBoard KingSide = FileEBB | FileFBB | FileGBB | FileHBB;


//===========================����====================================
_Score_ make_score(int mg, int eg);
static const _Value_ MidgameLimit = 15258, EndgameLimit = 3915;

const _Value_ Contempt = 0;			//��������
const _Value_ VALUE_INF = 20000;
const _Value_ VALUE_UNKNOWN = -30000;

//===========================Material=====================================
void psq_init();
void pawn_init();
//=================================
extern Phase gamePhase;
extern _Score_ non_pawn_material[_SIDE_COUNT_];


								   // Threshold for lazy and space evaluation
static const _Value_ LazyThreshold = _Value_(1500); // ��ɻ��������ͳ�����
static const _Value_ SpaceThreshold = _Value_(12222);

static const _Value_ SPACE_VALUE1 = 10;
static const _Value_ SPACE_VALUE2 = 10;
static const _Value_ PAWN_VALUE1 = 12; //�±�
static const _Value_ PAWN_VALUE2 = 13; //����
static const _Value_ PAWN_VALUE3 = 27; //ͨ·��
static const _Value_ chess_K_VALUE = 7;
static const _Value_ ROOK_VALUE = 7; //ռ��ͨ·�ĳ�

static const _Value_ flexValue[6] = { 0, 5, 5, 5, 5, 5 };	//�����ԵĶ��壨����������ԵĻ���
static const _Value_ Value2[6] = { 0, 7, 6, 5, 5, 5 };	//�������ӿ������� 
static const _Value_ pawnValue[8] = { 0, 0, 0, 0, 100, 1500, 2000, 0 };	//���ļ�ֵ













