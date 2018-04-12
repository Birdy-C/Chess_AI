#pragma once

#include "Movement.h"
#include "Macro.h"
#include <algorithm>
#include <random>

#define HASHTABLE_SIZE			((4)M - 1)

typedef uint64_t	_Zobrist64_;
typedef uint32_t	_Zobrist32_;
typedef uint16_t	_Zobrist16_;
typedef uint8_t		_Zobrist8_;

enum 
{ 
	HASH_NONE		= 0x0800,
	HASH_EVALUATE	= 0x1000,
	HASH_ALPHA		= 0x2000,
	HASH_BETA		= 0x4000, 
	HASH_PV			= 0x8000, 

	HASH_STEP_CT	= 0x07FF
};

typedef struct
{
	union{
		_Zobrist64_ zobrist64;
		struct{
			union{
				_Zobrist32_ low_32;
				_Zobrist8_  low_8[4];
			};
			_Zobrist32_ up_32;
		};
	};
} Zobrist;

class Hash
{
private:
	_Zobrist32_ zobrist_up;
	_Zobrist8_ zobrist_low;
	_Depth_ depth;
	uint16_t flag;
	_Value_ evaluation;
	Movement next_Best;

public:
	Hash(){}
	void Init(){ depth = INVALID; flag = HASH_NONE; next_Best.Clear(); }
	_Zobrist32_ Get_zobrist_up(){ return zobrist_up; }
	_Zobrist8_ Get_zobrist_low(){ return zobrist_low; }
	unsigned short Get_flag(){ return flag; }
	_Value_ Get_evaluation(){ return evaluation; }
	Movement Get_nextBest(){ return next_Best; }
	bool Exist_nextBest(){ return next_Best.Exist(); }
	_Depth_ Get_depth(){ return depth; }
	void Set_zobrist(const Zobrist &pZobrist){ zobrist_up = pZobrist.up_32; zobrist_low = pZobrist.low_8[3]; }
	void Set_Flag(const uint16_t &pFlag){ flag = pFlag; }
	void Set_evaluation(const _Value_ &pValue){ evaluation = pValue; }
	void Set_next_Best(const Movement &pNext_Best){ next_Best = pNext_Best; }
	void Set_depth(const unsigned char &pDepth){ depth = pDepth; }
};

class HashTable
{
private:
	Hash data[HASHTABLE_SIZE + 1];
	size_t size;
public:
	HashTable(size_t table_size){ size = table_size; }
	//HashTable(size_t table_size){ Hash_Table = new Hash[table_size]; size = table_size; }
	//~HashTable(){ delete[] Hash_Table; }
	void Clear(){ for (size_t i = 0; i < size; i++)	data[i].Init(); }
	_Value_ Probe_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &alpha, const _Value_ &beta, Movement &best_move);
	void Record_HashTable(const Zobrist &zobrist, const _Depth_ &depth, const _Value_ &val, const uint16_t &flag, const Movement &best_move);
	Hash* Get_HashNode(const _Zobrist32_ zobrist){ return data + (zobrist & HASHTABLE_SIZE); }
};
