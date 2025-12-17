I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*

*/





#include <avr/interrupt.h>
#include <avr/io.h>
#include "serialATmega.h"
#include "timerISR.h"

int phases[8] = {0b0001, 0b0011, 0b0010, 0b0110, 0b0100, 0b1100, 0b1000, 0b1001};

unsigned char direction = 0;
unsigned char button_pressed = 0;
unsigned char prev_button = 0;
unsigned char lock_state = 0;
unsigned char change_passcode = 0;
unsigned char step = 0;
unsigned char incorrect = 0;
unsigned char phase_index = 0;
unsigned char step_counter = 0;
unsigned char motor_active = 0;
bool direction_clockwise = false;
unsigned char blue_blink = 0;
unsigned char blue_blink_counter = 0;
unsigned char led_state = 0;
unsigned char passcode[4] = {1, 2, 3, 4};
unsigned char input_sequence[4] = {0, 0, 0, 0};
unsigned char new_passcode[4] = {0, 0, 0, 0};

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

#define NUM_TASKS 6

const unsigned long JOYSTICK_READ_PERIOD = 50;
const unsigned long DISPLAY_PERIOD = 50;
const unsigned long BUTTON_PERIOD = 100;
const unsigned long LOCK_PERIOD = 50;
const unsigned long STEPPER_PERIOD = 5;
const unsigned long LED_BLINK_PERIOD = 500;
const unsigned long GCD_PERIOD = 5;

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

    if (change_passcode) {
     pattern |= 0x80;
    }

    PORTD &= ~((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5) | (1 << 6) | (1 << 7));
    PORTB &= ~((1 << 0) | (1 << 1));

    if (pattern & 0x01){
      PORTD |= (1 << 7);
    }
    if (pattern & 0x02){ 
    PORTD |= (1 << 6);
    }
    if (pattern & 0x04){
      PORTD |= (1 << 5);
    } 
    if (pattern & 0x08){
      PORTD |= (1 << 4);
    }
    if (pattern & 0x10){
       PORTD |= (1 << 3);
    }
    if (pattern & 0x20){ 
      PORTD |= (1 << 2);
    }
    if (pattern & 0x40){ 
      PORTB |= (1 << 0);
    }
    if (pattern & 0x80){
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
    if (lock_state == 1 && change_passcode == 0) {
     change_passcode = 1;
     step = 0;

     for (int j = 0; j < 4; j++) {
      new_passcode[j] = 0;
     }

     PORTC = (PORTC & 0xFC) | 0x00;
    }
   }
   prev_button = button;
   break;
  default:
   break;
 }

 return state;
}

enum LockState { LOCK_INIT, LOCK_WAIT, LOCK_CHECK_SEQUENCE, CHANGE_PASSCODE };

int TickFct_Lock(int state) {
 switch (state) {
  case LOCK_INIT:
   step = 0;
   incorrect = 0;
   blue_blink = 0;
   blue_blink_counter = 0;
   change_passcode = 0;
   state = LOCK_WAIT;
   break;
  case LOCK_WAIT:
   if (direction != 0) {
    if (change_passcode) {
     state = CHANGE_PASSCODE;
    } else {
     state = LOCK_CHECK_SEQUENCE;
    }
   }
   break;
  case LOCK_CHECK_SEQUENCE:
   if (direction == 0) {
    state = LOCK_WAIT;
   }
   break;
  case CHANGE_PASSCODE:
   if (direction == 0) {
    state = LOCK_WAIT;
   }
   break;
  default:
   state = LOCK_INIT;
   break;
 }

 switch (state) {
  case LOCK_WAIT:
   break;

  case LOCK_CHECK_SEQUENCE:
   if (direction != 0) {
    input_sequence[step] = direction;
    step++;

    if (step == 1) {
     PORTC = (PORTC & 0xFC) | 0x01;
    } 
    else if (step == 2) {
     PORTC = (PORTC & 0xFC) | 0x02;
    } 
    else if (step == 3) {
     PORTC = (PORTC & 0xFC) | 0x03;
    }

    if (step >= 4) {
     step = 0;

     incorrect = 0;
     for (unsigned char j = 0; j < 4; j++) {
      if (input_sequence[j] != passcode[j]) {
       incorrect = 1;
       break;
      }
     }

     if (!incorrect) {
      if (lock_state == 0) {
       lock_state = 1;
       motor_active = 1;
       direction_clockwise = false; 

       PORTC = (PORTC & 0xFC) | 0x03;
      } 
      else {
       lock_state = 0;
       motor_active = 1;
       direction_clockwise = true;  

       PORTC = (PORTC & 0xFC) | 0x03;
      }
     } 
     else {
      blue_blink = 1;
      blue_blink_counter = 0;
      led_state = 1;
      PORTC = (PORTC & 0xFC) | 0x03;
     }
    }

    while (direction != 0) {
     direction = GetJoystickDirection();
    }
   }
   break;

  case CHANGE_PASSCODE:
   if (direction != 0) {
    new_passcode[step] = direction;
    step++;

    if (step == 1) {
     PORTC = (PORTC & 0xFC) | 0x01;
    } 
    else if (step == 2) {
     PORTC = (PORTC & 0xFC) | 0x02;
    } 
    else if (step == 3) {
     PORTC = (PORTC & 0xFC) | 0x03;
    }

    if (step >= 4) {
     for (int j = 0; j < 4; j++) {
      passcode[j] = new_passcode[j];
     }

     step = 0;
     change_passcode = 0;

     PORTC = (PORTC & 0xFC) | 0x00;
    }

    while (direction != 0) {
     direction = GetJoystickDirection();
    }
   }
   break;

  default:
   break;
 }

 return state;
}

enum LEDBlinkState { LED_INIT, LED_WAIT, LED_BLINK };

int TickFct_LEDBlink(int state) {
 switch (state) {
  case LED_INIT:
   state = LED_WAIT;
   break;
  case LED_WAIT:
   if (blue_blink) {
    state = LED_BLINK;
   }
   break;
  case LED_BLINK:
   if (!blue_blink) {
    state = LED_WAIT;
   }
   break;
  default:
   state = LED_INIT;
   break;
 }

 switch (state) {
  case LED_BLINK:
   if (led_state) {
    PORTC = (PORTC & 0xFC) | 0x03;
    led_state = 0;
   } else {
    PORTC = (PORTC & 0xFC) | 0x00;
    led_state = 1;
   }

   blue_blink_counter++;

   if (blue_blink_counter >= 8) {
    blue_blink = 0;
    blue_blink_counter = 0;
    PORTC = (PORTC & 0xFC) | 0x00;
   }
   break;
   
  default:
   break;
 }

 return state;
}

enum StepperState { STEP_INIT, STEP_WAIT, STEP_RUN };

int TickFct_MotorControl(int state) {
 switch (state) {
  case STEP_INIT:
   if (motor_active) {
    step_counter = 0;
    phase_index = 0;
    state = STEP_RUN;
    PORTC = (PORTC & 0xFC) | 0x03;
   }
   break;

  case STEP_RUN:
   if (step_counter >= 128) {
    step_counter = 0;
    motor_active = 0;
    incorrect = 0;
    phase_index = 0;
    state = STEP_INIT;
    PORTC = (PORTC & 0xFC) | 0x00;
    PORTB &= ~((1 << 2) | (1 << 3) | (1 << 4) | (1 << 5));
   }
   break;

  default:
   state = STEP_INIT;
   break;
 }

 if (state == STEP_RUN) {
  for (int step = 0; step < 2; step++) {
   if (!direction_clockwise) {
    PORTB = (PORTB & 0x03) | (phases[phase_index] << 2);
    phase_index++;

    if (phase_index >= 8) {
     phase_index = 0;
     step_counter++;
    }
   } 
   else {
    PORTB = (PORTB & 0x03) | (phases[7 - phase_index] << 2);
    phase_index++;
    if (phase_index >= 8) {
     phase_index = 0;
     step_counter++;
    }
   }

   if (step_counter >= 128) {
    break;
   }
  }
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
 serial_init(9600);

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
 i++;

 tasks[i].period = LOCK_PERIOD;
 tasks[i].state = LOCK_INIT;
 tasks[i].elapsedTime = LOCK_PERIOD;
 tasks[i].TickFct = &TickFct_Lock;
 i++;

 tasks[i].period = STEPPER_PERIOD;
 tasks[i].state = STEP_INIT;
 tasks[i].elapsedTime = STEPPER_PERIOD;
 tasks[i].TickFct = &TickFct_MotorControl;
 i++;

 tasks[i].period = LED_BLINK_PERIOD;
 tasks[i].state = LED_INIT;
 tasks[i].elapsedTime = LED_BLINK_PERIOD;
 tasks[i].TickFct = &TickFct_LEDBlink;

 TimerSet(GCD_PERIOD);
 TimerOn();

 while (1) {
 }

 return 0;
}