#include "MK64F12.h"


#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "fsl_sysmpu.h"
#include "board.h"
#include "diskio.h"
#include "ff.h"
//#include "fsl_debug_console.h"
#include "fsl_gpio.h"
#include "fsl_sd.h"
#include "fsl_sd_disk.h"
#include "uart.h"
#include "play_audio.h"
#include "gpio.h"

#include "dma_matrix.h"

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
int analize_directory(char* currdir, char*nextdir, int string_len);

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
	gpioMode(PORTNUM2PIN(PB,0),OUTPUT);

	do{
	} while(init_sd_card()!=0);

	DMAmatrixInit();

	/***mandamos por UART todos los archivos****/

	UART_Initialize(115200, 1);
	do{
	} while(inputEmpty()==true || retreiveInput()!='C');

	while(!inputEmpty()){
		retreiveInput();
	}

	char directory_string[1000];
	strcpy(directory_string, "");
	analize_directory(directory_string, "", 0);

	const char eof = 0x1a;
	UART_Send_Data(&eof, strlen(&eof));

	strcpy(directory_string, "");
	analize_directory(directory_string, "", 0);
	UART_Send_Data(&eof, strlen(&eof));

	strcpy(directory_string, "");
	analize_directory(directory_string, "", 0);
	UART_Send_Data(&eof, strlen(&eof));

    /*******AHORA ESA PUESTO ARBITRARIAMENTE ESTE ARCHIVO********/

	char selected_song [100];
	strcpy(selected_song, "");

	char end_of_song = false;
	char play_new_song = true;
	char pause = true;
	int eq_preset = NONE; //puede ser NONE, CLASSICAL, ROCK, URBAN
	int volumen = 1;

	while(true){ //loop principal

		//ACA HAY QUE CHEQUEAR UART Y UPDATEAR LOS FLAGS DE ARRIBA
		static waiting_4_songpath = false;
		static int newpath_idx = 0;
		static char newpath [50];

		if(inputEmpty()==false){
			if(waiting_4_songpath == false){
				int vol = 0;
				switch (retreiveInput()) {
					case 'V':
						while(inputEmpty()==true);
						vol+=10*(retreiveInput()-'0');
						while(inputEmpty()==true);
						vol+=retreiveInput()-'0';
						volumen = 32-vol;
						break;
					case 'N':
						eq_preset = NONE;
						break;
					case 'U':
						eq_preset = URBAN;
						break;
					case 'O':
						eq_preset = CLASSICAL;
						break;
					case 'K':
						eq_preset = ROCK;
						break;
					case 'A':
						pause =  !pause; //esto te lo pausa
						break;
					case 'R':
						pause = !pause;
						break;
					case 'L':
						waiting_4_songpath = true;
						newpath_idx=0;
						break;
					case 'X':
						end_of_song = false;
						play_new_song = true;
						pause = true;
						eq_preset = NONE; //puede ser NONE, CLASSICAL, ROCK, URBAN
						volumen = 1;
						break;
					case 'C':
						strcpy(directory_string, "");
						analize_directory(directory_string, "", 0);
						UART_Send_Data(&eof, strlen(&eof));
						strcpy(directory_string, "");
						analize_directory(directory_string, "", 0);
						UART_Send_Data(&eof, strlen(&eof));
						strcpy(directory_string, "");
						analize_directory(directory_string, "", 0);
						UART_Send_Data(&eof, strlen(&eof));
						break;
					default:
						break;
				}
			}
			else{
				//while(inputEmpty()==true);
				char new_char = retreiveInput();
				if(new_char == 0x1A){
					newpath[newpath_idx] = '\0';
					strcpy(selected_song, newpath);
					waiting_4_songpath = false;
					play_new_song = true;
					pause = false;
				}
				else{
					newpath[newpath_idx] = new_char;
					newpath_idx++;
				}


			}
		}



		//ACA PROCESAMOS LA CANCION

		if(pause == false && end_of_song == false){
			end_of_song = play_file(selected_song, play_new_song, volumen, eq_preset);
			uint16_t* fft_pointer = get_fft_array();

			int p = 0;

			for(p=0; p<8; p++){
				float level = (float)(*(fft_pointer+p));
				level = (level+1.1);
				level /= 1500.0;
				level *= 8.0;
				Matrix_ColByLevel(p,(uint8_t)level,p==7);
			}


			if(play_new_song){
				play_new_song = false;
			}
		}
		else if (end_of_song != false){
			pause = true;
			end_of_song = false;
			UART_Send_Data("E", strlen("E"));
		}
        else if(pause==true){
        	Matrix_TurnOff();
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
	//////PRINTF(currdir);
	if (f_opendir(&directory, currdir)) {
		//////PRINTF("Open directory failed.\r\n");
		return -1;
	}

    FILINFO files;
    FRESULT res;

    while (1) {

        res = f_readdir(&directory, &files);

        if (res != FR_OK || strlen(files.fname) == 0) {
            break;
        }
        if (strstr(files.fname, ".MP3")){
        	UART_Send_Data(currdir, strlen(currdir));
        	if(strcmp(currdir, "/")){
        		UART_Send_Data("/", strlen("/"));
        	}
        	UART_Send_Data(files.fname, strlen(files.fname));
            UART_Send_Data("\n", strlen("\n"));
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

		////PRINTF("\r\nPlease insert a card into board.\r\n");

		if (sdcardWaitCardInsert() != kStatus_Success) {
			return -1;
		}

		if (f_mount(&g_fileSystem, driverNumberBuffer, 0U)) {
			////PRINTF("Mount volume failed.\r\n");
			return -1;
		}

		#if (FF_FS_RPATH >= 2U)
			error = f_chdrive((char const *)&driverNumberBuffer[0U]);
			if (error) {
				////PRINTF("Change drive failed.\r\n");
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
        ////PRINTF("\r\nSD host init fail\r\n");
        return kStatus_Fail;
    }
    /* power off card */
    SD_PowerOffCard(g_sd.host.base, g_sd.usrParam.pwr);
    /* wait card insert */
    if (SD_WaitCardDetectStatus(SD_HOST_BASEADDR, &s_sdCardDetect, true) == kStatus_Success) {
        ////PRINTF("\r\nCard inserted.\r\n");
        /* power on the card */
        SD_PowerOnCard(g_sd.host.base, g_sd.usrParam.pwr);
    } else {
        ////PRINTF("\r\nCard detect fail.\r\n");
        return kStatus_Fail;
    }

    return kStatus_Success;
}

