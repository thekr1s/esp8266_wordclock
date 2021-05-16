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

void NetworkFunctionsEnter();
void NetworkFunctionsLeave();


#endif //WORDCLOCK_MAIN_H_
