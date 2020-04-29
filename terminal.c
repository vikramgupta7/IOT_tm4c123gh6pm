

/**
 * main.c
 */

#include<stdint.h>
#include<stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "uart0.h"
#include "terminal.h"
#include "commands.h"

#define DEBUG_TERMINAL

//int main(void)
//{
//    uart_command uart_command_var ={ .count = 0 }; //Setting initial count to zero.
//
//    // Initialize hardware
//    initHw();
//    initUart0();
//
//    setUart0BaudRate(115200, 40e6);
//
//    putsUart0("Enter Command\r\n");
//    putcUart0('>');
//
//    while(true)
//    {
//        getsUart0(&uart_command_var);
//    }
//	return 0;
//}


//void initHw()
//{
//    // Configure HW to work with 16 MHz XTAL, PLL enabled, system clock of 40 MHz
//    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
//
//    // Enable clocks
//    enablePort(PORTF);
//
//    // Configure LED and pushbutton pins
//    selectPinPushPullOutput(GREEN_LED);
//    selectPinPushPullOutput(RED_LED);
//}


void getsUart0(uart_command* uart_pointer)
{
    char c = getcUart0(); //Keep reading input from the terminal(UART).

    switch (c)
    {
        case 8: //Case for backspace.
            if(uart_pointer->count > 0)
            {
                putcUart0('\b');
                putcUart0(' ');
                putcUart0('\b');
                uart_pointer->count--;
            }
            break;
        case 13: //Case for new line.
            uart_pointer->buffer[uart_pointer->count]='\0';
            parseFields(uart_pointer);
            runCommands(uart_pointer);
            #ifdef DEBUG_TERMINAL
            putsUart0("\r\n");
            putsUart0("LINE BREAK");
            putsUart0("\r\n");
            #endif
            break;
        case 10: //Case of new line.
            uart_pointer->buffer[uart_pointer->count]='\0';
            parseFields(uart_pointer);
            runCommands();
            #ifdef DEBUG_TERMINAL
            putsUart0("\r\n");
            putsUart0("LINE BREAK");
            putsUart0("\r\n");
            #endif
            break;
        default:
            if(uart_pointer->count<=MAX_CHARS && c>=32)
            {
                putcUart0(c);
                uart_pointer->buffer[uart_pointer->count]=c;
                uart_pointer->count++;
            }
            else
            {
                uart_pointer->buffer[uart_pointer->count]='\0';
                #ifdef DEBUG_TERMINAL
                putsUart0("\r\n");
                putsUart0("CHARACTER LIMIT REACHED OR CHARACTER OUT OF SCOPE");
                putsUart0("\r\n");
                #endif
            }
        break;
    }
}

void parseFields(uart_command* uart_pointer)
{
    uart_pointer->fieldCount = 0;
    char current_field = 'z';
    uint8_t i = 0;
    for(i=0;(i<uart_pointer->count);i++)
    {
        if((uart_pointer->buffer[i]>=44 && uart_pointer->buffer[i]<=57))
        {
            if(current_field != 'n')
            {
                current_field = 'n';
                if(uart_pointer->fieldCount<MAX_FIELDS)
                {
                    uart_pointer->fieldType[uart_pointer->fieldCount] = current_field;
                    uart_pointer->fieldPosition[uart_pointer->fieldCount] = i;
                    uart_pointer->fieldCount++;
                }
            }
        }

        else if((uart_pointer->buffer[i]>=65 && uart_pointer->buffer[i]<=90) || (uart_pointer->buffer[i]>=97 && uart_pointer->buffer[i]<=122))
        {
            if(current_field !='a')
            {
                current_field = 'a';
                if(uart_pointer->fieldCount<MAX_FIELDS)
                {
                    uart_pointer->fieldType[uart_pointer->fieldCount] = current_field;
                    uart_pointer->fieldPosition[uart_pointer->fieldCount]=i;
                    uart_pointer->fieldCount++;
                }
            }
        }
        else
        {
            uart_pointer->buffer[i] = '\0';
            current_field = 'z';
        }
    }
    #ifdef DEBUG_TERMINAL
    for (i = 0; i < uart_pointer->fieldCount; i++)
    {
        putsUart0("\r\n");
        putcUart0(uart_pointer->fieldType[i]);
        putcUart0('\t');
        putsUart0(&uart_pointer->buffer[uart_pointer->fieldPosition[i]]);
    }
    #endif
    uart_pointer->count =0;
}



//
//void ifconfig()
//{
//    uint8_t i;
//    char str[10];
//    uint8_t mac[6];
//    uint8_t ip[4];
//    uint8_t dns[4];
//    etherGetMacAddress(mac);
//    putsUart0("\rHW: ");
//    for (i = 0; i < 6; i++)
//    {
//        sprintf(str, "%02x", mac[i]);
//        putsUart0(str);
//        if (i < 6-1)
//            putcUart0(':');
//    }
//    putcUart0('\n');
//    etherGetIpAddress(ip);
//    putsUart0("\rIP: ");
//    for (i = 0; i < 4; i++)
//    {
//        sprintf(str, "%u", ip[i]);
//        putsUart0(str);
//        if (i < 4-1)
//            putcUart0('.');
//    }
//    if (etherIsDhcpEnabled())
//        putsUart0(" (dhcp)");
//    else
//        putsUart0(" (static)");
//    putcUart0('\n');
//    etherGetIpSubnetMask(ip);
//    putsUart0("\rSN: ");
//    for (i = 0; i < 4; i++)
//    {
//        sprintf(str, "%u", ip[i]);
//        putsUart0(str);
//        if (i < 4-1)
//            putcUart0('.');
//    }
//    putcUart0('\n');
//    etherGetIpGatewayAddress(ip);
//    putsUart0("\rGW: ");
//    for (i = 0; i < 4; i++)
//    {
//        sprintf(str, "%u", ip[i]);
//        putsUart0(str);
//        if (i < 4-1)
//            putcUart0('.');
//    }
//    putcUart0('\n');
//    etherGetIpDnsAddress(dns);
//    putsUart0("\rDNS: ");
//    for (i = 0; i < 4; i++)
//    {
//        sprintf(str, "%u", dns[i]);
//        putsUart0(str);
//        if (i < 4-1)
//            putcUart0('.');
//    }
//    putsUart0("\r\n");
//    if (etherIsLinkUp())
//        putsUart0("Link is up\n");
//    else
//        putsUart0("Link is down\n");
//}

