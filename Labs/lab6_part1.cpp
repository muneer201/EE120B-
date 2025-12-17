I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*
*/



#include <avr/interrupt.h>
#include <avr/io.h>

#include "serialATmega.h"
#include "timerISR.h"

unsigned char direction = 0;
unsigned char button_count = 0;
unsigned char prev_button = 0;

unsigned char segment_patterns[5] = {0b01000000, 0b01011110, 0b00111110, 0b00111000, 0b01010000};

void ADC_init() {
 ADMUX = 0b01000000;
 ADCSRA = 0b10000111;
 ADCSRB = 0b00000000;
}

unsigned int ADC_read(unsigned char chnl) {
 ADMUX = (ADMUX & 0b11111000) | (chnl & 0b00001111);
 ADCSRA = ADCSRA | 0b01000000;
 while ((ADCSRA >> 6) & 0x01) {
 }

 uint8_t low, high;
 low = ADCL;
 high = ADCH;

 return ((high << 8) | low);
}

#define NUM_TASKS 3

const unsigned long JOYSTICK_READ_PERIOD = 50;
const unsigned long DISPLAY_PERIOD = 50;
const unsigned long BUTTON_PERIOD = 100;
const unsigned long GCD_PERIOD = 50;

typedef struct _task {
 int state;
 unsigned long period;
 unsigned long elapsedTime;
 int (*TickFct)(int);
} task;

task tasks[NUM_TASKS];

unsigned char GetJoystickDirection() {
 unsigned int x_val = ADC_read(3);
 unsigned int y_val = ADC_read(2);

 if (x_val < 300){
  return 3;
 } 
 if (x_val > 700){
  return 4;
 }
 if (y_val < 300){
  return 1;
 } 
 if (y_val > 700){
  return 2;
 } 
 return 0;
}

enum JoystickState { JOYSTICK_INIT, JOYSTICK_READ };

int TickFct_Joystick(int state) {
 switch (state) {
  case JOYSTICK_INIT:
   state = JOYSTICK_READ;
   break;
  case JOYSTICK_READ:
   state = JOYSTICK_READ;
   break;
  default:
   state = JOYSTICK_INIT;
   break;
 }

 switch (state) {
  case JOYSTICK_READ:
   direction = GetJoystickDirection();
   break;
  default:
   break;
 }

 return state;
}

enum DisplayState { DISPLAY_INIT, DISPLAY_UPDATE };

int TickFct_Display(int state) {
 switch (state) {
  case DISPLAY_INIT:
   state = DISPLAY_UPDATE;
   break;
  case DISPLAY_UPDATE:
   state = DISPLAY_UPDATE;
   break;
  default:
   state = DISPLAY_INIT;
   break;
 }

 switch (state) {
  case DISPLAY_UPDATE:
   if (direction >= 0 && direction <= 4) {
    unsigned char pattern = segment_patterns[direction];

    PORTD &= ~((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
    PORTB &= ~((1 << 0) | (1 << 1));

    if (pattern & 0x01) {
     PORTD |= (1 << 7);
    }
    if (pattern & 0x02) {
     PORTD |= (1 << 6);
    }
    if (pattern & 0x04) {
     PORTD |= (1 << 5);
    }
    if (pattern & 0x08) {
     PORTD |= (1 << 4);
    }
    if (pattern & 0x10) {
     PORTD |= (1 << 3);
    }
    if (pattern & 0x20) {
     PORTD |= (1 << 2);
    }
    if (pattern & 0x40) {
     PORTB |= (1 << 0);
    }
    if (pattern & 0x80) {
     PORTB |= (1 << 1);
    }
   }
   break;

  default:
   break;
 }

 return state;
}

enum ButtonState { BUTTON_INIT, BUTTON_CHECK };

int TickFct_Button(int state) {
 unsigned char button;
 if (PINC & (1 << 4)) {
  button = 0;
 } else {
  button = 1;
 }

 switch (state) {
  case BUTTON_INIT:
   prev_button = button;
   state = BUTTON_CHECK;
   break;
  case BUTTON_CHECK:
   state = BUTTON_CHECK;
   break;
  default:
   state = BUTTON_INIT;
   break;
 }

 switch (state) {
  case BUTTON_CHECK:
   if (button && !prev_button) {
    button_count = (button_count + 1) % 4;

    PORTC = (PORTC & 0xFC) | button_count;
   }
   prev_button = button;
   break;
  default:
   break;
 }

 return state;
}

void TimerISR() {
 for (unsigned int i = 0; i < NUM_TASKS; i++) {
  if (tasks[i].elapsedTime == tasks[i].period) {
   tasks[i].state = tasks[i].TickFct(tasks[i].state);
   tasks[i].elapsedTime = 0;
  }
  tasks[i].elapsedTime += GCD_PERIOD;
 }
}

int main(void) {
 DDRC = 0x03;
 PORTC = 0x10;
 DDRD = 0xFC;
 PORTD = 0x00;
 DDRB = 0x3F;
 PORTB = 0x00;

 ADC_init();

 unsigned char i = 0;

 tasks[i].period = JOYSTICK_READ_PERIOD;
 tasks[i].state = JOYSTICK_INIT;
 tasks[i].elapsedTime = JOYSTICK_READ_PERIOD;
 tasks[i].TickFct = &TickFct_Joystick;
 i++;

 tasks[i].period = DISPLAY_PERIOD;
 tasks[i].state = DISPLAY_INIT;
 tasks[i].elapsedTime = DISPLAY_PERIOD;
 tasks[i].TickFct = &TickFct_Display;
 i++;

 tasks[i].period = BUTTON_PERIOD;
 tasks[i].state = BUTTON_INIT;
 tasks[i].elapsedTime = BUTTON_PERIOD;
 tasks[i].TickFct = &TickFct_Button;

 TimerSet(GCD_PERIOD);
 TimerOn();

 while (1) {
 }

 return 0;
}