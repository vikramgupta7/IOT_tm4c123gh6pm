/*
 * mqtt.h
 *
 *  Created on: May 2, 2020
 *      Author: Vikram's ThinkPad
 */

#ifndef MQTT_H_
#define MQTT_H_


typedef struct _mqttFrame
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


#endif /* MQTT_H_ */
