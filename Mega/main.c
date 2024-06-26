// Mega

#define __AVR_ATmega2560__
#define F_CPU 16000000UL
#define FOSC 16000000UL // Clock Speed
#define BAUD 57600
#define MYUBRR (FOSC/16/BAUD-1)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdbool.h>
#include <stdio.h>
#include <util/setbaud.h>
#include <string.h>
#include "delay.h"
#include "keypad.h"


// Board states in enum
enum STATE {
    ARMED,
    TIMER,
    UNLOCKED,
    ALARM
};

// nasty global variables
int seconds = 0;


char password[4] = {'1', '2', '3', '4'};
char input[4];
int inputIndex = 0;


// Course material UART setup is used
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
    // if signal from uno, switch to TIMER
    uint8_t spi_send_data = "0";
    uint8_t spi_receive_data;

    uint8_t sensorDetectValue = 4;
    
    
    while (1) {

        PORTB &= ~(1 << PB0); // SS LOW
            
        SPDR = spi_send_data;
        while(!(SPSR & (1 << SPIF)))
        {
            /* wait until the transmission is complete */
            ;
        }
        
        spi_receive_data = SPDR;
        printf("%u\n", spi_receive_data);
            
            
        PORTB |= (1 << PB0); // SS HIGH
        
        // If true value from Uno, break from loop and switch to TIMER state
        if (spi_receive_data == sensorDetectValue) {

            // Set up timer interrupts
            DDRE |= (1 << PE3);
            // Enable interrupts
            sei();
            // Clear registers for timer
            TCCR3A = 0;
            TCCR3B = 0;
            TCNT3 = 0;

            // CTC
            OCR3A = 15624;
            
            // Prescaler 1024, bits CS32 and CS30 on.
            TCCR3B |= 0b000000101;
            // Output compare Match A interrupt enable
            TIMSK3 |= (1 << 1);

            // Set up timer led;
            PORTH |= (1 << PINH5);
            printf("Timer started");
            return TIMER;
        }
        DELAY_ms(1000);
    }
    return TIMER;
}

enum STATE 
timerState()
{
    // Get key from keypad input
    uint8_t key = KEYPAD_GetKey();

    // Return to start of timer if NULL
    if (key == NULL)
    {
        return TIMER;
    } 
    // Key was pressed, show green led for 5ms
    PORTH |= (1 << PINH6);
    DELAY_ms(5);
    PORTH &= ~(1 << PINH6);
    printf("pressed: %c\n", key);

    // Backspace, reduce index by one and return
    if (key == '*' && inputIndex > 0) {
        inputIndex--;
        printf("backspace, index:%d", inputIndex);
        key == NULL;
        printf("Current input:%c%c%c%c", input[0], input[1], input[2], input[3]);
        return TIMER;
    } 
    else if (key == '#') //enter (#) pressed, check the password
    {
        if (inputIndex < 3) {
            printf("Inputindex too low! (%d) reset\n", inputIndex);
            PORTB ^= (1 << PINB4);
            DELAY_ms(10);
            PORTB ^= (1 << PINB4);
            inputIndex = 0;
            return TIMER;
        }
        // check if input is correct
        printf("checking input\n");
        
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
            printf("WRONG PASSWORD\n");
            // Wrong password, led and buzzer on for 10 milliseconds, reset index
            PORTB ^= (1 << PINB4);
            DELAY_ms(10);
            PORTB ^= (1 << PINB4);
            inputIndex = 0;
            return ALARM;
        }
    }
    else {
        if (inputIndex > 3) {
            // Too many inputs, reset the input.
            printf("Max input index, reset.\n");
            PORTB ^= (1 << PINB4);
            DELAY_ms(10);
            PORTB ^= (1 << PINB4);
            inputIndex = 0;
            return TIMER;
        }
        input[inputIndex] = key;
        inputIndex++;
        printf("Current input:%c%c%c%c", input[0], input[1], input[2], input[3]);
    }
    // Reset key to null
    key = NULL;
   return TIMER; 
}

enum STATE 
alarmState()
{
    // activate the buzzer and return to TIMER
    // Buzzer on until correct password
    printf("ALARM");
    // Turn timer off
    TCCR3B &= 0x00;
    
    PORTB |= (1 << PINB4);
    return TIMER;
}

ISR (TIMER3_COMPA_vect) // Timer 1 ISR
{
    PORTH ^= (1 << PINH5);
    TCNT3 = 0;
    seconds++;
    if (seconds > 9) { // 10 seconds passed, alarm on.
        alarmState();
    }
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
    KEYPAD_Init();

    // set the buzzer pin to output
    DDRB |= (1 << PINB4);

    // set unlock pin to output
    DDRH |= (1 << PINH6);

    // Set the timer led pin to output
    DDRH |= (1 << PINH5);
    
    // Set up timer led pin to output
    DDRA |= (1 << PINA2);
    // Initial state to armed
    enum STATE state = ARMED;

    #pragma endregion

    // SPI setup
    DDRB |= (1 << PINB0) | (1 << PINB1) | (1 << PINB2);
    SPCR |= (1 << 6) | (1 << 4);
    SPCR |= (1 << 0);



    // main program loop
    while (1)
    {
        switch (state)
        {
            case ARMED:
                state = armedState();
                break;
            case TIMER:
                state = timerState();
                break;
            case UNLOCKED:
                TCCR3B &= 0x00;
                // empty the input buffer
                memset(input, 0, sizeof(input));
                inputIndex = 0;
                // turn on the unlocked pin
                PORTH |= (1 << PINH6);
                // Turn off timer pin
                PORTH &= ~(1<< PINH5);
                // Turn off buzzer and alarm
                PORTB &= ~(1<<PINB4);
                // wait for A key from keypad to arm again
                while (KEYPAD_GetKey() != 'A') {
                    ;
                }
                // Reset unlocked pin
                PORTH &= ~(1 << PINH6);
                // Reset timer second counter
                seconds = 0;
                state = ARMED;
                break;
            case ALARM:
                state = alarmState();
                // Buzzer
                break;
            default:
                break;
        }
    }
}