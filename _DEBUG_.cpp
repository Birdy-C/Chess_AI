#include "Chess.h"
#include <fstream>
#include <limits>
#include <string>

extern MyApp theApp;
extern HashTable Hash_Table;

unsigned long long node_count = 0, hit_count = 0;
extern double timing;

//用于将vector<Movement>中的着法按顺序输出至Media/record.txt供调试使用
void FILE_Print(const vector<Movement> &record)
{
	char op1 = 'A', op2 = 'A';
	size_t step = record.size();
	ofstream fs;
	ChessBoard bd;

	fs.open("./Media/record.txt", ios::out);

	for (size_t i = 0; i < record.size(); i++)
	{
		fs << i << "、";
		record[i].FPrint(fs, bd);
		bd.MoveChess(record[i]);
	}
	fs.close();
}

void Movement::FPrint(ostream &os, const ChessBoard &bd) const
{
	char op = 'A';

	switch (bd.pattern_at(Get_orig_pos()))
	{
	case chess_P: os << "P "; break;
	case chess_N: os << "N "; break;
	case chess_B: os << "B "; break;
	case chess_R: os << "R "; break;
	case chess_Q: os << "Q "; break;
	case chess_K: os << "K "; break;
	default: cout << "X "; break;
	}
	os << (char)(op + file_of(Get_orig_pos())) << 8 - rank_of(Get_orig_pos()) << " -> " << (char)(op + file_of(Get_dest_pos())) << 8 - rank_of(Get_dest_pos());

	switch (Get_move_type())
	{
	case MOVE_CAST:	     os << " CAST" << endl; break;
	case MOVE_ENPASS:    os << " ENPS" << endl; break;
	case MOVE_PROMOTION: os << " PROM" << endl; break;
	default:			 os << " NORM" << endl; break;
	}
}

void Movement::Print(const ChessBoard &bd) const
{
	char op = 'A';

	switch (bd.pattern_at(Get_orig_pos()))
	{
	case chess_P: cout << "P "; break;
	case chess_N: cout << "N "; break;
	case chess_B: cout << "B "; break;
	case chess_R: cout << "R "; break;
	case chess_Q: cout << "Q "; break;
	case chess_K: cout << "K "; break;
	default: cout << "X "; assert(false);  break;
	}
	cout << (char)(op + file_of(Get_orig_pos())) << 8 - rank_of(Get_orig_pos()) << " -> " << (char)(op + file_of(Get_dest_pos())) << 8 - rank_of(Get_dest_pos()) << "\t";

	if (MOVE_CAST == Get_move_type())
		cout << "cast" << endl;
	else if (MOVE_PROMOTION == Get_move_type())
		cout << "prom" << endl;
	else if (MOVE_ENPASS == Get_move_type())
		cout << "enps" << endl;
	else
		cout << endl;
}

//用于从path路径中读取记录文件，并将记录文件中的着法按顺序导入至Move_record，UI中按键盘P可根据Move_record自动进行一步落子
void MyApp::Load_Record(char path[80])
{
	ifstream is(path);
	int step;
	char line_orig, line_dest;
	string line_mvtp;
	_Pos_ orig, dest;
	_MoveType_ mvtp;

	if (!is)
		Error("_DEBUG_.cpp", "Load_Record", "Record file dosen't exist");

	while (is >> step)
	{
		is.seekg(4, ios::cur);
		is >> line_orig >> orig;
		is.seekg(4, ios::cur);
		is >> line_dest >> dest;
		is.seekg(1, ios::cur);
		is >> line_mvtp;

		if (line_orig < 'A' || line_orig > 'H' || line_dest < 'A' || line_dest > 'H' || orig < '1' || orig > '8' || dest < '1' || dest > '8')
			Error("Main.cpp", "RunApp", "Record content error");

		mvtp = line_mvtp == "NORM" ? MOVE_NORMAL :
			line_mvtp == "CAST" ? MOVE_CAST :
			line_mvtp == "ENPS" ? MOVE_ENPASS : MOVE_PROMOTION;

		orig = ('8' - orig) << 3;
		dest = ('8' - dest) << 3;
		orig = orig + (_Pos_)line_orig - 'A';
		dest = dest + (_Pos_)line_dest - 'A';

		Movement new_mv(orig, dest, mvtp);
		Move_record.push_back(new_mv);
	}
	is.close();

	cout << "Record loaded with " << step + 1 << "steps\n\n";
}

void MyApp::MonteCarlo()
{
	PVLine line;
	static int step, round = 0, round_1000 = 0, choice;
	Movement new_Move[MAX_BRANCH_COUNT];
	
	/*
	while (++round_1000 <= 1000)
	{
		unsigned branch_count, pt;
		_Board_Flag_ temp_flag;
		BitBoard bb[_SIDE_COUNT_][_BB_COUNT_], all, allt, allr45, alll45;

		round = 0;
		while (++round <= 1000)
		{
			step = 0;
			InitMatch();
			while (++step <= 300)
			{
				node_count++;
				//Board.evaluation(side);
				branch_count = Board.strict_expand(side, new_Move);

				if (branch_count != 0 && Board.draw50() < 50)
				{
					choice = rand32() % branch_count;

					MoveChess(new_Move[choice]);
					//Board.MoveChess(new_Move[choice]);
					//Board.undo_MoveChess(new_Move[choice]);

					if (Board.chess_count(WHITE_SIDE, chess_K) != 1 || Board.chess_count(BLACK_SIDE, chess_K) != 1)
					{
						FILE_Print(Move_record);
						system("cls");
						printf("Zobrist assertion failed at step %d\n", step);
						Error("_DEBUG_.cpp", "MonteCarlo", "Zobrist assertion failed, for record please view ./Media/record.txt");
					}
				}
				else
					break;
			}
		}
		printf("Round %4d finished\n", round_1000 * 1000);
	}*/
	

	//PV line
	
	
	//while (++round <= 1)
	{
		step = 0;
		while (++step <= 60)
		{
			//system("cls");
			//if ((val = Board.AlphaBeta(0, SEARCH_DEPTH, side, -INF, INF)) == -INF)
				//break;;

			ChessBoard temp = Board;
			if (side == WHITE_SIDE)
				MoveChess(temp.IterativeDeepening(side, step_count));
			if (Board.chess_count(WHITE_SIDE, chess_K) != 1 || Board.chess_count(BLACK_SIDE, chess_K) != 1)
			{
				FILE_Print(Move_record);
				system("cls");
				printf("King assertion failed at step %d\n", step);
				Error("_DEBUG_.cpp", "MonteCarlo", "Zobrist assertion failed, for record please view ./Media/record.txt");
			}
			else
			{
				unsigned short branch_count;
				branch_count = Board.strict_expand(side, new_Move);
				choice = rand32() % branch_count;
				MoveChess(new_Move[choice]);
			}
			cout << "step " << step << "  complete" << endl;
		}
		//InitMatch();
		//Zobrist_Init();
		//Hash_Table.Clear();
	}
}

//将当前局面下的所有合法落子点打印至屏幕
void ChessBoard::DEBUG_PRINT_EXPAND(const bool &side)
{
	Movement mv, mvtb[MAX_BRANCH_COUNT];
	MoveGenerator mv_gen(*this, mv, side);
	unsigned branch_count = 0;

	system("cls");

	//BitBoard a = 0;
	//cout << (int)Lsb64(a) << endl;

	branch_count = strict_expand(side, mvtb);
	for (unsigned i = 0; i < branch_count; i++)
	{
		cout << i+1 << "、  ";
		if (i < 9)
			cout << " ";
		mvtb[i].Print(*this);
	}

	/*
	while ((mv = mv_gen.next_move()).Exist())
	{
		cout << ++branch_count << "、  ";
		if (branch_count < 10)
			cout << " ";
		mv.Print(*this);
	}*/
}

bool ChessBoard::operator==(ChessBoard& pBoard)
{
	if (pBoard.flag != flag)
	{
		return false;
	}

	for (_ChessPattern_ pattern = chess_P; pattern <= chess_All; pattern++)
	{
		if (pBoard.BB[WHITE_SIDE][pattern] != BB[WHITE_SIDE][pattern] || pBoard.BB[BLACK_SIDE][pattern] != BB[BLACK_SIDE][pattern])
			return false;
	}

	for (_Pos_ p = 0; p < SQ_COUNT; p++)
	{
		if (pBoard.ChessArray[p] != ChessArray[p])
			return false;
	}

	for (_Pos_ p = 0; p < SQ_COUNT; p++)
	{
		if (pBoard.AttackRange[p] != AttackRange[p])
			return false;
	}

	return true;
}

unsigned short ChessBoard::strict_expand(const bool &side, Movement mvtb[MAX_BRANCH_COUNT])
{
	Movement mv;
	MoveGenerator mv_gen(*this, mv, side);
	unsigned short branch_count = 0;

	while ((mv = mv_gen.next_move()).Exist())
	{
		if (Legal(mv))
			mvtb[branch_count++] = mv;
	}	
	return branch_count;
}