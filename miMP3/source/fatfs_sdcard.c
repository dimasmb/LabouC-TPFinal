/*
 * The Clear BSD License
 * Copyright (c) 2015, Freescale Semiconductor, Inc.
 * Copyright 2016-2017 NXP
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted (subject to the limitations in the disclaimer below) provided
 *  that the following conditions are met:
 *
 * o Redistributions of source code must retain the above copyright notice, this list
 *   of conditions and the following disclaimer.
 *
 * o Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * o Neither the name of the copyright holder nor the names of its
 *   contributors may be used to endorse or promote products derived from this
 *   software without specific prior written permission.
 *
 * NO EXPRESS OR IMPLIED LICENSES TO ANY PARTY'S PATENT RIGHTS ARE GRANTED BY THIS LICENSE.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdio.h>
#include <string.h>
#include "fsl_sd.h"
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "fsl_sd_disk.h"
#include "board.h"
#include "fsl_gpio.h"
//funciones joaco
#include "dac_out.h"
#include "gpio.h"

#include "../helix/pub/mp3dec.h"

#include "fsl_sysmpu.h"
#include "pin_mux.h"
#include "clock_config.h"
/*******************************************************************************
 * Definitions
 ******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BUFFER_SIZE (100U)
/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
* @brief wait card insert function.
*/
static status_t sdcardWaitCardInsert(void);
void play_file(char *mp3_fname);



/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject;   /* File object */

//FOr mp3 decoder
#define FILE_READ_BUFFER_SIZE   (1024*16)
MP3FrameInfo                 mp3FrameInfo;
HMP3Decoder         hMP3Decoder;
uint8_t read_buff[FILE_READ_BUFFER_SIZE];
uint32_t bytes_read;
int    bytes_left;
char    *read_ptr;
int16_t pcm_buff[2304];
int16_t audio_buff[2304*4] = {0};
volatile uint32_t delay1 = 1000;
volatile uint32_t core_clock ;

uint8_t mp3_files[1000][15];    //to save file names
int mp3_file_index;
int mp3_total_files;

volatile uint8_t next, prev, replay, mute,ffd,reset, play, volume = 5;

/* @brief decription about the read/write buffer
* The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
* block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
* can define the block length by yourself if the card supports partial access.
* The address of the read/write buffer should align to the specific DMA data buffer address align value if
* DMA transfer is used, otherwise the buffer address is not important.
* At the same time buffer address/size should be aligned to the cache line size if cache is supported.
*/
SDK_ALIGN(uint8_t g_bufferWrite[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
/*! @brief SDMMC host detect card configuration */
static const sdmmchost_detect_card_t s_sdCardDetect = {
#ifndef BOARD_SD_DETECT_TYPE
    .cdType = kSDMMCHOST_DetectCardByGpioCD,
#else
    .cdType = BOARD_SD_DETECT_TYPE,
#endif
    .cdTimeOut_ms = (~0U),
};

/*! @brief SDMMC card power control configuration */
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
static const sdmmchost_pwr_card_t s_sdCardPwrCtrl = {
    .powerOn = BOARD_PowerOnSDCARD, .powerOnDelay_ms = 500U, .powerOff = BOARD_PowerOffSDCARD, .powerOffDelay_ms = 0U,
};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void)
{
    FRESULT error;
    DIR directory; /* Directory object */
    FILINFO fileInformation;
    UINT bytesWritten;
    UINT bytesRead;
    const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};
    volatile bool failedFlag = false;
    char ch = '0';
    BYTE work[FF_MAX_SS];

    BOARD_InitPins();
    BOARD_BootClockRUN();
    BOARD_InitDebugConsole();
    SYSMPU_Enable(SYSMPU, false);
    dac_out_init();

    PRINTF("\r\nFATFS example to demonstrate how to use FATFS with SD card.\r\n");

    PRINTF("\r\nPlease insert a card into board.\r\n");

    if (sdcardWaitCardInsert() != kStatus_Success)
    {
        return -1;
    }

    if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
    {
        PRINTF("Mount volume failed.\r\n");
        return -1;
    }

#if (FF_FS_RPATH >= 2U)
    error = f_chdrive((char const *)&driverNumberBuffer[0U]);
    if (error)
    {
        PRINTF("Change drive failed.\r\n");
        return -1;
    }
#endif


    PRINTF("\r\nList the file in that directory......\r\n");
    if (f_opendir(&directory, "/Musica"))
    {
        PRINTF("Open directory failed.\r\n");
        return -1;
    }

    /**************Lista de las canciones que hay*****/
    volatile FILINFO files;
    FRESULT res;
  
    while(1) {
    res =  f_readdir(&directory, &files);
    if( res != FR_OK || strlen(files.fname) == 0) {
      break;////f_opendir(&directory, "/");
    }
    
    if(strstr(files.fname, ".MP3")) {
      strcpy( mp3_files[mp3_file_index], files.fname );    //to save file names
      mp3_file_index++;
      mp3_total_files++;
      PRINTF("%s\r\n", files.fname);
      
    }
  }
    /****************************/
    mp3_file_index = 0;
    gpioMode(PORTNUM2PIN(PC,16), OUTPUT);
    play_file(mp3_files[mp3_file_index]);
}

static status_t sdcardWaitCardInsert(void)
{
    /* Save host information. */
    g_sd.host.base = SD_HOST_BASEADDR;
    g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
    /* card detect type */
    g_sd.usrParam.cd = &s_sdCardDetect;
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif
    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success)
    {
        PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }
    /* power off card */
    SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
    /* wait card insert */
    if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success)
    {
        PRINTF("\r\nCard inserted.\r\n");
        /* power on the card */
        SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
    }
    else
    {
        PRINTF("\r\nCard detect fail.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

void play_file(char *mp3_fname) {

  if(strlen(mp3_fname) == 0) 
    while(1);


  FIL fil;        /* File object */
  FRESULT fr;     /* FatFs return code */
  uint32_t time = 0;
  uint32_t seconds = 0, prev_seconds = 0, minutes = 0;
  
  /* Open a text file */
  fr = f_open(&fil, mp3_fname, FA_READ);
  
  
  if(fr) {
    while(1);
  }
  // Read ID3v2 Tag
  
  hMP3Decoder = MP3InitDecoder();

  bytes_left = 0;

   int offset, err;
   int outOfData = 0;

   unsigned int br, btr;

   int16_t *samples = pcm_buff;

   while(1) {
	  gpioWrite(PORTNUM2PIN(PC,16), HIGH);
	  for(int t=0; t<2*4;t++){
		  if( bytes_left < FILE_READ_BUFFER_SIZE/2 ) {      //Se crea un ping pong buffer
		          memcpy( read_buff, read_ptr, bytes_left );
		          read_ptr = read_buff;
		          btr = FILE_READ_BUFFER_SIZE - bytes_left;
		          fr = f_read(&fil, read_buff + bytes_left, btr, &br);
		          bytes_left = FILE_READ_BUFFER_SIZE;
		          if(fr || br < btr) {
		            f_close(&fil);
		            return;
		          }
		        }

		        offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
		        if(offset == -1 ) {
		          bytes_left = 0;
		        }
		        bytes_left -= offset;
		        read_ptr += offset;
		        err = MP3Decode(hMP3Decoder, (unsigned char**)&read_ptr, (int*)&bytes_left, samples, 0);
		        if (err) {
		          /* error occurred */
		          switch (err) {
		          case ERR_MP3_INDATA_UNDERFLOW:
		            outOfData = 1;
		            break;
		          case ERR_MP3_MAINDATA_UNDERFLOW:
		            /* do nothing - next call to decode will provide more mainData */
		            break;
		          case ERR_MP3_NULL_POINTER:
		            bytes_left -=1;
		            read_ptr+=1;
		          case ERR_MP3_FREE_BITRATE_SYNC:
		          default:
		            outOfData = 1;
		            break;
		          }
		        }
		        else {
		          MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
		          if (mp3FrameInfo.nChans == 2) {
		          	int32_t tmp = 0;
		          	static float aux_1 = 0;
		          	for(int i = 0; i < mp3FrameInfo.outputSamps; i+=2) {
		          		uint16_t aux = (uint16_t)(((samples[i]+samples[i+1])>>5)+2047);
		          			int index = t*1152+i/2;
		            	    audio_buff[index] = aux;
		  		   }
		          }
		        }

	  }
	  gpioWrite(PORTNUM2PIN(PC,16), LOW);

      if (!outOfData) {
    	  int status_buf=0;
    	  do{
    		  status_buf=fill_buffer(audio_buff);
    	  }while(status_buf==0);



      }
    }
  }
