#include "FreeRTOS.h"
#include "task.h"
#include "esp_glue.h"
#include "AddressableLedStrip.h"
#include "controller.h"
#include "displaySettings.h"
#include "animations.h"

typedef struct{
    int32_t row;
    int32_t col;
} TPos;

const uint32_t _batWidth = 3;

static uint32_t _batPos = 5;
static TPos _balPos = {1,6};
static TPos _balDir = {1,0};
static uint8_t br;


static void Init(){
    br = ApplyBrightness(255);
    AlsFill(0,0,0);
    for( uint32_t row = AlsGetRows(); row > AlsGetRows() - 4; row--) {
        for(uint32_t col = 0; col < AlsGetCols(); col++) {
            AlsSetLed(row, col, 0, 	br, 0);
        }

    }
    _balPos.row = 1;
    _balPos.col = 5;
    _balDir.row = 1;
    _balDir.col = 1;
}

static void DrawBat() {

    for(uint32_t col = 0; col < AlsGetCols(); col++) {
        if (col >= _batPos && col < _batPos + _batWidth) {
            AlsSetLed(0, col, 0, 0, br);
        } else {
            AlsSetLed(0, col, 0, 0, 0);
        }
    }
} 

static void UpdateBallPos(TPos* pPos) {
    pPos->col = pPos->col + _balDir.col;
    pPos->row = pPos->row + _balDir.row;
}

static bool BallHitsBat() {
    TPos newPos = _balPos;
    UpdateBallPos(&newPos);
    if (newPos.row == 0 &&  newPos.col >= _batPos && newPos.col < _batPos + _batWidth) {
        return TRUE;
    } else {
        return FALSE;
    }
}

static bool BallHitsWall() {
    uint8_t r,g,b = 0;
    TPos newPos = _balPos;
    if (_balDir.row > 0) {
        newPos.row += 1; 
        AlsGetLed(newPos.row, newPos.col, &r, &g, &b);
        AlsSetLed(newPos.row, newPos.col, 0, 0, 0);
        if ((r + g + b) != 0) {
            return TRUE;
        } 
    }
    return FALSE;
}

static void MoveBall() {
    AlsSetLed(_balPos.row, _balPos.col, 0, 0, 0);
    if (_balPos.row < 0) {
        DisplayWord("LOST!");
        Sleep(2000);
        Init();
    } else if (BallHitsBat()) {
        _balDir.row = -_balDir.row;
    } else if (BallHitsWall()) {
        _balDir.row = -_balDir.row;
    } else if(_balDir.row > 0 && _balPos.row >= AlsGetRows()-1) {
        _balDir.row = -_balDir.row;
    } else if((_balDir.col > 0 && _balPos.col >= AlsGetCols()-1) || (_balDir.col < 0 && _balPos.col <= 0)) {
        _balDir.col = -_balDir.col;
    }     
    _balPos.row += _balDir.row;
    _balPos.col += _balDir.col;
    AlsSetLed(_balPos.row, _balPos.col, br, 0, 0);
}



void DoBreakout(){
    uint32_t cycleTime = 300;
    Init();
    while (ControllerGameGet() == GAME_BREAKOUT) {
        SetInterrupted(FALSE);
        cycleTime = Sleep(cycleTime);
        TControllerAction action = ControllerGet();
        switch(action){
        case CONTROLLER_LEFT :_batPos = _batPos > 0 ? _batPos - 1: 0; break;
        case CONTROLLER_RIGHT:_batPos = _batPos < (AlsGetCols() - _batWidth) ? _batPos + 1 : AlsGetCols() - _batWidth; break;
        default: break;
        }
        DrawBat();
        if (cycleTime == 0) {
            MoveBall();
            cycleTime = 500;
        }
        AlsRefresh(ALSEFFECT_NONE);
    }
}