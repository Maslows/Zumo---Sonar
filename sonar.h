#ifndef SONAR_H
#define SONAR_H

#include "MKL46Z4.h"                    // Device header

#define SONAR_MAX_RANGE_CM 300
#define SONAR_MEAS_INTERVAL_MS 50
#define SONAR_AVG_MODE   0 // 0 - off, 1 – on
#define SONAR_AVG_NUMBER 5
#define SONAR_INTERUPT_PRIORITY 3


/********************************************//**
 *  Brief Define Sonar FSM states
 *  SONAR_IDLE            - Idle
 *  SONAR_TRIGGER_SENT    - A trigger was sent. Waiting for echo.
 *  SONAR_CAPTURE_START   - Reciving echo and measuring its lenght
 *  SONAR_CAPTUE_END      - Capture complete
 *  SONAR_CAPUTE_OVERFLOW - Echo length exceeded maximum value defined by SONAR_MAX_RANGE_CM
 ***********************************************/
typedef enum { SONAR_IDLE, 
							 SONAR_TRIGGER_SENT,
							 SONAR_CAPTURE_START, 
							 SONAR_CAPTURE_END,
							 SONAR_CAPTURE_OVERFLOW
} SonarFSM;
extern SonarFSM SonarState;

/********************************************//**
 *  Brief Define Sonar work mode
 *  CONTINUOUS - Repeat measurment every SONAR_MEAS_INTERVAL_MS
 *  SINGLE     - Perform single measurment and go idle
 ***********************************************/		
typedef enum { CONTINUOUS, 
							 SINGLE
} SonarWorkModes;
extern SonarWorkModes SonarMode; 


/********************************************//**
 *  Brief Initialize Sonar and required peripherials
 ***********************************************/
void Sonar_init(SonarWorkModes InitialWorkMode);
void SonarDistHandler(uint16_t distance); 

#endif 
