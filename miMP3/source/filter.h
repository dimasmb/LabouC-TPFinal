/*
 * filter.h
 *
 *  Created on: 26 oct. 2023
 *      Author: joaco
 */

#ifndef FILTER_H_
#define FILTER_H_

#include "stdint.h"

void convolve (double *p_coeffs, int p_coeffs_n,
			uint16_t *p_in, uint16_t *p_out, int n);

#endif /* FILTER_H_ */
