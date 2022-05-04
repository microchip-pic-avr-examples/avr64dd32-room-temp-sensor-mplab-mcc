#include "tempMonitor.h"
#include "MLX90632.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

#include "EEPROM_Locations.h"

#include "mcc_generated_files/system/system.h"

//State Machine for the main program
typedef enum  {
    TEMP_START = 0, TEMP_WAIT, TEMP_RESULTS, TEMP_ERROR_WAIT, TEMP_ERROR
} TemperatureMeasState;

//Temperature Measurement State Machine
static TemperatureMeasState tempState = TEMP_START;

//Set if results are ready to be printed
static bool temperatureResultsReady = false;

//If set to 'F', results will be printed in Fahrenheit
//Threshold inputs will be considered as numbers in Fahrenheit
static char tempUnit = DEFAULT_TEMP_UNIT;

//Init the Temp Monitor
void tempMonitor_init(bool safeStart)
{
    bool success;
    
#ifndef DV_FLOAT_OUT
    printf("Initializing MLX90632 Temperature Sensor...");
#endif
    
    if (safeStart)
    {
        //Button is Held - Safe Mode
        success = MLX90632_initDevice(true);
    }
    else
    {
        //Normal Init Sequence
        success = MLX90632_initDevice(false);
    }
            
    //EEPROM was successfully loaded
    if (MLX90632_cacheOK())
    {
    #ifndef DV_FLOAT_OUT
        //Load EEPROM Value for RTC Period
        printf("\r\nLoaded cached constants...");
    #endif
    }
    else
    {
    #ifndef DV_FLOAT_OUT
        printf("\r\nLoaded constants from sensor and reset to defaults...");
    #endif
    }
    
    //Print Result
    if (success)
    {
#ifndef DV_FLOAT_OUT
        printf("OK\r\n");
#endif
    }
    else
    {
#ifndef DV_FLOAT_OUT
        printf("FAILED\r\n");
#endif
        tempState = TEMP_ERROR;
    }
}

//Run the Temp Monitor's Finite State Machine
void tempMonitor_FSM(void)
{
    bool success;
    
    //MLX90632 Temperature Sensor State Machine
    switch (tempState)
    {
        case TEMP_START:
        {            
            //Start a temperature measurement
            success = MLX90632_startSingleConversion();

            //Move to the next state
            if (!success)
            {              
#ifndef DV_FLOAT_OUT
                printf("[ERR] Failed to start temp conversion in TEMP_START\r\n");
#endif
                tempState = TEMP_ERROR;
            }
            else
            {
                tempState = TEMP_WAIT;
            }

            break;
        }
        case TEMP_WAIT:
        {
            if (MLX90632_isDataReady())
            {
                //Get Results
                success = MLX90632_getResults();

                //Move to the next state
                if (!success)
                {
#ifndef DV_FLOAT_OUT
                    printf("[ERR] Failed to get temp data from MLX90632_getResults()\r\n");
#endif
                    tempState = TEMP_ERROR;
                }
                else
                {
                    tempState = TEMP_RESULTS;
                }
            }
            break;
        }
        case TEMP_RESULTS:
        {
            LED0_Toggle();

            //Compute Temperature
            success = MLX90632_computeTemperature();

            //Update State
            if (success)
            {
                temperatureResultsReady = true;
                tempState = TEMP_START;
            }
            else
            {
#ifndef DV_FLOAT_OUT
                printf("[ERR] Failed to compute temp from MLX90632_computeTemperature()\r\n");
#endif
                tempState = TEMP_ERROR;
            }
            
            break;
        }
        case TEMP_ERROR_WAIT:
        {
            //Wait...
            break;
        }
        case TEMP_ERROR:
        default:
        {
            tempState = TEMP_ERROR_WAIT;
        }
    }
}

//Returns true if results are ready
bool tempMonitor_getResultStatus(void)
{
    return temperatureResultsReady;
}

void tempMonitor_printResults(void)
{
    //Clear Flag
    temperatureResultsReady = false;
    
    //Get temp (in Celsius)
    float_hex sensorTemp, objTemp;
    sensorTemp.value = MLX90632_getSensorTemp();
    objTemp.value = MLX90632_getObjectTemp();
    
    if (tempUnit == 'F')
    {
        //Convert to Fahrenheit
        sensorTemp.value = (sensorTemp.value * 1.8) + 32;
        objTemp.value = (objTemp.value * 1.8) + 32;
    }
    else if (tempUnit == 'K')
    {
        //Convert to Kelvin
        sensorTemp.value += 273.15;
        objTemp.value += 273.15;
    }
    else if (tempUnit != 'C')
    {
        //Invalid Units
        
#ifndef DV_FLOAT_OUT
        //Print Constant String
        printf("[WARN] Invalid Unit Specifier for Temperature: ");
        
        //Then call sprintf to print the value
        printf("%c\r\n", tempUnit);
#endif
    }
    
#ifndef DV_FLOAT_OUT
    //Queue Data to Send
    printf("Sensor Temperature: %2.1f%c\r\nRoom Temperature: %2.1f%c\r\n\r\n",
        sensorTemp.value, tempUnit, objTemp.value, tempUnit);
#else
        
    //Data to send
    char text[11];
    
    //Header Start
    text[0] = 0xAA;
    
    text[1] = (((0xFF << 0) & objTemp.hexCode) >> 0) & 0xFF;
    text[2] = (((0xFF00) & objTemp.hexCode) >> 8) & 0xFF;
    text[3] = (((0xFF0000) & objTemp.hexCode) >> 16) & 0xFF;
    text[4] = (((0xFF000000) & objTemp.hexCode) >> 24) & 0xFF;
    
    text[5] = (((0xFF) & sensorTemp.hexCode) >> 0) & 0xFF;
    text[6] = (((0xFF00) & sensorTemp.hexCode) >> 8) & 0xFF;
    text[7] = (((0xFF0000) & sensorTemp.hexCode) >> 16) & 0xFF;
    text[8] = (((0xFF000000) & sensorTemp.hexCode) >> 24) & 0xFF;
    
    //Head End
    text[9] = 0x55;
    
    //Null Terminator for String
    text[10] = 0x00;
    
    printf(&text[0]);
    
#endif
    //Print String
}