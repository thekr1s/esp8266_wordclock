/*
 * wordclock_main.c
 *
 *  Created on: Oct 7, 2016
 *      Author: rutger.huijgen
 */

#ifndef WORDCLOCK_MAIN_H_
#define WORDCLOCK_MAIN_H_

void WordclockMain(void* p);
void ShowTime(int delayMS);
void TimeGet(uint32_t* h, uint32_t* m, uint32_t* s);

void NetworkFunctionsEnter();
void NetworkFunctionsLeave();


#endif //WORDCLOCK_MAIN_H_
