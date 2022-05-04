#include "TWI0_host.h"

#include <avr/io.h>
#include <stdbool.h>

#define TWI_READ true
#define TWI_WRITE false

#define TWI0_IS_CLOCKHELD() TWI0.MSTATUS & TWI_CLKHOLD_bm
#define TWI0_IS_BUSERR() TWI0.MSTATUS & TWI_BUSERR_bm
#define TWI0_IS_ARBLOST() TWI0.MSTATUS & TWI_ARBLOST_bm

#define CLIENT1_NACK() TWI0.MSTATUS & TWI_RXACK_bm
#define CLIENT1_ACK() !(TWI0.MSTATUS & TWI_RXACK_bm)

#define TWI0_IS_BUSBUSY() (((TWI0.MSTATUS & TWI_BUSSTATE_gm) == TWI_BUSSTATE_BUSY_gc))
//#define TWI_IS_BAD() ((TWI_IS_BUSERR()) | (TWI_IS_ARBLOST()) | (CLIENT_NACK()) | (TWI_IS_BUSBUSY()))

#define TWI0_WAIT() while (!((TWI0_IS_CLOCKHELD()) || (TWI0_IS_BUSERR()) || (TWI0_IS_ARBLOST()) || (TWI0_IS_BUSBUSY())))

bool isTWI0Bad(void)
{
    //Checks for: NACK, ARBLOST, BUSERR, Bus Busy
    if ((((TWI0.MSTATUS) & (TWI_RXACK_bm | TWI_ARBLOST_bm | TWI_BUSERR_bm)) > 0)
            || (TWI0_IS_BUSBUSY()))
    {
        return true;
    }
    return false;
}

void TWI0_initHost(void)
{        
    //Standard 100kHz TWI, 4 Cycle Hold, 50ns SDA Hold Time
    TWI0.CTRLA = TWI_SDAHOLD_50NS_gc; 
    
    //Clear Dual Control
    TWI0.DUALCTRL = 0x00;
    
    //Enable Run in Debug
    TWI0.DBGCTRL = TWI_DBGRUN_bm;
    
    //Clear MSTATUS (write 1 to flags). BUSSTATE set to idle
    TWI0.MSTATUS = TWI_RIF_bm | TWI_WIF_bm | TWI_CLKHOLD_bm | TWI_RXACK_bm |
            TWI_ARBLOST_bm | TWI_BUSERR_bm | TWI_BUSSTATE_IDLE_gc;
    
    //Set for 100kHz from a 4MHz oscillator
    TWI0.MBAUD = 15;
    
    //[No ISRs], Host Mode, 50us bus timeout
    TWI0.MCTRLA = TWI_ENABLE_bm | TWI_TIMEOUT_50US_gc;
}

void TWI0_initPins(void)
{
    //PA2/PA3
        
    //Output I/O
    PORTA.DIRSET = PIN2_bm | PIN3_bm;

#ifdef TWI0_ENABLE_PULLUPS
    //Enable Pull-Ups
    PORTA.PINCONFIG = PORT_PULLUPEN_bm;
#endif

    //Select PA2/PA3
    PORTA.PINCTRLUPD = PIN2_bm | PIN3_bm;
}

bool _startTWI0(uint8_t addr, bool read)
{
    //If the Bus is Busy
    if (TWI0_IS_BUSBUSY())
    {
        return false;
    }
    
    //Send Address
    TWI0.MADDR = (addr << 1) | read;
    
    //Wait...
    TWI0_WAIT();
                
    if (isTWI0Bad())
    {
        //Stop the Bus
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return false;
    }
    
    //TWI Started
    return true;
}

//Internal Function: Reads len bytes from TWI, then issues a bus STOP
void _readFromTWI0(uint8_t* data, uint8_t len)
{
    uint8_t bCount = 0;
    
    //Release the clock hold
    
    TWI0.MSTATUS = TWI_CLKHOLD_bm;
    
    //TWI0.MCTRLB = TWI_MCMD_RECVTRANS_gc;
    
    while (bCount < len)
    {
        //Wait...
        TWI0_WAIT();
        
        //Store data
        data[bCount] = TWI0.MDATA;

        //Increment the counter
        bCount += 1;
        
        if (bCount != len)
        {
            //If not done, then ACK and read the next byte
            TWI0.MCTRLB = TWI_ACKACT_ACK_gc | TWI_MCMD_RECVTRANS_gc;
        }
    }
    
    //NACK and STOP the bus
    TWI0.MCTRLB = TWI_ACKACT_NACK_gc | TWI_MCMD_STOP_gc;
}

//Internal Function: Write len bytes to TWI. Does NOT STOP the bus. Returns true if successful
bool _writeToTWI0(uint8_t* data, uint8_t len)
{
    uint8_t count = 0;
    
    while (count < len)
    {
        //Write a byte
        TWI0.MDATA = data[count];
        
        //Wait...
        TWI0_WAIT();
        
        //If the client NACKed, then abort the write
        if (CLIENT1_NACK())
        {
            return false;
        }
        
        //Increment the counter
        count++;
    }
    
    return true;
}

bool TWI0_sendByte(uint8_t addr, uint8_t data)
{
    //Address Client Device (Write)
    if (!_startTWI0(addr, TWI_WRITE))
        return false;
    
    bool success = _writeToTWI0(&data, 1);
    
    //Stop the bus
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    
    return success;
}

bool TWI0_sendBytes(uint8_t addr, uint8_t* data, uint8_t len)
{
    //Address Client Device (Write)
    if (!_startTWI0(addr, TWI_WRITE))
        return false;
    
    //Write the bytes to the client
    bool success = _writeToTWI0(data, len);

    //Stop the bus
    TWI0.MCTRLB = TWI_MCMD_STOP_gc;
    
    
    
    return success;
}

bool TWI0_readByte(uint8_t addr, uint8_t* data)
{
    //Address Client Device (Read)
    if (!_startTWI0(addr, TWI_READ))
        return false;
    
    //Read byte from client
    _readFromTWI0(data, 1);

    return true;
}


bool TWI0_readBytes(uint8_t addr, uint8_t* data, uint8_t len)
{
    //Address Client Device (Read)
    if (!_startTWI0(addr, TWI_READ))
        return false;
    
    //Read bytes from client
    _readFromTWI0(data, len);
    
    return true;
}

bool TWI0_sendAndReadBytes(uint8_t addr, uint8_t regAddress, uint8_t* data, uint8_t len)
{
    //Address Client Device (Write)
    if (!_startTWI0(addr, TWI_WRITE))
        return false;
        
    //Write register address
    if (!_writeToTWI0(&regAddress, 1))
    {
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return false;
    }
    
    //Restart the TWI Bus in READ mode
    TWI0.MADDR |= TWI_READ;
    TWI0.MCTRLB = TWI_MCMD_REPSTART_gc;
    
    //Wait...
    TWI0_WAIT();
    
    if (isTWI0Bad())
    {
        //Stop the TWI Bus if an error occurred
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return false;
    }
    
    //Read the data from the client
    _readFromTWI0(data, len);
    
    return true;
}

bool TWI0_sendsAndReadBytes(uint8_t addr, uint8_t* write, uint8_t writeLen, uint8_t* read, uint8_t readLen)
{
    //Address Client Device (Write)
    if (!_startTWI0(addr, TWI_WRITE))
        return false;
        
    //Write register address
    if (!_writeToTWI0(write, writeLen))
    {
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return false;
    }
    
    //Restart the TWI Bus in READ mode
    TWI0.MADDR |= TWI_READ;
    TWI0.MCTRLB = TWI_MCMD_REPSTART_gc;
    
    //Wait...
    TWI0_WAIT();
    
    if (isTWI0Bad())
    {
        //Stop the TWI Bus if an error occurred
        TWI0.MCTRLB = TWI_MCMD_STOP_gc;
        return false;
    }
    
    //Read the data from the client
    _readFromTWI0(read, readLen);
    
    return true;

}