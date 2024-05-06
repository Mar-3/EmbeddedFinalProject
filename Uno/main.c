// UNO

#define __AVR_ATmega328P__
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



// Board states in enum
enum boardStates {
  armed,
  timer,
  unlocked,
  alarm
};


// Course material UART setup is used
static void 
USART_init(uint16_t ubrr) // unsigned int
{
    /* Set baud rate in the USART Baud Rate Registers (UBRR) */
    UBRR0H = (unsigned char) (ubrr >> 8);
    UBRR0L = (unsigned char) ubrr;
    
    /* Enable receiver and transmitter on RX0 and TX0 */
    UCSR0B |= (1 << RXEN0) | (1 << TXEN0); //NOTE: the ATmega328p has 1 UART: 0
    
    /* Set frame format: 8 bit data, 2 stop bit */
    UCSR0C |= (1 << USBS0) | (3 << UCSZ00);
    
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
FILE uart_output = FDEV_SETUP_STREAM(USART_Transmit, NULL, _FDEV_SETUP_WRITE); FILE uart_input = FDEV_SETUP_STREAM(NULL, USART_Receive, _FDEV_SETUP_READ);

int main(void) 
{
    // initialize the UART with baud rate
    USART_init(MYUBRR);
   
    //redirect the stdin and stdout to USART for debugging
    stdout = &uart_output;
    stdin = &uart_input;

    // set pin 2 on the arduino uno to input for the motion sensor 
    DDRD &= ~(1 << PIND2);

    // set pin 8 on the arduino uno to output for the led
    DDRB |= (1 << PINB0);
     
    // SPI communication
    // 
    DDRB |= (1 << PINB4);
    SPCR = (1 << 6);

    uint8_t spi_send_data;
    uint8_t spi_receive_data;

    DELAY_ms(1000);
    while(1)
    {
        int sensorValue = PIND & (1 << PIND2);
        printf("%d", sensorValue);
        spi_send_data = sensorValue;
        
        while(!(SPSR & (1 << SPIF)))
        {
            /* wait until the transmission is complete */
            ;
        }
        SPDR = spi_send_data; // send byte using SPI data register
        DELAY_ms(200);
    }
}   