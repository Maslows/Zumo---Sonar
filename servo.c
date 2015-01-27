#include "servo.h"
#include "sonar.h"
#include <math.h>

/**
	@brief Define number of TPM2 ticks per microsecond
*/
#define SERVO_TICKS_PER_US 3	

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
	@brief Variable containing Servo current state 
*/
ServoState_t ServoState = IDLE;

/**
	@brief Variable containing current Servo Sweep mode. 
*/
ServoSweep_t ServoSweepMode = SCAN_AND_GO;

/**
	@brief Variable containing current Servo Lock range (cm) in ::SCAN_AND_LOCK mode
*/
uint16_t ServoLockRange = 30;


/**
	@brief Initialize Servo
	Initialize all necessary peripherals needed for servo control
	@param InitialWorkMode Define in which mode should servo operate
	@param InitialSweepMode Define Initial Sweep Mode
	@warning This function must be called AFTER ::Sonar_init !
*/
void Servo_init(ServoMode_t InitialWorkMode, ServoSweep_t InitialSweepMode){
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
	
	
	/* Initialize PIT for servo movement time tracking. Disable it at init. */
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;																	/* Enable PIT Clock Gating */
	PIT->CHANNEL[1].LDVAL = 0xFFFF;																		/* set PIT Load Value */
	PIT->CHANNEL[1].TCTRL |= PIT_TCTRL_TIE_MASK;  										/* Enable interrupts in PIT module on channel 1 */
	PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;  										/* Enable Timer on given channel on channel 1 */
	
	/* Configure NVIC for PIT interupt */
	NVIC_ClearPendingIRQ(PIT_IRQn);															/*  Clear NVIC pending PIT interupts */
	NVIC_EnableIRQ(PIT_IRQn);																		/*  Enable NVIC interrupts source for PIT */
	NVIC_SetPriority(PIT_IRQn, SONAR_INTERUPT_PRIORITY);				/*  Set PIT interrupt priority */	
	
	/* Enable PIT */
	PIT->MCR = 0x00;
	
	/* Enable TPM2 */
	TPM2->SC |= TPM_SC_CMOD(1);	

	/* Set servo mode */
	ServoChangeMode(InitialWorkMode);
	ServoChangeSweepMode(InitialSweepMode);
}

/**
	@brief Helper function which moves servo by #SERVO_STEP_DEG in ::sweep_direction
*/
void ServoMoveByStep(){
	int32_t NewPosition = ServoPosition;
	if (sweep_direction == RIGHT){
		NewPosition += SERVO_STEP_DEG;
		if (NewPosition >= SERVO_MOVEMENT_RANGE) sweep_direction = LEFT;
	} else {
		NewPosition -= SERVO_STEP_DEG;
		if (NewPosition <= -SERVO_MOVEMENT_RANGE) sweep_direction = RIGHT;
	}
	ServoMoveByDegree(NewPosition);	
}


/** 
	@brief Execute a sweep step depending on ::ServoSweepMode
*/
void ServoSweepStep(uint16_t distance){
	switch(ServoSweepMode){
		case SCAN_AND_GO:
				SonarDistHandler(distance, ServoPosition); 														/* Execute user results handler */
				ServoMoveByStep();
			break;
		case SCAN_AND_LOCK:
			if (distance == 0 || distance > ServoLockRange){
				ServoMoveByStep();
			} else {
				ServoState = LOCKED;
				SonarDistHandler(distance, ServoPosition); 													/* Execute user results handler */
				TPM1->CONTROLS[1].CnV = 15u;																				/* Enable trigger */
			}
			break;
	}
}

/**
	@brief This function allows safe changing of servo work mode.
	This function prevents some rare cases of deadlock due to timing 
	issues that might arise when Servo is changing mode.
*/
void ServoChangeMode(ServoMode_t NewMode){
	/* Make sure new mode is not the same as the current one */
	if(NewMode != ServoMode) {
		if (NewMode == SWEEP){
			ServoMode = SWEEP;
			SonarChangeMode(CONTINUOUS);							/* Sonar Must be in CONTINUOUS for servo to work in SWEEP mode */
		} else if (NewMode == MANUAL){
			ServoMode = MANUAL;
		}
	}
}

/**
	@brief This function allows safe changing of servo SWEEP mode.
*/
void ServoChangeSweepMode(ServoSweep_t NewSweep){
	if (NewSweep != ServoSweepMode){
		ServoSweepMode = NewSweep;
	}
}

/**
	@brief This function allows safe changing of servo ::SCAN_AND_LOCK mode lock range.
*/
void ServoChangeLockRange(uint16_t NewRange){
	if (NewRange != ServoLockRange && NewRange <= SONAR_MAX_RANGE_CM){
		ServoLockRange = NewRange;
	}
}

/**
	@brief Move servo to a given position in degrees
	This function allows movment of servo in a direction specified by an angle.
  It will try to predict requied time needed by servo to finish rotation and set 
  PIT_CH2 to countdown. It will also update ::ServoState appropriately.

	@remarks Avoid calling this function while servo is already moving.
  @param degree Desired new position, calculated from servo's normal.
*/
void ServoMoveByDegree(int32_t degree){
	
	/* First check if wanted angle is out of servo range. 
		 If yes, set servo to maximum possible position in wanted direction */
	if (degree > SERVO_MOVEMENT_RANGE ){
		degree = SERVO_MOVEMENT_RANGE;
	} else if ( degree < -SERVO_MOVEMENT_RANGE ) {
		degree = -SERVO_MOVEMENT_RANGE;
	}
	
	/* Check if new position is not the same as the current one */
	if (ServoPosition != degree) {
		uint32_t NewPosition,AngularDistance,TravelTime_ms;
		
		/* Disable Sonar while Servo is in motion */
		DisableSonar();
		  
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

/**
 @brief PIT ISR 
 This function handles PIT interupts
 - Channel 2: Used to keep track of servo movement time.
*/
void PIT_IRQHandler(void){
	if (PIT->CHANNEL[1].TFLG & PIT_TFLG_TIF_MASK) {
		/* CH2 ISR */
		/* Servo reached its destination. Stop countdown and enable trigger*/
		ServoState = IDLE;																						/* Set servo state to idle */
		PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;      						/* Disable timer */
		PIT->CHANNEL[1].TFLG  |= PIT_TFLG_TIF_MASK; 									/* Clear Interupt Flag */
		
		EnableSonar();																								/* Enable Sonar */
	}
}

