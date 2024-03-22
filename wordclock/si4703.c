/*
Based on the cpp and h file from https://forum.arduino.cc/t/translating-existing-si4703-library-for-using-esp8266/544012

Library for Sparkfun Si4703 breakout board.
Simon Monk. 2011-09-09

This is a library wrapper and a few extras to the excellent code produced
by Nathan Seidle from Sparkfun (Beerware).

Nathan's comments......

Look for serial output at 57600bps.

The Si4703 ACKs the first byte, and NACKs the 2nd byte of a read.

1/18 - after much hacking, I suggest NEVER write to a register without first reading the contents of a chip.
ie, don't updateRegisters without first readRegisters.

If anyone manages to get this datasheet downloaded
http://wenku.baidu.com/view/d6f0e6ee5ef7ba0d4a733b61.html
Please let us know. It seem to be the latest version of the programming guide. It had a change on page 12 (write 0x8100 to 0x07)
that allowed me to get the chip working..

Also, if you happen to find "AN243: Using RDS/RBDS with the Si4701/03", please share. I love it when companies refer to
documents that don't exist.

1/20 - Picking up FM stations from a plane flying over Portugal! Sweet! 93.9MHz sounds a little soft for my tastes,s but
it's in Porteguese.

ToDo:
Display current status (from 0x0A) - done 1/20/11
Add RDS decoding - works, sort of
Volume Up/Down - done 1/20/11
Mute toggle - done 1/20/11
Tune Up/Down - done 1/20/11
Read current channel (0xB0) - done 1/20/11
Setup for Europe - done 1/20/11
Seek up/down - done 1/25/11

The Si4703 breakout does work with line out into a stereo or other amplifier. Be sure to test with different length 3.5mm
cables. Too short of a cable may degrade reception.
*/

#include <espressif/esp_common.h>
#include <i2c/i2c.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include <hd44780/hd44780.h>
#include <sntp.h>

#include "esp_glue.h"
#include "si4703.h"

#define I2C_BUS 0
// #define SCL_PIN 12
// #define SDA_PIN 13
// #define RST_PIN 14

#define SCL_PIN  5 // D1
#define SDA_PIN  4 // D2
#define RST_PIN  14 // D5



#define LOW  0
#define HIGH 1

uint16_t si4703_registers[16]; //There are 16 registers, each 16 bits large


static const uint16_t  FAIL = 0;
static const uint16_t  SUCCESS = 1;

static const int  SI4703 = 0x10; //0b._001.0000 = I2C address of Si4703 - note that the Wire function assumes non-left-shifted I2C address, not 0b.0010.000W
static const uint16_t  I2C_FAIL_MAX = 10; //This is the number of attempts we will try to contact the device before erroring out

//Define the register names
static const uint16_t  DEVICEID = 0x00;
static const uint16_t  CHIPID = 0x01;
static const uint16_t  POWERCFG = 0x02;
static const uint16_t  CHANNEL = 0x03;
static const uint16_t  SYSCONFIG1 = 0x04;
static const uint16_t  SYSCONFIG2 = 0x05;
static const uint16_t  SYSCONFIG3 = 0x06;
static const uint16_t  STATUSRSSI = 0x0A;
static const uint16_t  READCHAN = 0x0B;
static const uint16_t  RDSA = 0x0C;
static const uint16_t  RDSB = 0x0D;
static const uint16_t  RDSC = 0x0E;
static const uint16_t  RDSD = 0x0F;

//Register 0x02 - POWERCFG
static const uint16_t  SMUTE = 15;
static const uint16_t  DMUTE = 14;
static const uint16_t  RDSM = 11;
static const uint16_t  SKMODE = 10;
static const uint16_t  SEEKUP = 9;
static const uint16_t  SEEK = 8;


static const uint16_t  SEEK_DOWN=0;
static const uint16_t  SEEK_UP=1;


//Register 0x03 - CHANNEL
static const uint16_t  TUNE = 15;

//Register 0x04 - SYSCONFIG1
static const uint16_t  RDS = 12;
static const uint16_t  DE = 11;

//Register 0x05 - SYSCONFIG2
static const uint16_t  SPACE1 = 5;
static const uint16_t  SPACE0 = 4;


static const uint16_t  BLER_MASK = 0x3;
//Register 0x0A - STATUSRSSI
static const uint16_t  RDSR = 15;
static const uint16_t  STC = 14;
static const uint16_t  SFBL = 13;
static const uint16_t  AFCRL = 12;
static const uint16_t  RDSS = 11;
static const uint16_t  BLERA = 9;
static const uint16_t  STEREO = 8;
static const uint16_t  RSSI = 0;
static const uint16_t  RSSI_MASK = 0xff;


//Register 0x0b - READCHAN
static const uint16_t  BLERB = 14;
static const uint16_t  BLERC = 12;
static const uint16_t  BLERD = 10;



static uint32_t millis() {
  return xTaskGetTickCount() * portTICK_PERIOD_MS;
}

//Read the entire register control set from 0x00 to 0x0F
static void readRegisters(){
  uint8_t regs[32];

  //si4703 begins reading from register upper register of 0x0A and reads to 0x0F, then loops to 0x00.
//   Wire.requestFrom(SI4703, 32); //We want to read the entire register set from 0x0A to 0x09 = 32 bytes.
  i2c_slave_read(I2C_BUS, SI4703, NULL, (uint8_t*)&regs[0], sizeof(regs));
  // printf("i2c_slave_read res: %d\n", res);
  // while(Wire.available() < 32) ; //Wait for 16 words/32 bytes to come back from slave I2C device
  // //We may want some time-out error here

  // //Remember, register 0x0A comes in first so we have to shuffle the array around a bit
  int i = 0;
  for(int x = 0x0A ; ; x++) { //Read in these 32 bytes
    if(x == 0x10) x = 0; //Loop back to zero
    si4703_registers[x] = regs[i] << 8;
    i++;
    si4703_registers[x] |= regs[i];
    i++;
    if(x == 0x09) break; //We're done!
  }
}

void si4703_dump_regs(){
  for (int i = 0; i < 16; i++) {
    printf("reg[%d]: 0x%04x\n", i, si4703_registers[i]);
  }
}

//Write the current 9 control registers (0x02 to 0x07) to the si4703
//It's a little weird, you don't write an I2C addres
//The si4703 assumes you are writing to 0x02 first, then increments
static uint8_t updateRegisters() {
  uint8_t regs[12];

  //A write command automatically begins with register 0x02 so no need to send a write-to address
  //First we send the 0x02 to 0x07 control registers
  //In general, we should not write to registers 0x08 and 0x09
  int i = 0;
  for(int regSpot = 0x02 ; regSpot < 0x08 ; regSpot++) {
    regs[i] = si4703_registers[regSpot] >> 8;
    i++;
    regs[i] = si4703_registers[regSpot] & 0x00FF;
    i++;
  }
  i2c_slave_write(I2C_BUS, SI4703, NULL, (uint8_t*)&regs[0], sizeof(regs));
  
  // //End this transmission
  // uint8_t ack = Wire.endTransmission();
  // if(ack != 0) { //We have a problem! 
  //   return(FAIL);
  // }

  return(SUCCESS);
}

//To get the si4703 inito 2-wire mode, SEN needs to be high and SDIO needs to be low after a reset
//The breakout board has SEN pulled high, but also has SDIO pulled high. Therefore, after a normal power up
//The si4703 will be in an unknown state. RST must be controlled
void si4703_init() 
{
  i2c_init(I2C_BUS, SCL_PIN, SDA_PIN, I2C_FREQ_100K);

  gpio_enable(RST_PIN, GPIO_OUTPUT);
  gpio_enable(SDA_PIN, GPIO_OUTPUT); //SDIO is connected to A4 for I2C
  gpio_write(SDA_PIN, LOW); //A low SDIO indicates a 2-wire interface
  gpio_write(RST_PIN, LOW); //Put si4703 into reset
  SleepNI(100); //Some delays while we allow pins to settle
  gpio_write(RST_PIN, HIGH); //Bring si4703 out of reset with SDIO set to low and SEN pulled high with on-board resistor
  SleepNI(100); //Allow si4703 to come out of reset

//   Wire.begin(); //Now that the unit is reset and I2C inteface mode, we need to begin I2C
  i2c_start(I2C_BUS);

  readRegisters(); //Read the current register set
  //si4703_registers[0x07] = 0xBC04; //Enable the oscillator, from AN230 page 9, rev 0.5 (DOES NOT WORK, wtf Silicon Labs datasheet?)
  si4703_registers[0x07] = 0x8100; //Enable the oscillator, from AN230 page 9, rev 0.61 (works)
  updateRegisters(); //Update

  SleepNI(500); //Wait for clock to settle - from AN230 page 9

  readRegisters(); //Read the current register set
  si4703_registers[POWERCFG] = 0x4001; //Power enable and verbose mode 
  //  si4703_registers[POWERCFG] |= (1<<SMUTE) | (1<<DMUTE); //Disable Mute, disable softmute
  si4703_registers[SYSCONFIG1] |= (1<<RDS); //Enable RDS

  si4703_registers[SYSCONFIG1] |= (1<<DE); //50kHz Europe setup
  si4703_registers[SYSCONFIG2] |= (1<<SPACE0); //100kHz channel spacing for Europe

  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= 0x0001; //Set volume to lowest
  updateRegisters(); //Update

  SleepNI(120); //Max powerup time, from datasheet page 13
  readRegisters(); //Read the current register set
}


void si4703_powerOn()
{
    si4703_init();
}

void si4703_powerOff()	//RAZ Added 4/1/2018
{
    gpio_enable(RST_PIN, GPIO_OUTPUT);
	gpio_write(RST_PIN, LOW); //Put si4703 into reset
}
void si4703_setChannel(int channel)
{
  //Freq(MHz) = 0.200(in USA) * Channel + 87.5MHz
  //97.3 = 0.2 * Chan + 87.5
  //9.8 / 0.2 = 49
  int newChannel = channel * 10; //973 * 10 = 9730
  newChannel -= 8750; //9730 - 8750 = 980
  newChannel /= 10; //980 / 10 = 98

  //These steps come from AN230 page 20 rev 0.5
  readRegisters();
  si4703_registers[CHANNEL] &= 0xFE00; //Clear out the channel bits
  si4703_registers[CHANNEL] |= newChannel; //Mask in the new channel
  si4703_registers[CHANNEL] |= (1<<TUNE); //Set the TUNE bit to start
  updateRegisters();

  //delay(60); //Wait 60ms - you can use or skip this delay

  //Poll to see if STC is set
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
  }

  readRegisters();
  si4703_registers[CHANNEL] &= ~(1<<TUNE); //Clear the tune after a tune has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }
}

//Seeks out the next available station
//Returns the freq if it made it
//Returns zero if failed
static int seek(uint8_t seekDirection){
  readRegisters();
  //Set seek mode wrap bit
  si4703_registers[POWERCFG] |= (1<<SKMODE); //Allow wrap
  //si4703_registers[POWERCFG] &= ~(1<<SKMODE); //Disallow wrap - if you disallow wrap, you may want to tune to 87.5 first
  if(seekDirection == SEEK_DOWN) si4703_registers[POWERCFG] &= ~(1<<SEEKUP); //Seek down is the default upon reset
  else si4703_registers[POWERCFG] |= 1<<SEEKUP; //Set the bit to seek up

  si4703_registers[POWERCFG] |= (1<<SEEK); //Start seek
  updateRegisters(); //Seeking will now start

  //Poll to see if STC is set
  while(1) {
    readRegisters();
    if((si4703_registers[STATUSRSSI] & (1<<STC)) != 0) break; //Tuning complete!
  }

  readRegisters();
  int valueSFBL = si4703_registers[STATUSRSSI] & (1<<SFBL); //Store the value of SFBL
  si4703_registers[POWERCFG] &= ~(1<<SEEK); //Clear the seek bit after seek has completed
  updateRegisters();

  //Wait for the si4703 to clear the STC as well
  while(1) {
    readRegisters();
    if( (si4703_registers[STATUSRSSI] & (1<<STC)) == 0) break; //Tuning complete!
  }

  if(valueSFBL) { //The bit was set indicating we hit a band limit or failed to find a station
    return(0);
  }
  return getChannel();
}

int si4703_seekUp()
{
	return seek(SEEK_UP);
}

int si4703_seekDown()
{
	return seek(SEEK_DOWN);
}

void si4703_setVolume(int volume)
{
  readRegisters(); //Read the current register set
  if(volume < 0) volume = 0;
  if (volume > 15) volume = 15;
  si4703_registers[SYSCONFIG2] &= 0xFFF0; //Clear volume bits
  si4703_registers[SYSCONFIG2] |= volume; //Set new volume
  updateRegisters(); //Update
}


uint32_t getBits(uint16_t word1, uint16_t word2, size_t starting_at, size_t nr_bits) {
  assert(nr_bits <= 32);
  return (((word1 << 16) + word2) >> starting_at) & ((1 << nr_bits) - 1);
}


void handle_group0(uint16_t b, uint16_t c, uint16_t d){
  static char text[9];
  static char prevtext[9];
  int offset = b & 0x3;

  offset *=2;
  text[offset] = d >> 8 & 0xff;
  text[offset + 1] = d & 0xff;

  if (offset == 6 && memcmp(text, prevtext,8) != 0) {
    text[8]=0;
    printf("RADIO test: %s\n", text);
    memcpy(prevtext, text,8);
    bzero(text,9);
  }


}

// message should be at least 9 chars
// result will be null terminated
// timeout in milliseconds
uint32_t si4703_waitRDSTime(long timeout)
{ 
  long endTime = millis() + timeout;
  int completedCount = 0;
  uint32_t rds_received = 0;

  while(completedCount < 4 && millis() < endTime) {
    readRegisters();
    uint8_t blera =  si4703_registers[STATUSRSSI] >> BLERA & BLER_MASK;
    uint8_t blerb =  si4703_registers[READCHAN] >> BLERB & BLER_MASK;
    uint8_t blerc =  si4703_registers[READCHAN] >> BLERC & BLER_MASK;
    uint8_t blerd =  si4703_registers[READCHAN] >> BLERD & BLER_MASK;

    if (blera == BLER_MASK || blerb == BLER_MASK || blerc == BLER_MASK || blerd == BLER_MASK){
      printf("Errors in RDS data, ignore\n");
    } else if(si4703_registers[STATUSRSSI] & (1<<RDSR)){
      rds_received = 1;
      // ls 2 bits of B determine the 4 letter pairs
      // once we have a full set return
      // if you get nothing after 20 readings return with empty string
      uint16_t block1 = si4703_registers[RDSA];
      uint16_t block2 = si4703_registers[RDSB];
      uint16_t block3 = si4703_registers[RDSC];
      uint16_t block4 = si4703_registers[RDSD];

      uint8_t group_code=block2>>12 & 0xf;
      char group_ab= (block2>>11 & 0x1 ? 'B' : 'A');
      // Serial.printf("RDS group: %d%c\n", group_code, group_ab);
      if (block1 < 0x8201 || block1 > 0x8205) {
        printf("RADIO PI code %04x not of NPO. Skip.\n", block1);
        return 0;
      } else if (group_code == 0) {
        handle_group0(block2, block3, block4);
      } else if (group_code == 4 && group_ab == 'A') {
        struct tm ts;
        // Group 4A, clock time
        // printf("RDS process CT: %04x %04x %04x %04x\n", block1, block2, block3, block4);
        uint32_t modified_julian_date = getBits(block2, block3, 1, 17);

        int year_utc  = (modified_julian_date - 15078.2) / 365.25;
        int month_utc = (modified_julian_date - 14956.1 -
                      (int)(year_utc * 365.25)) / 30.6001;
        int day_utc   = modified_julian_date - 14956 - (int)(year_utc * 365.25) -
                      (int)(month_utc * 30.6001);
        if (month_utc == 14 || month_utc == 15) {
          year_utc += 1;
          month_utc -= 12;
        }
        year_utc += 1900;
        month_utc -= 1;

        int hour_utc   = getBits(block3, block4, 12, 5);
        int minute_utc = getBits(0, block4, 6, 6);

        double local_offset = (getBits(0, block4, 5, 1) ? -1 : 1) *
                              getBits(0, block4, 0, 5) / 2.0;

        bool is_date_valid = hour_utc <= 23 && minute_utc <= 59 &&
                            (int)(local_offset) <= 14.0;
        if (is_date_valid) {
          int local_offset_hour = local_offset;
          int local_offset_min  = (local_offset - (int)(local_offset)) * 60.0;
          printf(
                    "clock time: %04d-%02d-%02dT%02d:%02d:00%s%02d:%02d\n",
                    year_utc, month_utc, day_utc,
                    hour_utc, minute_utc, local_offset > 0 ? "+" : "-",
                    local_offset_hour, abs(local_offset_min));

          // TODO: day_utc must be yearday instead of monthday
          ts.tm_year = year_utc - 1900;
          ts.tm_mon = month_utc - 1;
          ts.tm_mday = day_utc;
          ts.tm_hour = hour_utc;
          ts.tm_min = minute_utc;
          ts.tm_sec = 0;
          // struct timezone tzs = {0, 0};
          // tzs.tz_minuteswest = -local_offset_hour - local_offset_min;
          
          // sntp_set_timezone(&tzs);

          time_t epoch = mktime(&ts) + local_offset_hour * 3600; 
            // 0 + minute_utc*60 + hour_utc*3600 + day_utc*86400 +
            // (year_utc-70)*31536000 + ((year_utc-69)/4)*86400 -
            // ((year_utc-1)/100)*86400 + ((year_utc+299)/400)*86400;
          // printf("RDS TIME RECEIVED, epoch: %u\n", (uint32_t)epoch);
          return epoch;
       }
      }
    }
    SleepNI(41);
  }
  return rds_received; 
}

//Reads the current channel from READCHAN
//Returns a number like 973 for 97.3MHz
int getChannel() {
  readRegisters();
  int channel = si4703_registers[READCHAN] & 0x03FF; //Mask out everything but the lower 10 bits
  //Freq(MHz) = 0.100(in Europe) * Channel + 87.5MHz
  //X = 0.1 * Chan + 87.5
  channel += 875; //98 + 875 = 973
  return(channel);
}

void si4703_task(void* p) {
    const int RDS_TIMEOUT = 10000;
    const int MAX_NO_TIME_COUNT = 130000/ RDS_TIMEOUT;
    int no_time_count = 0;
    // si4703_setChannel(968);
    si4703_seekUp();
    printf("Radio channel: %d\n", getChannel());

    for(;;) {
        no_time_count++;
        uint32_t t = si4703_waitRDSTime(10000);
        if (t > 1000000) { // Dirty dirty dirty, i know.....
          no_time_count = 0;
          if (sdk_wifi_station_get_connect_status() != STATION_GOT_IP) {
            // We have no wifi, therefore use time from the radio 
            sntp_update_rtc(t, 0);
            uint32_t h=0, m =0, s=0;
            TimeGet(&h, &m, &s);
            printf("Updated ntp time via RDS: %02d:%02d:%02d\n",h,m,s);
          }
          // prevent read same registers twice and no need to read.
          SleepNI(1000);
        }
        uint16_t rssi = si4703_registers[STATUSRSSI] >> RSSI & RSSI_MASK;
        printf("RADIO chan: %d, RSSI: %d, res: %d\n", getChannel(), rssi, t);
        // low signal, no RDS data received or more than a minute no time message
        if (rssi < 20 || t == 0 || no_time_count == MAX_NO_TIME_COUNT) { 
          no_time_count = 0;
          si4703_seekUp();
          rssi = si4703_registers[STATUSRSSI] >> RSSI & RSSI_MASK;
          printf("RADIO seek new channel: %d, rssi: %d\n", getChannel(), rssi);
        }
    }
}

void si4703_task_init() {

 	si4703_init();
	xTaskCreate(si4703_task, "SI4703", 256, NULL, 3, NULL);


}
