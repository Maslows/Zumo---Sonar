#ifndef SERVO_H
#define SERVO_H

#include "MKL46Z4.h"                    // Device header
#define SERVO_MOVEMENT_MIN 900*3    /* Servo minimum PWM walue in timers ticks (us*3) */
#define SERVO_MOVEMENT_MAX 2100*3    /* Servo maximum PWM value in timer ticks (us*3) */
#define SERVO_SWEEP_STEPS_NBR 5     /* Must be odd number! */
#define SERVO_SWEEP_STEP ((SERVO_MOVEMENT_MAX-SERVO_MOVEMENT_MIN)/SERVO_SWEEP_STEPS_NBR)
void Servo_init(void);
void Servo_step(void);

#endif
