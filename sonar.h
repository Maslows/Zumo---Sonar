#ifndef SONAR_H
#define SONAR_H

#include "MKL46Z4.h"                    // Device header


/**
 @brief Define sonar IRQ priority
*/
#define SONAR_INTERUPT_PRIORITY 2

/**
 @brief Define sonar measurment interval.
 Triggers are send every ::SONAR_MEAS_INTERVAL_MS by TPM1. Setting lower value will increese
 measurement speed but it will also make sonar more prone to residiual echo from previous measurements.
 @warning Setting this to less than 35ms may produce sensor unsuable due to overlaping echos.
 @note This setting is not used when Servo is in ::SWEEP mode. In ::SWEEP mode measurements are done 
			 without any delay.
*/
#define SONAR_MEAS_INTERVAL_MS 35

/**
 @brief Defines maximum wanted range of a sonar
 All measurments which return value greather than this, will be changed into 0.
 It is useful when we want to limit background noise picked up by a sonar.
*/
#define SONAR_MAX_RANGE_CM 200


/**
	@brief Define number of measurment to average. When set to 1, measurments are not avergaged.
	In #SWEEP mode it is a good idea to slightly bigger number (3-10). Next measurement is done
	as soon as the previous one is done so the increese in delay is not big.
*/
#define SONAR_AVG_NUMBER 3

/**
	@brief Define maximum number of measurment retries.
	In #SINGLE mode when sonar fails to obtain usable data after #SONAR_MAXTRY it will return 0.
	In #CONTINOUS it will return 0 and retry again.
	In #SWEEP mode it will proceed with the next step
	@note This is a number of failed measurements IN ROW after which sonar gives up. 
				For example is set to 3 and 2 measurements fails and then next one is successful, 
				counter will be set back to 0.
*/
#define SONAR_MAXTRY 3


/**
 @brief Define Sonar work modes

	Sonar can operate in two modes. #CONTINUOUS  and #SINGLE. 
	In #CONTINUOUS mode Sonar triggers are generated automatically.
	In #SINGLE mode, measurment is done using ::SonarStartMeas and ::SonarGetDistance functions
	@note Setting servo to #SWEEP will automatically set Sonar mode to #CONTINUOUS
*/	
typedef enum { CONTINUOUS, /**< Repeat measurment every ::SONAR_MEAS_INTERVAL_MS */
							 SINGLE /**< Perform single measurment */
} SonarMode_t;


/* Global variables */
extern SonarMode_t SonarMode; 

/* Functions usable by user */
void Sonar_init(SonarMode_t InitialWorkMode);
void SonarDistHandler(uint16_t distance, int32_t angle); 
void SonarChangeMode(SonarMode_t NewMode);
void SonarStartMeas(int32_t angle); 
uint16_t SonarGetDistance(int32_t angle);

/* Functions NOT usable by user. API for Servo. */
void EnableSonar(void);
void DisableSonar(void);


#endif 
