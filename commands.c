/*
 * commands.c
 *
 *  Created on: Apr 27, 2020
 *      Author: Vikram's ThinkPad
 */

#include "commands.h"
#include "terminal.h"
#include "uart0.h"
#include "dhcp.h"
#include "ethernet.h"

#define DEBUG_COMMANDS

void runCommands(uart_command* uart_pointer)
{
#ifdef DEBUG_COMMANDS
    putsUart0("\r\nCommand runs\r\n");
#endif
    uint8_t i = 0;
    uint16_t sum =0;
    for(i=uart_pointer->fieldPosition[0];uart_pointer->buffer[i]!='\0';i++)
    {
        sum = sum+uart_pointer->buffer[i];
    }
    switch (sum)
    {
        case 415: //
            sum = 0;
#ifdef DEBUG_COMMANDS
            putsUart0("\r\nGot DHCP command\r\n");
#endif
            for(i=uart_pointer->fieldPosition[1];uart_pointer->buffer[i]!='\0';i++)
                sum = sum + uart_pointer->buffer[i];
            if(sum == 221) //on
            {
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot DHCP ON command\r\n");
#endif
                makeDhcpDiscoverPacket(data);
//                if(readEeprom(0x0) != 0xFFFFFFFF)
//                    writeEeprom(0x0,0xFFFFFFFF);
            }

            else if(sum == 315) //off
            {
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot DHCP OFF command\r\n");
#endif
//                if(readEeprom(0x0) != 0xF0F0F0F0)
//                    writeEeprom(0x0,0xF0F0F0F0);
            }

            else if(sum == 737) // release
            {
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot DHCP BIG RELEASE command\r\n");
#endif
//                if(etherIsDhcpEnabled())
//                {
//                    state = init_state;
//                    state();
//                }
//                elseif(!etherIsDhcpEnabled())
//                putsUart0("\r\nDHCP is off\r\n");
            }
            else if(sum == 751) //renew
            {
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot DHCP BIG REFRESH command\r\n");
#endif
//                if(etherIsDhcpEnabled())
//                {
//                    state = init_state;
//                    state();
//                }
//                elseif(!etherIsDhcpEnabled())
//                putsUart0("\r\nDHCP is off\r\n");

            }
            break;

        case 332:
            sum = 0;
#ifdef DEBUG_COMMANDS
            putsUart0("\r\nGot SET command\r\n");
#endif
            for(i=uart_pointer->fieldPosition[1];uart_pointer->buffer[i]!='\0';i++)
                sum = sum + uart_pointer->buffer[i];
//
//            uint8_t para_pos[4] = {0};
//            uint8_t para[4] = {0};
//            uint8_t j = 0;
//            para_pos[j] = uart_pointer->fieldPosition[2];
//            for(i=uart_pointer->fieldPosition[2];uart_pointer->buffer[i]!='\0';i++)
//            {
//                if(uart_pointer->buffer[i]=='.')
//                {
//                    uart_pointer->buffer[i]='\0';
//                    j++;
//                    para_pos[j] = i+1;
//                }
//            }
//
//            para[0] = (uint8_t)atoi(&uart_pointer->buffer[para_pos[0]]);
//            para[1] = (uint8_t)atoi(&uart_pointer->buffer[para_pos[1]]);
//            para[2] = (uint8_t)atoi(&uart_pointer->buffer[para_pos[2]]);
//            para[3] = (uint8_t)atoi(&uart_pointer->buffer[para_pos[3]]);
            switch (sum)
            {
            case 217: //ip
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot SET IP command\r\n");
#endif
//                etherSetIpAddress(para[0], para[1], para[2], para[3]);
//                writeEeprom(0x1,(para[0]<<24)+(para[1]<<16)+(para[2]<<8)+(para[3]));
                break;
            case 225:
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot SET SUBNET command\r\n");
#endif
//                etherSetIpSubnetMask(para[0], para[1], para[2], para[3]);
//                writeEeprom(0x2,(para[0]<<24)+(para[1]<<16)+(para[2]<<8)+(para[3]));
                break;
            case 222: //gateway
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot SET GATEWAY command\r\n");
#endif
//                etherSetIpGatewayAddress(para[0], para[1], para[2], para[3]);
//                writeEeprom(0x3,(para[0]<<24)+(para[1]<<16)+(para[2]<<8)+(para[3]));
                break;
            case 325: //dns
#ifdef DEBUG_COMMANDS
                putsUart0("\r\nGot SET DNS command\r\n");
#endif
//                etherSetIpDnsAddress(para[0], para[1], para[2], para[3]);
//                writeEeprom(0x4,(para[0]<<24)+(para[1]<<16)+(para[2]<<8)+(para[0]));
                break;
            default:
                break;
            }
            break;

        case 837:
#ifdef DEBUG_COMMANDS
            putsUart0("\r\nGot IFCONFIG command\r\n");
#endif
//            ifconfig();
            break;

        case 651:
#ifdef DEBUG_COMMANDS
            putsUart0("\r\nGot REBOOT command\r\n");
#endif
//            NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ ;
            break;
        default:
            break;
    }
}

