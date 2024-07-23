 //Direction used for seeking. Default is down
extern int si4703_channel;
extern int si4703_rssi;
extern char* si4703_rds_text;

void si4703_task_init();
bool si4703_radio_active();
