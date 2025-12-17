I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*
*/



#include "helper.h"
#include "periph.h"
#include "timerISR.h"
#define NUM_TASKS 2

const unsigned long LED_PERIOD = 300;
const unsigned long BUZZ_PERIOD = 50;



typedef struct _task {
 signed char state;          // Task's current state
 unsigned long period;       // Task period
 unsigned long elapsedTime;  // Time elapsed since last task tick
 int (*TickFct)(int);        // Task tick function
} task;
// TODO: Define Periods for each task
//  e.g. const unsined long TASK1_PERIOD = <PERIOD>
const unsigned long GCD_PERIOD = 1;  // TODO:Set the GCD Period
task tasks[NUM_TASKS];               // declared task array with 5 tasks

void TimerISR() {
 for (unsigned int i = 0; i < NUM_TASKS;
      i++) {  // Iterate through each task in the task array
  if (tasks[i].elapsedTime ==
      tasks[i].period) {  // Check if the task is ready to tick
   tasks[i].state = tasks[i].TickFct(
       tasks[i].state);       // Tick and set the next state for this task
   tasks[i].elapsedTime = 0;  // Reset the elapsed time for the next tick
  }
  tasks[i].elapsedTime +=
      GCD_PERIOD;  // Increment the elapsed time by GCD_PERIOD
 }
}
int stages[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};  // Stepper motor phases
// TODO: Create your tick functions for each task

unsigned char LEFTBUTTON_PRESSED = 0;
unsigned char RIGHTBUTTON_PRESSED = 0;
unsigned char JOY_STICK_BUTTON = 0;
unsigned char PrevRIGHTBUTTON_PRESSED = 0;
unsigned char prevLEFTBUTTON_PRESSED = 0;


enum LED_INDICATOR { Start, Left, Right };
unsigned char sequenceStep = 0;
unsigned char currentMode = Start;

int Led(int state) {
 prevLEFTBUTTON_PRESSED = LEFTBUTTON_PRESSED;
 PrevRIGHTBUTTON_PRESSED = RIGHTBUTTON_PRESSED;

 unsigned char pinC = PINC;

 LEFTBUTTON_PRESSED = !(pinC & (1 << 3));
 RIGHTBUTTON_PRESSED = !(pinC & (1 << 4));
 JOY_STICK_BUTTON = !(pinC & (1 << 2));

 switch (state) {
  case Start:
   PORTB &= ~(1 << 0);
   PORTD &= ~((1 << 7) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));

   currentMode = Start;
   sequenceStep = 0;

   if (LEFTBUTTON_PRESSED && !RIGHTBUTTON_PRESSED) {
    state = Left;
   }

   else if (RIGHTBUTTON_PRESSED && !LEFTBUTTON_PRESSED) {
    state = Right;
   }
   break;

  case Left:
   if (!LEFTBUTTON_PRESSED || RIGHTBUTTON_PRESSED) {
    state = Start;
   }
   break;

  case Right:
   if (!RIGHTBUTTON_PRESSED || LEFTBUTTON_PRESSED) {
    state = Start;
   }
   break;

  default:
   state = Start;
   break;
 }

 switch (state) {
  case Start:
   PORTB &= ~(1 << 0);
   PORTD &= ~((1 << 7) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
   sequenceStep = 0;
   break;

  case Left:
   PORTD &= ~((1 << 2) | (1 << 3) | (1 << 4));

   if (sequenceStep == 0) {
    PORTB |= (1 << 0);
    PORTD &= ~((1 << 7) | (1 << 5));
    sequenceStep = 1;
   } 
   
   
   
   else if (sequenceStep == 1) {
    PORTB |= (1 << 0);
    PORTD |= (1 << 7);
    PORTD &= ~(1 << 5);
    sequenceStep = 2;
   } 
   
   else if (sequenceStep == 2) {
    PORTB |= (1 << 0);
    PORTD |= ((1 << 7) | (1 << 5));
    sequenceStep = 3;
   } 
   
   else if (sequenceStep == 3) {
    PORTB &= ~(1 << 0);
    PORTD &= ~((1 << 7) | (1 << 5));
    sequenceStep = 0;
   }
   break;

  case Right:
   PORTB &= ~(1 << 0);
   PORTD &= ~((1 << 7) | (1 << 5));

   if (sequenceStep == 0) {
    PORTD |= (1 << 2);
    PORTD &= ~((1 << 3) | (1 << 4));
    sequenceStep = 1;
   } 
   
   
   else if (sequenceStep == 1) {
    PORTD |= ((1 << 2) | (1 << 3));
    PORTD &= ~(1 << 4);
    sequenceStep = 2;
   } 
   
   
   else if (sequenceStep == 2) {
    PORTD |= ((1 << 2) | (1 << 3) | (1 << 4));
    sequenceStep = 3;
   } 
   
   
   else if (sequenceStep == 3) {
    PORTD &= ~((1 << 2) | (1 << 3) | (1 << 4));
    sequenceStep = 0;
   }
   break;

  default:
   break;
 }

 return state;
}

enum Buzzer { Buzz_START, Buzz_OFF, BUZZ_ON };


int TickBuzzer(int state) {
 unsigned char pinC = PINC;
 JOY_STICK_BUTTON = !(pinC & (1 << 2));


 switch (state) {
  case Buzz_START:
   state = Buzz_OFF;
   break;

  case Buzz_OFF:
   if (JOY_STICK_BUTTON) {
    state = BUZZ_ON;
   }
   break;

  case BUZZ_ON:
   if (!JOY_STICK_BUTTON) {
    state = Buzz_OFF;
   }

   break;
  default:
   state = Buzz_START;
   break;

 }


 switch (state) {
  case Buzz_OFF:
   TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
   PORTD &= ~(1 << 6);  // pin:6
   break;
  case BUZZ_ON:


   TCCR0A |= (1 << COM0A1);
   OCR0A = 128;  // 50%
   break;

  default:

   break;
 }


 return state;
}


int main(void) {
 // TODO: initialize all your inputs and ouputs
 DDRB = 0xFF;
 PORTB = 0x00;
 DDRC = 0x00;
 PORTC = 0xFF;
 DDRD = 0xFF;
 PORTD = 0x00;

 DDRB |= (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2);

 ADC_init();  // initializes ADC

 TCCR0A = (0 << COM0A1) | (1 << WGM01) | (1 << WGM00);
 TCCR0B = (1 << CS01) | (1 << CS00);
 OCR0A = 0;

 
 tasks[0].period = LED_PERIOD;
 tasks[0].state = Start;
 tasks[0].elapsedTime = 0;
 tasks[0].TickFct = &Led;

 tasks[1].period = BUZZ_PERIOD;
 tasks[1].state = Buzz_START;
 tasks[1].elapsedTime = 0;
 tasks[1].TickFct = &TickBuzzer;

 

 TimerSet(GCD_PERIOD);
 TimerOn();
 while (1) {
 }
 return 0;
}


