#ifndef SERVO_H
#define SERVO_H

#include "MKL46Z4.h"                    							// Device header

/** 
	@brief Define minimum PWM lenght which servo can tolerate
	This macro defines the minimal PWM which can be used, 
	i.e. how far to the LEFT this servo can move
	@warning Setting this value below 800 may damage the servo. 
					 If you hear noisy clicking, power off the system!
*/					 
#define SERVO_MOVEMENT_MIN 2200*SERVO_TICKS_PER_US

/** 
	@brief Define maximum PWM lenght which servo can tolerate
	This macro defines the maximal PWM which can be used, 
	i.e. how far to the RIGHT this servo can move
	@warning Setting this value above 2200 may damage the servo. 
					 If you hear noisy clicking, power off the system!
*/	
#define SERVO_MOVEMENT_MAX 800*SERVO_TICKS_PER_US    

/**
 @brief Define how many degrees this servo can turn
 This marco defines how far to the left and to the right can servo operate
 It is used to map degrees into PWM width.
 This value may differ from servo to servo. It is recomended to measure it yourself.
*/
#define SERVO_MOVEMENT_RANGE 82 

/**
 @brief Define sweep step in degrees
 This marco defines a sweep step. Every step servo will rotate by this value.
*/
#define SERVO_STEP_DEG 30      									/* Sweep step in degrees */

/** 
		@brief Define Servo angular velocity in deg/s
		Due to lack of feedback from servo, we have to estimate its travel time 
		240deg/s is our observed velocity with a sonar as a load. 
		You can try tweeking this value for better performance in your setup 
		@warning Do not set this value too high as it may damage the servo
*/																					 
#define SERVO_ANGULAR_VELOCITY 200										

/** 
		@brief Define Servo work modes
*/
typedef enum { SWEEP,  /**< Constant sweep with step ::SERVO_STEP_DEG */
							 MANUAL, /**< Manual operation. You can set servo position by ::ServoMoveByDegree */
} ServoMode_t;

/**
	@brief Define Servo Sweep modes 
*/
typedef enum { SCAN_AND_GO,   /**<In this mode Sonar will scan each direction and proceed with next one  */
							 SCAN_AND_LOCK, /**<In this mode Sonar will scan each direction and 
																	lock when It finds something under ::ServoLockRange 
																	When lock is lost it will procced with next direction */
} ServoSweep_t;

/** 
		@brief Define Servo states
		@warning Due to lack of feedback, this states are only predictions! Real state may be different!
*/
typedef enum { MOVING, /**<Servo is moving */
							 IDLE,   /**< Servo reached its final position and is idle */
							 LOCKED, /**< Servo is locked in ::SCAN_AND_LOCK sweep mode */
} ServoState_t;

/* Global variables */
extern ServoMode_t ServoMode;
extern ServoState_t ServoState;
extern int32_t ServoPosition; 


/* Functions */
void Servo_init(ServoMode_t InitialWorkMode, ServoSweep_t InitialSweepMode);
void ServoMoveByDegree(int32_t degree);
void ServoSweepStep(uint16_t distance);
void ServoChangeMode(ServoMode_t NewMode);
void ServoChangeSweepMode(ServoSweep_t NewSweep);
void ServoChangeLockRange(uint16_t NewRange);


#endif
