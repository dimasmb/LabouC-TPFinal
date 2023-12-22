/*
 * fir.h
 *
 *  Created on: 22 dic. 2023
 *      Author: joaco
 */

#ifndef FIR_H_
#define FIR_H_

void firFloatInit( void );
void firFloat( float *coeffs, float *input, float *output,
       int length, int filterLength );
void intToFloat( int16_t *input, float *output, int length, char stereo);
void floatToInt( float *input, int16_t *output, int length );

//////////////////////////////////////////////////////////////
//  Test program
//////////////////////////////////////////////////////////////

// bandpass filter centred around 1000 Hz
// sampling rate = 8000 Hz

#define FILTER_LEN  63
const float coeffs[ FILTER_LEN ] =
{
   1.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0,   0.0,   0.0,
   0.0,  0.0,  0.0
};

#endif /* FIR_H_ */
