#include "sonar.h"
#include "servo.h"
#include "sLCD.h"
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
	@brief Define sonar work mode
	
	This variable sets sonar work mode. You can set it to different mode
	during run time
*/
SonarWorkModes SonarMode = SINGLE; /** Default sonar work mode is SINGLE */

/**
	@brief Defines current sonar state.
	@warning {Do NOT change this anywhere except SonarDistHandler(). 
						Doing so might result in deadlock.}
*/
SonarFSM SonarState = SONAR_IDLE;  

/**
	@brief Debug variable containing number of successful measurments
*/
uint32_t success =0;

/**
	@brief Debug variable containing number of failed measurments
*/
uint32_t fail =0;

/**
	@brief Debug variable containing number of failed measurments
*/
#define sonar_maxtry 2
uint32_t overflow_timeout = 0;

/**
 @brief Initialize Sonar and required peripherials
 @param InitialWorkMode Set initial sonar work mode
*/
void Sonar_init(SonarWorkModes InitialWorkMode){
	double pit_interval = 0.0;													

	/* Select clocks */
	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); 									/*set 'MCGFLLCLK clock or MCGPLLCLK/2' */
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK; 						/*set "MCGPLLCLK clock with  fixed divide by two" - 48MHz*/
	
	/* Enable clock gating for TMP1 and I/O ports */
	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK; 									/* Enable TMP1 clock gating */
	SIM->SCGC5 |= SIM_SCGC5_PORTE_MASK;									/* Enable PORTE clock gating */
	
	/* Set I/O ports */
	PORTE->PCR[20] |= PORT_PCR_MUX(0x3);								/* Set PortE[20] as TPM1_CH0 input */
	PORTE->PCR[21] |= PORT_PCR_MUX(0x1);								/* Set PortE[21] as GPIO */
	FPTE->PDDR |= ( 1 << 21 );													/* Set PortE[21] as output */
	
	/* Set TMP1 */
	TPM1->SC |= TPM_SC_PS(0x5);													/* Set clock prescaler to divide by 32. 1 Tick = 2/3us */
	TPM1->CNT = 0;																			/* Clear counter value */	
	TPM1->MOD = (25000*3)/2;                            /* Set max echo length to 25ms */
																											

	/* Configure TMP1_CH0. Echo Measurement*/
	TPM1->CONTROLS[0].CnSC |= TPM_CnSC_ELSB_MASK 	/* Set Ch0 to Input capture mode. */
												 | 	TPM_CnSC_ELSA_MASK  /* Rising and falling edge detection */
												 |  TPM_CnSC_CHIE_MASK; /* Enable channel interupts */

						
												 
	/* Configure TMP1_CH1. Trigger */
	TPM1->CONTROLS[1].CnSC |= TPM_CnSC_MSA_MASK 	/* Set Ch1 to software compare mode. */
																								/* Ch1 output is not controlled by TPM1! */
												 | TPM_CnSC_CHIE_MASK;  /* Enable channel interupts */
	TPM1->CONTROLS[1].CnV = 15u;									/* Set counter to 10us */
	
	/* Configure NVIC for TMP1 interupt  */
	NVIC_ClearPendingIRQ(TPM1_IRQn);
	NVIC_EnableIRQ(TPM1_IRQn);
	NVIC_SetPriority (TPM1_IRQn, SONAR_INTERUPT_PRIORITY);

	/* Set initial sonar work mode */
	SonarMode = InitialWorkMode;
	
	/* Initialize PIT for continous work mode */
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;																	/* Enable PIT Clock Gating */
	pit_interval = (SONAR_MEAS_INTERVAL_MS * 1.0E-3)/(1.0/24.0E6);    /* calculate PIT Load Value */
	PIT->CHANNEL[0].LDVAL = (uint32_t)pit_interval;										/* set PIT Load Value */
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;  										/* Enable interrupts in PIT module on channel 1 */
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;  										/* Enable Timer on given channel on channel 1 */
	
	/* Configure NVIC for PIT interupt */
	NVIC_ClearPendingIRQ(PIT_IRQn);															/*  Clear NVIC pending PIT interupts */
	NVIC_EnableIRQ(PIT_IRQn);																		/*  Enable NVIC interrupts source for PIT */
	NVIC_SetPriority(PIT_IRQn, SONAR_INTERUPT_PRIORITY);				/*  Set PIT interrupt priority */	
	
	/* Enable TPM1 */
	TPM1->SC |= TPM_SC_CMOD(1);
	
	/* Enable PIT */
	PIT->MCR = 0x00;
}


/**
 @brief TPM1 interupt handler

 Check which channel triggered interupt:
 - CH0: If sonar state is TRIGGER_SENT, clear counter and set sonar state to CAPTURE_START\n
      If sonar state is CAPTURE_START, process the result and set sonar state to CAPTURE_END
 - CH1:  Turn off trigger pin which was set maunally or by PIT in countinous mode
 - Timer Overflow: Set sonar to CAPTURE_OVERFLOW state 
*/
void TPM1_IRQHandler(void) {
	/* Ch1 ISR */
	if (TPM1->CONTROLS[1].CnSC & TPM_CnSC_CHF_MASK) {
					FPTE->PCOR |= (1 << 21);  															 			 /* Turn off trigger pin*/					
					TPM1->CONTROLS[1].CnSC &= ~TPM_CnSC_CHIE_MASK;								 /* Disable TMP1_Ch1 interupts */ 
					TPM1->CONTROLS[1].CnSC |= TPM_CnSC_CHF_MASK;									 /* Clear TMP1_Ch1 flag */
	} 
	
	/* Ch0 ISR */
	if (TPM1->CONTROLS[0].CnSC & TPM_CnSC_CHF_MASK) {
		if ( SonarState == SONAR_TRIGGER_SENT ) {
				TPM1->CNT = 0; 																							 /* Reset counter */
				SonarState = SONAR_CAPTURE_START;														 /* Change sonar state to CAPTURE_START */										 
		} else if (SonarState == SONAR_CAPTURE_START) {	
			  TPM1->SC |= TPM_SC_TOF_MASK;															   /* Clear TPM1 Overflow flag */
				TPM1->SC &= ~TPM_SC_TOIE_MASK; 															 /* Disable TPM1 Overflow interupt */
				SonarState = SONAR_CAPTURE_END;															 /* Change sonar state to CAPTURE_END */
				SonarDistHandler(TPM1->CONTROLS[0].CnV/SONAR_TICKS_PER_CM);  /* Execute user results handler */
			
				if (ServoMode == SWEEP && ServoState == IDLE) {						 /* Execute next servo step if enabled */
					Servo_sweep_step();
				}
				
				SonarState = SONAR_IDLE;															 			 /* Change sonar state to IDLE */
				success++;
		} else if ( SonarState == SONAR_CAPTURE_OVERFLOW ) {
				/* Wait until echo output is low */
			  SonarState = SONAR_IDLE;
		}
		TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;										 /* clear TMP1_Ch0 flag */
	} 
	
	/* TPM1 overflow handler */ 
	if ((TPM1->SC & TPM_SC_TOF_MASK ) && (TPM1->SC & TPM_SC_TOIE_MASK )) {
					 TPM1->SC |= TPM_SC_TOF_MASK;															/* Clear TPM1 Overflow flag */
					 TPM1->SC &= ~TPM_SC_TOIE_MASK; 													/* Disable TPM1 Overflow interupt */
					 SonarState = SONAR_CAPTURE_OVERFLOW;										  /* set Sonar to CAPTURE_OVERFLOW state */
					 SonarDistHandler(0u);																		/* Execute user results handler */
					 fail++;
	}	
}

/**
	@brief Send a single trigger
	
	Function sets GPIO pin high and resets TPM1 counter. When it reaches 10us TPM1
	ISR turns off the pin.
*/
void SendTrigger(void){
		SonarState = SONAR_TRIGGER_SENT;
		TPM1->CONTROLS[1].CnSC |= TPM_CnSC_CHIE_MASK;								 /* Enable TMP1_Ch1 interupts */
		TPM1->SC |= TPM_SC_TOF_MASK;															   /* Clear TPM1 Overflow flag */
		TPM1->SC |= TPM_SC_TOIE_MASK; 											         /* Enable TPM1 Overflow interupt */
		TPM1->CNT = 0; 																							 /* Reset counter */
		FPTE->PSOR |= (1 << 21);																		 /* Toggle Trigger pin on */
}

/**
 @brief PIT ISR 
 This function chandels PIT interupts
 - Channel 1: Controls the continous work of the sonar
 - Channel 2: Used to keep track of servo movement time.
*/
void PIT_IRQHandler(void){
	/* CH1 ISR */
	if (PIT->CHANNEL[0].TFLG & PIT_TFLG_TIF_MASK) {
			switch(SonarState) {
				case SONAR_IDLE:
						 if (ServoState == IDLE && SonarMode == CONTINUOUS) SendTrigger();
						 break;
				case SONAR_CAPTURE_OVERFLOW:
						 if ( (overflow_timeout++ > sonar_maxtry) && ServoMode == SWEEP) {
							SonarState = SONAR_IDLE;
							Servo_sweep_step();
							overflow_timeout = 0;
						 } else {
							 SendTrigger();
						 }
						 break;
				default:
						 break;
			}

		PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK; 									   /* Clear Interupt Flag */
	} else {
		/* CH2 ISR */
		/* Servo reached its destination. Stop countdown */
		ServoState = IDLE;																						 /* Set sonar state to idle */
		PIT->CHANNEL[1].TCTRL &= ~PIT_TCTRL_TEN_MASK;      						 /* Disable timer */
		PIT->CHANNEL[1].TFLG  |= PIT_TFLG_TIF_MASK; 									 /* Clear Interupt Flag */
	}
}

/** 
	@brief Initialize a single sonar measurment
	This function can be used to trigger a single measurment. 
	Interupt will trigger ::SonarDistHandler.
	@warning This function uses busy-waiting to check servo and sonar readiness
*/
void SonarStartMeas(void){
	
	/* change sonar mode to single */
	SonarMode = SINGLE;
	
	/* Wait for servo */
	while (ServoState != IDLE){}; 
		
	/* Wait for sonar */
  while (    SonarState != SONAR_CAPTURE_OVERFLOW 	
					&& SonarState != SONAR_IDLE 
					&& SonarState != SONAR_CAPTURE_END ){};
	/* Send sonar trigger */
	SendTrigger();
}; 


/** 
	@brief Initialize a single sonar measurment and return distance in cm.
	This function can be used to trigger a single measurment. 
	@warning This function uses busy-waiting to check servo and sonar readiness
	@return Measured distance in cm
*/
uint16_t SonarGetDistance(void){
		/* Wait for servo */
	while (ServoState != IDLE){}; 
		
	/* Wait for sonar */
  while (    SonarState != SONAR_CAPTURE_OVERFLOW 	
					&& SonarState != SONAR_IDLE 
					&& SonarState != SONAR_CAPTURE_END ){};
	/* Send sonar trigger */
	SendTrigger();
						
	/* wait for results */
  while (    SonarState != SONAR_CAPTURE_OVERFLOW 	
					&& SonarState != SONAR_IDLE 
					&& SonarState != SONAR_CAPTURE_END ){};
						
	return 	TPM1->CONTROLS[0].CnV/SONAR_TICKS_PER_CM;				
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
void SonarDistHandler(uint16_t distance_cm){	
	/* Your code here */
	char buffor[12];
	sprintf(buffor, "%04d,%04hu\n",ServoPosition,distance_cm);
	bt_sendStr(buffor);

	/* If you uncomment this line, sonar will proceed with the sweep
		 even if the measurment failed. If you leave this line commented, then
		 sonar will retry measurment untill usable data is obtained */	 
	//SonarState = SONAR_IDLE;
}

