// Ethernet Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL w/ ENC28J60
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// ENC28J60 Ethernet controller on SPI0
//   MOSI (SSI0Tx) on PA5
//   MISO (SSI0Rx) on PA4
//   SCLK (SSI0Clk) on PA2
//   ~CS (SW controlled) on PA3
//   WOL on PB3
//   INT on PC6

// Pinning for IoT projects with wireless modules:
// N24L01+ RF transceiver
//   MOSI (SSI0Tx) on PA5
//   MISO (SSI0Rx) on PA4
//   SCLK (SSI0Clk) on PA2
//   ~CS on PE0
//   INT on PB2
// Xbee module
//   DIN (UART1TX) on PC5
//   DOUT (UART1RX) on PC4

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "eth0.h"
#include "gpio.h"
#include "spi0.h"
#include "uart0.h"
#include "wait.h"
#include "terminal.h"
#include "ethernet.h"
#include "dhcp.h"
#include "timer.h"
#include "tcp.h"
//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    int i = 0;
    for(i = 0; i<MAX_PACKET_SIZE; i++)
        data[i] = 0;
    uart_command uart_command_var ={.count = 0 }; //Variable for terminal.
    uint8_t* udpData;


    // Init controller
    initHw();
    initTimer();

    // Setup UART0
    initUart0();
    setUart0BaudRate(115200, 40e6);

    // Init ethernet interface (eth0)
    putsUart0("\nStarting eth0\n");
    etherInit(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX);
    etherSetMacAddress(2, 3, 4, 5, 6, 7);
    etherDisableDhcpMode();
    etherSetIpAddress(192, 168, 1, 199);
    etherSetIpSubnetMask(255, 255, 255, 0);
    etherSetIpGatewayAddress(192, 168, 1, 1);
    waitMicrosecond(100000);
    displayConnectionInfo();

    // Flash LED
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(100000);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(100000);


    etherEnableDhcpMode();
    dhcpState = dhcpInit;
    if(etherIsDhcpEnabled())
        dhcpState();



    // Main Loop
    // RTOS and interrupts would greatly improve this code,
    // but the goal here is simplicity
    while (true)
    {
        // Put terminal processing here
        if (kbhitUart0())
        {
            getsUart0(&uart_command_var);
        }

        // Packet processing
        if (etherIsDataAvailable())
        {
            if (etherIsOverflow())
            {
                setPinValue(RED_LED, 1);
                waitMicrosecond(100000);
                setPinValue(RED_LED, 0);
            }

            // Get packet
            etherGetPacket(data, MAX_PACKET_SIZE);

            // Handle ARP request
            if (etherIsArpRequest(data))
            {
                putsUart0("GOT ARP PACKET");
                etherSendArpResponse(data);
            }

            // Handle IP datagram
            if (etherIsIp(data))
            {
            	if (etherIsIpUnicast(data))
            	{
            		// handle icmp ping request
					if (etherIsPingRequest(data))
					{
					    putsUart0("GOT PING PACKET");
					  etherSendPingResponse(data);
					}

					// Process UDP datagram
					// test this with a udp send utility like sendip
					//   if sender IP (-is) is 192.168.1.198, this will attempt to
					//   send the udp datagram (-d) to 192.168.1.199, port 1024 (-ud)
					// sudo sendip -p ipv4 -is 192.168.1.198 -p udp -ud 1024 -d "on" 192.168.1.199
                    // sudo sendip -p ipv4 -is 192.168.1.198 -p udp -ud 1024 -d "off" 192.168.1.199
					if (etherIsUdp(data))
					{
					    putsUart0("GOT UDP PACKET");
						udpData = etherGetUdpData(data);
						if (strcmp((char*)udpData, "on") == 0)
			                setPinValue(GREEN_LED, 1);
                        if (strcmp((char*)udpData, "off") == 0)
			                setPinValue(GREEN_LED, 0);
						etherSendUdpResponse(data, (uint8_t*)"Received", 9);
					}
                }
            }

            if(etherIsDhcpEnabled())
            {
                uint8_t dhcpReply = etherIsDhcp(data);
                if(dhcpReply == 2)
                {
                    putsUart0("\r\nGOT DHCP OFFER\r\n");
                    dhcpState = dhcpSelecting;
                    dhcpState();
                }
                else if(dhcpReply == 3)
                {
                    putsUart0("\r\nGOT DHCP ACK\r\n");
                    dhcpState = dhcpBound;
                    dhcpState();
                    ifconfig();
                }
            }
            uint8_t tcpReply = etherIsTcp(data);
            if(tcpReply == 2)
            {
                putsUart0("\r\nSYN Recieved\n");
                tcpState = tcpSynRecieved;
                tcpState();
            }
            else if(tcpReply == 3)
            {
                putsUart0("\r\nACK Recieved. Established State\n");
                tcpState = tcpEstablished;
                tcpState();
            }
            if(tcpState == tcpEstablished)
            {
                if(tcpReply == 4)
                {
                    putsUart0("GOT DATA");
                    tcpAcknowledge(data);
                }
            }
        }

    }
}

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    // Enable clocks
    enablePort(PORTF);
    _delay_cycles(3);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(BLUE_LED);
    selectPinDigitalInput(PUSH_BUTTON);
}

void displayConnectionInfo()
{
    uint8_t i;
    char str[10];
    uint8_t mac[6];
    uint8_t ip[4];
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
    if (etherIsLinkUp())
        putsUart0("\rLink is up\n\r");
    else
        putsUart0("\rLink is down\n\r");
}


