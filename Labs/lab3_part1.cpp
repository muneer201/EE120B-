I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*


*/




#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"
#include "serialATmega.h"



void ADC_init() {
    // TODO: figure out register values
    ADMUX = 0b01000000; //insert your value for ADMUX/
    ADCSRA = 0b10000111; //insert your value for ADCSRA/
    ADCSRB = 0b00000000; //insert your value for ADCSRB/
  }
  
  
  
unsigned int ADC_read(unsigned char chnl){
    //                              ^^^^ unsigned char chnl selects which pin you're reading analog signal from
  
    ADMUX  = (ADMUX & 0b11111000) | (chnl & 0b00001111); //set MUX3:0 bits without modifying any other bits/
    ADCSRA = ADCSRA | 0b01000000;  //set the bit that starts conversion in free running mode without modifying any other bit/
    while((ADCSRA >> 6)&0x01) {} //what does this line do?/

    uint8_t low, high;

    low = ADCL; //what should this get assigned with?/
    high = ADCH; //what should this get assigned with?/

    return ((high << 8) | low) ;
}

enum states{INIT, S0, S1 } state;
unsigned int adc_value;
  
void Tick() {
    // TODO: Implement your Tick Function
  switch (state) {
    case INIT:
    	state = S0;
    	break;
    
    case S0:
    	state = S1;
    	break;
    
    case S1:
    	state = INIT;
    	break;
    
    default:
    	state = INIT;
    	break;
  }
  
   switch (state) {
     case INIT:
     	// Do nothing
     	break;
     
     case S0:
     	// Get adc value
     	adc_value = ADC_read(0);
     	break;
     
     case S1:
     	// Print adc value
     	serial_println(adc_value);

     	
     	break;
     
     default:
     	break;
  }
  
}

int main(void)
{
    // TODO: Initialize your I/O pins
    DDRC = 0x00; 
    PORTC = 0xFF;

    DDRB = 0xFF;
    PORTB = 0x00;

    DDRD = 0xFF;
    PORTD= 0x00;

    ADC_init();

 serial_init(9600);
   TimerSet(500);
   TimerOn(); 
 state = INIT;
    
    while (1)
    {
      
		  Tick(); 
      //serial_println(100);
           // Execute one synchSM tick
      while (!TimerFlag){}  // Wait for SM period
      TimerFlag = 0;
    }

    return 0;
}