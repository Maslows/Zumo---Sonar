#ifndef SERVO_H
#define SERVO_H

#include "MKL46Z4.h"                    							// Device header

#define SERTO_TICKS_PER_US 3				 									/* us to timer tick conversion ratio */
#define SERVO_MOVEMENT_MIN 800*SERTO_TICKS_PER_US     /* Servo minimum PWM walue in timer ticks  */
#define SERVO_MOVEMENT_MAX 2200*SERTO_TICKS_PER_US    /* Servo maximum PWM value in timer ticks  */
#define SERVO_MOVEMENT_RANGE 82     									/* Movement range in degrees calculated from normal.  			 
																											   i.e. how far can servo move to the left or to the right   */
#define SERVO_SWEEP_STEP_DEG 60      									/* Sweep step in degrees */
#define SERVO_ANGULAR_VELOCITY 240										/* Due to lack of feedback from servo, we have to estimate its travel time 
																												 160deg/s is our observed velocity with a sonar as a load. 
																												 You can try tweeking this value for better performance in your setup */
																												 


typedef enum { SWEEP, 
							 MANUAL,
							 OFF
} ServoMode_t;
extern ServoMode_t ServoMode;

typedef enum { MOVING,
							 IDLE
} ServoState_t;
extern ServoState_t ServoState;


extern int32_t ServoPosition; /* Servo position in degrees from it's normal */



void Servo_init(ServoMode_t InitialWorkMode);
void Servo_move_by_degree(int32_t degree);
void Servo_sweep_step(void);

#endif
