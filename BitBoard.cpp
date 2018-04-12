#include "BitBoard.h"
#include <string>

static _Pos_ BSFTable[SQ_COUNT];

//������飺����BitBoard��1�ĸ���
unsigned short BitCount(BitBoard b)
{
	b = b - ((b >> 1) & 0x5555555555555555ULL);									
	b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);		
	b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;									
	return (uint16_t)((b * 0x0101010101010101ULL) >> 56);
}

//������飺��shadeΪ���֣�����BitBoard��1�ĸ���
unsigned short BitCount(BitBoard b, const BitBoard shade)
{
	b &= shade;
	b = b - ((b >> 1) & 0x5555555555555555ULL);									
	b = (b & 0x3333333333333333ULL) + ((b >> 2) & 0x3333333333333333ULL);		
	b = (b + (b >> 4)) & 0x0F0F0F0F0F0F0F0FULL;									
	return (uint16_t)((b * 0x0101010101010101ULL) >> 56);
}

//������飺����DeBruijin Magic number�ﵽ������ϣ����ȡLSB�����ܴ����Board�����򷵻�63
static size_t bsf_index(BitBoard b)
{
	static const uint64_t DeBruijn64 = 0x3F79D71B4CB0A89ULL;
	b ^= b - 1;
	return (b * DeBruijn64) >> 58;
}

//������飺BSF���ʼ��������LSB��minimal-perfect hashing
void BSFTable_Init()
{
	for (int i = 0; i < SQ_COUNT; i++)
		BSFTable[bsf_index(mask(i))] = i;
}

//������飺��Pos��ʽ��ȡ���λ
_Pos_ Lsb64(BitBoard &b)
{
	//assert(b);
	return BSFTable[bsf_index(b)];
}

//������飺��Pos��ʽ��ȡ���λ������λ����
_Pos_ pop_lsb(BitBoard &b)
{
	_Pos_ p = Lsb64(b);
	b &= b - 1;
	return p;
}