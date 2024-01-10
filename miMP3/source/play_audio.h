/*
 * play_audio.h
 *
 *  Created on: 9 ene. 2024
 *      Author: joaco
 */

#ifndef PLAY_AUDIO_H_
#define PLAY_AUDIO_H_

#include "equalizer.h"
#include "stdint.h"

int play_file(char *mp3_fname, char first_call, int volumen, int equalizer);

void play_file_output_init();
uint16_t* get_fft_array(void);

#endif /* PLAY_AUDIO_H_ */
