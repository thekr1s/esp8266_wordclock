#include "controller.h"

static TControllerAction _lastAction = CONTROLLER_NONE;
static TControllerGame _game = GAME_NONE;

void ControllerSet(TControllerAction action){
    _lastAction = action;
}

TControllerAction ControllerGet(){
    TControllerAction action = _lastAction;
    _lastAction = CONTROLLER_NONE;
    return action;
}

void ControllerGameSet(TControllerGame game){
    _game = game;
}

TControllerGame ControllerGameGet(){
    return _game;
}
