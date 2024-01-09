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



//para el button press de pausa

#include "gpio.h"

volatile bool g_ButtonPress = false;

void sw3_interrupt(){
	g_ButtonPress = !g_ButtonPress;
}

static int volumen = 1;

void sw2_interrupt(){
	volumen ++;
	if (volumen == 33) volumen = 1;
}

void init_sw(){
	gpioMode (PORTNUM2PIN(PA,4) , INPUT);
	gpioIRQ(PORTNUM2PIN(PA,4), GPIO_IRQ_MODE_FALLING_EDGE, sw3_interrupt);
	gpioMode (PORTNUM2PIN(PC,6) , INPUT);
	gpioIRQ(PORTNUM2PIN(PC,6), GPIO_IRQ_MODE_FALLING_EDGE, sw2_interrupt);

}

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

uint8_t mp3_files[1000][20];  // to save file names
int mp3_file_index;
int mp3_total_files;

uint8_t folder_files[1000][20];  // to save file names
int folder_file_index;
int folder_total_files;

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

	gpioMode(PORTNUM2PIN(PB,9), OUTPUT);
	gpioWrite (PORTNUM2PIN(PB,9), LOW);


	do{
	} while(init_sd_card()!=0);

	/*****Abrimos el directorio raíz*****/

	DIR directory; /* Directory object */

	PRINTF("\r\nList the file in that directory......\r\n");
	if (f_opendir(&directory, "/carpeta1/carpeta2")) {
		PRINTF("Open directory failed.\r\n");
		return -1;
	}

    /*****Listamos de las canciones que hay en mp3_files*****/

    FILINFO files;
    FRESULT res;

    while (1) {
        res = f_readdir(&directory, &files);
        if (res != FR_OK || strlen(files.fname) == 0) {
            break;  ////f_opendir(&directory, "/");
        }

        if (strstr(files.fname, ".MP3")) {
            strcpy(mp3_files[mp3_file_index], files.fname);  // to save file names
            mp3_file_index++;
            mp3_total_files++;
            PRINTF("%s\r\n", files.fname);
        }
        else if (!(strstr(files.fname, "SYSTEM"))){
            strcpy(folder_files[folder_file_index], files.fname);  // to save file names
            folder_file_index++;
            folder_total_files++;
        	PRINTF("%s\r\n", files.fname);
        }

    }

    /***************/

    mp3_file_index = 0; //ahora mismo elegimos directo la 3ra cancion que aparece y la cargamos
    play_file(mp3_files[mp3_file_index], true, volumen);

    init_sw();


    while(1){
    	if(!g_ButtonPress){
			gpioWrite (PORTNUM2PIN(PB,9), HIGH);
    		int end_of_song = play_file(mp3_files[mp3_file_index], false, volumen);
    		gpioWrite (PORTNUM2PIN(PB,9), LOW);
    		if (end_of_song){
    			break;
    		}
    	}
    }
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

