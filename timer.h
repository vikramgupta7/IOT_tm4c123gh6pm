/*
 * dhcp.h
 *
 *  Created on: Apr 29, 2020
 *      Author: Vikram's ThinkPad
 */

#ifndef TIMER_H_
#define TIMER_H_

#include <stdbool.h>
#include <stdint.h>

typedef void (*_callback)();
void initTimer();
bool startOneshotTimer(_callback callback, uint32_t seconds);
bool startPeriodicTimer(_callback callback, uint32_t seconds);
bool stopTimer(_callback callback);
bool restartTimer(_callback callback);
void tickIsr();


#endif
