#include "system.h"

#include <avr/io.h>

#include "GPIO.h"
#include "TWI0_host.h"
#include "usart0.h"
#include "RTC.h"

FUSES = {
	.WDTCFG = 0x00, // WDTCFG {PERIOD=OFF, WINDOW=OFF}
	.BODCFG = 0x00, // BODCFG {SLEEP=DISABLE, ACTIVE=DISABLE, SAMPFREQ=128Hz, LVL=BODLEVEL0}
	.OSCCFG = 0x00, // OSCCFG {CLKSEL=OSCHF}
	.SYSCFG0 = 0xD9, // SYSCFG0 {EESAVE=SET, RSTPINCFG=RST, UPDIPINCFG=UPDI, CRCSEL=CRC16, CRCSRC=NOCRC}
	.SYSCFG1 = 0x0D, // SYSCFG1 {SUT=16MS, MVSYSCFG=DUAL}
	.CODESIZE = 0x00, // CODESIZE {CODESIZE=User range:  0x0 - 0xFF}
	.BOOTSIZE = 0x00, // BOOTSIZE {BOOTSIZE=User range:  0x0 - 0xFF}
};

LOCKBITS = 0x5CC5C55C; // {KEY=NOLOCK}

//Inits CLKCTRL, EVSYS, WDT, and SLPCTRL
void System_initDevice(void)
{
    //Init CLKCTRL
    //Default is 4MHz, which is desired
    
    //Init SLPCTRL
    
    //Power Level = STANDBY, SLEEP ENABLED
    SLPCTRL.CTRLA = SLPCTRL_SMODE_STDBY_gc | SLPCTRL_SEN_bm;
}

//Inits peripherals
void System_initPeripherals(void)
{
    //Init GPIO
    GPIO_init();
                
    //Configure TWI0 for Thermometer
    TWI0_initHost();
    TWI0_initPins();
        
    //Configure USART 3 for USB
    USART0_init();
    USART0_initIO();
        
    //Enable USART for USB (TX Only)
    USART0_enableTX();
    
    //Init RTC
    RTC_init();
}