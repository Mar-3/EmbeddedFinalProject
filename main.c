// UNO
#define __AVR_ATmega328P__

#include "include/keypad.h"

static char correctPasscode[4] = {'1','2','3','4'};

// Used as reference for keypad https://arduinogetstarted.com/tutorials/arduino-keypad
#define ROW_NUM 4 //four rows
#define COLUMN_NUM 4 //three columns

// Keyboard inputs
static char keys[ROW_NUM][COLUMN_NUM] = {
  {'1','2','3', 'A'},
  {'4','5','6', 'B'},
  {'7','8','9', 'C'},
  {'*','0','#', 'D'}
};


uint8_t pin_rows[ROW_NUM] = {9, 8, 7, 6}; //connect to the row pinouts of the keypad
uint8_t pin_column[COLUMN_NUM] = {5, 4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), pin_rows, pin_column, ROW_NUM, COLUMN_NUM );


// Board states in enum
enum boardStates {
  armed,
  timer,
  checkingInput,
  unlocked,
  alarm
};

// Board state used when device is armed 
enum boardStates armedState(int sensorValue) {
  // If sensor value is above threshold, start timer
  // Else return the same state (armed)
  if (sensorValue > 1000) {
    return timer;
  }
  return armed;
};


// Get input from keypad, return input when 4 digits are inputted
enum boardStates getInput(char* input[]) {
  char key = keypad_GetKey();
  if (key) {
    // TODO add inputted key to the end of array (Dynamic array?)
    char input[sizeof(input) / sizeof(char) + 1] = key;
    if (sizeof(input) / sizeof(char)) {
      return checkingInput;
    }
  }
  return timer;
  
};

enum boardStates checkInput(char input[]) {
  if (correctPasscode == input) {
    return unlocked;
  }
  return alarm;
};

int unlockedState(void) {
  // TODO show username on screen
  char key = keypad_GetKey();
  if (key == 'A') {
    return armed;
  }
  return unlocked;
};


int alarmState() {
  // TODO get input for correct password?
  int buzzer = 0;
  while (1) {
    buzzer = 1000;
  }

};

int main(void) {
  enum boardStates state = unlocked;
  while (1) {

  }
};