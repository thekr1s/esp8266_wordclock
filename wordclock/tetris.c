#include "esp_glue.h"
#include "AddressableLedStrip.h"
#include "controller.h"
#include "tetris_pieces.h"

void HandleTetrisController(TPiece* piece){
	switch (ControllerGet()){
        case CONTROLLER_UP_RIGHT:
		case CONTROLLER_UP: TetrisRotateCW(piece); break; 
		case CONTROLLER_UP_LEFT: TetrisRotateCCW(piece); break; 
		case CONTROLLER_DOWN: TetrisMove(piece, DIR_DOWN); break; 
		case CONTROLLER_LEFT: TetrisMove(piece, DIR_LEFT); break; 
		case CONTROLLER_RIGHT: TetrisMove(piece, DIR_RIGHT); break; 
		default: break;
	}
}

void DoTetris() {
	uint32_t msLeft = 300;
	TPiece piece;
    AlsFill(0,0,0);
    TetrisNew(&piece);
    while (ControllerGameGet() == GAME_TETRIS) {
        for (int i = 0; i < 16; i++){
            AlsRefresh(ALSEFFECT_NONE);
            msLeft = Sleep(msLeft);
            SetInterrupted(false);
            if (msLeft > 0) {
                HandleTetrisController(&piece);
                i--;
            } else {
                if (TetrisPieceHit(&piece)) {
                    TetrisNew(&piece);
                } else {
                    TetrisMove(&piece, DIR_DOWN);
                }
                msLeft = 300;
            }
        }
    }
}
