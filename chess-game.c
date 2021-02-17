#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>
#include <assert.h>

#include "ass4.h"


typedef struct {
	char srcPiece, srcRow, srcCol, destPiece, destRow, destCol, promotionPiece;
	int iSrc, jSrc, iDest, jDest;
	int isWhite, isCapture, isPromotion, isCheck, isMate, isLegal;
} Move;

// Functions Declarations
void printColumns();
void printSpacers();
void printRow(char row[], int rowIdx);
void createRow(char fenString[], char boardRow[]);
char convertPieceChar(Move move);
Move initMove(char board[][SIZE], char pgn[], int isWhiteTurn);
Move parseSrcFromPgn(char pgn[], Move move);
Move parseDestFromPgn(char pgn[], Move move);
Move convertSrcMatrixIndex(Move move);
Move convertDestMatrixIndex(Move move);
Move parseConditionFromPgn(char pgn[], Move move);
char findDestPiece(int iDest, int jDest, char board[][SIZE]);
Move findOptionalPieceByMove(char board[][SIZE], Move move);
Move checkRookMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
Move checkKnightMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
Move checkBishopMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
Move checkQueenMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
Move checkKingMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
Move checkPawnMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc);
int isWhiteDest(char destPiece);
int sameColorPieceTest(char destPiece, int isWhite);
int noCaptureDestTest(int isCapture, char destPiece);
int noCaptureDeclareTest(char destPiece, int isWhite, int isCapture);
Move testCheckConditions(char board[][SIZE], Move move);
int isCheckCase(char board[][SIZE], int isWhiteMove, int isTheratToWhite);
int testBoardCheck(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite);
int checkTrialWithoutDeclare(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite);
int checkDeclareWithoutTrial(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite);
int moveCauseToCheckThreat(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite);
int limitedMoveInCheckCase(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite);


// Chess characters and PGN signs
const char PAWN = 'P';
const char ROOK = 'R';
const char KNIGHT = 'N';
const char BISHOP = 'B';
const char QUEEN = 'Q';
const char KING = 'K';
const char CAPTURE = 'x';
const char PROMOTION = '=';
const char CHECK = '+';
const char MATE = '#';
const char FIRST_COL = 'a';

// FEN separator for strtok()
const char SEP[] = "/";

// Board characters
const char EMPTY = ' ';

/*************************************************************************************************
*	Function name: toDigit
*	Input: char c
*	Output: char (values between 0-9)
*	Function Operation: the function converts the given char to a numerical value it represents.
*	the function throws assert note if the char which received is different from '0'-'9'.
***************************************************************************************************/
int toDigit(char c) {
	assert('0' <= c && c <= '9');
	return c - '0';
}

/*************************************************************************************************
*	Function name: printColumns
*	Input: None
*	Output: None
*	Function Operation: this function prints the visual representation of columns in a chessboard.
*	the function pass in loop on board size and print letter in any column between
*	a-z according to board size.
***************************************************************************************************/
void printColumns() {
	char column = toupper(FIRST_COL);
	printf("* |");
	for (int i = 0; i < SIZE; i++) {
		if (i) {
			printf(" ");
		}
		printf("%c", column);
		column++;
	}
	printf("| *\n");
}

/*************************************************************************************************
*	Function name: printSpacers
*	Input: None
*	Output: None
*	Function Operation: this function prints the visual representation of columns in a chessboard.
*	the function pass in loop on board size and print visual separators '-'
***************************************************************************************************/
void printSpacers() {
	printf("* -");
	for (int i = 0; i < SIZE; i++) {
		printf("--");
	}
	printf(" *\n");
}

/*************************************************************************************************
*	Function name: printRow
*	Input: char row[], int rowIdx
*	Output: None
*	Function Operation: this function prints row in chessboard according to array which recieved and
*	specific row number. the array which recieved include letters which represent chess pieces and
*	numers which represent the number of spaces between piece to piece.
***************************************************************************************************/
void printRow(char row[], int rowIdx) {
	printf("%d ", rowIdx);
	int i = 0;
	while (row[i]) {
		if (isdigit(row[i])) {
			int spaces = toDigit(row[i++]);
			for (int j = 0; j < spaces; j++) {
				printf("|%c", EMPTY);
			}
		}
		else {
			printf("|%c", row[i++]);
		}
	}
	printf("| %d\n", rowIdx);
}

/*************************************************************************************************
*	Function name: createRow
*	Input: char fenString[], char boardRow[]
*	Output: None
*	Function Operation: this function received pointer to 2D array which represents chess board.
*	According to FEN received, the function inserts the relevant pieces and spaces to board.
***************************************************************************************************/
void createRow(char fenString[], char boardRow[]) {

	int i = 0;
	int c = 0;

	/*
		While loop which inserts the relevant pieces and spaces.
	*/
	while (fenString[c] != '\0') {

		//If the FEN include digit, fill the row in spacse according to digit value
		if (isdigit(fenString[c])) {
			int spaces = toDigit(fenString[c]);
			for (int z = 0; z < spaces; z++) {
				boardRow[i] = EMPTY;
				i++;
			}
		}

		// If the FEN include letter, insert the relevant piece to row.
		else {

			boardRow[i] = fenString[c];
			i++;
		}
		c++;
	}

}

/*************************************************************************************************
*	Function name: createBoard
*	Input: char board[][SIZE], char fen[]
*	Output: None
*	Function Operation: this function create chessboard according to FEN which recieved
*	on 2D array given. At first, the function parse the FEN string to the rows that any part represents.
*	any partial FEN string which was parsed, will be send to function createRow which described below.
***************************************************************************************************/
void createBoard(char board[][SIZE], char fen[]) {

	int i = 0;
	char* fenRow = strtok(fen, SEP);
	char* partialFen[SIZE] = {'\0'};

	//Parsing the FEN to the rows that any part represents
	while (fenRow != NULL) {
		partialFen[i++] = fenRow;
		fenRow = strtok(NULL, SEP);
	}

	//For any row in partial FEN, direct to createRow function
	for (int i = 0; i < SIZE; i++) {
		createRow(partialFen[i], board[i]);
	}


}

/*************************************************************************************************
*	Function name: printBoard
*	Input: char board[][SIZE]
*	Output: None
*	Function Operation: this function prints the chess board which recived as 2D array.
*	the function use sub-functions: printColumns, printSpacers which described below in order
*	to create the visual chessboard frame. And the function pass on the 2D array given and print
*	the pieces and spaces in any row with separators between any square.
***************************************************************************************************/
void printBoard(char board[][SIZE]) {
	int rowIndex = SIZE;
	printColumns();
	printSpacers();

	//For loop which pass on the 2D array and the pieces and spaces.
	for (int i = 0; i < SIZE; i++) {
		printf("%d ", rowIndex);
		for (int j = 0; j < SIZE; j++) {
			printf("|%c", board[i][j]);
		}
		printf("| %d\n", rowIndex);
		rowIndex--;
	}
	printSpacers();
	printColumns();

}

/*************************************************************************************************
*	Function name: convertPieceChar
*	Input: Move move
*	Output: char move.srcPiece
*	Function Operation: this function recieved Struct Move. According to the color turn, return
*	the relevant source pieces char. capital case letters represent white pieces and lower case letters
*	represent black pieces.
***************************************************************************************************/
char convertPieceChar(Move move) {

	if (!move.isWhite) {
		return tolower(move.srcPiece);
	}
	return move.srcPiece;

}


//initialize move by parsing PGN


/*************************************************************************************************
*	Function name: initMove
*	Input: char board[][SIZE], char pgn[], int isWhiteTurn
*	Output: Move moveToCheck
*	Function Operation: this function initialize Struct Move according to PGN string which recived
*	and according to the turn color. the function use sub-functions which parse the infromation from
*	the PGN string. Then, when the Move is initialized, function send the move to additional
*	sub-function to check for optional piece which Meets Move conditions.
*	At the end, the initialized move return.
***************************************************************************************************/
Move initMove(char board[][SIZE], char pgn[], int isWhiteTurn) {

	Move initMove;

	// Default initialization as legal move
	initMove.isLegal = 1;

	// Define the color of turn
	if (isWhiteTurn) {
		initMove.isWhite = 1;
	}
	else {
		initMove.isWhite = 0;
	}

	// Define the source location on board (if exist) and the soruce piece type
	initMove = parseSrcFromPgn(pgn, initMove);

	// Define the destination location
	initMove = parseDestFromPgn(pgn, initMove);

	// Convert the source location on board to matrix indexs
	initMove = convertSrcMatrixIndex(initMove);

	// Convert the destination location on board to matrix indexs
	initMove = convertDestMatrixIndex(initMove);

	// Define the piece type which exist in destination location
	initMove.destPiece = findDestPiece(initMove.iDest, initMove.jDest, board);

	// Define move conditions such as: capture, check, mate, promotion
	initMove = parseConditionFromPgn(pgn, initMove);

	// Send the initialized Move to check for optional piece which Meets Move conditions.
	Move moveToCheck = findOptionalPieceByMove(board, initMove);

	return moveToCheck;
}

/*************************************************************************************************
*	Function name: parseSrcFromPgn
*	Input: char pgn[], Move move
*	Output: Move move
*	Function Operation: this function parse Source infromation from PGN and define it in Move.
*	(1)	function parse the source row of the piece by finding the relevant digit in PGN.
*	(2) function parse the source column of the piece by finding the relevant lower case letter in PGN.
*	(3) function parse the soruce piece type by finding the relevant capital case letter in PGN.
***************************************************************************************************/
Move parseSrcFromPgn(char pgn[], Move move) {

	// Varialbles which are used to count the chars to find the relevant row and column
	int srcRowCounter = 0;
	int srcColCounter = 0;

	// Varibale which are used as flags for row and column that has been found
	int srcRowFound = 0;
	int srcColFound = 0;

	// For loop which run on the length of the PGN string from the end
	for (int i = strlen(pgn) - 1; i >= 0; i--) {

		//If there is more than one digit, so the first digit represents the src row
		if (isdigit(pgn[i])) {
			srcRowCounter++;
			if (srcRowCounter > 1 && !srcRowFound) {
				move.srcRow = pgn[i];
				srcRowFound = 1;
			}
		}

		// If there is more than one lower case letter, so the first letter represents the src column
		if (pgn[i] >= 'a' && pgn[i] <= 'z' && pgn[i] != 'x') {
			srcColCounter++;
			if (srcColCounter > 1 && !srcColFound) {
				move.srcCol = pgn[i];
				srcColFound = 1;
			}
		}

	}

	if (!srcRowFound) {
		move.srcRow = '\0';
	}
	if (!srcColFound) {
		move.srcCol = '\0';
	}

	/*
	Checking the first char in PGN string.
	If there is char between A-Z , this char represents the source piece type
	If there is no capital char, so the relevant piece type is Pawn.
	*/
	if (pgn[0] >= 'A' && pgn[0] <= 'Z') {
		move.srcPiece = pgn[0];
	}
	else {
		move.srcPiece = 'P';
	}

	return move;

}

/*************************************************************************************************
*	Function name: parseDestFromPgn
*	Input: char pgn[], Move move
*	Output: Move move
*	Function Operation:
*	(1)	function parse the destination row of the piece by finding the relevant digit in PGN.
*	(2) function parse the destination column of the piece by finding the relevant lower case letter in PGN.
***************************************************************************************************/
Move parseDestFromPgn(char pgn[], Move move) {

	// Varibale which are used as flags for row and column that has been found
	int destRowFound = 0;
	int destColFound = 0;

	// For loop which run on the length of the PGN string from the end
	for (int i = strlen(pgn) - 1; i >= 0; i--) {
		//The first digit which found represents the destination row
		if (isdigit(pgn[i]) && !destRowFound) {
			move.destRow = pgn[i];
			destRowFound = 1;
		}
		//The first digit which found represents the destination column
		if (pgn[i] >= 'a' && pgn[i] <= 'z' && !destColFound) {
			move.destCol = pgn[i];
			destColFound = 1;
		}
	}

	return move;

}

/*************************************************************************************************
*	Function name: convertSrcMatrixIndex
*	Input: Move move
*	Output: Move move
*	Function Operation: the function converts the source row and column which initilaized in Move,
*	to the the appropriate index in 2D array. initialize the iSrc and jSrc in Move.
***************************************************************************************************/
Move convertSrcMatrixIndex(Move move) {

	//Default iSrc to use as flag for iSrc which not accepted
	move.iSrc = -1;

	/*
		If there is source row which was parsed from PGN,
		run on loop in SIZE length and convert it to the appropriate index in 2D array.
	*/
	if (move.srcRow != '\0') {
		int iSrc = SIZE - 1;
		for (int i = 1; i <= SIZE; i++) {
			if (toDigit(move.srcRow) == i) {
				move.iSrc = iSrc;
				break;
			}
			iSrc--;
		}
	}


	//Default jSrc to use as flag for jSrc which not accepted
	move.jSrc = -1;

	/*
		If there is source column which was parsed from PGN,
		run on loop in SIZE length and convert it to the appropriate index in 2D array.
	*/
	if (move.srcCol != '\0') {
		char jSrc = 'a';
		for (int i = 0; i < SIZE; i++) {
			if (move.srcCol == jSrc) {
				move.jSrc = i;
				break;
			}
			jSrc++;
		}
	}

	return move;
}

/*************************************************************************************************
*	Function name: convertDestMatrixIndex
*	Input: Move move
*	Output: Move move
*	Function Operation: the function converts the destination row and column which initilaized in Move,
*	to the the appropriate index in 2D array. initialize the iDest and jDest in Move.
***************************************************************************************************/
Move convertDestMatrixIndex(Move move) {


	int iDest = SIZE - 1;

	//For loop in SIZE length and convert the destination row to the appropriate index in 2D array.
	for (int i = 1; i <= SIZE; i++) {
		if (toDigit(move.destRow) == i) {
			move.iDest = iDest;
			break;
		}
		iDest--;
	}

	//For loop in SIZE length and convert the destination column to the appropriate index in 2D array.
	char jDest = 'a';
	for (int i = 0; i < SIZE; i++) {
		if (move.destCol == jDest) {
			move.jDest = i;
			break;
		}
		jDest++;
	}

	return move;
}

/*************************************************************************************************
*	Function name: parseConditionFromPgn
*	Input: char pgn[], Move move
*	Output: Move move
*	Function Operation: the function parses the information from PGN string about several conditions
*	such as capture, check, mate and promotion. any feature is initialized in Move.
***************************************************************************************************/
Move parseConditionFromPgn(char pgn[], Move move) {

	// Default initialization
	move.isCapture = 0;
	move.isCheck = 0;
	move.isMate = 0;
	move.isPromotion = 0;

	/*
		For loop which run on the PGN string length.
		If there is identification in PGN of condition sign
		so this feature is initialized in Move.
	*/
	for (int i = 0; i < strlen(pgn); i++) {
		if (pgn[i] == 'x') {
			move.isCapture = 1;
		}
		if (pgn[i] == '+') {
			move.isCheck = 1;
		}
		if (pgn[i] == '#') {
			move.isMate = 1;
		}
		if (pgn[i] == '=') {
			move.isPromotion = 1;
			move.promotionPiece = pgn[++i];;
		}
	}

	return move;
}

/*************************************************************************************************
*	Function name: findDestPiece
*	Input: int iDest, int jDest, char board[][SIZE]
*	Output: char destPiece
*	Function Operation: this function find the destination piece in the 2D array given according
*	to specific row and column which recieved.
***************************************************************************************************/
char findDestPiece(int iDest, int jDest, char board[][SIZE]) {
	char destPiece = board[iDest][jDest];
	return destPiece;
}



//Find optional source + tests


/*************************************************************************************************
*	Function name: findOptionalPieceByMove
*	Input: char board[][SIZE], Move move
*	Output: Move move
*	Function Operation: this function recieved initialized Move and according to the type of source
*	piece, try to find optional piece on board, which can make this Move.
*	Any type of piece has his appropriate way to locate His position on the board, in a way that it
*	will be able to arrive to the requested destination. When the optional piece is founded,
*	this option is sent to sub-function which check if the move is legal according to piece type.
***************************************************************************************************/
Move findOptionalPieceByMove(char board[][SIZE], Move move) {

	// Variables which used to hold the advance source row and source column, in case were provided in PGN
	int advRowSrc = move.iSrc;
	int advColSrc = move.jSrc;

	//Switch case according to source piece type
	switch (move.srcPiece) {

	/*
		In case of Rook.
		The Rook can move in straight lines along the columns or rows.
		There are two loops - one for rows and one for columns, with several
		iterations according to board SIZE value.
	*/
	case 'R': {

		Move rookMove = move;

		// Variable which holds the char which represents rook by color turn
		char rookChar = convertPieceChar(move);

		//For loop on rows and the column is constant
		for (int i = 0; i < SIZE; i++) {

			/*
				If there is square with Rook, send the optinal move to sub-function to check if legal.
				if this is legal Move, so the source row and column are defined.
				In case of two optional pieces were detected, there is advance information about the source row or column.
				So there is check if there is match between the row and column which founded.
			*/
			if (board[i][move.jDest] == rookChar && i != move.iDest) {
				rookMove = checkRookMove(board, move, i, move.jDest);
				if (rookMove.isLegal) {
					rookMove.iSrc = i;
					rookMove.jSrc = move.jDest;
					if ((advRowSrc >= 0 && rookMove.iSrc != advRowSrc) || (advColSrc >= 0 && rookMove.jSrc != advColSrc)) {
						continue;
					}
					return rookMove;
				}
			}
		}
		//For loop on columns and the row is constant
		for (int j = 0; j < SIZE; j++) {

			/*
			If there is square with Rook, send the optinal move to sub-function to check if legal.
			if this is legal Move, so the source row and column are defined.
			In case of two optional pieces were detected, there is advance information about the source row or column.
			So there is check if there is match between the row and column which founded.
			*/
			if (board[move.iDest][j] == rookChar && j != move.jDest) {
				rookMove = checkRookMove(board, move, move.iDest, j);
				if (rookMove.isLegal) {
					rookMove.iSrc = move.iDest;
					rookMove.jSrc = j;
					if ((advRowSrc >= 0 && rookMove.iSrc != advRowSrc) || (advColSrc >= 0 && rookMove.jSrc != advColSrc)) {
						continue;
					}
					return rookMove;
				}
			}
		}

		// In case no legal move detected, change rookMove.isLegal to 0 and return
		rookMove.isLegal = 0;
		return rookMove;

	}

	/*
		In case of Knight.
		The Knight can move in special movement that combines straight and diagonal steps,
		two and one or one and two in each direction.
		In order to locate the Knight, there is frame of squares, which include only the optional
		locations that the Knight can arrive from them to the requested destination.
		There are 2D array 5x5 which include the surrounding environment to the destination.
		And the cases which are checked are only in the relevant frame of squares.
	*/
	case 'N': {

		Move knightMove = move;

		// Variable which holds the char which represents Knight by color turn
		char knightChar = convertPieceChar(move);

		/*
			Variables which holds the values of the left corner in the relevant frame of squares
			according the surrounding environment to the destination.
		*/
		int rowIndex = move.iDest - 2;
		int colIndex = move.jDest - 2;

		// For loop which run on 2D array which represents surrounding environment
		for (int i = rowIndex; i <= rowIndex + 4; i++) {
			for (int j = colIndex; j <= colIndex + 4; j++)
			{
				//this is check that the indexs are not beyond the scope of board
				if (i >= 0 && i < SIZE && j >= 0 && j < SIZE)
				{
					// If block in order to check only the rows in frame of squares which relevant
					if (i == rowIndex || i == rowIndex + 4)
					{
						/*
						If there is square with Knight, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
						*/
						if (board[i][j] == knightChar)
						{
							knightMove = checkKnightMove(board, move, i, j);
							if (knightMove.isLegal) {
								knightMove.iSrc = i;
								knightMove.jSrc = j;
								if ((advRowSrc >= 0 && knightMove.iSrc != advRowSrc) || (advColSrc >= 0 && knightMove.jSrc != advColSrc)) {
									continue;
								}
								return knightMove;
							}
						}
					}
					// If block in order to check only the left columns in frame of squares which relevant
					else if (i != rowIndex && j == colIndex) {

						/*
						If there is square with Knight, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
						*/
						if (board[i][j] == knightChar)
						{
							knightMove = checkKnightMove(board, move, i, j);
							if (knightMove.isLegal) {
								knightMove.iSrc = i;
								knightMove.jSrc = j;
								if ((advRowSrc >= 0 && knightMove.iSrc != advRowSrc) || (advColSrc >= 0 && knightMove.jSrc != advColSrc)) {
									continue;
								}
								return knightMove;
							}
						}
					}
					// If block in order to check only the right columns in frame of squares which relevant
					else if (i != rowIndex && j == colIndex + 4) {

						/*
						If there is square with Knight, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
						*/
						if (board[i][j] == knightChar)
						{
							knightMove = checkKnightMove(board, move, i, j);
							if (knightMove.isLegal) {
								knightMove.iSrc = i;
								knightMove.jSrc = j;
								if ((advRowSrc >= 0 && knightMove.iSrc != advRowSrc) || (advColSrc >= 0 && knightMove.jSrc != advColSrc)) {
									continue;
								}
								return knightMove;
							}
						}
					}
				}
			}
		}

		// In case no legal move detected, change knightMove.isLegal to 0 and return
		knightMove.isLegal = 0;
		return knightMove;
	}

	/*
		In case of Bishop.
		The Bishop can move diagonally.
		There are two nested loops which pass on any square in order to locate Bishop pieces.
		In order to check if the optional piece which founded is relevant to Bishop movement,
		So we use in fabs() function. Check whether the difference between the source row and
		the destination row at absolute value equals the difference between the source column
		and the destination column.
	*/
	case 'B': {


		Move bishopMove = move;

		// Variable which holds the char which represents Bishop by color turn
		char bishopChar = convertPieceChar(move);

		//Two nested loops which pass on any square in order to locate Bishop
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {

				/*
					If there is square with Bishop, send the optinal move to sub-function to check if legal.
					if this is legal Move, so the source row and column are defined.
					In case of two optional pieces were detected, there is advance information about the source row or column.
					So there is check if there is match between the row and column which founded.
				*/
				if (board[i][j] == bishopChar)
				{
					//Checking if move diagonally by absolute values
					int isDiagonal = fabs(i - move.iDest) == fabs(j - move.jDest);
					if (isDiagonal) {
						bishopMove = checkBishopMove(board, move, i, j);
						if (bishopMove.isLegal) {
							bishopMove.iSrc = i;
							bishopMove.jSrc = j;
							if ((advRowSrc >= 0 && bishopMove.iSrc != advRowSrc) || (advColSrc >= 0 && bishopMove.jSrc != advColSrc)) {
								continue;
							}
							return bishopMove;
						}
					}
				}
			}
		}
		// In case no legal move detected, change bishopMove.isLegal to 0 and return
		bishopMove.isLegal = 0;
		return bishopMove;
	}

	/*
		In case of Queen.
		The Queen can move in any straight line. Column, row or diagonal.
		In case of row or column: There are two loops - one for rows and one for columns, with several
		iterations according to board SIZE value.
		In case of digonal:
		There are two nested loops which pass on any square.
		In order to check if the optional piece which founded is relevant to Queen diagonal movement,
		So we use in fabs() function. Check whether the difference between the source row and
		the destination row at absolute value equals the difference between the source column
		and the destination column.
	*/
	case 'Q': {

		Move queenMove = move;

		// Variable which holds the char which represents Bishop by color turn
		char queenChar = convertPieceChar(move);


		//Two nested loops which pass on any square in order to locate Queen
		for (int i = 0; i < SIZE; i++) {
			for (int j = 0; j < SIZE; j++) {


				/*
				If there is square with Queen, send the optinal move to sub-function to check if legal.
				if this is legal Move, so the source row and column are defined.
				In case of two optional pieces were detected, there is advance information about the source row or column.
				So there is check if there is match between the row and column which founded.
				*/
				if (board[i][j] == queenChar)
				{
					//If block to check column movement - check different rows and the column is constant
					if (j == move.jDest && i != move.iDest) {
						queenMove = checkQueenMove(board, move, i, move.jDest);
						if (queenMove.isLegal) {
							queenMove.iSrc = i;
							queenMove.jSrc = move.jDest;
							if ((advRowSrc >= 0 && queenMove.iSrc != advRowSrc) || (advColSrc >= 0 && queenMove.jSrc != advColSrc)) {
								continue;
							}
							return queenMove;
						}
					}
					//If block to check row movement - check different columns and the row is constant
					else if (i == move.iDest && j != move.jDest) {
						queenMove = checkQueenMove(board, move, move.iDest, j);
						if (queenMove.isLegal) {
							queenMove.iSrc = move.iDest;
							queenMove.jSrc = j;
							if ((advRowSrc >= 0 && queenMove.iSrc != advRowSrc) || (advColSrc >= 0 && queenMove.jSrc != advColSrc)) {
								continue;
							}
							return queenMove;
						}
					}
					else {
						//Checking if move diagonally by absolute value
						int isDiagonal = fabs(i - move.iDest) == fabs(j - move.jDest);
						if (isDiagonal) {
							queenMove = checkQueenMove(board, move, i, j);
							if (queenMove.isLegal) {
								queenMove.iSrc = i;
								queenMove.jSrc = j;
								if ((advRowSrc >= 0 && queenMove.iSrc != advRowSrc) || (advColSrc >= 0 && queenMove.jSrc != advColSrc)) {
									continue;
								}
								return queenMove;
							}
						}
					}
				}
			}
		}
		// In case no legal move detected, change queenMove.isLegal to 0 and return
		queenMove.isLegal = 0;
		return queenMove;
	}

	/*
		In case of King.
		The King can move one square anywhere in any direction.
		In order to locate the King, there is frame of squares, which include only the optional
		locations that the King can arrive from them to the requested destination.
		There are 2D array 3x3 which include the surrounding environment to the destination.
		And the cases which are checked are only in the relevant frame of squares.
	*/
	case 'K': {

		Move kingMove = move;

		// Variable which holds the char which represents King by color turn
		char kingChar = convertPieceChar(move);

		/*
			Variables which holds the values of the left corner in the relevant frame of squares
			according the surrounding environment to the destination.
		*/
		int rowIndex = move.iDest - 1;
		int colIndex = move.jDest - 1;

		// For loop which run on 2D array which represents surrounding environment
		for (int i = rowIndex; i <= rowIndex + 2; i++) {
			for (int j = colIndex; j <= colIndex + 2; j++) {

				//this is check that the indexs are not beyond the scope of board
				if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) {

					/*
						If there is square with King, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
					*/
					if (board[i][j] == kingChar) {
						kingMove = checkKingMove(board, move, i, j);
						if (kingMove.isLegal) {
							kingMove.iSrc = i;
							kingMove.jSrc = j;
							if ((advRowSrc >= 0 && kingMove.iSrc != advRowSrc) || (advColSrc >= 0 && kingMove.jSrc != advColSrc)) {
								continue;
							}
							return kingMove;
						}
					}
				}
			}
		}
		// In case no legal move detected, change kingMove.isLegal to 0 and return
		kingMove.isLegal = 0;
		return kingMove;
	}

	/*
		In case of Pawn.
		Pawn can only move one square at a time and only one way. In his first move on
		the second line, and only during that, he can move two squares forward or one square.
		Pawn can only move "forward", the opposite direction to the board where it starts.
		So there is two case of movements according to color turn.
		In order to locate the Pawn, there is frame of squares, which include only the optional
		locations that the Pawn can arrive from them to the requested destination.
		There are 2D array 2x3 which include the surrounding environment to the destination.
		And the cases which are checked are only in the relevant frame of squares.
	*/
	case 'P': {

		Move pawnMove = move;

		// Variable which holds the char which represents Pawn by color turn
		char pawnChar = convertPieceChar(move);

		// In case of white turn - forward means from top to bottom
		if (move.isWhite) {

			/*
			Variables which holds the values of the left corner in the relevant frame of squares
			according the surrounding environment to the destination.
			*/
			int rowIndex = move.iDest + 1;
			int colIndex = move.jDest - 1;

			// For loop which run on 2D array which represents surrounding environment
			for (int i = rowIndex; i <= rowIndex + 1; i++) {
				for (int j = colIndex; j <= colIndex + 2; j++) {

					//this is check that the indexs are not beyond the scope of board
					if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) {

						/*
						If there is square with Pawn, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
						*/
						if (board[i][j] == pawnChar) {
							pawnMove = checkPawnMove(board, move, i, j);
							if (pawnMove.isLegal) {
								pawnMove.iSrc = i;
								pawnMove.jSrc = j;
								if ((advRowSrc >= 0 && pawnMove.iSrc != advRowSrc) || (advColSrc >= 0 && pawnMove.jSrc != advColSrc)) {
									continue;
								}
								return pawnMove;
							}
						}
					}
				}
			}

		}

		// In case of black turn - forward means from bottom to top
		else {

			/*
				Variables which holds the values of the left corner in the relevant frame of squares
				according the surrounding environment to the destination.
			*/
			int rowIndex = move.iDest - 2;
			int colIndex = move.jDest - 1;

			// For loop which run on 2D array which represents surrounding environment
			for (int i = rowIndex; i <= rowIndex + 1; i++) {
				for (int j = colIndex; j <= colIndex + 2; j++) {

					//this is check that the indexs are not beyond the scope of board
					if (i >= 0 && i < SIZE && j >= 0 && j < SIZE) {

						/*
						If there is square with Pawn, send the optinal move to sub-function to check if legal.
						if this is legal Move, so the source row and column are defined.
						In case of two optional pieces were detected, there is advance information about the source row or column.
						So there is check if there is match between the row and column which founded.
						*/
						if (board[i][j] == pawnChar) {
							pawnMove = checkPawnMove(board, move, i, j);
							if (pawnMove.isLegal) {
								pawnMove.iSrc = i;
								pawnMove.jSrc = j;
								if ((advRowSrc >= 0 && pawnMove.iSrc != advRowSrc) || (advColSrc >= 0 && pawnMove.jSrc != advColSrc)) {
									continue;
								}
								return pawnMove;
							}
						}
					}
				}
			}
		}

		// In case no legal move detected, change pawnMove.isLegal to 0 and return
		pawnMove.isLegal = 0;
		return pawnMove;
	}
	}

	//In any case that no case chosen, change move.isLegal to 0 and return
	move.isLegal = 0;
	return move;


}

/*************************************************************************************************
*	Function name: checkRookMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of Rook piece is legal according to its rules. such as: clear way to destination, type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkRookMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {


	/*
		Clear way test:
		The Rook can move in straight lines along the columns or rows.
		So we need to check in any row and column between source to destination
		if there is piece which block the requested move.
		if the way is not clear, change move.isLegal to 0 and return.
	*/

	/*
		Checking clear way on row.
		In order to define the loop parameters, we check the value of destination row index
		and source row index.
	*/
	if (move.iDest == iOptSrc) {
		if (jOptSrc > move.jDest) {
			for (int j = move.jDest + 1; j < jOptSrc; j++) {
				//this is check that the indexs are not beyond the scope of board
				if (j >= 0 && j < SIZE) {
					if (board[move.iDest][j] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}
		}
		else {
			for (int j = jOptSrc + 1; j < move.jDest; j++) {
				//this is check that the indexs are not beyond the scope of board
				if (j >= 0 && j < SIZE) {
					if (board[move.iDest][j] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}

		}

	}
	/*
	Checking clear way on column.
	In order to define the loop parameters, we check the value of destination column index
	and source column index.
	*/
	else if (move.jDest == jOptSrc) {
		if (iOptSrc > move.iDest) {
			for (int i = move.iDest + 1; i < iOptSrc; i++) {
				if (i >= 0 && i < SIZE) {
					if (board[i][move.jDest] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}
		}
		else {
			for (int i = iOptSrc + 1; i < move.iDest; i++) {
				if (i >= 0 && i < SIZE) {
					if (board[i][move.jDest] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}

		}

	}

	/*
		Movement test:
		The Rook can move in straight lines along the columns or rows.
		So in case the rows of source and destination are differnet and in case
		the columns of source and destination are differnet. it means that the move
		is ilegal and change move.isLegal to 0 and return.
	*/
	if (move.iDest != iOptSrc && move.jDest != jOptSrc) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;
}

/*************************************************************************************************
*	Function name: checkKnightMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of Knight piece is legal according to his rules. such as: type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkKnightMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {

	/*
		Movement test:
		The Knight can move in special movement that combines straight and diagonal steps,
		two and one or one and two in each direction.
		So we use in fabs() function. Check whether the difference between the source row and
		the destination row at absolute value and the difference between the source column
		and the destination column at absolute value are 2 and 1, according to Knight legal steps.
		If the values are not match to his rules, change move.isLegal to 0 and return.
	*/
	int rowL = (fabs(move.iDest - iOptSrc) == 2 && fabs(move.jDest - jOptSrc) == 1);
	int colL = (fabs(move.iDest - iOptSrc) == 1 && fabs(move.jDest - jOptSrc) == 2);
	if (!rowL && !colL) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;


}

/*************************************************************************************************
*	Function name: checkBishopMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of Bishop piece is legal according to its rules. such as: clear way to destination, type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkBishopMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {

	/*
		Movement test:
		The Bishop can move diagonally.
		So we use in fabs() function. Check whether the difference between the source row and
		the destination row at absolute value equals the difference between the source column
		and the destination column. if the values not equal it means that Move is ilegal
		and change move.isLegal to 0 and return.
	*/
	int isDiagonal = fabs(iOptSrc - move.iDest) == fabs(jOptSrc - move.jDest);
	if (!isDiagonal) {
		move.isLegal = 0;
		return move;
	}

	// Variable which holds the char which represents Bishop by color turn
	char bishopChar = convertPieceChar(move);


	/*
		Clear way test:
		The Bishop can move diagonally.
		So we need to check in any diagonal line between source to destination
		if there is piece which block the requested move.
		We need to check diagonal lines in any direction: top right, top left, bottom right
		and bottom left. So there are several cases according to source and destionation values.
		if the way is not clear, change move.isLegal to 0 and return.
	*/

	int row = iOptSrc;
	int column = jOptSrc;

	// Check clear diagonal line in direction: top left
	if (iOptSrc < move.iDest && jOptSrc < move.jDest) {

		//While loop until run all on squares between source and destination
		row++;
		column++;
		while (!(row == move.iDest) && !(column == move.jDest)) {

			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {

				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
				row++;
				column++;
			}
		}
	}
	// Check clear diagonal line in direction: bottom left
	else if (iOptSrc > move.iDest && jOptSrc < move.jDest) {
		row--;
		column++;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {

			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row--;
			column++;
		}
	}
	// Check clear diagonal line in direction: top right
	else if (iOptSrc < move.iDest && jOptSrc > move.jDest) {
		row++;
		column--;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {

			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row++;
			column--;
		}
	}
	// Check clear diagonal line in direction: bottom left
	else if (iOptSrc > move.iDest&& jOptSrc > move.jDest) {
		row--;
		column--;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {

			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row--;
			column--;
		}
	}

	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;
}

/*************************************************************************************************
*	Function name: checkQueenMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of Queen piece is legal according to its rules. such as: clear way to destination, type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkQueenMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {

	/*
		Movement test:
		The Queen can move in any straight line. Column, row or diagonal.
		In case of row or column: we need to check in any row and column between source to destination
		if there is piece which block the requested move.
		In case of digonal: we use in fabs() function. Check whether the difference between the source
		row and the destination row at absolute value equals the difference between the source column
		and the destination column. if there both types of move not legal, change move.isLegal to 0 and return.
	*/
	int IllegalMovementCounter = 0;
	if (move.iDest != iOptSrc && move.jDest != jOptSrc) {
		IllegalMovementCounter++;
	}
	int isDiagonal = fabs(iOptSrc - move.iDest) == fabs(jOptSrc - move.jDest);
	if (!isDiagonal) {
		IllegalMovementCounter++;
	}
	if (IllegalMovementCounter == 2) {
		move.isLegal = 0;
		return move;
	}

	// Variable which holds the char which represents Bishop by color turn
	char queenChar = convertPieceChar(move);

	/*
		Clear way test:
		The Queen can move in any straight line. Column, row or diagonal.
		So we need to check in any row and column between source to destination
		if there is piece which block the requested move.
		And we need to check in any diagonal line between source to destination
		if there is piece which block the requested move.
		We need to check diagonal lines in any direction: top right, top left, bottom right
		and bottom left. So there are several cases according to source and destionation values.
		if the way is not clear, change move.isLegal to 0 and return.
	*/

	int row = iOptSrc;
	int column = jOptSrc;

	/* Checking straight line block */


	/*
		Checking clear way on row.
		In order to define the loop parameters, we check the value of destination row index
		and source row index.
	*/
	if (move.iDest == iOptSrc) {
		if (jOptSrc > move.jDest) {
			for (int j = move.jDest + 1; j < jOptSrc; j++) {
				//this is check that the indexs are not beyond the scope of board
				if (j >= 0 && j < SIZE) {
					if (board[move.iDest][j] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}
		}
		else {
			for (int j = jOptSrc + 1; j < move.jDest; j++) {
				//this is check that the indexs are not beyond the scope of board
				if (j >= 0 && j < SIZE) {
					if (board[move.iDest][j] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}

		}

	}
	/*
	Checking clear way on column.
	In order to define the loop parameters, we check the value of destination column index
	and source column index.
	*/
	else if (move.jDest == jOptSrc) {
		if (iOptSrc > move.iDest) {
			for (int i = move.iDest + 1; i < iOptSrc; i++) {
				//this is check that the indexs are not beyond the scope of board
				if (i >= 0 && i < SIZE) {
					if (board[i][move.jDest] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}
		}
		else {
			for (int i = iOptSrc + 1; i < move.iDest; i++) {
				//this is check that the indexs are not beyond the scope of board
				if (i >= 0 && i < SIZE) {
					if (board[i][move.jDest] != EMPTY) {
						move.isLegal = 0;
						return move;
					}
				}
			}

		}

	}


	/* Checking diagonal line block */

	// Check clear diagonal line in direction: top left
	if (iOptSrc < move.iDest && jOptSrc < move.jDest) {
		row++;
		column++;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {
			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row++;
			column++;
		}
	}

	// Check clear diagonal line in direction: bottom left
	else if (iOptSrc > move.iDest&& jOptSrc < move.jDest) {
		row--;
		column++;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {
			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row--;
			column++;
		}
	}
	// Check clear diagonal line in direction: top right
	else if (iOptSrc < move.iDest && jOptSrc > move.jDest) {
		row++;
		column--;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {
			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row++;
			column--;
		}
	}
	// Check clear diagonal line in direction: bottom left
	else if (iOptSrc > move.iDest&& jOptSrc > move.jDest) {
		row--;
		column--;
		//While loop until run all on squares between source and destination
		while (!(row == move.iDest) && !(column == move.jDest)) {
			//this is check that the indexs are not beyond the scope of board
			if (row >= 0 && row < SIZE && column >= 0 && column < SIZE) {
				if (board[row][column] != EMPTY) {
					move.isLegal = 0;
					return move;
				}
			}
			row--;
			column--;
		}
	}


	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;

}

/*************************************************************************************************
*	Function name: checkKingMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of King piece is legal according to its rules. such as: type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkKingMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {

	/*
		Movement test:
		The King can move one square anywhere in any direction.
		So we use in fabs() function. Check whether the difference between the source row and
		the destination row at absolute value and the difference between the source column
		and the destination column at absolute value are 0 or 1, according to King legal steps.
		If the values are not match to his rules, change move.isLegal to 0 and return.
	*/
	int rowCheck = (fabs(move.iDest - iOptSrc) == 0 || fabs(move.iDest - iOptSrc) == 1);
	int colCheck = (fabs(move.jDest - jOptSrc) == 0 || fabs(move.jDest - jOptSrc) == 1);
	if (!rowCheck || !colCheck) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;
}

/*************************************************************************************************
*	Function name: checkPawnMove
*	Input: char board[][SIZE], Move move, int iOptSrc, int jOptSrc
*	Output: Move move
*	Function Operation: this function check several condition in order to check if optional move
*	of Pawn piece is legal according to its rules. such as: clear way to destination, type of movement,
*	capture declaration without trial, destination which same color piece already located,
*	and capture trial without declaration. some of tests are using the optional source row and column
*	whihc recived from findOptionalPieceByMove function.
***************************************************************************************************/
Move checkPawnMove(char board[][SIZE], Move move, int iOptSrc, int jOptSrc) {

	/*
		Movement test:
		Pawn can only move one square at a time and only one way. In his first move on
		the second line, and only during that, he can move two squares forward or one square.
		Pawn can only move "forward", the opposite direction to the board where it starts.
		So there is two case of movements according to color turn.
		In addition, Pawn move in different way when it captured, it move diagonally.
		Moreover, there is special move to Pawn. When it arrives to the edge line on board,
		there is Promotion and type of piece is changed.
	*/


	// In case of white turn - forward means from top to bottom
	if (move.isWhite) {

		//check if there is Promotion
		if (move.iDest == 0) {
			if (!move.isPromotion) {
				move.isLegal = 0;
				return move;
			}
		}

		//In case of no capture - only forward steps available
		if (!move.isCapture)
		{
			//Checking forward movement - from top to bottom
			if (move.iDest > iOptSrc || jOptSrc != move.jDest) {
				move.isLegal = 0;
				return move;
			}

			//In case of second line, check if he try to move 2 steps or 1 steps
			if (iOptSrc == SIZE - 2) {
				if (iOptSrc - move.iDest > 2 || jOptSrc != move.jDest) {
					move.isLegal = 0;
					return move;
				}
			}
			else {
				//In any other line, check if he try to move 1 steps only
				if (iOptSrc - move.iDest > 1 || jOptSrc != move.jDest) {
					move.isLegal = 0;
					return move;
				}
			}
		}

		else {

			//In case of capture - only digonal steps available
			int iDiag = (move.iDest == iOptSrc - 1);
			int jDiag = (move.jDest == jOptSrc + 1 || move.jDest == jOptSrc - 1);
			if (!iDiag || !jDiag) {
				move.isLegal = 0;
				return move;
			}
		}

	}

	// In case of black turn - forward means from bottom to top
	else if (!move.isWhite) {

		//Check if there is Promotion
		if (move.iDest == SIZE - 1) {
			if (!move.isPromotion) {
				move.isLegal = 0;
				return move;
			}
		}

		//In case of no capture - only forward steps available
		if (!move.isCapture)
		{
			//Checking forward movement - from bottom to top
			if (move.iDest < iOptSrc) {
				move.isLegal = 0;
				return move;
			}

			//In case of second line, check if he try to move 2 steps or 1 steps
			if (iOptSrc == 1) {
				if (move.iDest - iOptSrc > 2 || jOptSrc != move.jDest) {
					move.isLegal = 0;
					return move;
				}
			}
			//In any other line, check if he try to move 1 steps only
			else {
				if (move.iDest - iOptSrc > 1 || jOptSrc != move.jDest) {
					move.isLegal = 0;
					return move;
				}
			}
		}

		else {

			//In case of capture - only digonal steps available
			int iDiag = (move.iDest == iOptSrc + 1);
			int jDiag = (move.jDest == jOptSrc + 1 || move.jDest == jOptSrc - 1);
			if (!iDiag || !jDiag) {
				move.isLegal = 0;
				return move;
			}
		}
	}

	/*
		Clear way test:
		In case that Pawm move 2 steps, there is option that there is piece which block it.
		So we need to check in any row and column between source to destination
		if there is piece which block the requested move.
		if the way is not clear, change move.isLegal to 0 and return.
	*/
	if (move.isWhite) {
		if (iOptSrc == SIZE - 2 && move.iDest == SIZE - 4) {
			if (board[SIZE - 3][move.jDest] != EMPTY) {
				move.isLegal = 0;
				return move;
			}
		}
	}
	else if (!move.isWhite) {
		if (iOptSrc == 1 && move.iDest == 3) {
			if (board[2][move.jDest] != EMPTY) {
				move.isLegal = 0;
				return move;
			}
		}
	}

	/*
		noCaptureDestTest:
		In case of capture declaration without trial, cause the destionation is empty.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.

	*/
	if (noCaptureDestTest(move.isCapture, move.destPiece)) {
		move.isLegal = 0;
		return move;
	}

	/*
		sameColorPieceTest:
		In case of destination which same color piece already located.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (sameColorPieceTest(move.destPiece, move.isWhite)) {
		move.isLegal = 0;
		return move;
	}

	/*
		noCaptureDeclareTest:
		In case of capture trial without declaration.
		change move.isLegal to 0 and return.
		Using sub-function, details about it near function implementation.
	*/
	if (noCaptureDeclareTest(move.destPiece, move.isWhite, move.isCapture)) {
		move.isLegal = 0;
		return move;
	}

	return move;
}


/*************************************************************************************************
*	Function name: isWhiteDest
*	Input: char destPiece
*	Output: int (0 or 1)
*	Function Operation: this function recieved the destination piece char from which was parsed from
*	the board 2D array. if this is lower case so it means that this is black piece, and 0 return.
*	if this is capital case so it means that this is white piece and 1 return.
***************************************************************************************************/
int isWhiteDest(char destPiece) {
	if (destPiece >= 'a' && destPiece <= 'z') {
		return 0;
	}
	if (destPiece >= 'A' && destPiece <= 'Z') {
		return 1;
	}
}

/*************************************************************************************************
*	Function name: sameColorPieceTest
*	Input: char destPiece, int isWhite
*	Output: int (0 or 1)
*	Function Operation: this function checks if in destination square, there is piece in the same
*	color of the player turn. this function will be used for checking legal move. By chess rules,
*	piece with specific color type can't move and capture another piece with same color in destination.
*	This function use sub-function isWhiteDest() which described above.
*	If there is piece with same color in destination - retrun 1
*	If there is no piece with same color in destination - retrun 0
***************************************************************************************************/
int sameColorPieceTest(char destPiece, int isWhite) {

	if (destPiece != EMPTY) {

		if (isWhite) {
			if (isWhiteDest(destPiece)) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if (!isWhiteDest(destPiece)) {
				return 1;
			}
			else {
				return 0;
			}

		}

	}

	return 0;

}

/*************************************************************************************************
*	Function name: noCaptureDestTest
*	Input: int isCapture, char destPiece
*	Output: int (0 or 1)
*	Function Operation: this function recieved flag if there is capture declaration and the kind
*	of piece which is located in destination. It may be that there is no piece in destination and it
*	is empty. In this case, the Move is ilegal. cause there is declaration on capture but there is no
*	piece in destination to be captured.
*	If there is capture declarartion but there is no piece in dest - return 1
*	else, if there is no capture declaration or there is piece in dest - return 0
***************************************************************************************************/
int noCaptureDestTest(int isCapture, char destPiece) {

	if (isCapture && destPiece == EMPTY) {
		return 1;
	}
	else {
		return 0;
	}
}

/*************************************************************************************************
*	Function name: noCaptureDeclareTest
*	Input: char destPiece, int isWhite, int isCapture
*	Output: int (0 or 1)
*	Function Operation: this function recieved flag if there is capture declaration, the color of turn
*	and the kind of piece which is located in destination. if the destination is not empty, and the
*	piece in destination is different from turn color - so capture need to be declared.
*	If there is no capture declaration - the move is ilegal.
*	If there is capture trial without declaration - return 1
*	Else if there is no capture or there is declaration - return 0
***************************************************************************************************/
int noCaptureDeclareTest(char destPiece, int isWhite, int isCapture) {

	if (destPiece != EMPTY) {

		if (isWhite) {
			if (!isWhiteDest(destPiece) && !(isCapture)) {
				return 1;
			}
			else {
				return 0;
			}
		}
		else {
			if (isWhiteDest(destPiece) && !(isCapture)) {
				return 1;
			}
			else {
				return 0;
			}

		}

	}

	return 0;
}


// Check tests

/*************************************************************************************************
*	Function name: testCheckConditions
*	Input: char board[][SIZE], Move move
*	Output: Move testCheckMove
*	Function Operation: this function gathers all the tests that need to be checked in check
*	situation. this function use sub-functions which will be described below.
*	Description about tests:
*	-In case of check trial without declaration, change testCheckMove.isLegal to 0 and return
*	-In case of check declaration without trial, change testCheckMove.isLegal to 0 and return
*	-In case of perfroming move leads to check threat, change testCheckMove.isLegal to 0 and return
*	-In case of current check situation, that illegal move try to be done, change testCheckMove.isLegal
* 	to 0 and return.
*	- In any other case, return change the original testCheckMove
***************************************************************************************************/
Move testCheckConditions(char board[][SIZE], Move move) {

	Move testCheckMove = move;

	if (checkTrialWithoutDeclare(board, testCheckMove, testCheckMove.isWhite, !testCheckMove.isWhite)) {
		testCheckMove.isLegal = 0;
		return testCheckMove;
	}

	if (checkDeclareWithoutTrial(board, testCheckMove, testCheckMove.isWhite, !testCheckMove.isWhite)) {
		testCheckMove.isLegal = 0;
		return testCheckMove;
	}

	if (moveCauseToCheckThreat(board, testCheckMove, !testCheckMove.isWhite, testCheckMove.isWhite)) {
		testCheckMove.isLegal = 0;
		return testCheckMove;
	}

	if (limitedMoveInCheckCase(board, testCheckMove, !testCheckMove.isWhite, testCheckMove.isWhite)) {
		testCheckMove.isLegal = 0;
		return testCheckMove;
	}

	return testCheckMove;

}

/*************************************************************************************************
*	Function name: isCheckCase
*	Input: char board[][SIZE], int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation: this function receives the currnt board as 2D array, flag for color of turn,
*	and flag for color to the threatened side. This function if there is any piece on board
*	that can make legal move and to capture the king. if there is Move which found, it means that there
*	is threat to king and this is check situation. If this is check case, return 1. if there is not
*	check case, return 0.
***************************************************************************************************/
int isCheckCase(char board[][SIZE], int isWhiteMove, int isTheratToWhite) {

	//Variable which represents the king which may be threatened
	char kingChar;

	Move optionalMove;
	Move updatedMove;

	/*
		Array which include all the optional piece which may make
		legal move and to capture the king.
	*/
	char optionalSrcPiece[] = { 'R', 'B', 'N', 'Q', 'K','P' };


	// Initialize the king char according to threatened side color
	if (isTheratToWhite) {
		kingChar = KING;
	}
	else {
		kingChar = tolower(KING);
	}

	//default optionalMove initialization
	optionalMove.destPiece = kingChar;
	optionalMove.isWhite = isWhiteMove;
	optionalMove.isCapture = 1;

	/*
		For loop on the 2D array board, to locate any optional king.
		when there is optional threatened king, there is another for loop which
		pass on any optional piece that may threat on the king.
		There is using findOptionalPieceByMove() in order to check if there is any
		legal move that may be made in order to capture the king.
		If there is legal move which found, it means that the board is in a check situation.
	*/
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			if (board[i][j] == kingChar) {
				for (int z = 0; z < sizeof(optionalSrcPiece); z++) {
					optionalMove.iDest = i;
					optionalMove.jDest = j;
					optionalMove.iSrc = -1;
					optionalMove.jSrc = -1;
					optionalMove.srcPiece = optionalSrcPiece[z];
					updatedMove = findOptionalPieceByMove(board, optionalMove);
					if (updatedMove.isLegal) {
						return 1;
					}
				}
			}
		}
	}

	return 0;
}

/*************************************************************************************************
*	Function name: testBoardCheck
*	Input: char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation: this function recieves current board, Move need to be checked, flag for color
*	of turn, and flag for color to the threatened side. this function creat copy of the original board.
*	Then the function perform the required move that give on the copied board. And then, there is using
*	isCheckCase() function in order to check if the required move leads to check situation.
*	If there is check case on the copied board - return 1
*	If there is no check case on the copied board - return 0
***************************************************************************************************/
int testBoardCheck(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite) {

	char copiedBoard[SIZE][SIZE];
	char promotionPieceChar;
	char srcPieceChar;

	// Copy the current board to new copied board
	for (int i = 0; i < SIZE; i++) {
		for (int j = 0; j < SIZE; j++) {
			copiedBoard[i][j] = board[i][j];
		}
	}

	// In case of white turn - define capital case the soruce piece char
	if (move.isWhite) {
		srcPieceChar = move.srcPiece;

		//If there is promotion - define capital case for promotion piece char
		if (move.isPromotion) {
			promotionPieceChar = move.promotionPiece;
		}

	}
	// In case of black turn - define lower case the soruce piece char
	else if (!move.isWhite) {
		srcPieceChar = tolower(move.srcPiece);

		//If there is promotion - define lower case for promotion piece char
		if (move.isPromotion) {
			promotionPieceChar = tolower(move.promotionPiece);

		}
	}


	//In case of Promotion, change the destination to hold the promotion piece char
	if (move.isPromotion) {
		copiedBoard[move.iDest][move.jDest] = promotionPieceChar;
	}

	//In case with no Promotion, change the destination to hold the source piece char
	else {
		copiedBoard[move.iDest][move.jDest] = srcPieceChar;
	}

	// Change the source location to be empty
	copiedBoard[move.iSrc][move.jSrc] = EMPTY;

	/*
		Send the copied board to isCheckCase() function in order
		to check if after performing move there is check situation.
	*/
	if (isCheckCase(copiedBoard, isWhiteMove, isTheratToWhite)) {
		return 1;
	}

	return 0;

}

/*************************************************************************************************
*	Function name: checkTrialWithoutDeclare
*	Input: char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation: this function check if there is check trial without declaration.
*	There is using in function testBoardCheck() which perfrom the move, and check if there is check
*	case in the new board position.
*	If there is check trial without declaration - return 1
*	If there is no check trial or there is declaration - return 0
***************************************************************************************************/
int checkTrialWithoutDeclare(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite) {

	int isCheckAfterMove = testBoardCheck(board, move, isWhiteMove, isTheratToWhite);
	if (!move.isCheck && !move.isMate && isCheckAfterMove) {
		return 1;
	}
	return 0;
}

/*************************************************************************************************
*	Function name: checkDeclareWithoutTrial
*	Input: char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation:  this function check if there is check declarattion without trial.
*	There is using in function testBoardCheck() which perfrom the move, and check if there is check
*	case in the new board position.
*	If there is check declaration without trial - return 1
*	If there is no check declaration or there is check trial - return 0
***************************************************************************************************/
int checkDeclareWithoutTrial(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite) {

	int isCheckAfterMove = testBoardCheck(board, move, isWhiteMove, isTheratToWhite);
	if ((move.isCheck || move.isMate) && !isCheckAfterMove) {
		return 1;
	}
	return 0;
}

/*************************************************************************************************
*	Function name: moveCauseToCheckThreat
*	Input: char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation: this function check if the move cause to check threat to the player side color.
*	According to chess rules, player can't make move that leads to a capture threat on his king.
*	There is using in function testBoardCheck() which perfrom the move, and check if there is check
*	case in the new board position.
*	If the move leads to check case on the player which its his trun - return 1
*	Id the move does not lead to check case - return 0
***************************************************************************************************/
int moveCauseToCheckThreat(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite) {


	int isCheckAfterMove = testBoardCheck(board, move, isWhiteMove, isTheratToWhite);
	if (isCheckAfterMove)
	{
		return 1;
	}
	return 0;
}

/*************************************************************************************************
*	Function name: limitedMoveInCheckCase
*	Input: char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite
*	Output: int (0 or 1)
*	Function Operation: this function check at first the current borad before perfroming the requested
*	move. If there is check situation on the color turn, it means that there are specific moves which
*	can be made in order to prevent king capture.
*	- Moving the king.
*	- Blocking the offensive line of the threatening piece.
*	- capturing of the threatening piece
*	Any other move is ilegal.
*	At first, there is using isCheckCase() function to check the current board.
*	If the board is already in check case, there is using in testBoardCheck() function which check
*	after performing the move, if the board is in check case.
*	If after the move, stiil there is check situation - it means that the move didnt prevent the
*	check threat and the move and return 1.
*	If there was no check situation on the original board, or the move prevented check
*	situation - return 0.
***************************************************************************************************/
int limitedMoveInCheckCase(char board[][SIZE], Move move, int isWhiteMove, int isTheratToWhite) {

	int isCheckAlready = isCheckCase(board, isWhiteMove, isTheratToWhite);
	if (isCheckAlready) {
		int isStillCheck = testBoardCheck(board, move, isWhiteMove, isTheratToWhite);
		if (isStillCheck) {
			return 1;
		}
		else {
			return 0;
		}
	}

	return 0;
}


// make move and perfrom change on board

/*************************************************************************************************
*	Function name: performMove
*	Input: char board[][SIZE], Move move
*	Output: None
*	Function Operation: this function recieves current board and Move that need to be performed
*	on the board. This function check which piece need to be located in the destination and remove
*	the piece from his source.
***************************************************************************************************/
void performMove(char board[][SIZE], Move move) {

	char promotionPieceChar;
	char srcPieceChar;

	// In case of white turn - define capital case the soruce piece char
	if (move.isWhite) {
		srcPieceChar = move.srcPiece;

		//If there is promotion - define capital case for promotion piece char
		if (move.isPromotion) {
			promotionPieceChar = move.promotionPiece;
		}

	}
	// In case of black turn - define lower case the soruce piece char
	else if (!move.isWhite) {
		srcPieceChar = tolower(move.srcPiece);
		//If there is promotion - define lower case for promotion piece char
		if (move.isPromotion) {
			promotionPieceChar = tolower(move.promotionPiece);

		}
	}

	//In case of Promotion, change the destination to hold the promotion piece char
	if (move.isPromotion) {
		board[move.iDest][move.jDest] = promotionPieceChar;
	}
	//In case with no Promotion, change the destination to hold the source piece char
	else {
		board[move.iDest][move.jDest] = srcPieceChar;
	}
	// Change the source location to be empty
	board[move.iSrc][move.jSrc] = EMPTY;
}

/*************************************************************************************************
*	Function name: makeMove
*	Input: char board[][SIZE], char pgn[], int isWhiteTurn
*	Output: int (0 or 1)
*	Function Operation: this function recieves board as 2D array, String of PGN and color turn.
	(1) At first, there is initialize of Move by using initMove() function which parse the infromation
		from PGN and look for optional move on board.
	(2) Then, there is testing of check conidtions on the current board after perfroming the initialized
		Move which back from initMove() function. the tests are done by testCheckConditions().
	(3) If the move which back from initMove() and from testCheckConditions() is legal, perform move
		on the board by using performMove() function and return 1. If the move is ilegal return 0.
***************************************************************************************************/
int makeMove(char board[][SIZE], char pgn[], int isWhiteTurn) {

	Move move = initMove(board, pgn, isWhiteTurn);

	if (move.isLegal) {
		move = testCheckConditions(board, move);
	}

	if (move.isLegal) {
		performMove(board, move);
		return 1;
	}
	return 0;
}
