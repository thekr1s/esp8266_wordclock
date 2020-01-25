#include <stdlib.h>
#include <string.h>
#include "tetris_pieces.h"
#include "font.h"
#include "AddressableLedStrip.h"
#include "settings.h"

TPiece i = {0, 

    {{0b0000,
      0b1111,
      0b0000,
      0b0000},

	 {0b0010,
	  0b0010,
	  0b0010,
	  0b0010},

	 {0b0000,
	  0b1111,
	  0b0000,
	  0b0000},

     {0b0010,
      0b0010,
      0b0010,
      0b0010}}

, COLOR_LIGHT_BLUE, 0, 0};



TPiece j = {0,

	    {{0b1000,
	      0b1110,
	      0b0000,
	      0b0000},

	     {0b0110,
	      0b0100,
	      0b0100,
	      0b0000},

	     {0b0000,
	      0b1110,
	      0b0010,
	      0b0000},

	     {0b0100,
	      0b0100,
	      0b1100,
	      0b0000}}
, COLOR_RED, 0, 0};


TPiece l = {0,

	   {{0b0000,
		 0b0010,
	     0b1110,
	     0b0000},

	    {0b0100,
	     0b0100,
	     0b0110,
	     0b0000},

	    {0b0000,
	     0b1110,
	     0b1000,
	     0b0000},

	    {0b0110,
	     0b0010,
	     0b0010,
	     0b0000}}

, COLOR_ORANGE, 0, 0};

TPiece o = {0,

	   {{0b0110,
		 0b0110,
		 0b0000,
		 0b0000},

		{0b0110,
		 0b0110,
		 0b0000,
		 0b0000},

		{0b0110,
		 0b0110,
		 0b0000,
		 0b0000},

		{0b0110,
		 0b0110,
		 0b0000,
		 0b0000}}

, COLOR_MAGENTHA, 0, 0};

TPiece s = {0,

		  {{0b0110,
		    0b1100,
		    0b0000,
		    0b0000},

		   {0b0100,
		    0b0110,
		    0b0010,
		    0b0000},

		   {0b0000,
		    0b0110,
		    0b1100,
		    0b0000},

		   {0b1000,
		    0b1100,
		    0b0100,
		    0b0000}}

, COLOR_BIT_PURPLE, 0, 0};

TPiece t = {0,

		  {{0b0100,
		    0b1110,
		    0b0000,
		    0b0000},

		   {0b0100,
		    0b0110,
		    0b0100,
		    0b0000},

		   {0b1110,
		    0b0100,
		    0b0000,
			0b0000},

		   {0b0010,
		    0b0110,
		    0b0010,
		    0b0000}}
, COLOR_GREEN, 0, 0};

TPiece z = {0,

		  {{0b1100,
		    0b0110,
		    0b0000,
		    0b0000},

		   {0b0010,
		    0b0110,
		    0b0100,
		    0b0000},
		   
		   {0b1100,
		    0b0110,
		    0b0000,
		    0b0000},

		   {0b0010,
		    0b0110,
		    0b0100,
		    0b0000}}


, COLOR_BLUE, 0, 0};

TPiece *piece_list[PIECE_COUNT] = {&i, &j, &l, &o, &s, &t, &z};


static void DrawPiece(TPiece *piece)
{
	FontPutCharTD(PIECE_BLOCKS_SIZE, PIECE_BLOCKS_SIZE, piece->y, piece->x, &piece->matrix[piece->n][0],
			RGB_FROM_COLOR_IDX(piece->color));
}

static void WipePiece(TPiece *piece)
{
	FontPutCharTD(PIECE_BLOCKS_SIZE, PIECE_BLOCKS_SIZE, piece->y, piece->x, &piece->matrix[piece->n][0],
				  0,0,0);
}


void TetrisRotateCW(TPiece *piece)
{
    WipePiece(piece);
    piece->n++;
    piece->n %= 4;
    DrawPiece(piece);
}

void TetrisRotateCCW(TPiece *piece)
{
    WipePiece(piece);
    piece->n = (--piece->n > -1) ? piece->n : 3;
    DrawPiece(piece);
}

static void piece_random(TPiece *dest)
{
    memcpy(dest, piece_list[rand() % PIECE_COUNT], sizeof(TPiece));
}

void TetrisNew(TPiece *piece)
{
    piece_random(piece);
    piece->x = (AlsGetCols() - PIECE_BLOCKS_SIZE) / 2;
    piece->y = AlsGetRows();
    DrawPiece(piece);
}

void TetrisMove(TPiece *piece, TDirection dir) {
    WipePiece(piece);
	switch (dir) {
	case DIR_UP   : piece->y += 1; break;
	case DIR_DOWN : piece->y -= 1; break;
	case DIR_LEFT : piece->x -= 1; break;
	case DIR_RIGHT: piece->x += 1; break;
	default: break;
	}
    DrawPiece(piece);
}

/**
 * check if the piece hits a LED that is on.
 **/
bool TetrisPieceHit(TPiece *piece) {

	uint8_t* pMatrix = piece->matrix[piece->n];
	uint8_t pieceHeight = 4; //sizeof(piece->matrix[0]);

	if (piece->y <= 0) {
		if (pMatrix[pieceHeight - 1 + piece->y] != 0) {
			return true;
		}
	}
	return false;

}