I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*

*/


#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "timerISR.h"
#include "serialATmega.h"
unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
  return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
             //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
  return ((x & (0x01 << k)) != 0);
}

//TODO: Copy and paste your nums[] and outNum[] from the previous lab. The wiring is still the same, so it should work instantly
int nums[16] = {0b00111111, // 0
               0b00000110, // 1
               0b01011011, // 2
               0b01001111, // 3 
               0b01100110, // 4
               0b01101101, // 5
               0b01111101, // 6
               0b00000111, // 7
               0b01111111, // 8
               0b01101111, // 9
               0b01110111, // a
               0b01111100, // b
               0b00111001, // c 
               0b01011110, // d
               0b01111001, // e
               0b01000000  // g sgement 
               }; 

//TODO: complete outNum()
void outNum(int num){

if( num == 1 || num == 2 || num == 3 || num == 7 || num == 15){
  PORTB = (nums[num] & 0b00111111);
  PORTD = (nums[num] & 0b01100000) << 1;

  PORTD |= (1 << PD6); 


}
else{
PORTB = (nums[num] & 0b00111111);


PORTD = (nums[num] & 0b01100000) << 1;

}

}


void ADC_init() {
   //TODO: Copy and paste your ADC_init from exercise 1
   ADMUX = 0b01000000; //insert your value for ADMUX/
   ADCSRA = 0b10000111;//insert your value for ADCSRA/
   ADCSRB = 0b00000000;//insert your value for ADCSRB/

}


unsigned int ADC_read(unsigned char chnl){
 //TODO: Copy and paste your ADC_read from exercise 1
 ADMUX  = (ADMUX & 0b11111000) | (chnl & 0b00001111); //set MUX3:0 bits without modifying any other bits/
   ADCSRA = ADCSRA | 0b01000000;  //set the bit that starts conversion in free running mode without modifying any other bit/
   while((ADCSRA >> 6)&0x01) {} //what does this line do?/

   uint8_t low, high;

   low = ADCL; //what should this get assigned with?/
   high = ADCH; //what should this get assigned with?/

   return ((high << 8) | low) ;
}

// Provided map()
long map(long x, long in_min, long in_max, long out_min, long out_max) {
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


int Value;
int Output_Value;

enum states { INIT, LED} state; //TODO: finish the enum for the SM

void Tick() {
 //TODO: declare your static variables here or declare it globally
 
 
 

 // State Transistions
 //TODO: complete transitions 
 switch (state) {
   case INIT:
   state = LED;
   break;
  
  
   case LED:
   state = LED;

   break;
   
  
   default:
        state = INIT;
        break;
  }


 // State Actions
 //TODO: complete transitions
 switch (state) {
   
   case INIT:
   
       
   outNum(0);
   
   break;

  
   case LED:

   Value = ADC_read(0);  

   Output_Value = map(Value, 0, 1023, 0, 9); // 0-9 
   outNum(Output_Value);
   PORTD &= ~(1 << PD3);
   PORTD &= ~(1 << PD4);
   PORTD &= ~(1 << PD2);

   if( Output_Value <= 3){


    //PORTD |= 0b00000100;
    PORTD |= (1 << PD2);
    PORTD &= ~(1 << PD3);
    PORTD &= ~(1 << PD4);

    //PORTD |= (1 << PD3); 
    //PORTD |= (1 << PD4); 
  }

  else if( Output_Value <= 4 || Output_Value < 8){
  
    
    PORTD &= ~(1 << PD2);
    PORTD |= (1 << PD3); 
    PORTD |= (1 << PD4); 

  }

   else if( Output_Value >= 8){
    PORTD &= ~(1 << PD3);
    PORTD &= ~(1 << PD2);
    PORTD |= (1 << PD4); 
  }
  
   break;


   default:
    
   break;
 }
}


int main(void)
{
 //TODO: initialize all outputs and inputs
   DDRB     = 0xFF;
   PORTB    = 0x00;
 
   DDRC    = 0x00;
   PORTC   = 0xFF;

   DDRD   = 0xFF;
   PORTD  = 0x00;
   PORTD &= ~(1 << 6);

   

   ADC_init();

 TimerSet(500); //TODO: Set your timer
 state = INIT;
 TimerOn();
   while (1)
   {

       Tick();      // Execute one synchSM tick
       while (!TimerFlag){}  // Wait for SM period
       TimerFlag = 0;        // Lower flag
   }

   return 0;
}