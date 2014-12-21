#include "MKL46Z4.h"                    // Device header
#include "sonar.h"
#include "servo.h"
#include "sLCD.h"


int main(void) {
	sLCD_init();
	Sonar_init(CONTINUOUS);
	Servo_init(SWEEP);

	
	while(1){
	};
}
	
