I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*
*/



#include "helper.h"
#include "periph.h"
#include "timerISR.h"
#define NUM_TASKS 4 

const unsigned long LED_PERIOD = 300;
const unsigned long BUZZ_PERIOD = 50;
const unsigned long STEPPER_PERIOD = 1;
const unsigned long SERVO_PERIOD = 20;  

const unsigned long maximum_velocity = 1;
const unsigned long minimum_velocity = 20;
const unsigned long JOYstick_Center = 500;
const unsigned long JOYstick_Deadzone = 100;

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

unsigned char phase = 0;
unsigned int  Counter_1 = 0;
unsigned char motor_ON = 0;
unsigned char direction_clockwise = 0;
unsigned int motor_speed = minimum_velocity;
unsigned int speed_counter = 0;

unsigned int cycle_counter = 0;

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

enum Buzzer { Buzz_START, Buzz_OFF, BUZZ_ON, Buzz_AUTO_ON, Buzz_AUTO_OFF };

int TickBuzzer(int state) {
 unsigned char pinC = PINC;
 JOY_STICK_BUTTON = !(pinC & (1 << 2));

 unsigned int y_val = ADC_read(0);
 unsigned char joystick_down = (y_val < 300);

 switch (state) {
  case Buzz_START:
   cycle_counter = 0;
   state = Buzz_OFF;
   break;

  case Buzz_OFF:
   if (JOY_STICK_BUTTON) {
    state = BUZZ_ON;
   } 
   
   else if (joystick_down) {
    cycle_counter = 0;
    state = Buzz_AUTO_ON;
   }
   break;

  case BUZZ_ON:
   if (!JOY_STICK_BUTTON) {
    state = Buzz_OFF;
   }
   break;

  case Buzz_AUTO_ON:
   if (!joystick_down) {
    state = Buzz_OFF;
   } 
   
   
   
   else if (cycle_counter >= 20) {
    cycle_counter = 0;
    state = Buzz_AUTO_OFF;
   }
   break;

  case Buzz_AUTO_OFF:
   if (!joystick_down) {
    state = Buzz_OFF;
   } 
   
   
   else if (cycle_counter >= 20) {
    cycle_counter = 0;
    state = Buzz_AUTO_ON;
   }
   break;

  default:
   state = Buzz_START;
   break;
 }

 switch (state) {
  case Buzz_OFF:
   TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
   PORTD &= ~(1 << 6);
   break;

  case BUZZ_ON:

   TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
   TCCR0B = (1 << CS01) | (1 << CS00);
   OCR0A = 128;  // 50%
   break;

  case Buzz_AUTO_ON:

   TCCR0A = (1 << COM0A1) | (1 << WGM01) | (1 << WGM00);
   TCCR0B = (1 << CS01);
   OCR0A = 128;  // 50% duty cycle
   cycle_counter++;
   break;

  case Buzz_AUTO_OFF:

   TCCR0A &= ~((1 << COM0A1) | (1 << COM0A0));
   PORTD &= ~(1 << 6);
   cycle_counter++;
   break;

  default:
   break;
 }

 return state;
}

enum StepperState { STEP_INIT, STEP_WAIT, STEP_RUN };

int TickStepper(int state) {
 unsigned int y_val = ADC_read(0);

 if (y_val < (JOYstick_Center - JOYstick_Deadzone)) {
  motor_ON = 1;
  direction_clockwise = 1;

  int displacement = JOYstick_Center - JOYstick_Deadzone - y_val;

  if (displacement < 0) displacement = 0;
  if (displacement > 412) displacement = 412;

  motor_speed = minimum_velocity - ((displacement * (minimum_velocity - maximum_velocity)) / 412);

  if (motor_speed < maximum_velocity) {
   motor_speed = maximum_velocity;
  }

  if (motor_speed > minimum_velocity) {
   motor_speed = minimum_velocity;
  }
 }

 else if (y_val > (JOYstick_Center + JOYstick_Deadzone)) {
  motor_ON = 1;
  direction_clockwise = 0;
  int displacement = y_val - (JOYstick_Center + JOYstick_Deadzone);
  if (displacement < 0) displacement = 0;
  if (displacement > 412) displacement = 412;

  motor_speed = minimum_velocity - ((displacement * (minimum_velocity - maximum_velocity)) / 412);

  if (motor_speed < maximum_velocity) {
   motor_speed = maximum_velocity;
  }

  if (motor_speed > minimum_velocity) {
   motor_speed = minimum_velocity;
  }

 }

 else {
  motor_ON = 0;
 }

 switch (state) {
  case STEP_INIT:
   if (motor_ON) {
    Counter_1 = 0;
    phase = 0;
    speed_counter = 0;
    state = STEP_RUN;
   }

   else {
    state = STEP_WAIT;
   }
   break;

  case STEP_WAIT:
   if (motor_ON) {
    state = STEP_RUN;
   }
   break;

  case STEP_RUN:
   if (!motor_ON || Counter_1 >= 128) {
    Counter_1 = 0;
    motor_ON = 0;
    phase = 0;
    state = STEP_WAIT;

    PORTB &= ~(0xF << 2);
   }
   break;

  default:
   state = STEP_INIT;
   break;
 }

 if (state == STEP_RUN) {
  speed_counter++;

  if (speed_counter >= motor_speed) {
   speed_counter = 0;

   if (direction_clockwise) {
    PORTB = (PORTB & 0x03) | (stages[phase] << 2);
    phase++;

    if (phase >= 8) {
     phase = 0;
     Counter_1++;
    }
   } 
   
   
   else {
   
   PORTB = (PORTB & 0x03) | (stages[7 - phase] << 2);
    phase++;

    if (phase >= 8) {
     phase = 0;
     Counter_1++;
    }
   }
  }
 } 
 
 
 else {
  PORTB &= ~(0xF << 2);
  
 }

 return state;
}

enum ServoState { SERVO_INIT, SERVO_RUN };

int TickServo(int state) {
 switch (state) {
  case SERVO_INIT:
   OCR1A = 2999;
   state = SERVO_RUN;
   break;

  case SERVO_RUN: {
   uint16_t adcValue = ADC_read(1);

   uint16_t servoPosition = 999 + ((uint32_t)adcValue * 4000) / 1023;

   OCR1A = servoPosition;
  }
   state = SERVO_RUN;
   break;

  default:
   state = SERVO_INIT;
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
 DDRB |= (1 << 1);

 ADC_init();  // initializes ADC

 TCCR0A = (0 << COM0A1) | (1 << WGM01) | (1 << WGM00);
 TCCR0B = (1 << CS01) | (1 << CS00);
 OCR0A = 0;

 ICR1 = 19999;
 TCCR1A = (1 << COM1A1) | (0 << COM1A0) | (1 << WGM11);
 TCCR1B = (1 << WGM13) | (1 << WGM12) | (1 << CS11);
 OCR1A = 1500;

 tasks[0].period = LED_PERIOD;
 tasks[0].state = Start;
 tasks[0].elapsedTime = 0;
 tasks[0].TickFct = &Led;

 tasks[1].period = BUZZ_PERIOD;
 tasks[1].state = Buzz_START;
 tasks[1].elapsedTime = 0;
 tasks[1].TickFct = &TickBuzzer;

 tasks[2].period = STEPPER_PERIOD;
 tasks[2].state = STEP_INIT;
 tasks[2].elapsedTime = 0;
 tasks[2].TickFct = &TickStepper;
 
 tasks[3].period = SERVO_PERIOD;
 tasks[3].state = SERVO_INIT;
 tasks[3].elapsedTime = 0;
 tasks[3].TickFct = &TickServo;

 TimerSet(GCD_PERIOD);
 TimerOn();
 while (1) {
 }
 return 0;
}