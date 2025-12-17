I acknowledge all content contained herein, excluding
template or example code, is my own original work.
*

*/



#include <avr/io.h>
#include <util/delay.h>

int main(void)
{
    // Set Ports to input/output
    DDRB = 0xFF; PORTB = 0x00; // Set PORTB to output
    DDRD = 0xFF; PORTD = 0x00; // Set PORTD to input
    DDRC = 0x00; PORTC = 0xFF; // Set PORTD to input


    // Create any variables here
    int Button_LEFT, Button_RIGHT;
    int Button_LEFT_NEW = 0, Button_RIGHT_NEW = 0;
    int LED_0 = 0, LED_1 = 0, LED_2 = 0, LED_3 = 0;

  
  PORTD = 0b00000000;


  
  
Exit:
    while (1)
    {
        
        Button_LEFT = (PINC >> 0) & 0x01;
        Button_RIGHT = (PINC >> 1) & 0x01;


       
        if (Button_RIGHT == 1 & Button_RIGHT_NEW == 1){
          goto Exit;
        }


        if (Button_RIGHT)
        {
           if (LED_0 == 0 & LED_1 == 0 & LED_2 == 0 & LED_3 == 0) { 
              PORTD = 0b00000100; 
              LED_0 = 1; 
            }
            else if (LED_1 == 1) { 
              PORTD = 0b00000100; 
              LED_0 = 1; 
              LED_1 = 0; 
            }
            else if (LED_2 == 1) { 
              PORTD = 0b00001000; 
              LED_1 = 1; 
              LED_2 = 0; 
            }
            else if (LED_3 == 1) { 
              PORTD = 0b00010000; 
              LED_2 = 1; 
              LED_3 = 0; }
            else if (LED_0 == 0) { 
              PORTD = 0b00000100; 
              LED_0 = 1; 
            }
        }
       else if (Button_LEFT == 1 & Button_LEFT_NEW == 1){
         goto Exit;
        }
			 else if (Button_LEFT ){
              if (LED_3 == 1) { 
                PORTD = 0b00100000; 
                LED_3 = 1; 
              }
            else if (LED_2 == 1) { 
              PORTD = 0b00100000; 
              LED_3 = 1; 
              LED_2 = 0; 
            }
            else if (LED_1 == 1) {
              PORTD = 0b00010000; 
              LED_2 = 1; 
              LED_1 = 0; 
            }
          else if (LED_0 == 1) { 
            PORTD = 0b00001000; 
            LED_1 = 1; 
            LED_0 = 0; 
          }
   else if (LED_3 == 0) { 
     PORTD = 0b00100000; 
     LED_3 = 1; 
   }
              
      
        } 
        Button_RIGHT_NEW = Button_RIGHT;
        Button_LEFT_NEW = Button_LEFT; 
      //_delay_ms(500); 
    }
    return 0;
}