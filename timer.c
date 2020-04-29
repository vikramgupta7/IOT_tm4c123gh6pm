/*
 * TIMER_LED.c
 *
 *  Created on: Mar 18, 2020
 *      Author: Vikram's ThinkPad
 */


#include "tm4c123gh6pm.h"
#include <stdbool.h>
#include <stdint.h>
#include "gpio.h"
# include <stdlib.h>
#include <timer.h>
//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------
#define NUM_TIMERS 10
_callback fn[NUM_TIMERS];
uint32_t period[NUM_TIMERS];
uint32_t ticks[NUM_TIMERS];
bool reload[NUM_TIMERS];



#define RED_LED PORTF,1
#define BLUE_LED PORTF,2
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4


//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
void initTimer()
{
    uint8_t i;

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R4;
    _delay_cycles(3);
    // Configure Timer 4 for 1 sec tick
    TIMER4_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER4_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER4_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER4_TAILR_R = 40000000;                       // set load value (1 Hz rate)
    TIMER4_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    TIMER4_IMR_R |= TIMER_IMR_TATOIM;                // turn-on interrupt
    NVIC_EN2_R |= 1 << (INT_TIMER4A-80);             // turn-on interrupt 86 (TIMER4A)

    for (i = 0; i < NUM_TIMERS; i++)
    {
        period[i] = 0;
        ticks[i] = 0;
        fn[i] = NULL;
        reload[i] = false;
    }
}

bool startOneshotTimer(_callback callback, uint32_t seconds)
{
    uint8_t i = 0;
    bool found = false;
    while (i < NUM_TIMERS && !found)
    {
        found = fn[i] == NULL;
        if (found)
        {
            period[i] = seconds;
            ticks[i] = seconds;
            fn[i] = callback;
            reload[i] = false;
        }
        i++;
    }
    return found;
}

bool startPeriodicTimer(_callback callback, uint32_t seconds)
{
    uint8_t i = 0;
    bool found = false;
    while (i < NUM_TIMERS && !found)
    {
        found = fn[i] == NULL;
        if (found)
        {
            period[i] = seconds;
            ticks[i] = seconds;
            fn[i] = callback;
            reload[i] = true;
        }
        i++;
    }
    return found;
}

bool stopTimer(_callback callback)
{
     uint8_t i = 0;
     bool found = false;
     while (i < NUM_TIMERS && !found)
     {
         found = fn[i] == callback;
         if (found)
             ticks[i] = 0;
         i++;
     }
     return found;
}

bool restartTimer(_callback callback)
{
     uint8_t i = 0;
     bool found = false;
     while (i < NUM_TIMERS && !found)
     {
         found = fn[i] == callback;
         if (found)
             ticks[i] = period[i];
         i++;
     }
     return found;
}

void tickIsr()
{
    uint8_t i;
    for (i = 0; i < NUM_TIMERS; i++)
    {
        if (ticks[i] != 0)
        {
            ticks[i]--;
            if (ticks[i] == 0)
            {
                if (reload[i])
                    ticks[i] = period[i];
                (*fn[i])();
            }
        }
    }
    TIMER4_ICR_R = TIMER_ICR_TATOCINT;
}

// Placeholder random number function
uint32_t random32()
{
    return TIMER4_TAV_R;
}

//void flash()
//{
//    setPinValue(BLUE_LED, !getPinValue(BLUE_LED));
//}
//
//void flash2()
//{
//    setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
//}
//
//void flash3()
//{
//    setPinValue(RED_LED, 1);
//}
//
//void flash4()
//{
//    setPinValue(RED_LED, 0);
//    startPeriodicTimer(flash, 2);
//    startPeriodicTimer(flash2, 3);
//}
//
//
//
//void main()
//{
//    initHw();
//    initTimer();
//    startOneshotTimer(flash3, 5);
//    startOneshotTimer(flash4, 10);
//}

