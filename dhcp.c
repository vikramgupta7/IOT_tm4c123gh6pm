/*
 * dhcp.c
 *
 *  Created on: Apr 27, 2020
 *      Author: Vikram's ThinkPad
 */


#include <stdint.h>
#include "eth0.h"
#include "dhcp.h"
#include <stdio.h>
#include "uart0.h"
#include "timer.h"
#include "ethernet.h"


#define DHCP_STATE_MACHINE_DEBUG

void makeDhcpDiscoverPacket(uint8_t packet[])
{
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    ip->revSize = 0x45;
    udpFrame* udp = (udpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));
    dhcpFrame* dhcp = (dhcpFrame*)&udp->data;

    uint8_t i;
    uint16_t udpSize = 259;
    uint16_t tmp16;


    for (i = 0; i < HW_ADD_LENGTH; i++)
    ether->destAddress[i] = 0xFF;
    etherGetMacAddress(&ether->sourceAddress[0]);
    ether->frameType = htons(0x0800);

    ip->typeOfService = 0;
    ip->length = htons(328); //ethernet 14bytes + IP frame 20 bytes + UDP 8 bytes + 258 DHCP for discover.
    ip->id = 0;
    ip->flagsAndOffset = 0;
    ip->ttl = 128;
    ip->protocol = 17;
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        ip->destIp[i] = 255;
        ip->sourceIp[i] = 0;
    }


    udp->sourcePort = htons(68);
    udp->destPort = htons(67);
    udp->length = htons(308);


    dhcp->op = 1;
    dhcp->htype = 1;
    dhcp->hlen = 6;
    dhcp->hops = 0;
    dhcp->xid = 0;
    dhcp->secs = 0;
    dhcp->flags = 0;
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        dhcp->ciaddr[i] = 0;
        dhcp->yiaddr[i] = 0;
        dhcp->siaddr[i] = 0;
        dhcp->giaddr[i] = 0;
        dhcp->chaddr[i] = 0;
        dhcp->chaddr[i+4] = 0;
        dhcp->chaddr[i+8] = 0;
        dhcp->chaddr[i+12] = 0;
    }

    etherGetMacAddress(&dhcp->chaddr[0]);
    dhcp->magicCookie = (htons(0x5363) << 16)+htons(0x6382);

    for(i = 0; i < 192; i++)
        dhcp->data[i] = 0;

    dhcp->options[0] = 0x35;
    dhcp->options[1] = 0x01;
    dhcp->options[2] = 0x01;
    dhcp->options[3] = 61;
    dhcp->options[4] = 7;
    dhcp->options[5] = 0x01;
    etherGetMacAddress(&dhcp->options[6]);
    dhcp->options[12] = 0x37;
    dhcp->options[13] = 0x04;
    dhcp->options[14] = 0x01;
    dhcp->options[15] = 0x03;
    dhcp->options[16] = 0x0f;
    dhcp->options[17] = 0x06;
    dhcp->options[18] = 0xff;



//ip header size + udp header size + dhcp packet size
    ip->length = htons(((ip->revSize & 0xF) * 4) + 8 + udpSize);
    // 32-bit sum over ip header
    sum = 0;
    etherSumWords(&ip->revSize, 10);
    etherSumWords(ip->sourceIp, ((ip->revSize & 0xF) * 4) - 12);
    ip->headerChecksum = getEtherChecksum();
    udp->length = htons(8 + udpSize);  //vikram - udp frame + dhcp packet size
    // 32-bit sum over pseudo-header
    sum = 0;
    etherSumWords(ip->sourceIp, 8);
    tmp16 = ip->protocol;
    sum += (tmp16 & 0xff) << 8;
    etherSumWords(&udp->length,  2);
    // add udp header except crc
    etherSumWords(udp, 6);
    etherSumWords(&udp->data, udpSize);
    udp->check = getEtherChecksum();

    // send packet with size = ether + udp hdr + ip header + udp_size
    etherPutPacket(ether, 22 + ((ip->revSize & 0xF) * 4) + udpSize);
}


uint8_t etherIsDhcp(uint8_t packet[])
{
    //Returns the following values.
    //ok = 1 DHCP packet.
    //ok = 2 DHCP offer.
    //ok = 3 DHCP ACK.
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    udpFrame* udp = (udpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));
    dhcpFrame* dhcp = (dhcpFrame*)&udp->data;
    uint8_t ok;
    ok = ((ether->frameType == htons(0x0800)) && (ip->protocol == 17) && (udp->sourcePort == htons(67)) && (udp->destPort == htons(68)) && (dhcp->op == 2));
//     && (dhcp->options[0] == 53)
    char str[10];
    sprintf(str, "%02x", ip->protocol);
    putsUart0(str);
    putsUart0("\r\n");
    if(ok == 1)
    {
        if(dhcp->options[2] == 2)
            ok = 2;
        if(dhcp->options[2] == 5)
            ok = 3;
    }

    return ok;
}


void makeDhcpRequestPacket(uint8_t packet[])
{
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    udpFrame* udp = (udpFrame*)((uint8_t*)ip + 20);
    dhcpFrame* dhcp = (dhcpFrame*)&udp->data;

    uint8_t i;
    uint16_t udpSize = 250;
    uint16_t tmp16;
//
    for (i = 0; i < 6; i++)
        ether->destAddress[i] = 0xFF;
    etherGetMacAddress(&ether->sourceAddress[0]);
    ether->frameType = htons(0x0800);


    ip->revSize = 0x45;
    ip->typeOfService = 0;
    ip->length = htons(328);
    ip->id = 0;
    ip->flagsAndOffset = 0;
    ip->ttl = 128;
    ip->protocol = 17;
    for (i = 0; i < 4; i++)
    {
        ip->destIp[i] = 255;
        ip->sourceIp[i] = 0;
        dhcp->yiaddr[i] = 0;
    }

    udp->sourcePort = htons(68);
    udp->destPort = htons(67);
    udp->length = htons(308);


    dhcp->op = 1;
    dhcp->htype = 1;
    dhcp->hlen = 6;
    dhcp->hops = 0;
    dhcp->xid = 0;
    dhcp->secs = 0;
    dhcp->flags = 0;

    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        dhcp->ciaddr[i] = 0;
        dhcp->yiaddr[i] = 0;
        dhcp->siaddr[i] = 0;
        dhcp->giaddr[i] = 0;
        dhcp->chaddr[i] = 0;
        dhcp->chaddr[i+4] = 0;
        dhcp->chaddr[i+8] = 0;
        dhcp->chaddr[i+12] = 0;
    }

    etherGetMacAddress(&dhcp->chaddr[0]);
    dhcp->magicCookie = (htons(0x5363) << 16)+htons(0x6382);

    for(i = 0; i < 192; i++)
        dhcp->data[i] = 0;

    dhcp->options[0] = 0x35;
    dhcp->options[1] = 0x01;
    dhcp->options[2] = 0x03;
    dhcp->options[3] = 54;
    dhcp->options[4] = 0x04;
    dhcp->options[5] = dhcp->siaddr[0];
    dhcp->options[6] = dhcp->siaddr[1];
    dhcp->options[7] = dhcp->siaddr[2];
    dhcp->options[8] = dhcp->siaddr[3];
    dhcp->options[9] = 0xff;



//ip header size + udp header size + dhcp packet size
    ip->length = htons(((ip->revSize & 0xF) * 4) + 8 + udpSize);
    // 32-bit sum over ip header
    sum = 0;
    etherSumWords(&ip->revSize, 10);
    etherSumWords(ip->sourceIp, ((ip->revSize & 0xF) * 4) - 12);
    ip->headerChecksum = getEtherChecksum();
    udp->length = htons(8 + udpSize);  //vikram - udp frame + dhcp packet size
    // 32-bit sum over pseudo-header
    sum = 0;
    etherSumWords(ip->sourceIp, 8);
    tmp16 = ip->protocol;
    sum += (tmp16 & 0xff) << 8;
    etherSumWords(&udp->length, 2);
    // add udp header except crc
    etherSumWords(udp, 6);
    etherSumWords(&udp->data, udpSize);
    udp->check = getEtherChecksum();

    // send packet with size = ether + udp hdr + ip header + udp_size
    etherPutPacket(ether, 22 + ((ip->revSize & 0xF) * 4) + udpSize);
}


    void flash()
    {
        putsUart0("\nGOT DHCP OFFER\n");
    }


void acceptDhcp(uint8_t packet[])
{
    etherFrame* ether = (etherFrame*)packet;
    ipFrame* ip = (ipFrame*)&ether->data;
    udpFrame* udp = (udpFrame*)((uint8_t*)ip + ((ip->revSize & 0xF) * 4));
    dhcpFrame* dhcp = (dhcpFrame*)&udp->data;

    etherSetIpAddress(dhcp->yiaddr[0],dhcp->yiaddr[1],dhcp->yiaddr[2],dhcp->yiaddr[3]);
    etherSetIpSubnetMask(dhcp->options[5],dhcp->options[6],dhcp->options[7],dhcp->options[8]);
    etherSetIpGatewayAddress(dhcp->options[11], dhcp->options[12], dhcp->options[13], dhcp->options[14]);
    etherSetIpDnsAddress(dhcp->options[20],dhcp->options[21],dhcp->options[22],dhcp->options[23]);
    dhcpLeaseTime = ((dhcp->options[30] << 24) + (dhcp->options[31] << 16) + (dhcp->options[32] << 8) + (dhcp->options[33]));
}

void ifconfig()
{
    uint8_t i;
    char str[10];
    uint8_t mac[6];
    uint8_t ip[4];
    uint8_t dns[4];
    etherGetMacAddress(mac);
    putsUart0("\rHW: ");
    for (i = 0; i < 6; i++)
    {
        sprintf(str, "%02x", mac[i]);
        putsUart0(str);
        if (i < 6-1)
            putcUart0(':');
    }
    putcUart0('\n');
    etherGetIpAddress(ip);
    putsUart0("\rIP: ");
    for (i = 0; i < 4; i++)
    {
        sprintf(str, "%u", ip[i]);
        putsUart0(str);
        if (i < 4-1)
            putcUart0('.');
    }
    if (etherIsDhcpEnabled())
        putsUart0(" (dhcp)");
    else
        putsUart0(" (static)");
    putcUart0('\n');
    etherGetIpSubnetMask(ip);
    putsUart0("\rSN: ");
    for (i = 0; i < 4; i++)
    {
        sprintf(str, "%u", ip[i]);
        putsUart0(str);
        if (i < 4-1)
            putcUart0('.');
    }
    putcUart0('\n');
    etherGetIpGatewayAddress(ip);
    putsUart0("\rGW: ");
    for (i = 0; i < 4; i++)
    {
        sprintf(str, "%u", ip[i]);
        putsUart0(str);
        if (i < 4-1)
            putcUart0('.');
    }
    putcUart0('\n');
    etherGetIpDnsAddress(dns);
    putsUart0("\rDNS: ");
    for (i = 0; i < 4; i++)
    {
        sprintf(str, "%u", dns[i]);
        putsUart0(str);
        if (i < 4-1)
            putcUart0('.');
    }
    putsUart0("\r\n");
    if (etherIsDhcpEnabled())
    {
        sprintf(str, "%u", dhcpLeaseTime);
        putsUart0(str);
    }
    putsUart0("\r\n");
    if (etherIsLinkUp())
        putsUart0("Link is up\n");
    else
        putsUart0("Link is down\n");
}

//-----------------------------------------------------------------------------
// DHCP timer functions
//-----------------------------------------------------------------------------
void t1_expired()
{
    putsUart0("\r\nT1 Expired\n");
    dhcpState = dhcpRenew;
    dhcpState();
}

void t2_expired()
{
    putsUart0("\r\nT2 Expired\n");
    dhcpState = dhcpRebinding;
    dhcpState();
}

void t3_expired()
{
    putsUart0("\r\nT3 Expired\n");
    dhcpState = dhcpInit;
    dhcpState();
}

//-----------------------------------------------------------------------------
// Call functions for state machine
//-----------------------------------------------------------------------------

void callDiscover()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nSent DHCP discover message\r\n");
#endif
    makeDhcpDiscoverPacket(data);
}
void callRequest()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nSent DHCP request message\r\n");
#endif
    makeDhcpRequestPacket(data);
}
//-----------------------------------------------------------------------------
// DHCP state machine functions
//-----------------------------------------------------------------------------

void dhcpInit()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nDHCP Init State\r\n");
#endif
    startPeriodicTimer(callDiscover, 10);
}

void dhcpSelecting()
{
    putsUart0("\r\nDHCP Selecting State\r\n");
    //Write ARP code.
    stopTimer(callDiscover);
    dhcpState = dhcpRequesting;
    dhcpState();
}

void dhcpRequesting()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nDHCP Requesting State\r\n");
#endif
    callRequest();
}

void dhcpBound()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nDHCP Bound State\r\n");
#endif
    acceptDhcp(data);

    startOneshotTimer(t1_expired, dhcpLeaseTime/2);
    startOneshotTimer(t2_expired, dhcpLeaseTime*0.875);
    startOneshotTimer(t3_expired, dhcpLeaseTime);
}

void dhcpRenew()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nDHCP Renew State\r\n");
#endif
    callRequest();
}

void dhcpRebinding()
{
#ifdef DHCP_STATE_MACHINE_DEBUG
    putsUart0("\r\nDHCP Rebinding State\r\n");
#endif
    startPeriodicTimer(callDiscover, 10);
}
