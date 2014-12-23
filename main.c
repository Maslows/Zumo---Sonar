#include "MKL46Z4.h"                    // Device header
#include "sonar.h"
#include "servo.h"
#include "sLCD.h"
#include "uart.h"


int main(void) {
	sLCD_init();
	UART0_init(115200);
	Sonar_init(CONTINUOUS);
	Servo_init(MANUAL);

	
	while(1){ 
		
	};
}
	
