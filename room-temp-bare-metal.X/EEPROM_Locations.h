/*
© 2022 Microchip Technology Inc. and its subsidiaries

Subject to your compliance with these terms, you may use Microchip software and any derivatives exclusively with Microchip products. You are responsible for complying with 3rd party license terms applicable to your use of 3rd party software (including open source software) that may accompany Microchip software. SOFTWARE IS “AS IS.” NO WARRANTIES, WHETHER EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE, INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE FULLEST EXTENT ALLOWED BY LAW, MICROCHIP’S TOTAL LIABILITY ON ALL CLAIMS RELATED TO THE SOFTWARE WILL NOT EXCEED AMOUNT OF FEES, IF ANY, YOU PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE
*/

#ifndef MLX90632_EEPROM_DEFINES_H
#define	MLX90632_EEPROM_DEFINES_H

#ifdef	__cplusplus
extern "C" {
#endif

/*
 * Important! These locations are for the MCU's EEPROM ONLY
 * Not for use with the sensor itself
 */

//MLX90632 Temp Sensor Fields

#define MEM_DEVICE_ID1 (0x00)                   //0x1400
#define MEM_DEVICE_ID2 (2 + MEM_DEVICE_ID1)     //0x1402
#define MEM_DEVICE_ID3 (2 + MEM_DEVICE_ID2)     //0x1404

#define MEM_P_R (2 + MEM_DEVICE_ID3)            //0x1406
#define MEM_P_G (4 + MEM_P_R)                   //0x140A
#define MEM_P_T (4 + MEM_P_G)                   //0x140E
#define MEM_P_O (4 + MEM_P_T)                   //0x1412
#define MEM_EA  (4 + MEM_P_O)                   //0x1416
#define MEM_EB  (4 + MEM_EA)                    //0x141A
#define MEM_FA  (4 + MEM_EB)                    //0x141E
#define MEM_FB  (4 + MEM_FA)                    //0x1422
#define MEM_GA  (4 + MEM_FB)                    //0x1426
#define MEM_GB  (4 + MEM_GA)                    //0x142A
#define MEM_KA  (4 + MEM_GB)                    //0x142E
#define MEM_HA  (4 + MEM_KA)                    //0x1432
#define MEM_HB  (4 + MEM_HA)                    //0x1436

//Memory Validation (2 bytes) - XOR of 16-bit Device IDs
#define MEM_VALIDATE (4 + MEM_HB)               //0x143A

#ifdef	__cplusplus
}
#endif

#endif	/* MLX90632_EEPROM_DEFINES_H */
