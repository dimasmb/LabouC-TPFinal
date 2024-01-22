/*
 * play_audio.c
 *
 *  Created on: 9 ene. 2024
 *      Author: joaco
 */

//INCLUDES

#include "../helix/pub/mp3dec.h"
#include "clock_config.h"
#include "dac_out.h"
#include "fsl_sysmpu.h"
#include "gpio.h"
#include "pin_mux.h"
#include "equalizer.h"
#include "fft.h"

#include "ff.h"

//ARREGLOS, BUFFERS, CONSTANTES, ETC


#define MP3FRAMES_PER_OUTBUF 4
#define OUTBUFLEN 1152*MP3FRAMES_PER_OUTBUF
#define FFT_BINS 8

// For mp3 decoder
#define FILE_READ_BUFFER_SIZE (1024 * 16)
MP3FrameInfo mp3FrameInfo;
HMP3Decoder hMP3Decoder = 0;
uint8_t read_buff[FILE_READ_BUFFER_SIZE];
uint32_t bytes_read;
int bytes_left;
char *read_ptr;
int16_t pcm_buff[2304];
int16_t audio_buff[OUTBUFLEN] = {0};
float audio_buff_float[OUTBUFLEN] = {0};
uint16_t fft_array[FFT_BINS] = {0.0};


const float volumes[32] = {
		1.0 ,
		0.8912509381337456 ,
		0.7943282347242815 ,
		0.7079457843841379 ,
		0.6309573444801932 ,
		0.5623413251903491 ,
		0.5011872336272722 ,
		0.44668359215096315 ,
		0.3981071705534972 ,
		0.35481338923357547 ,
		0.31622776601683794 ,
		0.28183829312644537 ,
		0.251188643150958 ,
		0.22387211385683395 ,
		0.19952623149688797 ,
		0.1778279410038923 ,
		0.15848931924611134 ,
		0.14125375446227545 ,
		0.12589254117941673 ,
		0.11220184543019636 ,
		0.1 ,
		0.08912509381337455 ,
		0.07943282347242814 ,
		0.0707945784384138 ,
		0.06309573444801933 ,
		0.05623413251903491 ,
		0.05011872336272722 ,
		0.0446683592150963 ,
		0.039810717055349734 ,
		0.03548133892335755 ,
		0.03162277660168379 ,
		0.028183829312644536 ,
		};

//FUNCIONES EXTERNAS
void play_file_output_init(){
	dac_out_init();
};


int play_file(char *mp3_fname, char first_call, int volumen, int equalizer) {

	static FIL fil;    /* File object */
	static FRESULT fr; /* FatFs return code */
	static uint32_t time, prev_seconds, minutes;
	static uint32_t seconds;
	static int offset, err;
	static int outOfData;
    static unsigned int br, btr;
    static int16_t *samples;

    static char last_segment_was_loaded = 1;


    if (first_call){
    	last_segment_was_loaded = true;
    }

    if(!last_segment_was_loaded){
    	last_segment_was_loaded = fill_dma_buffer(audio_buff);
    }
    else{

    	equalizer_init(equalizer);

    	if(first_call){
    			 // puede ser ROCK CLASSICAL URBAN o NONE
    			if (strlen(mp3_fname) == 0) {
    			    	while (1);
    			    }

    			    time = 0;
    			    seconds = 0, prev_seconds = 0, minutes = 0;

    			    /* Open a text file */
    			    fr = f_open(&fil, mp3_fname, FA_READ);

    			    if (fr) {
    			        while (1);
    			    }

    			    // Read ID3v2 Tag
    			    if(hMP3Decoder){
    			    	MP3FreeDecoder(hMP3Decoder);
    			    }
    			    hMP3Decoder = 0;
    			    hMP3Decoder = MP3InitDecoder();
    			    bytes_left = 0;
    			    outOfData = 0;
    				samples = pcm_buff;
    		}


    		for (int t = 0; t < MP3FRAMES_PER_OUTBUF; t++) {
    			if (bytes_left < FILE_READ_BUFFER_SIZE / 2) {  // Se crea un ping pong buffer
    				memcpy(read_buff, read_ptr, bytes_left);
    				read_ptr = read_buff;
    				btr = FILE_READ_BUFFER_SIZE - bytes_left;
    				fr = f_read(&fil, read_buff + bytes_left, btr, &br);
    				bytes_left = FILE_READ_BUFFER_SIZE;
    				if (fr || br < btr) {
    					f_close(&fil);
    					return;
    				}
    			}

    			offset = MP3FindSyncWord((unsigned char *)read_ptr, bytes_left);
    			if (offset == -1) {
    				bytes_left = 0;
    			}
    			bytes_left -= offset;
    			read_ptr += offset;
    			err = MP3Decode(hMP3Decoder, (unsigned char **)&read_ptr, (int *)&bytes_left, samples, 0);
    			if (err) {
    				/* error occurred */
    				switch (err) {
    					case ERR_MP3_INDATA_UNDERFLOW:
    						outOfData = 1;
    						break;
    					case ERR_MP3_NULL_POINTER:
    						bytes_left -= 1;
    						read_ptr += 1;
    					default:
    						break;
    				}
    			} else {
    				MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
    				if (mp3FrameInfo.nChans == 2) {
    					for (int i = 0; i < mp3FrameInfo.outputSamps; i += 2) {
    						uint16_t aux = (uint16_t)(((samples[i] + samples[i + 1]) >> 5)*volumes[volumen] + 2047);
    						int index = t * 1152 + i / 2;
    						audio_buff[index] = aux;
    					}
    				} else if (mp3FrameInfo.nChans == 1) {
    					for (int i = 0; i < mp3FrameInfo.outputSamps; i += 1) {
    						uint16_t aux = (uint16_t)(((samples[i] + samples[i + 1]) >> 5)*volumes[volumen] + 2047);
    						int index = t * 1152 + i;
    						audio_buff[index] = aux;
    					}
    				}
    			}
    		}

    		if (!outOfData) {
    			int status_buf = 0;

    			intToFloat(audio_buff, audio_buff_float, 2304*2);
    			equalize(audio_buff_float, audio_buff_float);
    			floatToInt(audio_buff_float, audio_buff, 2304*2);

    			last_segment_was_loaded = fill_dma_buffer(audio_buff);


    			get_fft(audio_buff_float, fft_array, FFT_BINS); //Es muy importante que esto este depsues de fill_dma_buffer :)



    		}
    }
	return outOfData;

}

uint16_t* get_fft_array(void) {
	return fft_array;
}
