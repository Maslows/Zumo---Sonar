#include "servo.h"
const uint16_t servo_step = ((SERVO_MOVEMENT_MAX-SERVO_MOVEMENT_MIN)/SERVO_SWEEP_STEPS_NBR);
enum {RIGHT,LEFT} sweep_direction = RIGHT; 

void Servo_init(void){
	/* Enable clock gating for TMP1 and I/O ports */
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK; 
	SIM->SCGC6 |= SIM_SCGC6_TPM2_MASK;
	
	/* Set PORTE22 as TPM2 output */
	PORTE->PCR[22] |= PORT_PCR_MUX(0x3);
	
	/* Set timer clock prescaler and MOD */
	TPM2->SC |= TPM_SC_PS(0x4);          								  /* divide clock by 16 48MHZ/16.   3 ticks = 1u */
	TPM2->MOD = 45000u;  																	/* set MOD to value 15ms */

	/* Configure TPM2_CH0. Edge aligned PWM and center servo at start */
	TPM2->CONTROLS[0].CnSC |= TPM_CnSC_ELSB_MASK | TPM_CnSC_MSB_MASK; /* set TPM0, channel 2 to edge-aligned PWM high-true pulses */
	TPM2->CONTROLS[0].CnV = 1500*3; 																	/* Center servo. 1.5ms */
	
	/* Enable TPM2 */
	TPM2->SC |= TPM_SC_CMOD(1);		
}

void Servo_step(){
	uint16_t NewPosition = TPM2->CONTROLS[0].CnV;
	
	if (sweep_direction == RIGHT){
		NewPosition += servo_step;
		if (NewPosition >= SERVO_MOVEMENT_MAX){
			sweep_direction = LEFT;
			TPM2->CONTROLS[0].CnV = SERVO_MOVEMENT_MAX;
		} else {
			TPM2->CONTROLS[0].CnV = NewPosition;
		}
	} else {
		NewPosition -= servo_step;
		if (NewPosition <= SERVO_MOVEMENT_MIN){
			sweep_direction = RIGHT;
			TPM2->CONTROLS[0].CnV = SERVO_MOVEMENT_MIN;
		} else {
			TPM2->CONTROLS[0].CnV = NewPosition;
		}		
	}
}
