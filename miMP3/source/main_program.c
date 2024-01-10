#include "MK64F12.h"


#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "fsl_sysmpu.h"
#include "board.h"
#include "diskio.h"
#include "ff.h"
#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"

#include "play_audio.h"

int analize_directory(char* currdir, char*nextdir, int string_len);

/*******************************************************************************
 * Definitions
 ******************************************************************************/



/*******************************************************************************
 * Prototypes
 ******************************************************************************/
/*!
 * @brief wait card insert function.
 */
char init_sd_card();
static status_t sdcardWaitCardInsert(void);

/*******************************************************************************
 * Variables
 ******************************************************************************/
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject;   /* File object */

volatile uint32_t delay1 = 1000;
volatile uint32_t core_clock;

volatile uint8_t next, prev, replay, mute, ffd, reset, play, volume = 5;

/* @brief decription about the read/write buffer
 * The size of the read/write buffer should be a multiple of 512, since SDHC/SDXC card uses 512-byte fixed
 * block length and this driver example is enabled with a SDHC/SDXC card.If you are using a SDSC card, you
 * can define the block length by yourself if the card supports partial access.
 * The address of the read/write buffer should align to the specific DMA data buffer address align value if
 * DMA transfer is used, otherwise the buffer address is not important.
 * At the same time buffer address/size should be aligned to the cache line size if cache is supported.
 */

#define BUFFER_SIZE (100U)

SDK_ALIGN(uint8_t g_bufferWrite[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)],
          MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
SDK_ALIGN(uint8_t g_bufferRead[SDK_SIZEALIGN(BUFFER_SIZE, SDMMC_DATA_BUFFER_ALIGN_CACHE)], MAX(SDMMC_DATA_BUFFER_ALIGN_CACHE, SDMMCHOST_DMA_BUFFER_ADDR_ALIGN));
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
    .powerOn = BOARD_PowerOnSDCARD,
    .powerOnDelay_ms = 500U,
    .powerOff = BOARD_PowerOffSDCARD,
    .powerOffDelay_ms = 0U,
};
#endif
/*******************************************************************************
 * Code
 ******************************************************************************/

/*!
 * @brief Main function
 */
int main(void) {

	play_file_output_init();

	do{
	} while(init_sd_card()!=0);

	/***mandamos por UART todos los archivos****/
	char directory_string[1000];
	strcpy(directory_string, "");
	analize_directory(directory_string, "", 0);

    /*******AHORA ESA PUESTO ARBITRARIAMENTE ESTE ARCHIVO********/

	char selected_song [100];
	strcpy(selected_song, "/CARPETA1/CARPETA2/HIGHER~1.MP3");

	char end_of_song = false;
	char play_new_song = true;
	char pause = false;
	int eq_preset = NONE; //puede ser NONE, CLASSICAL, ROCK, URBAN
	int volumen = 1;



	int count = 0;
	while(true){ //loop principal

		//ACA HAY QUE CHEQUEAR UART Y UPDATEAR LOS FLAGS DE ARRIBA

		if(pause == false || end_of_song == false){
			if(play_new_song){
				play_file(selected_song, play_new_song, volumen, eq_preset);
				play_new_song = false;
			}
			else{
				end_of_song = play_file(selected_song, play_new_song, volumen, eq_preset);
				uint16_t* fft_pointer = get_fft_array();
				/*//ESTO ES PARA TESTEAR EL CAMBIO DE CANCION, LA REINICIA AL TOQUE
				if((count ++)==1000000){
					play_new_song = true;
					count = 0;
				}*/

			}

		}
	}
}

int analize_directory(char* currdir, char*nextdir, int string_len){
	int len = string_len;
	if(strcmp(currdir, "/")){
		strcat(currdir,"/");
	}
	strcat(currdir, nextdir);


	/*****Abrimos el directorio raíz*****/

	DIR directory; /* Directory object */

	//PRINTF("\r\nList the file in that directory...");
	//PRINTF(currdir);
	if (f_opendir(&directory, currdir)) {
		//PRINTF("Open directory failed.\r\n");
		return -1;
	}

    FILINFO files;
    FRESULT res;

    while (1) {

        res = f_readdir(&directory, &files);

        if (res != FR_OK || strlen(files.fname) == 0) {
            break;
        }
        if (strstr(files.fname, ".MP3")) {
        	PRINTF("\r\n");
        	PRINTF(currdir);
        	if(strcmp(currdir, "/")){
        		PRINTF("/");
        	}
            PRINTF(files.fname);
            PRINTF("\r\n");
        }
        else if (!(strstr(files.fname, "SYSTEM"))){
        	analize_directory(currdir, files.fname, len + strlen(files.fname));
        	currdir[string_len]='\0';
        }
    }
    return 1;
}

char init_sd_card(){

		FRESULT error;

		const TCHAR driverNumberBuffer[3U] = {SDDISK + '0', ':', '/'};

		BOARD_InitPins();
		BOARD_BootClockRUN();
		BOARD_InitDebugConsole();
		SYSMPU_Enable(SYSMPU, false);

		PRINTF("\r\nPlease insert a card into board.\r\n");

		if (sdcardWaitCardInsert() != kStatus_Success) {
			return -1;
		}

		if (f_mount(&g_fileSystem, driverNumberBuffer, 0U)) {
			PRINTF("Mount volume failed.\r\n");
			return -1;
		}

		#if (FF_FS_RPATH >= 2U)
			error = f_chdrive((char const *)&driverNumberBuffer[0U]);
			if (error) {
				PRINTF("Change drive failed.\r\n");
				return -1;
			}
		#endif

		return 0;
	}

static status_t sdcardWaitCardInsert(void) {
    /* Save host information. */
    g_sd.host.base = SD_HOST_BASEADDR;
    g_sd.host.sourceClock_Hz = SD_HOST_CLK_FREQ;
    /* card detect type */
    g_sd.usrParam.cd = &s_sdCardDetect;
#if defined DEMO_SDCARD_POWER_CTRL_FUNCTION_EXIST
    g_sd.usrParam.pwr = &s_sdCardPwrCtrl;
#endif
    /* SD host init function */
    if (SD_HostInit(&g_sd) != kStatus_Success) {
        PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }
    /* power off card */
    SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
    /* wait card insert */
    if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success) {
        PRINTF("\r\nCard inserted.\r\n");
        /* power on the card */
        SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
    } else {
        PRINTF("\r\nCard detect fail.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

