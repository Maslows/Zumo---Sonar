#include "sonar.h"
#include "servo.h"
#include <cstdio>
#include "bluetooth.h"

/**
	@brief Define number of TPM1 ticks per 1cm
	At 48MHz, 1 Tick coresponds to 2/3us. Knowing that 1cm ~ 54us we obtain 1cm = 87 ticks
	This value is only an aproximation as it does not take into account variance of speed of
	sound due to temperature and humidity 
	@todo Modify this parameter to include variance of speed of sound due to temperature
*/
#define SONAR_TICKS_PER_CM 87u		 

/**
	@brief Echo pin mask
*/
#define ECHO_MASK (1 << 20)

/** 
	@brief Define sonar work mode
	
	This variable sets sonar work mode. You can change it using ::SonarChangeMode
	@warning Do NOT modify this variable by hand!
*/
SonarMode_t SonarMode = SINGLE; /** Default sonar work mode is SINGLE */ 

/**
	@brief Debug variable containing total number of successful measurments
*/
uint32_t success = 0;

/**
	@brief Debug variable containing total number of failed measurments
*/
uint32_t fail = 0;

/**
	@brief Variable containing number of failed measurments tries; Used to timeout measurement.
*/
uint8_t retry_counter = 0;

/**
	@brief Buffer containing previous measurments. Used only when averaging is enabled.
*/
uint16_t AvgBuffer[SONAR_AVG_NUMBER];

/** 
	@brief Pointer indicating last measurement sample in ::AvgBuffer
*/
uint8_t AvgPointer = 0;

/**
@brief Variable containing result of single measurment done using ::SonarStartMeas or ::SonarGetDistance. 
  It is also used to determind how to return result to the user.
	If this variable is set to -1 before measurement, Sonar will set it to obtained result.
	If this variable is set to 0, Sonar will trigger ::SonarDistHandler with obtained result.
	@warning Do not read or write this variable by hand!
*/
int16_t SingleResult = 0;


/**
 @brief Initialize Sonar and required peripherials
 @param InitialWorkMode Set initial sonar work mode
*/
void Sonar_init(SonarMode_t InitialWorkMode){										

	/* Select clocks */
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); 									/*set 'MCGFLLCLK clock or MCGPLLCLK/2' */
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK; 						/*set "MCGPLLCLK clock with  fixed divide by two" - 48MHz*/
	
	/* Enable clock gating for TMP1 and I/O ports */
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; 									/* Enable TMP1 clock gating */
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;									/* Enable PORTE clock gating */
	
	/* Set I/O ports */
	PORTE->PCR[20] |= PORT_PCR_MUX(0x3);								/* Set PortE[20] as TPM1_CH0 input -- echo */
	PORTE->PCR[21] |= PORT_PCR_MUX(0x3);								/* Set PortE[21] as TPM1_CH1 output -- trigger*/
	FPTE->PDDR |= ( 1 << 21 );													/* Set PortE[21] as output */
	FPTE->PSOR |= (1 << 21);
	
	/* Set TMP1 clock */
	TPM1->SC |= TPM_SC_PS(0x5);													/* Set clock prescaler to divide by 32. 1 Tick = 2/3us */
	TPM1->CNT = 0;																			/* Clear counter value */	
	TPM1->MOD = (SONAR_MEAS_INTERVAL_MS*1000*3)/2;      /* Set measurment interval */
																											

	/* Configure TMP1_CH0. Echo Measurement*/
	TPM1->CONTROLS[0].CnSC |= TPM_CnSC_ELSB_MASK 				/* Set Ch0 to Input capture mode. */
												 | 	TPM_CnSC_ELSA_MASK  			/* Rising and falling edge detection */
												 |  TPM_CnSC_CHIE_MASK; 			/* Enable channel interupts */

										 
	/* Configure TMP1_CH1. Trigger */
	TPM1->CONTROLS[1].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK; /* Set to EPWM - high true pusles */									 
	TPM1->CONTROLS[1].CnV = 15u;																			/* Set counter to 10us */
	
	/* Configure NVIC for TMP1 interupt  */
	NVIC_ClearPendingIRQ(TPM1_IRQn);
	NVIC_EnableIRQ(TPM1_IRQn);
	NVIC_SetPriority (TPM1_IRQn, SONAR_INTERUPT_PRIORITY);

	/* Set initial sonar work mode */
	SonarChangeMode(InitialWorkMode);
	
	/* Enable TPM1 */
	TPM1->SC |= TPM_SC_CMOD(1);

}

/** 
 @brief Helper function which returns result to the user in case of single measurement. 
*/
void ReturnSingleMeas(uint16_t result){
	DisableSonar();															
	/* Check how to return data */
	switch(SingleResult){
		case 0: SonarDistHandler(result, ServoPosition); 												/* Execute user results handler */
		break;	
		
		case -1: SingleResult = result;																					/* Set variable to obtained result */
		break;
	}
}

/**
	@brief Helper function which calculates average of all samples in ::AvgBuffer.
*/
uint16_t CalculateAverage(){
	uint16_t result = 0;
	uint8_t  i = 0;																														
	
	for (i = 0; i < SONAR_AVG_NUMBER; i++){
		result += AvgBuffer[i];
	}
	
	return result/(double)SONAR_AVG_NUMBER;
};

/**
 @brief TPM1 interupt handler

 Check which channel triggered interupt:
 - CH0: Detect rising/falling edge on echo pin.
*/
void TPM1_IRQHandler(void) {	
	/* Check if it's rising edge interupt or falling edge interupt on Ch0*/

	/* Rising edge */
	if (FPTE->PDIR & ECHO_MASK){ 
		  TPM1->CONTROLS[1].CnV = 0;														  				 /* Disable trigger */
			TPM1->CNT = 0; 																							 	   /* Reset counter */
			TPM1->SC |= TPM_SC_TOF_MASK;															       /* Clear TPM1 Overflow flag */
		  TPM1->SC |= TPM_SC_TOF_MASK;															       /* Double buffered  */
		
	/* Falling edge */
	} else {	
			/* Check if timer overflowed while echo was high */
			if ( TPM1->SC & TPM_SC_TOF_MASK )	{
				TPM1->SC |= TPM_SC_TOF_MASK;															  /* Clear TPM1 Overflow flag */
				fail++;																											/* Increment debug variable */
				retry_counter++;																					  /* Increment overflow counter */ 
				TPM1->CONTROLS[1].CnV = 15u;																/* Enable trigger */
				/* If we reach retry limit, proceed with next sweep step */
				if (ServoMode == SWEEP && retry_counter > SONAR_MAXTRY) {
					retry_counter = 0;																														/* reset retry counter */
					Servo_sweep_step();																														/* Execute next serwo step */
				}
				
				/* Successful measurment */
			}	else { 	
				uint16_t result = TPM1->CONTROLS[0].CnV/SONAR_TICKS_PER_CM;	                    /* Get result */
				
				/* Check if result is smaller than defined max range */
				if (result < SONAR_MAX_RANGE_CM) {
					/* Add result to buffer */
					retry_counter = 0;
					AvgBuffer[AvgPointer] = result;
					AvgPointer = (AvgPointer + 1) % SONAR_AVG_NUMBER;
					success++;
				} else {
					/* If obtained sample is out of range, increment timeout counter */ 
					retry_counter++;
					TPM1->CONTROLS[1].CnV = 15u;																									/* Enable trigger */
				}
				
				/* Depending on the settings, decide what to do next with obtained result */
				
				/* If Servo is in SWEEP mode and we collect SONAR_AVG_NUMBER samples
					 call user handler and procceed with the sweep */
				if (ServoMode == SWEEP && AvgPointer == 0 && retry_counter == 0) {						 	/* Execute next servo step if enabled */
					result = CalculateAverage();																									/* Calculate average of all collected samples */
					SonarDistHandler(result, ServoPosition); 																			/* Execute user results handler */	
					Servo_sweep_step();
					
				/* If measurment is successful but result is out of range	SONAR_MAXTRY times
					 proceed with sweep */
			  } else if (ServoMode == SWEEP && retry_counter > SONAR_MAXTRY ) {
					retry_counter = 0;																													/* reset retry counter  */
					Servo_sweep_step();																													/* Execute next serwo step */
					
				/* If Servo is in manual mode and Sonar is set to CONTINUOUS work, just call user handler */	
				} else if ( ServoMode == MANUAL && SonarMode == CONTINUOUS ) {
					if (retry_counter == 0) {
						result = CalculateAverage();																									/* Calculate average of all collected samples */
						SonarDistHandler(result, ServoPosition); 																			/* Execute user results handler */
					} else if (retry_counter >= SONAR_MAXTRY) {
						retry_counter = 0;
						SonarDistHandler(0, ServoPosition); 																			/* Execute user results handler */	
					}
					TPM1->CONTROLS[1].CnV = 15u;																									/* Enable trigger */
		
				/* If Sonar is set to SINGLE work mode and we collect enought samples return value
					 and disable sonar */
			  } else if (SonarMode == SINGLE && AvgPointer == 0 ) {
					if (retry_counter == 0) {
						result = CalculateAverage();																									/* Calculate average of all collected samples */
						ReturnSingleMeas(result);
					}	else if (retry_counter >= SONAR_MAXTRY){
						retry_counter = 0;
						ReturnSingleMeas(0);
					}
				 
				/* Wait for more samples */
				} else {
					TPM1->CONTROLS[1].CnV = 15u;																						/* Enable trigger */
				}
		}
		
		/* If Servo mode is set to SWEEP or Sonar to SINGLE, reset counter right after detecting falling edge.
       This will force next trigger as soon as sonar is ready to recieve another echo.
			 It is possible to use it also in MANUAL mode, but if something is nearby sonar,
			 it will produce A LOT of samples. In SWEEP mode this is not an issude due to the
			 fact that sonar if offline when servo is changing position. */
		if (ServoMode == SWEEP || SonarMode == SINGLE){
			TPM1->CNT = 0;
		}
	}
	TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;										 /* clear TMP1_Ch0 flag */
} 

/**
	@brief This function allows save changing of sonar work mode.
*/
void SonarChangeMode(SonarMode_t NewMode){
	/* Make sure new mode is not the same as the current one */
	if(NewMode != SonarMode) {
		if (NewMode == CONTINUOUS){
			SonarMode = CONTINUOUS;
			EnableSonar();
		} else if (NewMode == SINGLE){
			/* If Servo is in SWEEP mode change it to MANUAL */
			if (ServoMode == SWEEP){
				ServoChangeMode(MANUAL);
			}
			DisableSonar();
			SonarMode = SINGLE;
		}
	}
}

/**
	@brief Disable all sonar operations and interupts.
*/
void DisableSonar(void){
	TPM1->CONTROLS[0].CnSC &= ~TPM_CnSC_CHIE_MASK;								/* Disable Sonar interupts */
	TPM1->CONTROLS[1].CnV = 0u;																	  /* Disable trigger */
	TPM1->CNT = 0;																							  /* Reset counter */
};

/**
	@brief Enable  sonar operations and interupts.
*/
void EnableSonar(void){
	/* If servo is IDLE, start measurement right away. 
	   Otherwise, wait for servo (PIT) to enable measurement */
	if (ServoState == IDLE) {
		TPM1->CONTROLS[1].CnV = 15u;																/* Enable trigger */
		TPM1->CNT = 0;																							/* Reset counter */
		retry_counter = 0;																				  /* Clear overflow timeout */
		TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;								/* Clear Sonar interupt flag */
		TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;								/* Double buffer */
		TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHIE_MASK;								/* Enable Sonar interupts */
	}
};

/** 
	@brief Initialize a single sonar measurment
	This function can be used to trigger a single measurment. 
	Interupt will trigger ::SonarDistHandler.
	@param angle Specify at what Servo angle measurement should be taken
	@warning This function uses busy-waiting to check servo and sonar readiness
*/
void SonarStartMeas(int32_t angle){
	Servo_move_by_degree(angle);		/* Set servo to desired position */
	SingleResult = 0;								/* Set SingleResult to 0 to indicate how 
																		 result should be returned. 0 = Interupt. */
	SonarChangeMode(SINGLE);				/* Change sonar mode and enable if not set to single already*/
	EnableSonar();
}; 


/** 
	@brief Initialize a single sonar measurment and return distance in cm.
	This function can be used to trigger a single measurment. 
  @param angle Specify at what Servo angle measurement should be taken
	@warning This function uses busy-waiting to check servo and sonar readiness
	@return Measured distance in cm
*/
uint16_t SonarGetDistance(int32_t angle){
	Servo_move_by_degree(angle);			/* Set servo to desired position */
	SingleResult = -1;								/* Set SingleResult to 0 to indicate how 
																		   result should be returned. 1 = write result to SingleResult. */
	SonarChangeMode(SINGLE);				  /* Change sonar mode and enable if not set to single already*/
	EnableSonar();
	
	while(SingleResult == -1){};		/* busy-wait for result */
	return SingleResult;
}; 


/** 
	@brief User handler for sonar measurment results
	This function handles results from sonar's continuous measurments.
	It is also called when user triggered single measurment finishes.
  User can specify here what to do with the results.
	If sonar is used, user can obtain the angle at which the measurment was taken
  by reading global variable ::ServoPosition
	@param distance_cm Measured distance in cm. 
				 If measurment failed or was out of range
				 function will be called with parameter 0.
*/
void SonarDistHandler(uint16_t distance_cm, int32_t angle){	
	/* Your code here */
	char buffor[12];
	sprintf(buffor, "%04d,%04hu\n",angle,distance_cm);
	bt_sendStr(buffor);
}

