/*
 * play_audio.h
 *
 *  Created on: 9 ene. 2024
 *      Author: joaco
 */

#ifndef PLAY_AUDIO_H_
#define PLAY_AUDIO_H_

int play_file_output_init();

int play_file(char *mp3_fname, char first_call, int volumen);

#endif /* PLAY_AUDIO_H_ */
