#ifndef TETRIS_PIECES_H
#define TETRIS_PIECES_H

#include "displaySettings.h"

#define PIECE_COUNT 7
#define PIECE_ROWS 4
#define PIECE_COLUMNS 4
#define PIECE_POINTS (PIECE_ROWS * PIECE_COLUMNS)
#define PIECE_BLOCKS_SIZE 4

typedef struct {
    int n;
    uint8_t matrix[4][4];
    TColorIdx colorIdx;
    int x;
    int y;
} TPiece;

typedef enum {
	DIR_UP,
	DIR_DOWN,
	DIR_LEFT,
	DIR_RIGHT
} TDirection;

void TetrisRotateCW(TPiece *piece);
void TetrisRotateCCW(TPiece *piece);
void TetrisNew(TPiece *piece);
void TetrisDraw(TPiece *piece);
void TetrisMove(TPiece *piece, TDirection dir);
bool TetrisPieceHit(TPiece *piece);

#endif

