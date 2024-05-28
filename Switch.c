/*
 * Switch.c
 *
 *  Created on: April 2024
 *      Author:
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table
void Switch_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PB19INDEX]=0X00050081;//INPUTS
    IOMUX->SECCFG.PINCM[PB16INDEX]=0X00050081;//pull down resistors
    IOMUX->SECCFG.PINCM[PB13INDEX]=0X00050081;//INPUTS
    IOMUX->SECCFG.PINCM[PB17INDEX]=0X00050081;
}
// return current state of switches
uint32_t Switch_In(void){
    // write this
    uint32_t data, input;
         data = GPIOB->DIN31_0;
         data= data>>13;//inputs are pb19-16
         input= data&0x59;

       return input;
   //replace this your code
}
