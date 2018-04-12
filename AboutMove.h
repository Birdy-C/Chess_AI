#pragma once

extern BitBoard eat_Pawn[2][64];
extern BitBoard move_Knight[64];
extern BitBoard move_King[64];
extern BitBoard rank_attacks[64][256];
extern BitBoard file_attacks[64][256];
extern BitBoard diag_A8H1_attacks[64][256];
extern BitBoard diag_H8A1_attacks[64][256];

extern const _Pos_ start_pos_A8H1[64];
extern const _Pos_ start_pos_H8A1[64];

//���ڰ��ѽ����ŷ����ɵ��赲��������
extern BitBoard Between_bit[64][64];

//�����ж������ӺϷ����ж��м��pos1��pos2��pos3�Ƿ�λ��ͬһֱ���ϣ��÷�ΪLine_by_2pos[pos1][pos2] & mask(pos3]������һ��ֱ����ʱ����0
extern BitBoard Line_by_2pos[64][64];	

inline bool At_same_line(const _Pos_ &a, const _Pos_ &b, const _Pos_ &c)
{
	return 0 != (Line_by_2pos[a][b] & mask(c));
}