#include "sonar.h"
#include "servo.h"
#include "sLCD.h"
#define SONAR_TICKS_PER_CM 87u		 /*  Tick = 2/3us. 1cm ~ 54us.   1cm = 87 ticks */
#define SONAR_TICKS_PER_MM 8.7f 	 /* TODO: Include temperature influence on speed of sound */

SonarWorkModes SonarMode = SINGLE; /** Default sonar work mode is SINGLE */
SonarFSM SonarState = SONAR_IDLE;  /** Initialize sonar state to IDLE */

/* Debug variables */
uint32_t success =0;
uint32_t fail =0;
/********************************************//**
 *  Brief Initialize Sonar and required peripherials
 ***********************************************/
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
	TPM1->MOD = (35000*3)/2;                            /* Set max echo length to 35ms */
																											

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
	pit_interval = (SONAR_MEAS_INTERVAL_MS * 1.0E-3)/(1.0/24.0E6);    								/* calculate PIT Load Value */
	PIT->CHANNEL[0].LDVAL = (uint32_t)pit_interval;										/* set PIT Load Value */
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;  										/* Enable interrupts in PIT module on channel 1 */
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;  										/* Enable Timer on given channel on channel 1 */
	
	/* Configure NVIC for PIT interupt */
	NVIC_ClearPendingIRQ(PIT_IRQn);											/*  Clear NVIC pending PIT interupts */
	NVIC_EnableIRQ(PIT_IRQn);														/*  Enable NVIC interrupts source for PIT */
	NVIC_SetPriority(PIT_IRQn, SONAR_INTERUPT_PRIORITY);				/*  Set PIT interrupt priority */	
	
	/* Enable TPM1 */
	TPM1->SC |= TPM_SC_CMOD(1);
	
	/* Enable PIT */
	PIT->MCR = 0x00;
}


/********************************************//**
 *  Brief TPM1 interupt handler
 *  Check which channel triggered interupt
 *  CH0  - If sonar state is TRIGGER_SENT, clear counter and set sonar state to CAPTURE_START
 *         If sonar state is CAPTURE_START, process the result and set sonar state to CAPTURE_END
 *  CH1  - Turn off trigger pin which was set maunally or by PIT in countinous mode
 *  Timer Overflow - Set sonar to CAPTURE_OVERFLOW state 
 ***********************************************/
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
		} else  {	
			  TPM1->SC |= TPM_SC_TOF_MASK;															   /* Clear TPM1 Overflow flag */
				TPM1->SC &= ~TPM_SC_TOIE_MASK; 															 /* Disable TPM1 Overflow interupt */
				SonarDistHandler(TPM1->CONTROLS[0].CnV/SONAR_TICKS_PER_MM);  /* Execute user results handler */
				SonarState = SONAR_CAPTURE_END;															 /* Change sonar state to CAPTURE_END */
				success++;
		}
		TPM1->CONTROLS[0].CnSC |= TPM_CnSC_CHF_MASK;										 /* clear TMP1_Ch0 flag */
	} 
	
	/* TPM1 overflow handler */ 
	if ((TPM1->SC & TPM_SC_TOF_MASK ) && (TPM1->SC & TPM_SC_TOIE_MASK )) {
					 TPM1->SC |= TPM_SC_TOF_MASK;															/* Clear TPM1 Overflow flag */
					 TPM1->SC &= ~TPM_SC_TOIE_MASK; 													/* Disable TPM1 Overflow interupt */
					 SonarState = SONAR_CAPTURE_OVERFLOW;										  /* set Sonar to CAPTURE_OVERFLOW state */
					 fail++;
					 SonarDistHandler(0u);
					 SonarState = SONAR_IDLE;
	}	
}

void SendTrigger(void){
		SonarState = SONAR_TRIGGER_SENT;
		TPM1->CONTROLS[1].CnSC |= TPM_CnSC_CHIE_MASK;								 /* Enable TMP1_Ch1 interupts */
		TPM1->SC |= TPM_SC_TOF_MASK;															   /* Clear TPM1 Overflow flag */
		TPM1->SC |= TPM_SC_TOIE_MASK; 											         /* Enable TPM1 Overflow interupt */
		TPM1->CNT = 0; 																							 /* Reset counter */
		FPTE->PSOR |= (1 << 21);																		 /* Toggle Trigger pin on */
}

/********************************************//**
 *  Brief PIT IRQ
 ***********************************************/
void PIT_IRQHandler(void){
	//if (SonarMode == CONTINUOUS) {
	if (SonarMode == CONTINUOUS) {
		switch(SonarState) {
			case SONAR_IDLE:
					 //Servo_step();
					 SendTrigger();
					 break;
			case SONAR_CAPTURE_OVERFLOW: 
					 SendTrigger();
					 Servo_step();
					 break;
			case SONAR_CAPTURE_END:
					 Servo_step();
					 SendTrigger();
					 break;
			default:
					 //Servo_step();
					 break;
		}
	PIT->CHANNEL[0].TFLG |= PIT_TFLG_TIF_MASK; 									 /* Clear Interupt Flag */
	}
}

void SonarDistHandler(uint16_t distance){
	sLCD_DisplayDec(distance);
}

