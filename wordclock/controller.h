typedef enum {
    CONTROLLER_NONE,
    CONTROLLER_UP,
    CONTROLLER_DOWN,
    CONTROLLER_LEFT,
    CONTROLLER_RIGHT,
    CONTROLLER_CENTER,
    CONTROLLER_UP_LEFT,
    CONTROLLER_UP_RIGHT
}TControllerAction;

typedef enum {
    GAME_NONE,
    GAME_TETRIS,
    GAME_BREAKOUT,
    GAME_PONG,
}TControllerGame;

void ControllerSet(TControllerAction action);
TControllerAction ControllerGet();

void ControllerGameSet(TControllerGame game);
TControllerGame ControllerGameGet();