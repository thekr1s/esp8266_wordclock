#include "FreeRTOS.h"
#include "task.h"
#include "esp_glue.h"
#include "controller.h"

static TControllerAction _lastAction = CONTROLLER_NONE;
static TControllerGame _game = GAME_NONE;
static int _lastActionTime = 0;

void ControllerSet(TControllerAction action){
    _lastAction = action;
    _lastActionTime = xTaskGetTickCount();

}

TControllerAction ControllerGet(){
    TControllerAction action = _lastAction;
    _lastAction = CONTROLLER_NONE;
    return action;
}

void ControllerGameSet(TControllerGame game){
    _game = game;
    _lastActionTime = xTaskGetTickCount();
}

TControllerGame ControllerGameGet(){
    if (GetTicksDiffMs(_lastActionTime, xTaskGetTickCount()) > 30000) {
        // Timeout... Long time no action. Stop game
        ControllerGameSet(GAME_NONE);
    }
    return _game;
}
