/*
 * LED.c
 *
 *
 *Authors: Janani Ramamoorthy and Alyssa Palacios
 *Modified April 2024
 */
#include <ti/devices/msp/msp.h>
#include "../inc/LaunchPad.h"
// LaunchPad.h defines all the indices into the PINCM table

// initialize your LEDs
void LED_Init(void){
    // write this
    IOMUX->SECCFG.PINCM[PA18INDEX]=0X00000081;
    IOMUX->SECCFG.PINCM[PA17INDEX]=0X00000081;
    IOMUX->SECCFG.PINCM[PA16INDEX]=0X00000081;
    GPIOA->DOE31_0 |= (1<<16) + (1<<17) + (1<<18);
}
// data specifies which LED to turn on


void LED_On(uint32_t data){
    /*uint32_t leddata, ledinput;
     leddata = GPIOA->DIN31_0;
        leddata= leddata>>16;//inputs are pb18-16
        ledinput= leddata&0x07;*/
    // write this
    if(data==1){
        GPIOA->DOUTSET31_0=0x20000;
        GPIOA->DOUTCLR31_0=0x50000;
    }
    else if(data==2){
        GPIOA->DOUTSET31_0=0x10000;
        GPIOA->DOUTCLR31_0=0x60000;
    }
    else if(data==3){
        GPIOA->DOUTSET31_0=0x40000;
        GPIOA->DOUTCLR31_0=0x70000;
    }
    // use DOUTSET31_0 register so it does not interfere with other GPIO

}

// data specifies which LED to turn off
void LED_Off( uint32_t data){
  /*  uint32_t leddata, ledinput;
     leddata = GPIOA->DIN31_0;
        leddata= leddata>>16;//inputs are pb18-16
        ledinput= leddata&0x07;*/
    // write this
    if(data==1){
            GPIOA->DOUTCLR31_0=0x60000;
        }
        else if(data==8){
            GPIOA->DOUTCLR31_0=0x50000;
        }
        else if(data==16){
            GPIOA->DOUTCLR31_0=0x30000;
        }
        else if(data==0){
            GPIOA->DOUTCLR31_0=0x70000;
        }
    // use DOUTCLR31_0 register so it does not interfere with other GPIO

}

// data specifies which LED to toggle
void LED_Toggle(uint32_t data){
    // write this
    // use DOUTTGL31_0 register so it does not interfere with other GPIO

}
