#include"Chess.h"

extern _Bit64_ mask[64];

/*
vector<Movement> ChessBoard::strict_expand(const bool &side)
{
	vector<Movement> result;
	Movement current_Movement;
	_Bit64_ table_bit, target_bit, total_bit, current_bit;
	ChessBoard temp;
	_Pos_ check_pos[2];
	enum check_type checktype[2];

	switch (DeltaCheck(side, check_pos, checktype))
	{
	case 1:
		return Search_RidCheck(side, check_pos[0], checktype[0]);
	}

	if (WHITE_SIDE == side)
	{
		for (_ChessPattern_ LPattern = chess_P; LPattern <= chess_K; LPattern++)
		{
			table_bit = White[LPattern].GetData();
			target_bit = total_bit & (~total_bit + 1);
			while (target_bit)
			{
				total_bit = movement_step((_ChessType_)LPattern, Trans_Bit64toPos(target_bit));
				current_bit = total_bit & (~total_bit + 1);
				while (current_bit)
				{
					temp = *this;
					if (temp.Move(target_bit, current_bit, LPattern, WHITE_SIDE))
					{
						current_Movement.SetValue(Trans_Bit64toPos(target_bit), Trans_Bit64toPos(current_bit), LPattern);
						result.push_back(current_Movement);
					}
					total_bit ^= current_bit;
					current_bit = total_bit & (~total_bit + 1);
				}
			}
			table_bit ^= target_bit;
			target_bit = total_bit & (~total_bit + 1);
		}
	}
	else
	{
		for (_ChessPattern_ LPattern = chess_P; LPattern <= chess_K; LPattern++)
		{
			table_bit = Black[LPattern].GetData();
			target_bit = total_bit & (~total_bit + 1);
			while (target_bit)
			{
				total_bit = movement_step((_ChessType_)(LPattern | BLACK_CHESS_BIT), Trans_Bit64toPos(target_bit));
				current_bit = total_bit & (~total_bit + 1);
				while (current_bit)
				{
					current_Movement.SetValue(Trans_Bit64toPos(target_bit), Trans_Bit64toPos(current_bit), LPattern);
					result.push_back(current_Movement);
					total_bit ^= current_bit;
					current_bit = total_bit & (~total_bit + 1);
				}
			}
			table_bit ^= target_bit;
			target_bit = total_bit & (~total_bit + 1);
		}
	}
	return result;
}*/