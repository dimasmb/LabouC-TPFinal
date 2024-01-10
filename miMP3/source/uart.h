/*
 * uart.h
 *
 *  Created on: 10 ene. 2024
 *      Author: joaco
 */

#ifndef UART_H_
#define UART_H_

#include "MK64F12.h"
#include <stdint.h>
#include "hardware.h"
#include <stdbool.h>

void UART_Send_Data(char *tx_data, int datalen);
void UART_Initialize (int baudrate, char parity); //0= no parity, 1=even parity, 2=even parity
bool inputEmpty();
char retreiveInput();

#endif /* UART_H_ */
