#include "BitBoard.h"
#include <string>

static _Pos_ BSFTable[SQ_COUNT];

//函数简介：返回BitBoard中1的个数
unsigned short BitCount(BitBoard b)
{
	b = b - ((b >> 1) & 0x5555555555555555ULL);									
	b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);		
	b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;									
	return (uint16_t)((b * 0x0101010101010101ULL) >> 56);
}

//函数简介：以shade为遮罩，返回BitBoard中1的个数
unsigned short BitCount(BitBoard b, const BitBoard shade)
{
	b &= shade;
	b = b - ((b >> 1) & 0x5555555555555555ULL);									
	b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);		
	b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;									
	return (uint16_t)((b * 0x0101010101010101ULL) >> 56);
}

//函数简介：利用DeBruijin Magic number达到完美哈希以求取LSB，不能传入空Board，否则返回63
static size_t bsf_index(BitBoard b)
{
	static const uint64_t DeBruijn64 = 0x3F79D71B4CB0A89ULL;
	b ^= b - 1;
	return (b * DeBruijn64) >> 58;
}

//函数简介：BSF表初始化，用于LSB的minimal-perfect hashing
void BSFTable_Init()
{
	for (int i = 0; i < SQ_COUNT; i++)
		BSFTable[bsf_index(mask(i))] = i;
}

//函数简介：以Pos形式求取最低位
_Pos_ Lsb64(BitBoard &b)
{
	//assert(b);
	return BSFTable[bsf_index(b)];
}

//函数简介：以Pos形式求取最低位并将该位置零
_Pos_ pop_lsb(BitBoard &b)
{
	_Pos_ p = Lsb64(b);
	b &= b - 1;
	return p;
}