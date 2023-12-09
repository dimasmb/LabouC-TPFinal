/*
 * ping_pong_buffer.h
 *
 *  Created on: 31 oct. 2023
 *      Author: joaco
 */

#ifndef DRIVERS_PING_PONG_BUFFER_H_
#define DRIVERS_PING_PONG_BUFFER_H_

#define BUFLEN 2304
//#define BUFLEN 1152

typedef struct {
	uint16_t* current_buffer;
	uint16_t* next_buffer;
	uint16_t buffer1[BUFLEN];
	uint16_t buffer2[BUFLEN];
	int next_buffer_ready; //next buffer ready nos indica si ya
								//hay un prox bufferlisto para salir por el DAC
	int wait; //wait se pone en 1 si se termino de sacar por DAC el buffer actual
				 // y el proximo buffer todavía no está cargado. Se vuelve a poner en 0 si
				 // se termina de cargar un buffer nuevo
	int load_counter; //lleva la cuenta de cual es el proximo indice que se cargará en el dac
} ping_pong_buffer_t;

#endif /* DRIVERS_PING_PONG_BUFFER_H_ */
