I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*

*/




#include <avr/io.h>
#include <util/delay.h>



unsigned char SetBit(unsigned char x, unsigned char k, unsigned char b) {
   return (b ?  (x | (0x01 << k))  :  (x & ~(0x01 << k)) );
              //   Set bit to 1           Set bit to 0
}

unsigned char GetBit(unsigned char x, unsigned char k) {
   return ((x & (0x01 << k)) != 0);
}

// TODO: complete nums
// the index is also the digit it represents
// eg: nums[0]="0", nums[9]="9", nums[15]="f"
// bits layout:   gfedcba   (REVERSE ORDER!)

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
                0b01110001  // f 
                }; 

//TODO: complete outNum()
void outNum(int num){
  PORTB = PORTB = nums[num] & 0b00011111; // depends on your wiring. Assign bits (e-a), which are bits (4-0) from nums[] to register PORTB
  PORTD = (nums[num] & 0b01100000) << 1;   // Assign bits (g & f) which are bits 6 & 5 of nums[] to register PORTD
}




int main(void)
{
  
 
    // Set Ports to input/output
    DDRB = 0xFF; PORTB = 0x00; // Set PORTB to output
    DDRD = 0xFF; PORTD = 0x00; // Set PORTD to input
    DDRC = 0x00; PORTC = 0xFF; // Set PORTD to input


    // Create any variables here
    int Button_LEFT, Button_RIGHT;
  int i = 1;;
  int k = 15;

  int Button_LEFT_NEW = 0, Button_RIGHT_NEW = 0;
   int LED_0 = 0, LED_1 = 0, LED_2 = 0, LED_3 = 0;
//PORTB = 0b00000000;
PORTD = 0b00000000;

  
   PORTB = nums[0] & 0b00011111;
   PORTD = (nums[0] & 0b01100000) << 1;
  
    Exit:
    while (1)
    {
        Button_LEFT = (PINC >> 0) & 0x01;
        Button_RIGHT = (PINC >> 1) & 0x01;

		
      if (Button_RIGHT == 1 && Button_RIGHT_NEW == 1){
          goto Exit;
        }

      
      if (Button_LEFT == 1 && Button_LEFT_NEW == 1){
         goto Exit;
        }

      if (Button_LEFT ){
          
        PORTD = PORTD - 0b00000100;
        
        Button_LEFT = (PINC >> 0) & 0x01;
        Button_RIGHT = (PINC >> 1) & 0x01;
        PORTB = nums[k] & 0b00011111;
        PORTD = (nums[k] & 0b01100000) << 1;
        for(int m=1; m <=k; m++)
        PORTD =  PORTD + 0b00000100;
            
        i=k+1;
        k--;
       
        if (k > 15) 
        k = 0;
        if (k < 0) 
        k = 15;
        if (i > 15) 
        i = 0;
        if (i < 0) 
        i = 15;

        Button_LEFT = (PINC >> 0) & 0x01;
        Button_RIGHT = (PINC >> 1) & 0x01;

      }

      if (Button_RIGHT){
        
        PORTD = PORTD + 0b00000100;

        Button_LEFT = (PINC >> 0) & 0x01;
        Button_RIGHT = (PINC >> 1) & 0x01;
        PORTB = nums[i] & 0b00011111;
        PORTD = (nums[i] & 0b01100000) << 1;
        for(int j=1; j <=i; j++)
        PORTD =  PORTD + 0b00000100;
     
        k=i-1;
        i++;
      if (i > 15) 
      i = 0;
      if (i < 0) 
      i = 15;
      if (k > 15) 
      k = 0;
      if (k < 0) 
      k = 15;
      }

      Button_RIGHT_NEW = Button_RIGHT;
      Button_LEFT_NEW = Button_LEFT; 
_delay_ms(200); 
           
    }
    return 0;
}
