/*
#define pcMax 6
#define coMax 2
#define xpMax 8
#define ypMax 8
#define BLACK 1
#define WHITE 0

class ChessBoard;
extern int rand();
//放在ChessBoard类中
static unsigned long long zobrist[pcMax][coMax][xpMax][ypMax];//pc为棋子类型，co为棋子颜色，xp为横坐标，yp为纵坐标
unsigned long long rand64(void) //返回64位的随机数
{
	return rand() ^ ((unsigned long long)rand() << 15) ^ ((unsigned long long)rand() << 30) ^ ((unsigned long long)rand() << 45) ^ ((unsigned long long)rand() << 60);
}

//全局的hash函数
unsigned long long ChessBoard::zobristvalue(void) {
    unsigned long long ZobristValue = 0;
	for( int x = 0; x < 8; x++ )
        for( int y = 0; y < 8; y++ ){
            int result = DeltaChess(x, y);
        if ( -1 != result ) {
            if ( 0 == result&8 )
                ZobristValue = ZobristValue^zobrist[result][WHITE][x][y];
            else
                ZobristValue = ZobristValue^zobrist[result-8][BLACK][x][y];
}
}
    return 	ZobristValue;
}



//传入参数为移动棋子的新旧坐标，返回新盘面下的zobrist值
//仅在程序运行开始时执行一次,初始化ZobristValue的值
void ChessBoard::Zobrist_Init(unsigned long long zobrist[pcMax][coMax][xpMax][ypMax], unsigned long long& ZobristValue)
{
	for (int i = 0; i < pcMax; i++)
		for (int j = 0; j < coMax; j++)
			for (int k = 0; k < xpMax; j++)
				for (int l = 0; l < ypMax; l++)
					zobrist[i][j][k][l] = rand64();

	for (int n = 0; n < 8; n++) {
		int result = DeltaChess(0, n);
		if (result & 8 == 0)
			ZobristValue = ZobristValue^zobrist[result][WHITE][0][n];
		else
			ZobristValue = ZobristValue^zobrist[result - 8][BLACK][0][n];
	}

	for (int n = 0; n < 8; n++) {
		int result = DeltaChess(1, n);
		if (result & 8 == 0)
			ZobristValue = ZobristValue^zobrist[result][WHITE][1][n];
		else
			ZobristValue = ZobristValue^zobrist[result - 8][BLACK][1][n];
	}

	for (int n = 0; n < 8; n++) {
		int result = DeltaChess(6, n);
		if (result & 8 == 0)
			ZobristValue = ZobristValue^zobrist[result][WHITE][6][n];
		else
			ZobristValue = ZobristValue^zobrist[result - 8][BLACK][6][n];
	}

	for (int n = 0; n < 8; n++) {
		int result = DeltaChess(7, n);
		if (result & 8 == 0)
			ZobristValue = ZobristValue^zobrist[result][WHITE][7][n];
		else
			ZobristValue = ZobristValue^zobrist[result - 8][BLACK][7][n];
	}
}
unsigned long long WhiteFirst = rand64();
//一步一更新的函数，考虑了吃子，但没有考虑王车易位和升变
unsigned long long ChessBoard::zobristvalue(int oldx, int oldy, int newx, int newy, unsigned long long ZobristValue)
{
	int move_chessmen = DeltaChess(oldx, oldy);
	int checker = DeltaChess(newx, newy);

	if ((checker != -1) && (checker ^ 8 == 0))
		ZobristValue = ZobristValue^zobrist[move_chessmen][WHITE][newx][newy];

	if ((checker != -1) && (checker ^ 8 != 0))
		ZobristValue = ZobristValue^zobrist[move_chessmen - 8][BLACK][newx][newy];

	if (move_chessmen & 8 == 0)
		return ZobristValue^zobrist[move_chessmen][WHITE][newx][newy] ^ zobrist[move_chessmen][WHITE][oldx][oldy];
	else
		return ZobristValue^zobrist[move_chessmen - 8][BLACK][newx][newy] ^ zobrist[move_chessmen - 8][BLACK][oldx][oldy];
}*/