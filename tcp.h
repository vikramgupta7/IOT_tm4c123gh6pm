/*
 * tcp.h
 *
 *  Created on: Apr 30, 2020
 *      Author: Vikram's ThinkPad
 */

#ifndef TCP_H_
#define TCP_H_

#include <stdint.h>


typedef struct _tcpFrame
{
    uint16_t sourcePort;
    uint16_t destinationPort;
    uint32_t sequenceNo;
    uint32_t acknowledgementNo;
    uint8_t dataOffset;
    uint8_t flags;
    uint16_t windowSize;
    uint16_t checksum;
    uint16_t urgentPointer;
    uint8_t options[0];
} tcpFrame;


void tcpAckConReq(uint8_t packet[]);
uint8_t etherIsTcp(uint8_t packet[]);


#endif /* TCP_H_ */
