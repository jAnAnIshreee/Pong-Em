
// Lab9Main.c
// Runs on MSPM0G3507
// Lab 9 ECE319K
// Janani Ramamoorthy and Alyssa Palacios
// Last Modified: April 2024

#include <stdio.h>
#include <stdint.h>
#include <ti/devices/msp/msp.h>
#include "../inc/ST7735.h"
#include "../inc/Clock.h"
#include "../inc/LaunchPad.h"
#include "../inc/TExaS.h"
#include "../inc/Timer.h"
#include "../inc/ADC1.h"
#include "../inc/DAC5.h"
#include "SmallFont.h"
#include "LED.h"
#include "Switch.h"
#include "Sound.h"
#include "images/images.h"
// ****note to ECE319K students****
// the data sheet says the ADC does not work when clock is 80 MHz
// however, the ADC seems to work on my boards at 80 MHz
// I suggest you try 80MHz, but if it doesn't work, switch to 40MHz
void PLL_Init(void){ // set phase lock loop (PLL)
  // Clock_Init40MHz(); // run this line for 40MHz
  Clock_Init80MHz(0);   // run this line for 80MHz
}

uint32_t M=1;
uint32_t Random32(void){
  M = 1664525*M+1013904223;
  return M;
}
uint32_t Random(uint32_t n){
  return (Random32()>>16)%n;
}
//GLOBAL VARIABLES
int32_t language=0;
int32_t NEEDRESTART=0;
int32_t PAUSED=0;
int32_t player_score = 0;
int32_t computer_score = 0;
int32_t totalPlayerScore = 0;
int32_t totalCompScore = 0;
int32_t endRound = 0;
int32_t LEVEL=1;
//SEMAPHORES
 int NeedToDrawNet=0;
 int NeedToDrawball = 0;
 int NeedToDrawCompPaddle = 0;
 int NeedToDrawUserPaddle = 0;
 int isGameOver = 0;
 int NeedBlankScreen = 0;
 int NeedToDrawLEFTBORDER=0;
 int NeedToDrawRIGHTBORDER=0;
 int NeedToDrawScore=0;
 int updateScores = 0;
struct paddle{
    int32_t x, oldx;
    int32_t y, oldy;
    const uint16_t *redpaddle;
    int16_t height;
    int16_t width;
    int16_t speed;

};
typedef struct paddle paddle_t;
paddle_t paddle[2]; //paddle 0 is the computer, 1 is person

struct ball{
    int32_t x, oldx;
    int32_t y, oldy;
    const uint16_t *orangepong;
    int16_t height, width;
    int32_t vx, vy;
};
typedef struct ball ball_t;
ball_t theball;


int check_score(){
    if (player_score == 5){
        //reset scores for both players
       endRound = 1;
        totalPlayerScore++;
        PAUSED=1;
        return 1;
        }

    if(computer_score == 5){
        //reset score for both players
        endRound = 1;
        totalCompScore++;
        PAUSED=1;
        return 2;
    }

    //if no one is at a score of 5, return 0
    return 0;
}

int collision_checker(ball_t b, paddle_t p){
    //returning 0 means no collision, returning 1 means collision happened
    //checks left, right, top and bottom points of ball and paddle to determine if there was overlap

int ballMinX = b.x;
int ballMaxX = b.x + b.width;
int ballMinY = b.y - b.height;
int ballMaxY = b.y;

int paddleMinX = p.x;
int paddleMaxX = p.x + p.width;
int paddleMinY = p.y - p.height;
int paddleMaxY = p.y;
 //checking for top paddle
if ((ballMaxX >= paddleMinX && ballMinX <= paddleMaxX) &&
    ((ballMaxY >= paddleMinY && ballMaxY <= paddleMaxY) ||
     (ballMinY <= paddleMaxY && ballMinY >= paddleMinY))) {
    return 1; // Collision happened
}

// Checking for collision between the left and right edges of the ball and paddle
if ((ballMaxY >= paddleMinY && ballMinY <= paddleMaxY) &&
    ((ballMaxX >= paddleMinX && ballMaxX <= paddleMaxX) ||
     (ballMinX <= paddleMaxX && ballMinX >= paddleMinX))) {
    return 1; // Collision happened
}

if ((ballMinY <= paddleMaxY && ballMaxY >= paddleMinY) &&
        ((ballMaxX >= paddleMinX && ballMaxX <= paddleMaxX) ||
         (ballMinX <= paddleMaxX && ballMinX >= paddleMinX))) {
        return 1; // Collision happened
    }

return 0;
}

uint32_t comp_paddle_speed = 5; //default

void move_ball(){
    //before we move ball, we set the oldx and oldy equal to current values
    theball.oldx = theball.x;
    theball.oldy = theball.y;

    //this moves ball according to its velocity
    theball.x += theball.vx;
    theball.y += theball.vy;
    NeedToDrawball = 1;
    //we treat the "first" player as computer, second player as a person
    if (theball.y < 15) {
        //this means the computer missed the ball, and the player gets a point
        player_score++;
        theball.x = 58;
        theball.y = 86;
        theball.vx = Random(3);
        theball.vy = 1;


        //reset ball/paddles to initial positions, we can decide this
        Sound_POINT();
        //play score sound
        updateScores =1;
        NeedToDrawball = 1;
        PAUSED=1;
    }

    if(theball.y > 155){
        //this means player missed the ball and computer gets a point
        computer_score++;
        //reset ball/paddle position

        theball.x = 58;
                theball.y = 86;
                theball.vx = Random(3);
                theball.vy = 1;

        //play score sound
                Sound_POINT();
                updateScores = 1;
                NeedToDrawball = 1;
                PAUSED=1;

    }

    if (theball.y>92 || theball.y<81){
        NeedToDrawNet=1;
    }

    if(theball.x < 7 || theball.x > 107){
        NeedToDrawLEFTBORDER=1;
        NeedToDrawRIGHTBORDER=1;
        //we change the direction of motion to be opposite if it hits a wall
        theball.vx = -theball.vx;
        //play wall sound
        Sound_WALL();
    }

    for (int8_t j = 0; j <= 1; j++){
            int wasThereCollision = collision_checker(theball, paddle[j]);
            uint32_t increment;
            int increment_signed;

            if (wasThereCollision == 1){
                //play ball sound
                Sound_PINGPONGHIT();
                //if ball was moving left when collision happened
                if (theball.vx < 0){
                    //we increase its velocity and change its direction of motion
                    increment = Random(3);
                    comp_paddle_speed = increment;
                   theball.vx = theball.vx - increment;
                    theball.vx = -theball.vx;

                }
                //if the ball was moving right when collision happened
                else{
                    increment = Random(3);
                    comp_paddle_speed = increment;
                   theball.vx = theball.vx + increment;
                    theball.vx = -theball.vx;
                }

                //if ball was moving down when collision happened
                if (theball.vy > 0){
                    increment = Random(3);
                    theball.vy = theball.vy + increment;
                    theball.vy = -theball.vy;
                }

                //if the ball was moving up when collision happened
                else{
                    increment = Random(3);
                    theball.vy = theball.vy - increment;
                    theball.vy = -theball.vy;
                }
            }
        }
    }


void move_computerpaddle(){
    int timeUntilCollision = (paddle[0].y - theball.y)/theball.vy;
    int predictedXCoord = theball.x + (theball.vx * timeUntilCollision);

    if(predictedXCoord < (paddle[0].x + (paddle[0].width/2))){
        paddle[0].x = paddle[0].x - 3;
    }else if (predictedXCoord > (paddle[0].x + (paddle[0].width/2))){
        paddle[0].x = paddle[0].x + 3;
    }


    if (paddle[0].x > 108){
        paddle[0].x = 108;
    }
    if (paddle[0].x < 3){
        paddle[0].x = 3;
    }

NeedToDrawCompPaddle = 1;
}

void level2(void){
    ST7735_SetCursor(1, 6);
     printf("      ROUND 2");
     ST7735_SetCursor(1, 8);
 printf("     UT  vs. OU");
      LED_On(LEVEL);
    Clock_Delay1ms(4500);
 ST7735_DrawBitmap(0, 160, simplepongboard, 128,160);
  NEEDRESTART=1;
}

void level3(void){
    ST7735_SetCursor(1, 6);
    printf("      ROUND 3");
    ST7735_SetCursor(1, 8);
 printf("   UT  vs. ALABAMA");
     LED_On(LEVEL);
   Clock_Delay1ms(4500);
   ST7735_DrawBitmap(0, 160, simplepongboard, 128,160);
    NEEDRESTART=1;
}

void RE_START(void){
    paddle[0].x = 3;
          paddle[1].x = 3;
          paddle[0].y = 8;
          paddle[1].y = 155;
          paddle[0].width = 16;
          paddle[1].width= 16;
          paddle[0].height = 4;
          paddle[1].height = 4;
          paddle[1].oldx = 3;
          paddle[1].oldy = 155;
          paddle[0].oldx = 3;
          paddle[0].oldy = 8;
          paddle[0].speed = 5;
          theball.x = 58;
          theball.y = 82;
          theball.width = 12;
          theball.height = 12;
          theball.vx = Random(3);
          theball.vy = Random(3);
          if(theball.vx==0){
              theball.vx = 3;

          }
          if(theball.vy==0){
              theball.vy=3;
          }
          NeedToDrawball = 0;
           NeedToDrawCompPaddle = 0;
          NeedToDrawUserPaddle = 0;
          NeedBlankScreen = 0;
    endRound = 0;
     PAUSED=0;
     player_score = 0;
    computer_score = 0;
}
// games  engine runs at 30Hz
void TIMG12_IRQHandler(void){uint32_t pos,msg;
  if((TIMG12->CPU_INT.IIDX) == 1){ // this will acknowledge
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
// game engine goes here
    // 1) sample slide pot

    //game initializations
  /*paddle[0].x = 3;
    paddle[1].x = 155;
    paddle[0].y = 8;
    paddle[1].y = 109;
    paddle[0].width = 16;
    paddle[1].width = 16;
    paddle[0].height = 4;
    paddle[1].height = 4;
    theball.x = 58;
    theball.y = 86;
    theball.width = 12;
    theball.height = 12;
    theball.vx = 1;
    theball.vy = 1;
*/
    //draws initial images
   //NeedToDrawUserPaddle=1;
  //NeedToDrawCompPaddle = 1;
  //NeedToDrawball = 1;
if (PAUSED==0){

    isGameOver = check_score();
   if(isGameOver==0){
        paddle[1].x = (128*ADCin())/4096;
        if (paddle[1].x > 108){
                paddle[1].x = 108;
            }
            if (paddle[1].x < 3){
                paddle[1].x = 3;
            }
        NeedToDrawUserPaddle = 1;
       move_ball();
     move_computerpaddle();
      //NeedToDrawCompPaddle = 1;
      //NeedToDrawball = 1;
    }

 //  else if(isGameOver==1||isGameOver == 2){
        //someone won
     //  NeedToDrawUserPaddle = 0;
    //   NeedToDrawCompPaddle = 0;
    //    NeedBlankScreen = 1;

 //   }
    // game engine goes here
   // 1) sample slide pot
    // 2) read input switches
    // 3) move sprites
    // 4) start sounds
    // 5) set semaphore: this is the NeedtoDraw thing
    // NO LCD OUTPUT IN INTERRUPT SERVICE ROUTINES
    GPIOB->DOUTTGL31_0 = GREEN; // toggle PB27 (minimally intrusive debugging)
  }
}
}
void  spanish(void){
    ST7735_FillScreen(0x0000);
        ST7735_SetCursor(1, 1);
        ST7735_OutString("\xAD");
          ST7735_OutString("BIENVENIDO!");
          ST7735_SetCursor(1, 2);
          ST7735_OutString("\xADUnete al embate"); //¡Únete al embate hacia la victoria! ¡Vamos a alcanzar una puntuación de 5 antes de que nuestro rival siquiera se dé cuenta de lo que les golpeó! ¡Ánimo Texas!");
          ST7735_SetCursor(1, 3);
          ST7735_OutString("hacia la victoria!");
          ST7735_SetCursor(1, 4);

              ST7735_OutString("\xADVamos a alcanzar");
              ST7735_SetCursor(1, 5);
              ST7735_OutString("una puntuaci\xA2n de 5");
              ST7735_SetCursor(1, 6);
              ST7735_OutString("antes de que");
        ST7735_SetCursor(1, 7);
       ST7735_OutString("nuestro rival");
       ST7735_SetCursor(1, 8);
        ST7735_OutString("siquiera se d\x82");
        ST7735_SetCursor(1, 9);
                  ST7735_OutString("cuenta de lo");
            ST7735_SetCursor(1, 10);
           ST7735_OutString("que les golpe\xA2!");
           ST7735_SetCursor(1, 11);
           ST7735_OutString("\xAD");
            ST7735_OutString("Animo Texas!");
            ST7735_SetCursor(1, 12);
          ST7735_OutString("Presiona el bot\xA2n");
          ST7735_SetCursor(1, 13);
          ST7735_OutString("de arriba para");
          ST7735_SetCursor(1, 14);
                    ST7735_OutString("EMPEZAR");
}

void english(void){
    ST7735_FillScreen(0x0000);
      ST7735_SetCursor(1, 1);
        ST7735_OutString("WELCOME!");
        ST7735_SetCursor(1, 2);
        ST7735_OutString("Join the stampede");
        ST7735_SetCursor(1, 3);
        ST7735_OutString("to victory! Let's");
        ST7735_SetCursor(1, 4);
            ST7735_OutString("wrangle up a score");
            ST7735_SetCursor(1, 5);
            ST7735_OutString("of 5 before our");
            ST7735_SetCursor(1, 6);
            ST7735_OutString("rival even knows");
      ST7735_SetCursor(1, 7);
     ST7735_OutString("what hit 'em!");
     ST7735_SetCursor(1, 8);
      ST7735_OutString("Texas Fight, y'all!");
      ST7735_SetCursor(1, 9);
                  ST7735_OutString("");
            ST7735_SetCursor(1, 10);
           ST7735_OutString("Press top button");
           ST7735_SetCursor(1, 11);
            ST7735_OutString("to START");
}

// games  engine runs at 30Hz


uint8_t TExaS_LaunchPadLogicPB27PB26(void){
  return (0x80|((GPIOB->DOUT31_0>>26)&0x03));
}

typedef enum {English, Spanish, Portuguese, French} Language_t;
Language_t myLanguage=English;
typedef enum {HELLO, GOODBYE, LANGUAGE} phrase_t;
const char Hello_English[] ="Hello";
const char Hello_Spanish[] ="\xADHola!";
const char Hello_Portuguese[] = "Ol\xA0";
const char Hello_French[] ="All\x83";
const char Goodbye_English[]="Goodbye";
const char Goodbye_Spanish[]="Adi\xA2s";
const char Goodbye_Portuguese[] = "Tchau";
const char Goodbye_French[] = "Au revoir";
const char Language_English[]="English";
const char Language_Spanish[]="Espa\xA4ol";
const char Language_Portuguese[]="Portugu\x88s";
const char Language_French[]="Fran\x87" "ais";
const char *Phrases[3][4]={
  {Hello_English,Hello_Spanish,Hello_Portuguese,Hello_French},
  {Goodbye_English,Goodbye_Spanish,Goodbye_Portuguese,Goodbye_French},
  {Language_English,Language_Spanish,Language_Portuguese,Language_French}
};
// use main1 to observe special characters


// use main2 to observe graphics


// use main3 to test switches and LEDs

// use main4 to test sound outputs


// ALL ST7735 OUTPUT MUST OCCUR IN MAIN
int main(void){ // final main
  __disable_irq();
  PLL_Init(); // set bus speed
  LaunchPad_Init();
  ST7735_InitPrintf();
    //note: if you colors are weird, see different options for
    // ST7735_InitR(INITR_REDTAB); inside ST7735_InitPrintf()l
  ST7735_FillScreen(ST7735_BLACK);
  ADCinit();     //PB18 = ADC1 channel 5, slidepot
  Switch_Init(); // initialize switches
  LED_Init();    // initialize LED
  Sound_Init();  // initialize sound

  TExaS_Init(0,0,&TExaS_LaunchPadLogicPB27PB26); // PB27 and PB26
    // initialize interrupts on TimerG12 at 30 Hz
  Clock_Delay1ms(1000);
     uint32_t data=Switch_In();
    ST7735_DrawBitmap(0, 160, pongemhomescreen, 128,160);
    while((data & 0x48)==0){
    data=Switch_In();
     }

    if((data & 0x40)==0x40){
        english();
    }
    else if((data & 0x08)==0x08){
        spanish();
    }

data=Switch_In();
    while(data!=1){
        data=Switch_In();
    }

    //////LEVEL 1///////
    if(data==1){
        ST7735_FillScreen(ST7735_BLACK);
        ST7735_SetCursor(1, 8);

                            printf("         VS.");

       // ST7735_DrawBitmap(49, 96, versus, 33,31);
        ST7735_DrawBitmap(34, 146, ut, 62,35);
        ST7735_DrawBitmap(10, 46, am, 31,24);
        ST7735_DrawBitmap(50, 48, ou, 27,33);
        ST7735_DrawBitmap(81, 46, alabama, 35,28);
        Clock_Delay1ms(4500);
        ST7735_FillScreen(ST7735_BLACK);
        ST7735_SetCursor(1, 6);

                     printf("      ROUND 1");
                     ST7735_SetCursor(1, 8);
                 printf("    UT  vs. A&M");
        LED_On(LEVEL);
        Clock_Delay1ms(4500);
        ST7735_DrawBitmap(0, 160, simplepongboard, 128,160);


    }




  //ST7735_DrawBitmap(58, 86, orangepong, 12,12);
 // ST7735_DrawBitmap(3, 155, greenpaddle, 16,4);
  //ST7735_DrawBitmap(3, 8, redpaddle, 16,4);
    paddle[0].x = 3;
       paddle[1].x = 3;
       paddle[0].y = 8;
       paddle[1].y = 155;
       paddle[0].width = 16;
       paddle[1].width= 16;
       paddle[0].height = 4;
       paddle[1].height = 4;
       paddle[1].oldx = 3;
       paddle[1].oldy = 155;
       paddle[0].oldx = 3;
       paddle[0].oldy = 8;
       paddle[0].speed = 5;
       theball.x = 58;
       theball.y = 82;
       theball.width = 12;
       theball.height = 12;
       theball.vx = Random(3);
       theball.vy = Random(3);

       NeedToDrawball = 0;
        NeedToDrawCompPaddle = 0;
       NeedToDrawUserPaddle = 0;
       NeedBlankScreen = 0;

  TimerG12_IntArm(80000000/30,2);
  // initialize all data structures
  __enable_irq();
  while(1){
      if(NEEDRESTART==1){
          RE_START();
          NEEDRESTART=0;

      }

        if(NeedToDrawUserPaddle == 1){
           ST7735_DrawBitmap(paddle[1].oldx, paddle[1].oldy, blankpaddle, paddle[1].width, paddle[1].height);
           ST7735_DrawBitmap(paddle[1].x, paddle[1].y, redpaddle, paddle[1].width, paddle[1].height);
           paddle[1].oldx = paddle[1].x;
           paddle[1].oldy = paddle[1].y;
           NeedToDrawUserPaddle = 0;
        }

        if(NeedToDrawCompPaddle == 1){
            ST7735_DrawBitmap(paddle[0].oldx, paddle[0].oldy, blankpaddle, paddle[0].width, paddle[0].height);
            ST7735_DrawBitmap(paddle[0].x, paddle[0].y, redpaddle, paddle[0].width, paddle[0].height);
            paddle[0].oldx = paddle[0].x;
            paddle[0].oldy = paddle[0].y;
            NeedToDrawCompPaddle = 0;
    }

        if(NeedToDrawball == 1){
          ST7735_DrawBitmap(theball.oldx, theball.oldy, blankpong, theball.width, theball.height);

            ST7735_DrawBitmap(theball.x, theball.y, orangepong, theball.width, theball.height);
            NeedToDrawball = 0;
        }

        if(NeedToDrawNet==1){
            ST7735_DrawBitmap(0, 82, net, 128, 6);
            NeedToDrawNet=0;
        }

        if(NeedBlankScreen == 1){
            ST7735_FillScreen(ST7735_BLACK);
            NeedToDrawUserPaddle = 0;
            NeedToDrawCompPaddle = 0;
            NeedBlankScreen = 0;
        }
        if(NeedToDrawRIGHTBORDER==1){
            ST7735_DrawBitmap(124, 160, RIGHTBORDER, 4, 160);
                        NeedToDrawRIGHTBORDER=0;
        }
        if(NeedToDrawLEFTBORDER==1){
            ST7735_DrawBitmap(0, 160, LEFTBORDER, 3, 160);
                      NeedToDrawLEFTBORDER=0;
        }



        if(updateScores == 1 && PAUSED==1){
            ST7735_FillScreen(0x0000);
                  ST7735_SetCursor(1, 1);
                    printf("UT score: %d", player_score);
                    ST7735_SetCursor(1, 2);
                    printf("Opponent score: %d", computer_score);
                    Clock_Delay1ms(3000);
                    ST7735_DrawBitmap(0, 160, simplepongboard, 128,160);
                    PAUSED=0;
                    updateScores = 0;


        }

        if(endRound == 1 && PAUSED==1){
            ST7735_FillScreen(0x0000);
            ST7735_SetCursor(1, 1);
            printf("ROUND %d OVER!", LEVEL);
            ST7735_SetCursor(1, 2);
            printf("UT Overall Score: %d", totalPlayerScore);
            ST7735_SetCursor(1, 3);
            printf("Opponent Overall");
            ST7735_SetCursor(1, 4);
            printf("Score: %d", totalCompScore);
            Clock_Delay1ms(4500);
            ST7735_FillScreen(ST7735_BLACK);
            LEVEL++;
            if(LEVEL>3){

                ST7735_SetCursor(1, 1);
                            printf("    FINAL SCORE:");
                            ST7735_SetCursor(1, 2);
                            printf("       UT = %d", totalPlayerScore);
                            ST7735_SetCursor(1, 3);
                                                        printf("     OPPS = %d", totalCompScore);


                ST7735_SetCursor(1, 13);
                            printf("PRESS TOP BUTTON TO");
                            ST7735_SetCursor(1, 14);
                            printf("     PLAY AGAIN");

                           ST7735_SetCursor(1, 7);
                            printf("     HOOK 'EM!");
                            data=Switch_In();
                                while(data!=1){
                                    data=Switch_In();
                                }
                                ST7735_DrawBitmap(0, 160, simplepongboard, 128,160);
                                RE_START();
                                   LEVEL = 1;
                                  continue;
            }

            if(LEVEL==2){

    level2();




                }
                if(LEVEL==3){
                    level3();

                }

        }

  }


}

