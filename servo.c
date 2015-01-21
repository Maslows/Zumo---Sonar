#include "servo.h"
#include <math.h>

/**
	@brief Define servo movement direction in sweep mode 
	@warning Do NOT modify this! Doing so might result in damaging a servo.
*/
enum {RIGHT,LEFT} sweep_direction = RIGHT; 

/** 
	@brief Define Servo Work mode 
  Manual mode is default.
*/
ServoMode_t ServoMode = MANUAL; 

/**
 @brief Define Servo Position in degrees.
 This variable contains predicted position of a servo. 
 @warning This is only the prediction and might have some
          error due to wrong value of ::SERVO_MOVEMENT_RANGE
          or due to nonlinearity of a servo.
*/
int32_t ServoPosition = 0;

/**
	@brief Define Servo current state 
*/
ServoState_t ServoState = IDLE;


/**
	@brief Initialize Servo
	Initialize all necessary peripherals needed for servo control
	@param InitialWorkMode Define in which mode should servo operate
	@warning This function must be called AFTER ::Sonar_init !
*/
void Servo_init(ServoMode_t InitialWorkMode){
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
	
	/* Set servo mode */
	ServoMode = InitialWorkMode;
	
	/* Initialize PIT for servo movement time tracking. Disable it at init. */
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;																	/* Enable PIT Clock Gating */
	PIT->CHANNEL[1].LDVAL = 0xFFFF;																		/* set PIT Load Value */
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TIE_MASK;  										/* Enable interrupts in PIT module on channel 1 */
	PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;  										/* Enable Timer on given channel on channel 1 */
	
	/* Enable PIT */
	PIT->MCR = 0x00;
	
	/* Enable TPM2 */
	TPM2->SC |= TPM_SC_CMOD(1);		
}

/** 
	@brief Execute a sweep step
	Each time this function is called, servo will move by ::SERVO_SWEEP_STEP_DEG in
  ::sweep_direction. If it reaches ::SERVO_MOVEMENT_MAX it will change the direction of the sweep
*/
void Servo_sweep_step(){
	int32_t NewPosition = ServoPosition;
	
	if (sweep_direction == RIGHT){
		NewPosition += SERVO_SWEEP_STEP_DEG;
		if (NewPosition >= SERVO_MOVEMENT_RANGE) sweep_direction = LEFT;
	} else {
		NewPosition -= SERVO_SWEEP_STEP_DEG;
		if (NewPosition <= -SERVO_MOVEMENT_RANGE) sweep_direction = RIGHT;
	}
	Servo_move_by_degree(NewPosition);	
}

/**
	@brief Move servo to a given position in degrees
	This function allows movment of servo in a direction specified by an angle.
  It will try to predict requied time needed by servo to finish rotation and set 
  PIT_CH2 to countdown. It will also update ::ServoState appropriately.

	@remarks Avoid calling this function while servo is already moving.
  @param degree Desired new position, calculated from servo's normal.
*/
void Servo_move_by_degree(int32_t degree){
	uint32_t NewPosition,AngularDistance,TravelTime_ms;
	/* First check if wanted degree is out of servo range. 
		 If yes, set servo to maximum possible possition in wanted direction */
	if (degree >= SERVO_MOVEMENT_RANGE ){
		TPM2->CONTROLS[0].CnV = SERVO_MOVEMENT_MAX;
	} else if ( degree <= -SERVO_MOVEMENT_RANGE ) {
		TPM2->CONTROLS[0].CnV = SERVO_MOVEMENT_MIN;
	} else { 
		/* Recalculate degrees to us and set servo 
			 For now assume that servo is linear */
		NewPosition = (((degree+SERVO_MOVEMENT_RANGE))*(SERVO_MOVEMENT_MAX-SERVO_MOVEMENT_MIN))/(2*SERVO_MOVEMENT_RANGE);
		NewPosition += SERVO_MOVEMENT_MIN;
		TPM2->CONTROLS[0].CnV = NewPosition;
				
		/* Calculate servo movement distance and time */
		AngularDistance = sqrt((degree-ServoPosition)*(degree-ServoPosition));
		TravelTime_ms = AngularDistance*1000/SERVO_ANGULAR_VELOCITY;
		
		/* Change Servo State to moving and update its oposition */
		ServoState = MOVING;
		ServoPosition = degree;
		
		/* Set PIT_CH2 to Travel Time and start countdown */
		PIT->CHANNEL[1].LDVAL = TravelTime_ms*24E3;  			/* Clock runs at 24MHz */
		PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TEN_MASK;      /* Enable timer */
	}		
};

