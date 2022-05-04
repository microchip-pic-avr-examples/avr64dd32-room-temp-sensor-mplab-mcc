#include "GPIO.h"

#include <avr/io.h>

//Inits. General Purpose I/O
void GPIO_init(void)
{
    //PF6 - SW0 Input
    //Inverted Logic, Enable Pull-Up
    PORTF.PIN6CTRL = PORT_INVEN_bm | PORT_PULLUPEN_bm;
    
    //PF5 - LED0 Output
    //Output, Start LOW
    PORTF.DIRSET = PIN5_bm;
}
