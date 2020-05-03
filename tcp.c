/*
 * tcp.c
 *
 *  Created on: Apr 30, 2020
 *      Author: Vikram's ThinkPad
 */

#include "tcp.h"
#include "eth0.h"
#include "ethernet.h"

void tcpSendSynAck(uint8_t packet[])
{
    uint16_t temp = 0;
    uint8_t i = 0;
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    tcpFrame* tcp = (tcpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));

    for (i = 0; i < 6; i++)
        ether->destAddress[i] = ether->sourceAddress[i];

    for (i = 0; i < 4; i++)
        ip->destIp[i] = ip->sourceIp[i];

    etherGetIpAddress(ip->sourceIp);
    etherGetMacAddress(ether->sourceAddress);


    temp = tcp->sourcePort;
    tcp->sourcePort = tcp->destinationPort;
    tcp->destinationPort = temp;
    tcp->options[2] = 0x3;
    tcp->options[3] = 0xE8;
    tcp->options[7] = 0;

//    65 [11]


    tcp->acknowledgementNo = htons32(htons32(tcp->sequenceNo) + 1);
    tcp->flags = 0x012;
    tcp->windowSize = htons(1000);

    sum = 0;
    etherSumWords(&ip->revSize, 10);
    etherSumWords(ip->sourceIp, ((ip->revSize & 0xF) * 4) - 12);
    ip->headerChecksum = getEtherChecksum();

    sum = 0;
    tcp->checksum = 0;
    etherSumWords(ip->sourceIp, 8);
    temp = ip->protocol;
    sum += (temp & 0xff) << 8;
    sum += ((tcp->dataOffset >> 4)*4) << 8;
//    etherSumWords(temp);
    // add udp header except crc
    etherSumWords(tcp, 32);
//    etherSumWords(&udp->data, udpSize);
    tcp->checksum = getEtherChecksum();
    etherPutPacket(ether, 14 + ((ip->revSize & 0xF) * 4) + ((tcp->dataOffset >> 4)*4));//
}


uint8_t etherIsTcp(uint8_t packet[])
{
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    tcpFrame* tcp = (tcpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));
    uint8_t ok = 0;
    if(ip->protocol == 6)
    {
        ok = 1;
        if(tcp->flags == 0x002)
        {
            ok = 2; //Recieved SYN
        }
        if(tcp->flags == 0x010) //Recieved ACK. Could be ACK for data or handshake.
        {
            ok = 3;
        }
        if(tcp->flags == 0x018) //Recieved ACK with PSH.
        {
            ok = 4;
        }
    }
    return ok;
}

void tcpListen()
{

}

void tcpSynRecieved()
{
    tcpSendSynAck(data);
}

void tcpEstablished()
{

}

void tcpAcknowledge(uint8_t packet[])
    {
        uint16_t temp = 0;
        uint8_t i = 0;
        etherFrame* ether = (etherFrame*)packet;
        ipFrame* ip = (ipFrame*)&ether->data;
        tcpFrame* tcp = (tcpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));

        for (i = 0; i < 6; i++)
            ether->destAddress[i] = ether->sourceAddress[i];

        for (i = 0; i < 4; i++)
            ip->destIp[i] = ip->sourceIp[i];

        etherGetIpAddress(ip->sourceIp);
        etherGetMacAddress(ether->sourceAddress);


        temp = tcp->sourcePort;
        tcp->sourcePort = tcp->destinationPort;
        tcp->destinationPort = temp;


        tcp->acknowledgementNo = htons32(htons32(tcp->sequenceNo) + 1);
        tcp->flags = 0x10;
        tcp->windowSize = htons(1000);

        sum = 0;
        etherSumWords(&ip->revSize, 10);
        etherSumWords(ip->sourceIp, ((ip->revSize & 0xF) * 4) - 12);
        ip->headerChecksum = getEtherChecksum();

        sum = 0;
        tcp->checksum = 0;
        etherSumWords(ip->sourceIp, 8);
        temp = ip->protocol;
        sum += (temp & 0xff) << 8;
        sum += ((tcp->dataOffset >> 4)*4) << 8;
    //    etherSumWords(temp);
        // add udp header except crc
        etherSumWords(tcp, 32);
    //    etherSumWords(&udp->data, udpSize);
        tcp->checksum = getEtherChecksum();
        etherPutPacket(ether, 14 + ((ip->revSize & 0xF) * 4) + ((tcp->dataOffset >> 4)*4));//
    }
