/*
 * equalizer.h
 *
 *  Created on: 26 dic. 2023
 *      Author: joaco
 */

#ifndef EQUALIZER_H_
#define EQUALIZER_H_

enum preset_type {
  NONE,
  CLASSICAL,
  ROCK,
  URBAN
};

void equalizer_init(int preset);

void equalize(float* input, float* output);

void intToFloat( int16_t *input, float *output, int length);

void floatToInt( float *input, int16_t *output, int length);



#endif /* ECUALIZADOR_EQUALIZER_H_ */
