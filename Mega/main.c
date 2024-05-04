// Mega

#define __AVR_ATmega2560__
#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 57600
#define MYUBRR (FOSC/16/BAUD-1)

#include <avr/io.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/setbaud.h>
#include <string.h>
#include "delay.h"
#include "keypad.h"

#define READ_PIN(port, pin) (PIN ## port & (1 << PIN ## pin))

// Board states in enum
enum STATE {
    ARMED,
    TIMER,
    UNLOCKED,
    ALARM
};

char keypad[4][4] = {
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

char password[4] = {'1', '2', '3', '4'};
char input[4];
int inputIndex = 0;

#pragma region USART
static void 
USART_init(uint16_t ubrr) // unsigned int
{
    /* Set baud rate in the USART Baud Rate Registers (UBRR) */
    UBRR0H = (unsigned char) (ubrr >> 8);
    UBRR0L = (unsigned char) ubrr;
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //NOTE: the ATmega328p has 1 UART: 0
    // UCSR0B |= (1 << 4) | (1 << 3);
    
    /* Set frame format: 8 bit data, 2 stop bit */
    UCSR0C |= (1 << USBS0) | (3 << UCSZ00);
    // UCSR0C |= (1 << 3) | (3 << 1);
    
}

static void
USART_Transmit(unsigned char data, FILE *stream)
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << UDRE0)))
    {
        ;
    }
        
    /* Put the data into a buffer, then send/transmit the data */
    UDR0 = data;
}

static char
USART_Receive(FILE *stream)
{
    /* Wait until the transmit buffer is empty*/
    while(!(UCSR0A & (1 << UDRE0)))
    {
        ;
    }
    
    /* Get the received data from the buffer */
    return UDR0;
}

// Setup the stream functions for UART
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE);
FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

#pragma endregion

enum STATE 
armedState()
{
    // listen for keypad input and signal from uno
    // if signal from uno, switch to TIMER
    // if keypad input, switch to UNLOCKED or ALARM 
    
    // check if keypad input
    return TIMER;
}

enum STATE 
timerState()
{
   // TODO: start timer for 10 seconds

    uint8_t key = KEYPAD_GetKey();
    printf("Key: %c\n", key);
    
    return TIMER;

    // add key to input
    input[inputIndex] = key;
    inputIndex++;

    if (inputIndex == 4)
    {
        // check if input is correct
        bool correct = true;
        for (int i = 0; i < 4; i++)
        {
            if (input[i] != password[i])
            {
                correct = false;
            }
        }

        if (correct)
        {
            return UNLOCKED;
        } else {
            return ALARM;
        }
    }

    // print the keypad inputs to stdout

    // if keypad input is correct, switch to UNLOCKED

    // if keypad input is incorrect, switch to ALARM


    // if timer expires, switch to ALARM
    return UNLOCKED;
}

void
initKeypad() 
{

    
    // keypad is connected to physical pins 2-9
    // set the pins to input
    DDRD &= ~(1 << PIND2);
    DDRD &= ~(1 << PIND3);
    DDRD &= ~(1 << PIND4);
    DDRD &= ~(1 << PIND5);
    DDRD &= ~(1 << PIND6);
    DDRD &= ~(1 << PIND7);
    DDRB &= ~(1 << PINB0);
    DDRB &= ~(1 << PINB1);
    
    // set the pins to pull-up
    PORTD |= (1 << PIND2);
    PORTD |= (1 << PIND3);
    PORTD |= (1 << PIND4);
    PORTD |= (1 << PIND5);
    PORTD |= (1 << PIND6);
    PORTD |= (1 << PIND7);
    PORTB |= (1 << PINB0);
    PORTB |= (1 << PINB1);

}

int 
main(void)
{
    #pragma region init
    // init USART
    USART_init(MYUBRR);
    stdout = &uart_output;
    stdin = &uart_input;

    // init keypad
    //initKeypad();
    KEYPAD_Init();

    // init state
    enum STATE state = TIMER;
    #pragma endregion
    printf("Starting...\n");

    // main program loop
    while (1)
    {
        switch (state)
        {
            case ARMED:
                printf("ARMED\n");
                // listen for keypad input and signal from uno
                // if signal from uno, switch to TIMER
                // if keypad input, switch to UNLOCKED or ALARM 
                state = armedState();
                break;
            case TIMER:
                state = timerState();
                break;
            case UNLOCKED:
                printf("UNLOCKED\n");
                break;
            case ALARM:
                printf("ALARM\n");
                break;
            default:
                break;
        }
    }


}