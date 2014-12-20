#ifndef sLCD_H
#define sLCD_H

#include "MKL46Z4.h"

// define numer of front and back plane pins
#define LCD_N_FRONT 8
#define LCD_N_BACK 4

// create macros for segments (according to table on page 5 of this LAB)
// Pin 1 -> (Digit*2 - 1), Pin 2 -> Digit*2
// Pin 1 Pin 2
// COM0 D Dec
// COM1 E C
// COM2 G B
// COM3 F A
#define LCD_S_D 0x11 // segment D
#define LCD_S_E 0x22 // segment E
#define LCD_S_G 0x44 // segment G
#define LCD_S_F 0x88 // segment F
#define LCD_S_DEC 0x11
#define LCD_S_C 0x22
#define LCD_S_B 0x44
#define LCD_S_A 0x88
#define LCD_C 0x00 // clear

// create macro for each pin
#define LCD_FRONT0 37u
#define LCD_FRONT1 17u
#define LCD_FRONT2 7u
#define LCD_FRONT3 8u
#define LCD_FRONT4 53u
#define LCD_FRONT5 38u
#define LCD_FRONT6 10u
#define LCD_FRONT7 11u
#define LCD_BACK0 40u
#define LCD_BACK1 52u
#define LCD_BACK2 19u
#define LCD_BACK3 18u

// variables which make two arrays indexing pin number on the LCD
// to pin number on the microcontroller
const static uint8_t LCD_Front_Pin[] = {LCD_FRONT0,LCD_FRONT1,LCD_FRONT2,LCD_FRONT3,LCD_FRONT4,LCD_FRONT5,LCD_FRONT6,LCD_FRONT7};
const static uint8_t LCD_Back_Pin[] = {LCD_BACK0,LCD_BACK1,LCD_BACK2,LCD_BACK3};

//Initialise Display settings
void sLCD_init(void);

//Display value at given digit
void sLCD_set(uint8_t value,uint8_t digit);

// Clear given digit
void sLCD_clear(uint8_t digit);

// Turn On and Off DP1
void sLCD_DP1On(void);
void sLCD_DP1Off(void);

// Turn On and Off DP2
void sLCD_DP2On(void);
void sLCD_DP2Off(void);

// Turn On and Off DP3
void sLCD_DP3On(void);
void sLCD_DP3Off(void);

// Turn On and Off colon
void sLCD_ColonOn(void);
void sLCD_ColonOff(void);

void sLCD_DisplayDec(uint16_t value);
void sLCD_DisplayHex(uint16_t value);
void sLCD_DisplayBin(uint16_t value);
void sLCD_DisplayFloat(float value);
//Display error
void sLCD_DisplayError(uint8_t errno);

#endif

