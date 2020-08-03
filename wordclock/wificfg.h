
/*
 * Start the Wifi Configuration http server task. The IP port number
 * and a path dispatch list are needed. The dispatch list can not be
 * stack allocated as it is passed to another task.
 */
void wificfg_init();