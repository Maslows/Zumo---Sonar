#include "MKL46Z4.h"

// initialize motor driver
void motorDriverInit(void);
void TPM0_IRQnHandler(void);

void driveForwardLeftTrack( int predkosc);
void driveForwardDistLeftTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveForwardRightTrack( int predkosc);
void driveForwardDistRightTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveReverseLeftTrack( int predkosc);
void driveReverseDistLeftTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]
void driveReverseRightTrack( int predkosc);
void driveReverseDistRightTrack( int predkosc , int droga); // predkosc intiger 0 - 100
																									         // droga intiger [ cm ]

void driveForward(int predkosc); // predkosc intiger 0 - 100 , nonlimit way
void driveForwardDist( int predkosc , int droga); // predkosc intiger 0 - 100
																									// droga intiger [ cm ] 
void driveReverse(int predkosc ); //  predkosc intiger 0 - 100 , nonlimit way
void driveReverseDist( int predkosc , int droga); // predkosc intiger 0 - 100
																									// droga intiger [ cm ] 
void stop(void);
void acceleration( int oile); // oile is intiger 0 - 100 

void turnLeft( int kat , int predkosc); // kat jest w stopniach , podanie kat > 360 , zostanie wykonany obrót 
void turnRight( int kat , int predkosc);// kat jest w stopniach , podanie kat > 360 , zostanie wykonany obrót

extern uint8_t speed;

