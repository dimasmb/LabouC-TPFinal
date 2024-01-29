/*
 * dac_out.h
 *
 *  Created on: 18 oct. 2023
 *      Author: joaco
 */


#ifndef DAC_OUT_H_
#define DAC_OUT_H_

#include "DAC.h"
//#include "SysTick.h"

void dac_out_init();
void dac_out_pause();
void dac_out_unpause();

int fill_dma_buffer(uint16_t* original_buf);
void normalizeDoubleVector(uint16_t* output, double* input, int length);


#endif /* DAC_OUT_H_ */
