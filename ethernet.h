/*
 * ethernet.h
 *
 *  Created on: Apr 28, 2020
 *      Author: Vikram's ThinkPad
 */

#ifndef ETHERNET_H_
#define ETHERNET_H_


// Pins
#define RED_LED PORTF,1
#define BLUE_LED PORTF,2
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4

// Max packet is calculated as:
// Ether frame header (18) + Max MTU (1500) + CRC (4)
#define MAX_PACKET_SIZE 1522
uint8_t data[MAX_PACKET_SIZE];


void displayConnectionInfo();

#endif /* ETHERNET_H_ */
