#include "MKL46Z4.h"                    // Device header
#include "sonar.h"
#include "servo.h"
#include "bluetooth.h"
#include "motordriver.h"

#include "string.h"
#include "stdlib.h"
#include "stdio.h"

char tab[BUFF_SIZE];

int main(void) {
	
	
	bt_init(BAUD_RATE);
	Sonar_init(CONTINUOUS);
	Servo_init(MANUAL);
	motorDriverInit();
	

	
	while(1){ 
		bt_getStr( tab );								// Get string from buffer
		if(strlen( tab )){							// If isn't empty...
			//bt_sendStr( tab );						// ...send it back.
			if ( strcmp(tab, "w") == 0 ) {
				driveForward(speed);
			}
			else if ( strcmp(tab, "s") == 0) {
				driveReverse(speed);
			}
			else if ( strcmp(tab, " ") == 0) {
				stop();
			} 
			else if ( strcmp(tab, "a") == 0) {
				driveReverseLeftTrack(40); 
				driveForwardRightTrack(40);
			}
			else if ( strcmp(tab, "d") == 0) {
				driveForwardLeftTrack(40); 
				driveReverseRightTrack(40);
			}
			else if ( strcmp(tab, "ServoOn") == 0) {
				ServoChangeMode(SWEEP);
			}
			else if ( strcmp(tab, "ServoCenter") == 0) {
				ServoChangeMode(MANUAL);
				Servo_move_by_degree(0);
			}
			else if ( strcmp(tab, "SonarStartMeas") == 0) {
				SonarStartMeas(0);
			}
			else if ( strcmp(tab, "SonarGetDistance") == 0) {
				char buffor[12];
				sprintf(buffor, "%04d,%04hu\n",30,SonarGetDistance(30));
				bt_sendStr(buffor);
			}
			else if ( strcmp(tab, "e") == 0) {
				if (speed<=90){
					speed+=10;
				}
			}
			else if ( strcmp(tab, "q") == 0) {
				if (speed>=10){
					speed-=10;
				}
			}
			else if (strncmp(tab, "speed",5) == 0){
				int new_speed = 0;
				sscanf(tab,"speed%du",&new_speed);
				if (new_speed >= 0 && new_speed <= 100){
					speed = new_speed;
				}
			}
			else if (strncmp(tab, "servopos",8) == 0){
				int new_deg = 0;
				sscanf(tab,"servopos%d",&new_deg);
				if (new_deg >= -82 && new_deg <= 82){
					ServoMode = MANUAL;
					Servo_move_by_degree(new_deg);
				}
			}
			else if ( strcmp(tab, "ping") == 0) {
				bt_sendStr("pong");
			}
		}
	} 
};

	
