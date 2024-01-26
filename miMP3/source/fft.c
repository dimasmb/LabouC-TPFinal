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
		for (int j=1; j<bins_per_output_bin; j++){
			total = total + testOutput[c*bins_per_output_bin+j];
		}

		out_buffer[c] = (total<65535.0)?(uint16_t)total:65535;
	}
	return;
}

/*
 	float32_t total = 0.0;
	for (int j=1; j<2; j++){
		total = total + testOutput[j];
	}
	out_buffer[0] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=2; j<5; j++){
		total = total + testOutput[j];
	}
	out_buffer[1] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=6; j<16; j++){
		total = total + testOutput[j];
	}
	out_buffer[2] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=17; j<44; j++){
		total = total + testOutput[j];
	}
	out_buffer[3] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=45; j<303; j++){
		total = total + testOutput[j];
	}
	out_buffer[4] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=117; j<788; j++){
		total = total + testOutput[j];
	}
	out_buffer[5] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=304; j<16; j++){
		total = total + testOutput[j];
	}
	out_buffer[6] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;

	for (int j=789; j<2045; j++){
		total = total + testOutput[j];
	}
	out_buffer[7] = (total<65535.0)?(uint16_t)total:65535;
	total = 0.0;
  */

