#ifndef UART_H
#define UART_H

#include "MKL46Z4.h"                    // Device header

#define Q_SIZE 64
#define BUS_CLOCK 48000000u

void UART0_init(uint32_t baud_rate);
void UART0_Transmit_Poll(uint8_t data);
uint8_t UART0_Receive_Poll(void);

typedef struct {
	unsigned char Data[Q_SIZE];
	unsigned int Head; // points to oldest data element
	unsigned int Tail; // points to next free space
	unsigned int Size; // quantity of elements in queue
} Q_T;

extern Q_T TxQ, RxQ;

int Q_Enqueue(Q_T * q, unsigned char d);
unsigned char Q_Dequeue(Q_T * q);
int Q_Empty(Q_T * q);
int Q_Full(Q_T * q);
void UART_WriteString(char * input);
void UART_ReadString(char * output, uint32_t output_length);

#endif
