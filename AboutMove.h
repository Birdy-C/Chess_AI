#pragma once
#include"Chess.h"

extern _Bit64_ move_Pawn[2][64];
extern _Bit64_ move_Knight[64];
extern _Bit64_ move_King[64];
extern  _Bit64_ rank_attacks[64][256];	//ÐÐ
extern  _Bit64_ file_attacks[64][256];	//ÁÐ
extern  _Bit64_ diag_A8H1_attacks[64][256];
extern  _Bit64_ diag_H8A1_attacks[64][256];

extern const short getline_shift[64];
extern const short getline_shift_trans[64];
extern const short length_A8H1_diag[64];
extern const short length_H8A1_diag[64];
extern const short shade_A8H1[64];
extern const short shade_H8A1[64];
extern const _Pos_ start_pos_A8H1[64];
extern const _Pos_ start_pos_H8A1[64];