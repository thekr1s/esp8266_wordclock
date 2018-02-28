/*
 * event_handler.c
 *
 *  Created on: Oct 24, 2016
 *      Author: robert
 */
#include <FreeRTOS.h>
#include <queue.h>
#include <task.h>
#include <timers.h>

#include <stdio.h>
#include <time.h>

#include "AddressableLedStrip.h"
#include "clock_words.h"
#include "buttons.h"


typedef enum {
	 EVENT_BUTTON_PRESSED,
	 EVENT_BUTTON_TIMER,
	 EVENT_NEXT_MINUTE,
} TEvent;
static xQueueHandle _msgQueue;
static const portTickType  _buttonTimerInterval = 200 / portTICK_RATE_MS;
static xTimerHandle _buttonTimer;
static xTimerHandle _nextMinuteTimer;
static bool _fStop = false;

static void EventTask(void *parameters){
	TEvent event;
	portBASE_TYPE result;

	while (!_fStop) {
		result = xQueueReceive(_msgQueue, &event, portMAX_DELAY);
		if (result == pdTRUE) {
			switch (event) {
			case EVENT_NEXT_MINUTE:
				//DisplayTime(ALSEFFECT_RANDOM_EFFECT);
				break;

			default:
				printf("%s: unhandled event: %d\n", __FUNCTION__, event);
				break;


			}

		}

	}
}
static void SendEvent(TEvent event) {
	xQueueSend(_msgQueue, &event, portMAX_DELAY);
}

static void ButtonHandler(xTimerHandle xTimer){

	//printf("%s: TODO: check if button(s) pressed and send event\n", __FUNCTION__);

	//xTimerStart(_nextMinuteTimer, 0);


}
static void NextMinuteHandler(xTimerHandle xTimer){
	portTickType newPeriod;

	newPeriod = 61 - ((time(NULL) + 1) % 60);
	newPeriod *= 1000;
	newPeriod /= portTICK_RATE_MS;
	xTimerChangePeriod(_nextMinuteTimer, newPeriod, 0);
	xTimerStart(_nextMinuteTimer, 0);
	SendEvent(EVENT_NEXT_MINUTE);
}

/**
 * Force display update after setting time
 */
void EvtHdlTimeChanged(){
	xTimerStop(_nextMinuteTimer, 0);
	NextMinuteHandler(NULL);
}

void EvtHdlInit() {
	_buttonTimer = xTimerCreate((signed char *)"Button timer", _buttonTimerInterval, pdFALSE, NULL, ButtonHandler);
	_nextMinuteTimer = xTimerCreate((signed char *)"Next minute timer", _buttonTimerInterval, pdFALSE, NULL, NextMinuteHandler);
	_msgQueue = xQueueCreate(2, sizeof(uint32_t));
	xTaskCreate(EventTask, (signed char *)"Event task", 1024, NULL, 1, NULL);
}

void EvtHdlButtonStateChange() {
	if (pdFAIL == xTimerStart(_buttonTimer, 0)) {
		printf("%s: Timer set failed\n", __FUNCTION__);
		return;
	}
	ButtonHandler(NULL);

}

void EvtHdlButtonRelease() {
}



