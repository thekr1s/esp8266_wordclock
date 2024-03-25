 //Direction used for seeking. Default is down
extern int si4703_channel;
extern char* si4703_rds_text;

void si4703_task_init();
int si4703_seekUp();
int getChannel();
void si4703_readRDS(char* buffer, long timeout);
uint32_t si4703_waitRDSTime(long timeout);


void si4703_dump_regs();
