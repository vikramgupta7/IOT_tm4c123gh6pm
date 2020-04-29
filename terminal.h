/*
 * terminal.h
 *
 *  Created on: Mar 17, 2020
 *      Author: Vikram's ThinkPad
 */

#ifndef TERMINAL_H_
#define TERMINAL_H_


#include <stdint.h>
#include <stdbool.h>

#define MAX_CHARS 80
#define MAX_FIELDS 5

// Pins
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3

typedef struct uart_command
    {
    char buffer[MAX_CHARS+1];
    uint8_t count;
    uint8_t fieldCount;
    uint8_t fieldPosition[MAX_FIELDS];
    char fieldType[MAX_FIELDS];
    } uart_command;


void initHw();
void getsUart0(uart_command* uart_pointer);
void parseFields(uart_command* uart_pointer);
//void checkUartCommand(uart_command* uart_pointer);
//void ifconfig();
#endif
