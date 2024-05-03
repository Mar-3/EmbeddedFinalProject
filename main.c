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
#include "keypad.h"
#include "delay.h"

static char correctPasscode[] = "1234";

// Board states in enum
enum boardStates {
  armed,
  timer,
  unlocked,
  alarm
};

// NOTE: USART = Universal Synchronous Asynchronous Receiver Transmitter
//       UART  = Universal Asynchronous Receiver Transmitter

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


// Board state used when device is armed 
enum boardStates armedState(int sensorValue) {
  // If sensor value is above threshold, start timer
  // Else return the same state (armed)
  if (sensorValue > 1000) {
    printf("Movement detected, switching to timer");
    return timer;
  }
  return armed;
};


// Get input from keypad, return input when 4 digits are inputted
enum boardStates getInputState() {
  uint8_t key;
  key = KEYPAD_GetKey();
  printf("%s\n", key);
  if (key == '1') {
    return unlocked;
  }
  return alarm;
  
};

int unlockedState(void) {
  // TODO show username on screen?
  //char key = KEYPAD_GetKey(); 
  puts("Unlocked! Press 'A' to arm");
  uint8_t key = KEYPAD_GetKey();
  if (key=='A') {
    return armed;
  }
  return unlocked;
};


int alarmState(void) {
  // TODO get input for correct password?
  int buzzer = 0;
  while (1) {
    buzzer = 1000;
    PORTB ^= (1 << PINB1);
    _delay_ms(500);
    printf("%d", buzzer);
  }

};

int main(void) {


 // initialize the UART with baud rate
USART_init(MYUBRR);
   
//redirect the stdin and stdout to USART for debugging
stdout = &uart_output;
stdin = &uart_input;
 
KEYPAD_Init();

// Starting with timer state to test input 
enum boardStates state = timer;
 
while (1) {
   switch (state) {
    case armed:
      state = armedState(1001);
      break;
    case timer:
      state = getInputState();
      break;
     case unlocked:
      state = unlockedState();
      break;
     case alarm:
      state = alarmState();
      break;
   }

 }
};