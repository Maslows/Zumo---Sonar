#include "motorDriver.h"

#define PRIORITY_INTERRUPT_DRIVE (0u)
#define V_MOD 1023
#define LEFT (1u<<13)
#define RIGHT (1u<<9)

uint32_t tmp = 0;

uint32_t ACTUAL_DISTANCE = 0;
uint8_t FLAG_LIMIT_DIST = 0;
uint32_t LIMIT = 0;
uint8_t  speed = 50;

void motorDriverInit(void){
	
	// CLOCK_SETUP 1
	// 1 ... Multipurpose Clock Generator (MCG) in PLL Engaged External (PEE) mode
  //       Reference clock source for MCG module is an external crystal 8MHz
  //       Core clock = 48MHz, BusClock = 24MHz
	
	//
	SIM -> SCGC5 |= SIM_SCGC5_PORTA_MASK 
	              | SIM_SCGC5_PORTC_MASK 
	              | SIM_SCGC5_PORTD_MASK;
	
	//
	SIM -> SCGC6 |= SIM_SCGC6_TPM0_MASK;
	
	//
	PORTA ->PCR[6] |= PORT_PCR_MUX(3); // TPM0_CH3 - encoder
	PORTA ->PCR[13] |= PORT_PCR_MUX(1); // PHASE - Left
	PORTC ->PCR[9] |= PORT_PCR_MUX(1); // PHASE - Right
	PORTD ->PCR[2] |= PORT_PCR_MUX(4); // TPM0_CH2 - PWM - Right
	PORTD ->PCR[4] |= PORT_PCR_MUX(4); // TPM0_CH4 - PWM - Left
	PORTD ->PCR[5] |= PORT_PCR_MUX(4); //TPM0_CH5 - encoder
	
	// OUTPUT pin
	PTA->PDDR |= (1u<<13);
	PTC->PDDR |= (1u<<9);
	
	
	
	////////////////////// PWM /////////////////////////////////
	
	//select source reference TMP0

	SIM->SOPT2 |= SIM_SOPT2_TPMSRC(1); // ?set 'MCGFLLCLK clock or MCGPLLCLK/2'
	
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;// set "MCGPLLCLK clock with  fixed divide by two"
	
	// set "up-counting"
	TPM0->SC &= ~TPM_SC_CPWMS_MASK; // default set
	
	// divide by 1
	TPM0->SC &= ~TPM_SC_PS_MASK; // the same TPM_SC_PS(0)
	
	// clear counter
	TPM0->CNT = 0x00; 
	
	// set MOD for PWM period equal 4095 ( 12 bit)
	TPM0->MOD = V_MOD;
	
	//////////CHANNEL ENGINE ////////////////////////////////
	//Right engine
	// set TPM0 channel 2 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[2].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	// Default value for Right engine
	TPM0->CONTROLS[2].CnV = 0; // STOP
	
	//Left engine
	// set TPM0 channel 4 - "Edge-aligned PWM High-true pulses"
	TPM0->CONTROLS[4].CnSC |= TPM_CnSC_MSB_MASK | TPM_CnSC_ELSB_MASK;
	// Default value for Left engine
	TPM0->CONTROLS[4].CnV = 0; // STOP
	
	//////////CHANNEL ENCODER////////////////////////////////
	//Left Engine
	// set TPM0 channel 3 - "Edge-aligned PWM High-true pulses"
	//TPM0->CONTROLS[3].CnSC |= TPM_CnSC_ELSA_MASK |	
	//												  TPM_CnSC_ELSB_MASK;
	// enable interrupt for channel 3
	//TPM0->CONTROLS[3].CnSC |= TPM_CnSC_CHIE_MASK;
	
	//Right Engine
	// set TPM0 channel 3 - "Edge-aligned PWM High-true pulses"
	//TPM0->CONTROLS[5].CnSC |= TPM_CnSC_ELSA_MASK |	
	//												  TPM_CnSC_ELSB_MASK;
	// enable interrupt for channel 3
	//TPM0->CONTROLS[5].CnSC |= TPM_CnSC_CHIE_MASK;
	
	
	///NVIC_ClearPendingIRQ(TPM0_IRQn);				/* Clear NVIC any pending interrupts on TPM0 */
	//NVIC_EnableIRQ(TPM0_IRQn);							/* Enable NVIC interrupts source for TPM0 module */
	
	//NVIC_SetPriority (TPM0_IRQn, PRIORITY_INTERRUPT_DRIVE);	// priority interrupt
	
	// enable counter
	TPM0->SC |= TPM_SC_CMOD(1);
	
}

void TPM0_IRQnHandler(void){
	
	if(FLAG_LIMIT_DIST){
		
		if( LIMIT <= ACTUAL_DISTANCE)
		{
			stop();
			FLAG_LIMIT_DIST = 0;
		}
	}
	////// tutaj jeszcze kod ktory bd liczyl dystans z enkodera////////////////
	
	
	
}

void driveForwardLeftTrack( int predkosc){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTA->PCOR = LEFT; // clear , set 0 mean forward
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = 1023 + 1;
		
	    }
	    else {
		
	       TPM0->CONTROLS[4].CnV = 1023*predkosc/100 ;
	
      }
	}
	
}

void driveForwardDistLeftTrack( int predkosc , int droga){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTA->PCOR = LEFT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = predkosc/100 * V_MOD;
	
         }
				
		FLAG_LIMIT_DIST = 1;
		LIMIT = droga; // info dla enkodera
				 
				 
	}
		
}

void driveForwardRightTrack( int predkosc){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTC->PCOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD*predkosc/100 ;
	
         }
	}
	
}

void driveForwardDistRightTrack( int predkosc , int droga){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTC->PCOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = predkosc/100 * V_MOD;
	
         }
		
    FLAG_LIMIT_DIST = 1;				 
		LIMIT = droga; // info dla enkodera
				 
				 
	}
		
}

void stop(void){
	
	
	TPM0->CONTROLS[2].CnV = 0; // stop RIGHT
	TPM0->CONTROLS[4].CnV = 0; // stop LEFT
	
	
}

void driveReverseLeftTrack( int predkosc){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTA->PSOR = LEFT; // clear , set 0 mean forward
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = V_MOD *predkosc/100 ;
	
         }
	}
	
}

void driveReverseDistLeftTrack( int predkosc , int droga){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTA->PSOR = LEFT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[4].CnV = predkosc/100 * V_MOD;
	
         }
		
    FLAG_LIMIT_DIST = 1;				 
		LIMIT = droga; // info dla enkodera
				 
				 
	}
		
}

void driveReverseRightTrack( int predkosc){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTC->PSOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = V_MOD*predkosc/100 ;
	
         }
	}
	
}

void driveReverseDistRightTrack( int predkosc , int droga){
	
	ACTUAL_DISTANCE = 0;
	
	if( predkosc >=0){
		
	PTC->PSOR = RIGHT;
	
	    if(predkosc >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else {
		
	       TPM0->CONTROLS[2].CnV = predkosc/100 * V_MOD;
	
         }
				
		FLAG_LIMIT_DIST = 1;		
		LIMIT = droga; // info dla enkodera
				 
				 
	}
		
}


void driveForward(int predkosc){
	
	driveForwardLeftTrack(predkosc);
	driveForwardRightTrack(predkosc);

}

void driveForwardDist( int predkosc , int droga){
	
	
	driveForwardDistLeftTrack( predkosc , droga);
	driveForwardDistRightTrack( predkosc , droga);
	
}


void driveReverse(int predkosc){
	
	driveReverseLeftTrack(predkosc);
	driveReverseRightTrack(predkosc);
	
	
	
}

void driveReverseDist( int predkosc , int droga){
	
	driveReverseDistLeftTrack( predkosc , droga);
	driveReverseDistRightTrack( predkosc , droga);
	
	
}

void acceleration( int oile){
	 
	// teraz jeszcze trzeba zrobic powolne przyspieszenie
	
	//LEFT ENGINE
	tmp = TPM0->CONTROLS[4].CnV * 100 / V_MOD  ;
	
	      if(tmp + oile >= 100){
		
		     TPM0->CONTROLS[4].CnV = V_MOD + 1;
		
	      }
	      else if(tmp + oile <= 0 ){
					
					TPM0->CONTROLS[4].CnV = 0;
				}
				else{
		
	       TPM0->CONTROLS[4].CnV = tmp + oile;
	
         }
				
	//RIGHT ENGINE
	tmp = TPM0->CONTROLS[2].CnV / V_MOD * 100.0 ;
	
	      if(tmp + oile >= 100){
		
		     TPM0->CONTROLS[2].CnV = V_MOD + 1;
		
	      }
	      else if(tmp + oile <= 0 ){
					
					TPM0->CONTROLS[2].CnV = 0;
				}
				else{
		
	       TPM0->CONTROLS[2].CnV = tmp + oile;
	
         }
	
}

/////////////////////////////dorzucic sprawdzanie kompasem //////////////////////////////
void turnLeft( int kat, int predkosc ){
	
	//tmp = kat/10 * 
	//droga = kat/10 * 2 ;
	
	driveReverseDistLeftTrack(predkosc , kat/10 * 2 );
	driveForwardDistRightTrack(predkosc , kat/10 * 2 ); // tu wystarczy wywolac funkcje bez zdefinowanej drogi
	                                                    // poniewaz definicja LIMIT nastepuje w funkcji wczesniej
	
}

void turnRight( int kat , int predkosc){
	
	driveReverseDistRightTrack(predkosc , kat/10 * 2 );
	driveForwardDistLeftTrack(predkosc , kat/10 * 2 );// tu wystarczy wywolac funkcje bez zdefinowanej drogi
	                                                    // poniewaz definicja LIMIT nastepuje w funkcji wczesniej
	
	
}
