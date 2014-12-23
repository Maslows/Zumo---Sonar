#include "uart.h"


Q_T TxQ, RxQ;

/* Initialize a queue */
void Q_Init(Q_T * q){
	uint32_t i;
	for(i=0; i<Q_SIZE; i++)
		q->Data[i] = 0; // for debugging
	q->Head = 0;
	q->Tail = 0;
	q->Size = 0;
}

int Q_Empty(Q_T * q) {
	return q->Size ==0;
}

int Q_Full(Q_T * q) {
	return q->Size == Q_SIZE;
}

int Q_Enqueue(Q_T * q, unsigned char d) {
	// What if queue is full?
	if (!Q_Full(q)) {
		q->Data[q->Tail++] = d;
		q->Tail %= Q_SIZE;
		q->Size++;
		UART0->C2 |= UART_C2_TIE_MASK;
		return 1; // success
	} else
		return 0; // failure
}

unsigned char Q_Dequeue(Q_T * q){
	// Check if not empty
	unsigned char t = 0;
	if (!Q_Empty(q)) {
		t = q->Data[q->Head];
		q->Data[q->Head++] = 0; // for debugging
		q->Head %= Q_SIZE;
		q->Size--;
	}
	return t;
}

void UART_WriteString(char * input){
	uint32_t i=0;
	while (input[i] != '\0'){
		Q_Enqueue(&TxQ,input[i++]);
	}
}

void UART_ReadString(char * output, uint32_t output_length){
	uint32_t i=0;
	while (i<output_length){
		output[i++] = Q_Dequeue(&RxQ);
	}
	output[i]='\0';
}


void UART0_init(uint32_t baud_rate) {
	uint32_t divisor;
	//enable clock to UART and PORT A
	SIM->SCGC4 |= SIM_SCGC4_UART0_MASK;
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	SIM->SOPT2 |= SIM_SOPT2_UART0SRC(1);
	SIM->SOPT2 |= SIM_SOPT2_PLLFLLSEL_MASK;
	
	// connect UART to pins for PTA1 PTA2
	PORTA->PCR[1] = PORT_PCR_MUX(2);
	PORTA->PCR[2] = PORT_PCR_MUX(2);
	
	// ensure tx and rx are disabled before configuration
	UART0->C2 &= ~(UART_C2_TE_MASK | UART_C2_RE_MASK);
	
	//Set baud rate
	divisor = BUS_CLOCK/(baud_rate*16);
	UART0->BDH = UART_BDH_SBR(divisor>>8);
	UART0->BDL = UART_BDL_SBR(divisor);
	
	// 8 bits, 1 stop bits , no parity
	UART0->C1 = 0;
	UART0->S2 = 0;
	UART0->C3 = 0;
	
	Q_Init(&TxQ);
	Q_Init(&RxQ);
	
	NVIC_SetPriority(UART0_IRQn, 3);
	NVIC_ClearPendingIRQ(UART0_IRQn);
	NVIC_EnableIRQ(UART0_IRQn);
	
	UART0->C2 |= UART_C2_RIE_MASK;
	
	/* Enable transmiter and reciver */
	UART0->C2 = UART_C2_TE_MASK | UART_C2_RE_MASK;
}

void UART0_IRQHandler(void) {
	/* Tx */
	if (UART0->S1 & UART_S1_TDRE_MASK) {
			//can send another character
			if (!Q_Empty(&TxQ)) {
				//osMutexWait(TxMutex_id,osWaitForever);
				UART0->D = Q_Dequeue(&TxQ);
				//osMutexRelease(TxMutex_id);
				UART0->C2 |= UART_C2_TIE_MASK;			
			} else {
				// queue is empty so disable tx
				UART0->C2 &= ~UART_C2_TIE_MASK;
			}
	}

	/* Rx */
	if (UART0->S1 & UART_S1_RDRF_MASK) {
		if (!Q_Full(&RxQ)) {
			Q_Enqueue(&RxQ, UART0->D);
		} else {
			// error - queue full
			while(1);
		}		
	}
	NVIC_ClearPendingIRQ(UART0_IRQn);
}


void UART0_Transmit_Poll(uint8_t data) {
	// wait until transmit data register is empty
	while (!(UART0->S1 & UART_S1_TDRE_MASK));
	UART0->D = data;
}

uint8_t UART0_Receive_Poll(void){
	// wait until recive data register is full
	while (!(UART0->S1 & UART_S1_RDRF_MASK));
	return UART0->D;
}

