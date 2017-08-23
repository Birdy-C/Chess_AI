#include"Chess.h"

extern _Bit64_ mask[64];
extern _Bit64_ Between_bit[64][64];
extern _Bit64_ move_King[64];

/*只有这个函数在判断时是会检测出两个王互将的情况*/
bool ChessBoard::DeltaAttack(const bool &side, const _Pos_ &pos)
{
	if (WHITE_SIDE == side)
	{
		if (movement_step(chess_B, pos) & (Black[chess_B].GetData() | Black[chess_Q].GetData()))
			return true;
		if (movement_step(chess_R, pos) & (Black[chess_R].GetData() | Black[chess_Q].GetData()))
			return true;
		if (movement_step(chess_N, pos) & Black[chess_N].GetData())
			return true;
		if (Black[chess_P].GetData() & (mask[pos] >> 9) & (_Bit64_)0x7F7F7F7F7F7F7F7F |
			(Black[chess_P].GetData() & (mask[pos] >> 7) & (_Bit64_)0xFEFEFEFEFEFEFEFE))
			return true;
		if (move_King[pos] & Black[chess_K].GetData())
			return true;
	}
	else
	{
		if (movement_step(chess_B | BLACK_CHESS_BIT, pos) & (White[chess_B].GetData() | White[chess_Q].GetData()))
			return true;
		if (movement_step(chess_R | BLACK_CHESS_BIT, pos) & (White[chess_R].GetData() | White[chess_Q].GetData()))
			return true;
		if (movement_step(chess_N | BLACK_CHESS_BIT, pos) & White[chess_N].GetData())
			return true;
		if (White[chess_P].GetData() & (mask[pos] << 9) & (_Bit64_)0xFEFEFEFEFEFEFEFE |
			(White[chess_P].GetData() & (mask[pos] << 7) & (_Bit64_)0x7F7F7F7F7F7F7F7F))
			return true;
		if (move_King[pos] & White[chess_K].GetData())
			return true;
	}
	return false;
}

unsigned short ChessBoard::DeltaCheck(const bool &side, _Pos_ check_pos[2], enum check_type checktype[2])
{
	_Pos_ pos = King_pos[side];
	_Bit64_ temp_bit;
	unsigned short check_count = 0;

	if (WHITE_SIDE == side)
	{
		temp_bit = movement_step(chess_B, pos) & (Black[chess_B].GetData() | Black[chess_Q].GetData());
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Diag_Check;

		temp_bit = movement_step(chess_R, pos) & (Black[chess_R].GetData() | Black[chess_Q].GetData());
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Straight_Check;

		temp_bit = movement_step(chess_N, pos) & Black[chess_N].GetData();
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Knight_Check;

		temp_bit = (Black[chess_P].GetData() & (mask[pos] >> 9) & (_Bit64_)0x7F7F7F7F7F7F7F7F |
			(Black[chess_P].GetData() & (mask[pos] >> 7) & (_Bit64_)0xFEFEFEFEFEFEFEFE));
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Pawn_Check;
	}
	else
	{
		temp_bit = movement_step(chess_B | BLACK_CHESS_BIT, pos) & (White[chess_B].GetData() | White[chess_Q].GetData());
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Diag_Check;

		temp_bit = movement_step(chess_R | BLACK_CHESS_BIT, pos) & (White[chess_R].GetData() | White[chess_Q].GetData());
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Straight_Check;

		temp_bit = movement_step(chess_N | BLACK_CHESS_BIT, pos) & White[chess_N].GetData();
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Knight_Check;

		temp_bit = (White[chess_P].GetData() & (mask[pos] << 9) & (_Bit64_)0xFEFEFEFEFEFEFEFE |
			(White[chess_P].GetData() & (mask[pos] << 7) & (_Bit64_)0x7F7F7F7F7F7F7F7F));
		if (temp_bit)
			check_pos[check_count] = Trans_Bit64toPos(temp_bit), checktype[check_count++] = Pawn_Check;
	}
	return check_count;
}

void ChessBoard::Search_RidCheck(const bool &side, const _Pos_ &check_pos, const enum check_type &LChecktype, vector<Movement> &result)
{
	Movement new_Movement;
	_Bit64_ total_bit, current_bit;

	//移动王避免将军的可行走步
	Search_RidDoubleCheck(side, result);

	//吃掉将军的敌子的可行走步
	if (WHITE_SIDE == side)
	{
		for (_ChessPattern_ LPattern = chess_N; LPattern <= chess_Q; LPattern++)
		{
			total_bit = movement_step((_ChessType_)(LPattern | 8), check_pos) & White[LPattern].GetData();
			current_bit = total_bit & (~total_bit + 1);
			while (current_bit)
			{
				new_Movement.SetValue(Trans_Bit64toPos(current_bit), check_pos, LPattern);
				result.push_back(new_Movement);
				total_bit ^= current_bit;
				current_bit = total_bit & (~total_bit + 1);
			}
		}
		if (((White[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) >> 7) & mask[check_pos])
		{
			new_Movement.SetValue(check_pos + 7, check_pos, chess_P);
			result.push_back(new_Movement);
		}
		if (((White[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) >> 9) & mask[check_pos])
		{
			new_Movement.SetValue(check_pos + 9, check_pos, chess_P);
			result.push_back(new_Movement);
		}
		//敌方将军的兵被我方通过过路兵吃掉
		if (En_passant == (check_pos & 7) && LChecktype == Pawn_Check && (check_pos >> 3) == 3)
		{
			if (White[chess_P].GetData() & (mask[check_pos - 1] & (_Bit64_)0x7F7F7F7F7F7F7F7F))
			{
				new_Movement.SetValue(check_pos - 1, check_pos - 8, chess_P);
				result.push_back(new_Movement);
			}
			if (White[chess_P].GetData() & (mask[check_pos + 1] & (_Bit64_)0xFEFEFEFEFEFEFEFE))
			{
				new_Movement.SetValue(check_pos + 1, check_pos - 8, chess_P);
				result.push_back(new_Movement);
			}
		}
	}
	else
	{
		for (_ChessPattern_ LPattern = chess_N; LPattern <= chess_Q; LPattern++)
		{
			total_bit = movement_step((_ChessType_)LPattern, check_pos) & Black[LPattern].GetData();
			current_bit = current_bit = total_bit & (~total_bit + 1);
			while (current_bit)
			{
				if (movement_step(LPattern | BLACK_CHESS_BIT, Trans_Bit64toPos(current_bit) & mask[check_pos]))
				{
					new_Movement.SetValue(Trans_Bit64toPos(current_bit), check_pos, LPattern);
					result.push_back(new_Movement);
				}
				total_bit ^= current_bit;
				current_bit = total_bit & (~total_bit + 1);
			}
		}
		if (((Black[chess_P].GetData() & (_Bit64_)0xFEFEFEFEFEFEFEFE) << 7) & mask[check_pos])
		{
			new_Movement.SetValue(check_pos - 7, check_pos, chess_P);
			result.push_back(new_Movement);
		}
		if (((Black[chess_P].GetData() & (_Bit64_)0x7F7F7F7F7F7F7F7F) << 9) & mask[check_pos])
		{
			new_Movement.SetValue(check_pos - 9, check_pos, chess_P);
			result.push_back(new_Movement);
		}
		//敌方将军的兵被我方通过过路兵吃掉
		if (En_passant == (check_pos & 7) && LChecktype == Pawn_Check && (check_pos >> 3) == 4)
		{
			if (Black[chess_P].GetData() & (mask[check_pos - 1] & (_Bit64_)0x7F7F7F7F7F7F7F7F))
			{
				new_Movement.SetValue(check_pos - 1, check_pos + 8, chess_P);
				result.push_back(new_Movement);
			}
			if (Black[chess_P].GetData() & (mask[check_pos + 1] & (_Bit64_)0xFEFEFEFEFEFEFEFE))
			{
				new_Movement.SetValue(check_pos + 1, check_pos + 8, chess_P);
				result.push_back(new_Movement);
			}
		}
	}

	//阻挡将军的敌子的可行走步
	if (WHITE_SIDE == side)
	{
		if (LChecktype == Diag_Check || LChecktype == Straight_Check)
		{
			_Bit64_ table_bit, target_bit;

			table_bit = Between_bit[check_pos][King_pos[WHITE_SIDE]];
			target_bit = table_bit & (~table_bit + 1);
			while (target_bit)
			{
				for (_ChessPattern_ LPattern = chess_N; LPattern <= chess_Q; LPattern++)
				{
					total_bit = movement_step((_ChessType_)(LPattern | BLACK_CHESS_BIT), Trans_Bit64toPos(target_bit)) & White[LPattern].GetData();
					current_bit = total_bit & (~total_bit + 1);
					while (current_bit)
					{
						new_Movement.SetValue(Trans_Bit64toPos(current_bit), Trans_Bit64toPos(target_bit), LPattern);
						result.push_back(new_Movement);
						total_bit ^= current_bit;
						current_bit = total_bit & (~total_bit + 1);
					}
				}
				if (White[chess_P].GetData() & target_bit)
				{
					total_bit = White[chess_P].GetData();
					current_bit = total_bit & (~total_bit + 1);
					while (current_bit)
					{
						new_Movement.SetValue(Trans_Bit64toPos(current_bit), Trans_Bit64toPos(target_bit), chess_P);
						result.push_back(new_Movement);
						total_bit ^= current_bit;
						current_bit = total_bit & (~total_bit + 1);
					}
				}
				table_bit ^= target_bit;
				target_bit = table_bit & (~table_bit + 1);
			}		
		}
	}
	else
	{
		if (LChecktype == Diag_Check || LChecktype == Straight_Check)
		{
			_Bit64_ table_bit, target_bit;

			table_bit = Between_bit[check_pos][King_pos[BLACK_SIDE]];
			target_bit = table_bit & (~table_bit + 1);
			while (target_bit)
			{
				for (_ChessPattern_ LPattern = chess_P; LPattern <= chess_Q; LPattern++)
				{
					total_bit = movement_step((_ChessType_)(LPattern), Trans_Bit64toPos(target_bit)) & Black[LPattern].GetData();
					current_bit = total_bit & (~total_bit + 1);
					while (current_bit)
					{
						new_Movement.SetValue(Trans_Bit64toPos(current_bit), Trans_Bit64toPos(target_bit), LPattern);
						result.push_back(new_Movement);
						total_bit ^= current_bit;
						current_bit = total_bit & (~total_bit + 1);
					}
				}
				if (Black[chess_P].GetData() & target_bit)
				{
					total_bit = Black[chess_P].GetData();
					current_bit = total_bit & (~total_bit + 1);
					while (current_bit)
					{
						new_Movement.SetValue(Trans_Bit64toPos(current_bit), Trans_Bit64toPos(target_bit), chess_P);
						result.push_back(new_Movement);
						total_bit ^= current_bit;
						current_bit = total_bit & (~total_bit + 1);
					}
				}
				table_bit ^= target_bit;
				target_bit = table_bit & (~table_bit + 1);
			}
		}
	}
}

void ChessBoard::Search_RidDoubleCheck(const bool &side, vector<Movement> &result)
{
	Movement new_Movement;
	_Bit64_ KingMove_bit, KingMove_Tempbit, temp_bit = 0;
	_Pos_ temp_pos, King_orig = King_pos[side];

	if (WHITE_SIDE == side)
	{
		KingMove_bit = movement_step(chess_K, King_pos[side]);
		KingMove_Tempbit = KingMove_bit;

		while (1)
		{
			KingMove_Tempbit ^= temp_bit;
			temp_bit = KingMove_Tempbit & ((~KingMove_Tempbit) + 1);

			if (!temp_bit)
				break;
			else
				temp_pos = Trans_Bit64toPos(temp_bit);

			Move(King_pos[side], temp_pos, chess_K, side);		//暂时改变国王的位置

			if (movement_step(chess_B, temp_pos) & (Black[chess_B].GetData() | Black[chess_Q].GetData()))
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (movement_step(chess_R, temp_pos) & (Black[chess_R].GetData() | Black[chess_Q].GetData()))
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (movement_step(chess_N, temp_pos) & Black[chess_N].GetData())
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (Black[chess_P].GetData() & (mask[temp_pos] >> 9) & (_Bit64_)0x7F7F7F7F7F7F7F7F |
				(Black[chess_P].GetData() & (mask[temp_pos] >> 7) & (_Bit64_)0xFEFEFEFEFEFEFEFE))
			{
				KingMove_bit ^= temp_bit; continue;
			}
		}
	}
	else
	{
		KingMove_bit = movement_step(chess_K | BLACK_CHESS_BIT, King_pos[side]);
		KingMove_Tempbit = KingMove_bit;

		while (1)
		{
			KingMove_Tempbit ^= temp_bit;
			temp_bit = KingMove_Tempbit & (~KingMove_Tempbit + 1);

			if (!temp_bit)
				break;
			else
				temp_pos = Trans_Bit64toPos(temp_bit);

			Move(King_pos[side], temp_pos, chess_K, side);		//暂时改变国王的位置

			if (movement_step(chess_B | BLACK_CHESS_BIT, temp_pos) & (White[chess_B].GetData() | White[chess_Q].GetData()))
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (movement_step(chess_R | BLACK_CHESS_BIT, temp_pos) & (White[chess_R].GetData() | White[chess_Q].GetData()))
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (movement_step(chess_N | BLACK_CHESS_BIT, temp_pos) & White[chess_N].GetData())
			{
				KingMove_bit ^= temp_bit; continue;
			}
			if (White[chess_P].GetData() & (mask[temp_pos] << 9) & (_Bit64_)0xFEFEFEFEFEFEFEFE |
				(White[chess_P].GetData() & (mask[temp_pos] << 7) & (_Bit64_)0x7F7F7F7F7F7F7F7F))
			{
				KingMove_bit ^= temp_bit; continue;
			}
		}
	}

	Move(King_pos[side], King_orig, chess_K, side);

	temp_bit = KingMove_bit & (~KingMove_bit + 1);
	while (temp_bit)
	{
		new_Movement.SetValue(King_pos[side], Trans_Bit64toPos(temp_bit), chess_K);
		result.push_back(new_Movement);
		KingMove_bit ^= temp_bit;
		temp_bit = KingMove_bit & (~KingMove_bit + 1);
	}
}