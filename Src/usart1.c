#include "usart1.h"

void uart1_setup() {
	//Set all configs to Reset Value
	USART1->CR1 = 0x00;
	USART1->CR2 = 0x00;
	USART1->CR3 = 0x00;
	USART1->BRR = 0x00;
	USART1->GTPR = 0x00;
	USART1->RQR = 0x00;
	USART1->ISR = 0x020000C0;

	USART1->CR1 &= ~(USART_CR1_M); //Set word length 8

	USART1->CR2 |= USART_CR2_RTOEN; //enable receiver timeout
	USART1->RTOR = 0x01; //receiver times out after one full bit without next start bit

	USART1->BRR = 0x683; //Set baudrate to 9600 (Fck = 16MHz)
}

void uart1_enable() {
	USART1->CR1 |= USART_CR1_UE; //USART1 Enable
}
void uart1_disable() {
	USART1->CR1 &= ~USART_CR1_UE; //USART1 Disable
}
void uart1_enable_tx() {
	USART1->CR1 |= USART_CR1_TE; // USART Transmitter enabled
}
void uart1_disable_tx() {
	USART1->CR1 &= ~USART_CR1_TE; //USART Transmitter disabled
}
void uart1_enable_rx() {
	USART1->CR1 |= USART_CR1_RE; //USART Receiver enabled
}
void uart1_disable_rx() {
	USART1->CR1 &= ~USART_CR1_RE; //USART Receiver disabled
}

void uart1_sendByte(uint8_t data) {
	uart1_sendArray(&data, 1U);
}
void uart1_sendArray(uint8_t data[], uint32_t length) {
	USART1->ICR &= USART_ICR_TCCF; //clear transfer complete flag
	uint32_t pointer = 0; // what array element to send
	while (1) {
		if (data[pointer] != '\0' && pointer < length) { //check if not string termination character or end of array
			while (!(USART1->ISR & USART_ISR_TXE)); //wait while data is sent out to shift register
			USART1->TDR = data[pointer];
			pointer++;
		} else
			break;
	}
	while (!(USART1->ISR & USART_ISR_TC)); //Wait while Transfer Complete flag is set
	USART1->ICR &= USART_ICR_TCCF; //clear transfer complete flag
}

void uart1_receiveByte(uint8_t *data) {

	uart1_receiveArray(data, 1U);

}
void uart1_receiveArray(uint8_t *arraypointer, uint32_t length) {
	uint8_t *currentpointer = arraypointer;
	for (uint32_t k = 0; k < length; k++) { //fill the buffer with nulls so that it doesn't retain data from previous transmissions
		*(arraypointer + k) = '\0';
	}
	while (!(USART1->ISR & USART_ISR_RXNE)); //wait indefinitely for the beginning of the transmission
	USART1->ICR |= USART_ICR_RTOCF; //make sure receiver timeout flag is cleared
	while (!(USART1->ISR & USART_ISR_RTOF)) { //while not receiver timeout
		if (((USART1->ISR & USART_ISR_RXNE)) && (currentpointer < arraypointer + length)) { //if buffer is not full yet and there is new data
			*currentpointer = USART1->RDR;
			currentpointer++;
			USART1->RQR |= USART_RQR_RXFRQ; //clear RXNE data received flag
		}
	}
	USART1->RQR |= USART_RQR_RXFRQ; //clear RXNE data received flag for buffer overflow data
	USART1->ICR |= USART_ICR_ORECF; //clear receiver overrun flag for buffer array overflow
	USART1->ICR |= USART_ICR_RTOCF; //when receiver timed out and we're done, clear the flag
}

