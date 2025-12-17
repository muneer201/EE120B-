I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*

*/




#include <avr/interrupt.h>
#include <avr/io.h>

#include "helper.h"
#include "periph.h"
#include "timerISR.h"

unsigned int distance_CM = 0;
unsigned char display_mode = 0;

typedef struct task {
 int state;                  // Current state of the task
 unsigned long period;       // Rate at which the task should tick
 unsigned long elapsedTime;  // Time since task's previous tick
 int (*TickFct)(int);        // Function to call for task's tick
} task;

task tasks[3];

const unsigned char tasksNum = 3;
const unsigned long periodSonar = 1000;
const unsigned long periodDisplay = 1;
const unsigned long periodLeftButton = 200;

int TickFct_Sonar(int state);
int TickFct_Display(int state);
int TickFct_LeftButton(int state);

void TimerISR() {
 unsigned char i;
 for (i = 0; i < tasksNum; i++) {
  if (tasks[i].elapsedTime >= tasks[i].period) {
   tasks[i].state = tasks[i].TickFct(tasks[i].state);
   tasks[i].elapsedTime = 0;
  }
  tasks[i].elapsedTime += periodDisplay;
 }
}

enum SonarStates { Start, SONAR_READ };

int TickFct_Sonar(int state) {
 switch (state) {
  case Start:
   state = SONAR_READ;
   break;
  case SONAR_READ:
   state = SONAR_READ;
   break;
  default:
   state = Start;
   break;
 }

 switch (state) {
  case Start:
   break;
  case SONAR_READ:
   distance_CM = 0;
   sonar_read();
   distance_CM = (sonar_read() + 0.5);
   break;
 }

 return state;
}

enum DisplayStates { Start_Display, Display };
unsigned char SelectD1_D4 = 0;

int TickFct_Display(int state) {
 unsigned int display_value;
 unsigned char digits[4];

 switch (state) {
  case Start_Display:
   state = Display;
   break;
  case Display:
   state = Display;
   break;
  default:
   state = Start_Display;
   break;
 }

 switch (state) {
  case Start_Display:
   SelectD1_D4 = 0;

   break;

  case Display:
   if (display_mode > 0) {
    display_value = (distance_CM * 100) / 254;
   } else {
    display_value = distance_CM;
   }

   digits[0] = display_value / 1000;
   digits[1] = (display_value / 100) % 10;
   digits[2] = (display_value / 10) % 10;
   digits[3] = display_value % 10;

   PORTB = SetBit(PORTB, 2, 1);
   PORTB = SetBit(PORTB, 3, 1);
   PORTB = SetBit(PORTB, 4, 1);
   PORTB = SetBit(PORTB, 5, 1);

   if (SelectD1_D4 == 0) {
    PORTB = SetBit(PORTB, 5, 0);  // D1
   } else if (SelectD1_D4 == 1) {
    PORTB = SetBit(PORTB, 4, 0);  // D2
   } else if (SelectD1_D4 == 2) {
    PORTB = SetBit(PORTB, 3, 0);  // D3
   } else if (SelectD1_D4 == 3) {
    PORTB = SetBit(PORTB, 2, 0);  // D4
   }

   outNum(digits[SelectD1_D4]);

   SelectD1_D4 = (SelectD1_D4 + 1) % 4;
   break;
 }

 return state;
}

enum ButtonStates { Start_Leftbutton, WAIT };

unsigned char Button = 0;

int TickFct_LeftButton(int state) {
 unsigned char Button_A1 = (PINC >> 1) & 0x01;

 switch (state) {
  case Start_Leftbutton:
   state = WAIT;
   Button = 0;
   display_mode = 0;
   break;

  case WAIT:
   if (!Button_A1 && !Button) {
    Button = 1;
   } else if (Button_A1 && Button) {
    Button = 0;
    display_mode = !display_mode;
   }
   break;

  default:
   state = Start_Leftbutton;
   break;
 }

 return state;
}

int main(void) {
 DDRB = 0XFE;
 PORTB = 0x01;
 DDRC = 0xFC;
 PORTC = 0x03;
 DDRD = 0xFF;
 PORTD = 0x00;

 sonar_init();
 ADC_init();
 unsigned char i = 0;

 tasks[i].state = Start;
 tasks[i].period = periodSonar;
 tasks[i].elapsedTime = tasks[i].period;
 tasks[i].TickFct = &TickFct_Sonar;
 ++i;

 tasks[i].state = Start_Display;
 tasks[i].period = periodDisplay;
 tasks[i].elapsedTime = tasks[i].period;
 tasks[i].TickFct = &TickFct_Display;
 ++i;

 tasks[i].state = Start_Leftbutton;
 tasks[i].period = periodLeftButton;
 tasks[i].elapsedTime = tasks[i].period;
 tasks[i].TickFct = &TickFct_LeftButton;

 TimerSet(periodDisplay);
 TimerOn();

 while (1) {
 }

 return 0;
}