 //Direction used for seeking. Default is down

void si4703_task_init();
void si4703_init();
int si4703_seekUp();
int getChannel();
void si4703_readRDS(char* buffer, long timeout);
uint32_t si4703_waitRDSTime(long timeout);


void si4703_dump_regs();
