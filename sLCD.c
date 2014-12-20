#include "sLCD.h"

void sLCD_set(uint8_t value,uint8_t digit){
// value – value to display 0-F,
// digit – position on which value is displayed
	
// Check if value to be displayed is in allowed range. If not display Err 1
if(value>0x0F){
	sLCD_DisplayError(0x01);	
	return;
};

// Check if digit is valid. If not display err2
if(digit>4 || digit<1){
	sLCD_DisplayError(0x02);
	return;
}
	
if(value==0x00){ // to display ‘0’ enable segments
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_E |LCD_S_F); 
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
}
else if(value==0x01){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_C);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
}
else if(value==0x02){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G | LCD_S_E | LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B);
}
else if(value==0x03){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_D | LCD_S_G);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_B | LCD_S_C);
}
else if(value==0x04){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_G);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B | LCD_S_C);
}
else if(value==0x05){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_G| LCD_S_D); 
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
}
else if(value==0x06){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F | LCD_S_G |LCD_S_E |LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A | LCD_S_C);
}
else if(value==0x07){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_C);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A| LCD_S_B | LCD_S_C);
}
else if(value==0x08){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_G|LCD_S_E|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A|LCD_S_B|LCD_S_C);
}
else if(value==0x09){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_G|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A|LCD_S_B|LCD_S_C);
}
else if(value==0x0A){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_E|LCD_S_G);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A|LCD_S_B|LCD_S_C);
}
else if(value==0x0B){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_G|LCD_S_E|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_C);
}
else if(value==0x0C){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_E|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
}
else if(value==0x0D){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_G|LCD_S_E|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_B|LCD_S_C);
}
else if(value==0x0E){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_G|LCD_S_E|LCD_S_D);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
}
else if(value==0x0F){
LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_S_F|LCD_S_G|LCD_S_E);
LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_S_A);
}
}

// clear digit
void sLCD_clear(uint8_t digit){
	if(digit>4 || digit<1){
		sLCD_DisplayError(0x02);
		return;
	} else {
		LCD->WF8B[LCD_Front_Pin[((2*digit)-2)]] = (LCD_C);
		LCD->WF8B[LCD_Front_Pin[((2*digit)-1)]] = (LCD_C);
	}
}


void sLCD_DisplayError(uint8_t errno){
	
	sLCD_DP3Off(); // turn off dot
	
	LCD->WF8B[LCD_Front_Pin[0]] = (LCD_S_F|LCD_S_G|LCD_S_E|LCD_S_D); 
	LCD->WF8B[LCD_Front_Pin[1]] = (LCD_S_A);
	
	LCD->WF8B[LCD_Front_Pin[2]] =  (LCD_S_G);
	LCD->WF8B[LCD_Front_Pin[3]] =	 (LCD_S_C);
	
	LCD->WF8B[LCD_Front_Pin[4]] =	 (LCD_S_G);
	LCD->WF8B[LCD_Front_Pin[5]] =  (LCD_S_C);
	
	if(errno < 0x10){
		sLCD_set(errno,4);
	} else {
		LCD->WF8B[LCD_Front_Pin[4]] =	 (LCD_C);
		LCD->WF8B[LCD_Front_Pin[5]] =  (LCD_C);
	};
}

// Turn On and Off DP1
void sLCD_DP1On(void){
	LCD->WF8B[LCD_Front_Pin[1]] |= LCD_S_DEC;
};
void sLCD_DP1Off(void){
	LCD->WF8B[LCD_Front_Pin[1]] &= ~LCD_S_DEC;
}

// Turn On and Off DP2
void sLCD_DP2On(void){
	LCD->WF8B[LCD_Front_Pin[3]] |= LCD_S_DEC;
};
void sLCD_DP2Off(void){
	LCD->WF8B[LCD_Front_Pin[3]] &= ~LCD_S_DEC;
}

// Turn On and Off DP3
void sLCD_DP3On(void){
	LCD->WF8B[LCD_Front_Pin[5]] |= LCD_S_DEC;
};
void sLCD_DP3Off(void){
	LCD->WF8B[LCD_Front_Pin[5]] &= ~LCD_S_DEC;
}

// Turn On and Off colon
void sLCD_ColonOn(void){
	LCD->WF8B[LCD_Front_Pin[7]] |= LCD_S_DEC;
}

void sLCD_ColonOff(void){
	LCD->WF8B[LCD_Front_Pin[7]] &= ~LCD_S_DEC;
};


void sLCD_DisplayDec(uint16_t value){
	// Check if the wanted value can fit on the display. Skip leading zeroes i.e. dipslay 16 as 16. Not as 0016.
	uint16_t emptyZero = 1; // 1 if all previous digids were 0
	uint16_t digitValue;
	
	sLCD_DP3Off(); // turn off dot
	
	if (value>9999) {
		sLCD_DisplayError(0x03);
		return;
	}
	
	
	digitValue = value/1000;
	if (digitValue == 0) {
		sLCD_clear(1);
	} else {
		sLCD_set(digitValue,1);
		emptyZero = 0;
	}
	
	digitValue = (value - (value/1000)*1000)/100;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(2);
	} else {
		sLCD_set(digitValue,2);
		emptyZero = 0;
	}
	
	digitValue = (value - (value/100)*100)/10;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(3);
	} else {
		sLCD_set(digitValue,3);
		emptyZero = 0;
	}
	
	digitValue = (value - (value/10)*10);
	sLCD_set(digitValue,4);	
}

void sLCD_DisplayFloat(float value_f){
	// Check if the wanted value can fit on the display. Skip leading zeroes i.e. dipslay 16 as 16. Not as 0016.
	uint16_t emptyZero = 1; // 1 if all previous digids were 0
	uint16_t digitValue, value;
	uint8_t dotPosition;
	
	
	
	if (value_f>9999) {
		sLCD_DisplayError(0x03);
		return;
	} else if ( value_f >= 1000 ) {
		value = (uint16_t)value_f;
	} else if ( value_f < 1000 && value_f >= 100 ) {
		value = (uint16_t)(value_f*10.0);
		dotPosition = 3;
	} else if ( value_f < 100 && value_f >= 10 ) {
		value = (uint16_t)(value_f*100.0);
		dotPosition = 2;
	} else {
		value = (uint16_t)(value_f*1000.0);
		dotPosition = 1;
		emptyZero = 0;
	}
	
	
	digitValue = value/1000;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(1);
	} else {
		sLCD_set(digitValue,1);
		if (dotPosition == 1) sLCD_DP1On(); 	        // turn on dot!
		emptyZero = 0;
	}
	
	digitValue = (value - (value/1000)*1000)/100;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(2);
	} else {
		sLCD_set(digitValue,2);
		if (dotPosition == 2) sLCD_DP2On(); 	        // turn on dot!
		emptyZero = 0;
	}
	
	digitValue = (value - (value/100)*100)/10;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(3);
	} else {
		sLCD_set(digitValue,3);
		if (dotPosition == 3) sLCD_DP3On(); 	        // turn on dot!
	}
	
	digitValue = (value - (value/10)*10);
	sLCD_set(digitValue,4);	
}


void sLCD_DisplayHex(uint16_t value){
	// Skip leading zeroes i.e. dipslay 16 as 16. Not as 0016.
	uint16_t emptyZero = 1; // 1 if all previous digids were 0
	uint16_t digitValue;
	
	sLCD_DP3Off(); // turn off dot
	
	digitValue = value & 0xF000;
	if (digitValue == 0) {
		sLCD_clear(1);
	} else {
		sLCD_set(digitValue>>12,1);
		emptyZero = 0;
	}
	
	digitValue = value & 0x0F00;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(2);
	} else {
		sLCD_set(digitValue>>8,2);
		emptyZero = 0;
	}
	
	digitValue = value & 0x00F0;
	if (digitValue == 0 && emptyZero == 1) {
		sLCD_clear(3);
	} else {
		sLCD_set(digitValue>>4,3);
	}
	
	digitValue = value & 0x000F;
		sLCD_set(digitValue,4);
	
}

void sLCD_DisplayBin(uint16_t value) {
	// Check if BIN number will fit on the display.
	
	uint16_t digitValue;
	
	sLCD_DP3Off(); // turn off dot
	
	if (value>0xF) {
		sLCD_DisplayError(0x03);
		return;
	}
	
	digitValue = value & 0x8;
	sLCD_set(digitValue>>3,1);

	
	digitValue = value & 0x4;
	sLCD_set(digitValue>>2,2);

	
	digitValue = value & 0x2;
	sLCD_set(digitValue>>1,3);

	
	digitValue = value & 0x1;
	sLCD_set(digitValue,4);

}


void sLCD_init(){

// enable clock ports B, C, D, E, and SegLCD Peripheral
SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK|SIM_SCGC5_PORTC_MASK|SIM_SCGC5_PORTD_MASK|SIM_SCGC5_PORTE_MASK|SIM_SCGC5_SLCD_MASK;
	
// disable LCD while configuring
LCD->GCR |= LCD_GCR_PADSAFE_MASK; // disable fronplane backplane operations
LCD->GCR &= ~LCD_GCR_LCDEN_MASK; // Clear LCDEN
	
// configure all used pins for driving LCD,
// set pins to MUX 0 (ALT0) for normal LCD display operation
PORTD->PCR[0] = PORT_PCR_MUX(0u); //LCD_P40 - 1(COM0)
PORTE->PCR[4] = PORT_PCR_MUX(0u); //LCD_P52 - 2 (COM1)
PORTB->PCR[23] = PORT_PCR_MUX(0u); //LCD_P19 - 3 (COM3)
PORTB->PCR[22] = PORT_PCR_MUX(0u); // LCD_P18 - 4 (COM4)
PORTC->PCR[17] = PORT_PCR_MUX(0u); // LCD_P37 - 5
PORTB->PCR[21] = PORT_PCR_MUX(0u); // LCD_P17 - 6
PORTB->PCR[7] = PORT_PCR_MUX(0u); // LCD_P7 - 7
PORTB->PCR[8] = PORT_PCR_MUX(0u); // LCD_P8 - 8
PORTE->PCR[5] = PORT_PCR_MUX(0u); // LCD_P53 - 9
PORTC->PCR[18] = PORT_PCR_MUX(0u); // LCD_P38 - 10
PORTB->PCR[10] = PORT_PCR_MUX(0u); // LCD_P10 - 11
PORTB->PCR[11] = PORT_PCR_MUX(0u);  // LCD_P11 - 12

// configure LCD registers
LCD->GCR = // Set LCD General Control register
LCD_GCR_RVTRIM(0x00) | //Set Regulated Voltage Trim
LCD_GCR_CPSEL_MASK | // Enable Charge Pump
LCD_GCR_LADJ(0x03) | // Load Adjust - 11 - Slowest clock source for charge pump
LCD_GCR_VSUPPLY_MASK | // Voltage Supply Control 
LCD_GCR_ALTDIV(0x00) | //LCD AlternateClock Divider 0 - No division
LCD_GCR_SOURCE_MASK | //LCD Clock Source Select - Alternative clock source
LCD_GCR_LCLK(0x01) | // LCD Clock Prescaler
LCD_GCR_DUTY(0x03); // Set duty cycle to 1/4

// control blinking of LCD
LCD->AR = LCD_AR_BRATE(0x03); // Blink-rate configuration

// FDCR configuration
LCD->FDCR = 0x00000000; // Clear Fault Detect Control Register

// Pin Enable Register, controls which of the possible LCD pins are used
// for pins >= 32, use PEN[1] (pin number - 32)
LCD->PEN[0] =	LCD_PEN_PEN(1u<<19 )	| // LCD_P19
							LCD_PEN_PEN(1u<<18) 	| // LCD_P18
							LCD_PEN_PEN(1u<<17) 	| // LCD_P17
							LCD_PEN_PEN(1u<<7) 		| // LCD_P7
							LCD_PEN_PEN(1u<<8) 		| // LCD_P8
							LCD_PEN_PEN(1u<<10) 	| // LCD_P10
							LCD_PEN_PEN(1u<<11); 		// LCD_P11
							
LCD->PEN[1] =	LCD_PEN_PEN(1u<<8) 		| // LCD_P40
							LCD_PEN_PEN(1u<<20) 	| //LCD_P52
							LCD_PEN_PEN(1u<<5) 		| //LCD_P37
							LCD_PEN_PEN(1u<<21)		| //LCD_P53
							LCD_PEN_PEN(1u<<6); 		// LCD_P38

// back plane enable register, controls which pins in LCD->PEN are Back Plane
LCD->BPEN[0] =	LCD_BPEN_BPEN(1u<<19) | 
								LCD_BPEN_BPEN(1u<<18);
LCD->BPEN[1] =	LCD_BPEN_BPEN(1u<<8) |
								LCD_BPEN_BPEN(1u<<20);
								
// waveform configuration (Waveform register) – 4 active, all the rest not
// on 8 bits we are supposed to achieve 4 equal phases
// (44.3.7 w KL46 Reference Manual)
LCD->WF[0] =	LCD_WF_WF0(0x00) |
							LCD_WF_WF1(0x00) |
							LCD_WF_WF2(0x00) |
							LCD_WF_WF3(0x00);
							
LCD->WF[1] =	LCD_WF_WF4(0x00) |
							LCD_WF_WF5(0x00) |
							LCD_WF_WF6(0x00) |
							LCD_WF_WF7(0x00);
							
LCD->WF[2] =	LCD_WF_WF8(0x00) |
							LCD_WF_WF9(0x00) |
							LCD_WF_WF10(0x00) |
							LCD_WF_WF11(0x00);
							
LCD->WF[3] =	LCD_WF_WF12(0x00) |
							LCD_WF_WF13(0x00) |
							LCD_WF_WF14(0x00) |
							LCD_WF_WF15(0x00);
							
LCD->WF[4] =	LCD_WF_WF16(0x00) |
							LCD_WF_WF17(0x00) |
							LCD_WF_WF18(0x88) | // COM3 (10001000)
							LCD_WF_WF19(0x44); // COM2 (01000100)
							
LCD->WF[5] =	LCD_WF_WF20(0x00) |
							LCD_WF_WF21(0x00) |
							LCD_WF_WF22(0x00) |
							LCD_WF_WF23(0x00);
							
LCD->WF[6] =	LCD_WF_WF24(0x00) |
							LCD_WF_WF25(0x00) |
							LCD_WF_WF26(0x00) |
							LCD_WF_WF27(0x00);
							
LCD->WF[7] =	LCD_WF_WF28(0x00) |
							LCD_WF_WF29(0x00) |
							LCD_WF_WF30(0x00) |
							LCD_WF_WF31(0x00);
							
LCD->WF[8] =	LCD_WF_WF32(0x00) |
							LCD_WF_WF33(0x00) |
							LCD_WF_WF34(0x00) |
							LCD_WF_WF35(0x00);
							
LCD->WF[9] =	LCD_WF_WF36(0x00) |
							LCD_WF_WF37(0x00) |
							LCD_WF_WF38(0x00) |
							LCD_WF_WF39(0x00);
							
LCD->WF[10] =	LCD_WF_WF40(0x11) | // COM0 (00010001)
							LCD_WF_WF41(0x00) |
							LCD_WF_WF42(0x00) |
							LCD_WF_WF43(0x00);
							
LCD->WF[11] =	LCD_WF_WF44(0x00) |
							LCD_WF_WF45(0x00) |
							LCD_WF_WF46(0x00) |
							LCD_WF_WF47(0x00);
							
LCD->WF[12] = LCD_WF_WF48(0x00) |
							LCD_WF_WF49(0x00) |
							LCD_WF_WF50(0x00) |
							LCD_WF_WF51(0x00);
							
LCD->WF[13] = LCD_WF_WF52(0x22) | // COM1 (00100010)
							LCD_WF_WF53(0x00) |
							LCD_WF_WF54(0x00) |
							LCD_WF_WF55(0x00);
							
LCD->WF[14] = LCD_WF_WF56(0x00) |
							LCD_WF_WF57(0x00) |
							LCD_WF_WF58(0x00) |
							LCD_WF_WF59(0x00);
							
LCD->WF[15] =	LCD_WF_WF60(0x00) |
							LCD_WF_WF61(0x00) |
							LCD_WF_WF62(0x00) |
							LCD_WF_WF63(0x00);
// end of configuration, so enable LCD

LCD->GCR &= ~LCD_GCR_PADSAFE_MASK; // Enable Front/Back plane operations
LCD->GCR |= LCD_GCR_LCDEN_MASK; // enable LCDEN
}
