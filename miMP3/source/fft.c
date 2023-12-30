/*
 * fft.c
 *
 *  Created on: 30 dic. 2023
 *      Author: joaco
 */


#include "arm_math.h"
#include "arm_const_structs.h"


/* ------------------------------------------------------------------
* Global variables for FFT Bin Example
* ------------------------------------------------------------------- */

float32_t testOutput[2048];

void get_fft(float* testInput_f32_10khz, uint16_t* out_buffer, int outlen){

	uint32_t ifftFlag = 0;
	uint32_t doBitReverse = 1;

	/* Process the data through the CFFT/CIFFT module */
	arm_cfft_f32(&arm_cfft_sR_f32_len2048, testInput_f32_10khz, ifftFlag, doBitReverse);
	/* Process the data through the Complex Magnitude Module for
	calculating the magnitude at each bin */
	arm_cmplx_mag_f32(testInput_f32_10khz, testOutput, 2048);

	int bins_per_output_bin = 2048/outlen;
	int c;
	for (c = 0; c < outlen; c++){

		float32_t total = 0.0;
		for (int j=0; j<bins_per_output_bin; j++){
			total = total + testOutput[c*bins_per_output_bin+j];
		}

		out_buffer[c] = (total<65535.0)?(uint16_t)total:65535;
	}
	return;
}

