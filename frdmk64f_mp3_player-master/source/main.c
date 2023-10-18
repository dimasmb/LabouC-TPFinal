#include <stdio.h>
#include <string.h>
#include "fsl_debug_console.h"
#include "ff.h"
#include "diskio.h"
#include "board.h"
#include "fsl_dac.h"

#include "fsl_sysmpu.h"
#include "pin_mux.h"
#include "clock_config.h"

#include "mp3dec.h"


#include <math.h>

#include "timer1.h"

#include "fsl_dac.h"
#include "fsl_pit.h"
#include "fsl_dmamux.h"
#include "fsl_edma.h"
#include "fsl_port.h"
#include "fsl_gpio.h"
#include "i2c0_user.h"
#include "ir_capture.h"
#include "glcd.h"
#include "fonts/font5x7.h"
//#include "fonts/Liberation_Sans11x14_Numbers.h"
#include "fonts/Liberation_Sans15x21_Numbers.h"

/*******************************************************************************
* Definitions
******************************************************************************/

/* buffer size (in byte) for read/write operations */
#define BOARD_LED_GPIO BOARD_LED_RED_GPIO
#define BOARD_LED_GPIO_PIN BOARD_LED_RED_GPIO_PIN
#define BOARD_SW_GPIO BOARD_SW3_GPIO
#define BOARD_SW_PORT BOARD_SW3_PORT
#define BOARD_SW_GPIO_PIN BOARD_SW3_GPIO_PIN
#define BOARD_SW_IRQ BOARD_SW3_IRQ
#define BOARD_SW_IRQ_HANDLER BOARD_SW3_IRQ_HANDLER
#define BOARD_SW_NAME BOARD_SW3_NAME

///// ir commands
#define NEXT 0x20
#define PREV 0x21
#define REPLAY 0xA
#define FFW 0x16
#define PLAY 0x17
#define VOLUME_UP 0x10
#define VOLUME_DOWN 0x11
#define MUTE    0xD
#define BASS_BOOSTED 0X12


volatile bool g_ButtonPress = false;
volatile uint8_t forced_mono,bass_boosted,fast_forward;

void BOARD_SW_IRQ_HANDLER(void)
{
  /* Clear external interrupt flag. */
  GPIO_ClearPinsInterruptFlags(BOARD_SW_GPIO, 1U << BOARD_SW_GPIO_PIN);
  /* Change state of button. */
  g_ButtonPress = true;
}


/*******************************************************************************
* Prototypes
******************************************************************************/
void ProvideAudioBuffer(int16_t *samples, int cnt) ;
static uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize);
static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize);
void RunDACsine(int sample_rate, int output_samples);
void play_file(char *mp3_fname);

/*******************************************************************************
* Variables
******************************************************************************/
static FATFS g_fileSystem; /* File system object */
static FIL g_fileObject;   /* File object */

//static uint8_t g_bufferWrite[BUFFER_SIZE]; /* Write buffer */
//static uint8_t g_bufferRead[BUFFER_SIZE];  /* Read buffer */



#define FILE_READ_BUFFER_SIZE   (1024*16)
MP3FrameInfo                 mp3FrameInfo;
HMP3Decoder         hMP3Decoder;
uint8_t read_buff[FILE_READ_BUFFER_SIZE];
uint32_t bytes_read;
int    bytes_left;
char    *read_ptr;
int16_t pcm_buff[2304];
int16_t audio_buff[2304*2];
volatile uint32_t delay1 = 1000;
volatile uint32_t core_clock ;

uint8_t mp3_files[1000][15];    //to save file names
int mp3_file_index;
int mp3_total_files;

volatile uint8_t next, prev, replay, mute,ffd,reset, play, volume = 5;
enum play_e {
  eREPLAY, ePREVIOUS, eNEXT
};
/*******************************************************************************
* Code
******************************************************************************/

//DAC
dac_config_t dacConfigStruct;    
uint32_t dacValue;

volatile uint32_t r1,r2;

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
  BYTE work[_MAX_SS];
  
  /* Define the init structure for the input switch pin */
  gpio_pin_config_t sw_config = { 
    kGPIO_DigitalInput, 0,
  };
  
  /* Define the init structure for the output LED pin */
  gpio_pin_config_t led_config = {
    kGPIO_DigitalOutput, 0,
  };
  
  
  BOARD_InitPins();
  BOARD_BootClockRUN();
  BOARD_InitDebugConsole();
  SYSMPU_Enable(SYSMPU, false);
  
  core_clock =   CLOCK_GetFreq(kCLOCK_CoreSysClk);
  LED_BLUE_INIT(1);
  i2c0_user_init();
  timer1_init();  
  ir_capture_init();
  oled_init();
  glcd_clear();
  // glcd_test_text_up_down();    
  
  /* Init input switch GPIO. */
  PORT_SetPinInterruptConfig(BOARD_SW_PORT, BOARD_SW_GPIO_PIN, kPORT_InterruptFallingEdge);
  EnableIRQ(BOARD_SW_IRQ);
  GPIO_PinInit(BOARD_SW_GPIO, BOARD_SW_GPIO_PIN, &sw_config);
  
  /* Init output LED GPIO. */
  GPIO_PinInit(BOARD_LED_GPIO, BOARD_LED_GPIO_PIN, &led_config);
  
  GPIO_WritePinOutput(GPIOB, BOARD_LED_RED_GPIO_PIN, 1);
  //  
  //  
  //  DAC_GetDefaultConfig(&dacConfigStruct);
  //  DAC_Init(DAC0, &dacConfigStruct);
  //  DAC_Enable(DAC0, true);             /* Enable output. */
  //  DAC_SetBufferReadPointer(DAC0, 0U); /* Make sure the read pointer to the start. */
  //  
  /* Enable output. */
  
  
  PRINTF("\r\nFATFS example to demonstrate how to use FATFS with SD card.\r\n");
  
  PRINTF("\r\nPlease insert a card into board.\r\n");
  
  if (f_mount(&g_fileSystem, driverNumberBuffer, 0U))
  {
    PRINTF("Mount volume failed.\r\n");
    return -1;
  }
  
#if (_FS_RPATH >= 2U)
  error = f_chdrive((char const *)&driverNumberBuffer[0U]);
  if (error)
  {
    PRINTF("Change drive failed.\r\n");
    return -1;
  }
#endif
  
  PRINTF("\r\nList the file in that directory......\r\n");
  if (f_opendir(&directory, "/"))
  {
    PRINTF("Open directory failed.\r\n");
    return -1;
    
    
  }
  
  
  volatile FILINFO files;
  FRESULT res;
  
  char mp3_fname[50];
  memset(mp3_fname, 0, 50);
  while(1) {
    res =  f_readdir(&directory, &files);
    if( res != FR_OK || strlen(files.fname) == 0) {
      break;////f_opendir(&directory, "/");
    }
    
    if(strstr(files.fname, ".MP3")) {
      strcpy( mp3_files[mp3_file_index], files.fname );    //to save file names
      mp3_file_index++;
      mp3_total_files++;
      
    }
  }

  mp3_file_index = 0;
  while( 1 ) {

    glcd_clear_buffer();
    glcd_tiny_set_font(Font5x7,5,7,32,127);
    char file_name_and_index[50];
    sprintf(file_name_and_index, "%s (%d/%d)", mp3_files[mp3_file_index], mp3_file_index + 1, mp3_total_files);
    glcd_draw_string_xy(0,0, file_name_and_index);


    play_file(mp3_files[mp3_file_index]);

    switch (play) {
    case eREPLAY:
      break;
    case ePREVIOUS:
      mp3_file_index --;
      if(mp3_file_index < 0 ) {
        mp3_file_index = mp3_total_files - 1;
      }
      break;
    case eNEXT:
      mp3_file_index++;
      if(mp3_file_index >= mp3_total_files ) {
        mp3_file_index = 0;
      }
      break;
    }

  }
  
  
}

void play_file(char *mp3_fname) {

  if(strlen(mp3_fname) == 0) 
    while(1);
  
  
  uint8_t dac_started = 0;
  
  
  FIL fil;        /* File object */
  char line[100]; /* Line buffer */
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
  
  
  char szArtist[120];
  char szTitle[120];
  Mp3ReadId3V2Tag(&fil, szArtist, sizeof(szArtist), szTitle, sizeof(szTitle));
  
  
 // uint32_t size = f_size(&fil);
  //uint32_t read_size = 0;
  
  bytes_left = 0;
//  read_ptr = read_buff;
  
  
  int offset, err;
  int outOfData = 0;
  
  unsigned int br, btr;
  
  //delay(1000);
  
  int16_t *samples = pcm_buff;
  


  while(1) {    
    if(ir_data_available()) {
      uint8_t cmd = read_ir_data();
      switch(cmd) {
      case NEXT:
        next = 1;
        break;

      case PREV:
        prev = 1;
        break;

      case REPLAY:
        replay = 1;

        break;

      case FFW:
        fast_forward = 1;        

        break;

      case PLAY:
        fast_forward = 0;

        break;

      case VOLUME_UP:
        if(volume < 10)
        {
          volume++;
        }
        break;

      case VOLUME_DOWN:
        if(volume != 0)
        {
          volume--;
        }
        break;

      case MUTE:
        {

          static uint8_t mute = 0;
          static uint8_t volume_backup;
          mute ^= 1;
          if(mute) {
            volume_backup = volume;
            volume = 0;
          } else
            volume = volume_backup;
        }
        break;

      case BASS_BOOSTED:
    	  bass_boosted ^= 1;
      }
      
      
    }
    
    if( bytes_left < FILE_READ_BUFFER_SIZE/2 ) {
      memcpy( read_buff, read_ptr, bytes_left );
      read_ptr = read_buff;
      btr = FILE_READ_BUFFER_SIZE - bytes_left;


      //GPIO_TogglePinsOutput(BOARD_LED_BLUE_GPIO, 1U << BOARD_LED_BLUE_GPIO_PIN);
      GPIO_WritePinOutput(GPIOB, BOARD_LED_BLUE_GPIO_PIN, 0);
      fr = f_read(&fil, read_buff + bytes_left, btr, &br);
      GPIO_WritePinOutput(GPIOB, BOARD_LED_BLUE_GPIO_PIN, 1);


      bytes_left = FILE_READ_BUFFER_SIZE;
      
      if(fr || br < btr) {
        f_close(&fil);
        return;//while(1);//change 
      }
    }
    
    offset = MP3FindSyncWord((unsigned char*)read_ptr, bytes_left);
    if(offset == -1 ) {        
      bytes_left = 0;
      continue;
      
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
    } else {
      // no error
      MP3GetLastFrameInfo(hMP3Decoder, &mp3FrameInfo);
      if(!dac_started) {
        dac_started = 1;
        RunDACsine(mp3FrameInfo.samprate, mp3FrameInfo.outputSamps);
        DAC_Enable(DAC0, true);
        
      }
      // Duplicate data in case of mono to maintain playback speed
      if (mp3FrameInfo.nChans == 1) {
        for(int i = mp3FrameInfo.outputSamps;i >= 0;i--) {
          samples[2 * i]=samples[i];
          samples[2 * i + 1]=samples[i];
        }
        mp3FrameInfo.outputSamps *= 2;
      }
    }
    if (!outOfData) {      
      ProvideAudioBuffer(samples, mp3FrameInfo.outputSamps);
      
      time += mp3FrameInfo.outputSamps/2;
      if(time > mp3FrameInfo.samprate) {
        time -= mp3FrameInfo.samprate;
        seconds++;
        if(seconds >= 60) {
          minutes++;
          seconds = 0;
        }
      }
      
      
      if(prev_seconds != seconds) {
        char time_s[10];
        sprintf(time_s, "%02d:%02d", minutes, seconds);
        
        
        glcd_set_font(Liberation_Sans15x21_Numbers,15,21,46,57);
        //glcd_set_font(Liberation_Sans27x36_Numbers,27,36,46,57);
        //glcd_set_font(Bebas_Neue20x36_Bold_Numbers,20,36,46,57);
        //glcd_set_font(Bebas_Neue18x36_Numbers,18,36,46,57);
        //glcd_set_font(HelveticaNeueLT_Com_57_Cn23x35_Numbers,23,35,46,57); // commercial font - not for public distribution
        
        glcd_draw_string_xy(0,8,time_s);
        
        glcd_write();    
        
        prev_seconds = seconds;
      }
      
      if (g_ButtonPress || next || prev || replay)
      {
        if(next) {
          next = 0;
          play = eNEXT;
        } else if( prev ) {
          prev = 0;
          play = ePREVIOUS;
        } else if ( replay ){
          play = eREPLAY;
          replay = 0;
        }
        //GPIO_TogglePinsOutput(BOARD_LED_GPIO, 1U << BOARD_LED_GPIO_PIN);
        g_ButtonPress = false;
        memset(audio_buff, 0, sizeof(audio_buff));
        f_close(&fil);
        return;
      }
      
      


    }      
  }
}

float a0 = 0.00005029912027879971;
float a1 = 0.00010059824055759942;
float a2 = 0.00005029912027879971;
float b1 = -1.9821252053783214;
float b2 = 0.9823264018594366;

void ProvideAudioBuffer(int16_t *samples, int cnt) 
{
  
  static float z1,z2;
  
  static uint8_t state = 0;
  
  int32_t tmp = 0;
  if(1) {
    for(int i = 0; i < cnt; i++) {
      if(i%2 == 0) {
        tmp =   samples[i] + samples[i+1];
        tmp /= 2;
        samples[i] = (int16_t)tmp * (int32_t)volume / 10;;
      }
    }    
  }
  
  
  float w,out;//, in;
  

  if(bass_boosted) {
    for(int i = 0; i < cnt; i++) {
      if( i % 2 == 0) {
        w = (float)samples[i] - b1*z1 - b2*z2;
        out = a0*w + a1*z1 + a2*z2;
        z2 = z1;
        z1 = w;
      }
      samples[i] = (int16_t)out;
    }
  }
  
  
  

  if(state == 0) {


    r1 =  EDMA_GetRemainingMajorLoopCount(DMA0, 0) - cnt/2;
    while( EDMA_GetRemainingMajorLoopCount(DMA0, 0) > cnt/2 ) {
      if(fast_forward){

        goto end1;
      }
    }


    for(int i = 0; i < cnt; i++) {
      audio_buff[i] = *samples / 16;
      audio_buff[i] += (4096/2);
      samples++;
    }
    state = 1;
  end1:
    return;
  }
  
  if(state == 1) {
    r2 = EDMA_GetRemainingMajorLoopCount(DMA0, 0);
    while( EDMA_GetRemainingMajorLoopCount(DMA0, 0) < cnt/2 ) {
      if(fast_forward) {
        goto end2;
      }
    }



    for(int i = 0; i < cnt; i++) {
      audio_buff[i + cnt] = *samples / 16;
      audio_buff[i + cnt] += (4096/2);
      samples++;
    }
  end2:
    state = 0;
  }
  
  
  
  
}





/*
* Taken from
* http://www.mikrocontroller.net/topic/252319
*/

static uint32_t Mp3ReadId3V2Text(FIL* pInFile, uint32_t unDataLen, char* pszBuffer, uint32_t unBufferSize)
{
  UINT unRead = 0;
  BYTE byEncoding = 0;
  if((f_read(pInFile, &byEncoding, 1, &unRead) == FR_OK) && (unRead == 1))
  {
    unDataLen--;
    if(unDataLen <= (unBufferSize - 1))
    {
      if((f_read(pInFile, pszBuffer, unDataLen, &unRead) == FR_OK) ||
         (unRead == unDataLen))
      {
        if(byEncoding == 0)
        {
          // ISO-8859-1 multibyte
          // just add a terminating zero
          pszBuffer[unDataLen] = 0;
        }
        else if(byEncoding == 1)
        {
          // UTF16LE unicode
          uint32_t r = 0;
          uint32_t w = 0;
          if((unDataLen > 2) && (pszBuffer[0] == 0xFF) && (pszBuffer[1] == 0xFE))
          {
            // ignore BOM, assume LE
            r = 2;
          }
          for(; r < unDataLen; r += 2, w += 1)
          {
            // should be acceptable for 7 bit ascii
            pszBuffer[w] = pszBuffer[r];
          }
          pszBuffer[w] = 0;
        }
      }
      else
      {
        return 1;
      }
    }
    else
    {
      // we won't read a partial text
      if(f_lseek(pInFile, f_tell(pInFile) + unDataLen) != FR_OK)
      {
        return 1;
      }
    }
  }
  else
  {
    return 1;
  }
  return 0;
}

/*
* Taken from
* http://www.mikrocontroller.net/topic/252319
*/
static uint32_t Mp3ReadId3V2Tag(FIL* pInFile, char* pszArtist, uint32_t unArtistSize, char* pszTitle, uint32_t unTitleSize)
{
  pszArtist[0] = 0;
  pszTitle[0] = 0;
  
  BYTE id3hd[10];
  UINT unRead = 0;
  if((f_read(pInFile, id3hd, 10, &unRead) != FR_OK) || (unRead != 10))
  {
    return 1;
  }
  else
  {
    uint32_t unSkip = 0;
    if((unRead == 10) &&
       (id3hd[0] == 'I') &&
         (id3hd[1] == 'D') &&
           (id3hd[2] == '3'))
    {
      unSkip += 10;
      unSkip = ((id3hd[6] & 0x7f) << 21) | ((id3hd[7] & 0x7f) << 14) | ((id3hd[8] & 0x7f) << 7) | (id3hd[9] & 0x7f);
      
      // try to get some information from the tag
      // skip the extended header, if present
      uint8_t unVersion = id3hd[3];
      if(id3hd[5] & 0x40)
      {
        BYTE exhd[4];
        f_read(pInFile, exhd, 4, &unRead);
        size_t unExHdrSkip = ((exhd[0] & 0x7f) << 21) | ((exhd[1] & 0x7f) << 14) | ((exhd[2] & 0x7f) << 7) | (exhd[3] & 0x7f);
        unExHdrSkip -= 4;
        if(f_lseek(pInFile, f_tell(pInFile) + unExHdrSkip) != FR_OK)
        {
          return 1;
        }
      }
      uint32_t nFramesToRead = 2;
      while(nFramesToRead > 0)
      {
        char frhd[10];
        if((f_read(pInFile, frhd, 10, &unRead) != FR_OK) || (unRead != 10))
        {
          return 1;
        }
        if((frhd[0] == 0) || (strncmp(frhd, "3DI", 3) == 0))
        {
          break;
        }
        char szFrameId[5] = {0, 0, 0, 0, 0};
        memcpy(szFrameId, frhd, 4);
        uint32_t unFrameSize = 0;
        uint32_t i = 0;
        for(; i < 4; i++)
        {
          if(unVersion == 3)
          {
            // ID3v2.3
            unFrameSize <<= 8;
            unFrameSize += frhd[i + 4];
          }
          if(unVersion == 4)
          {
            // ID3v2.4
            unFrameSize <<= 7;
            unFrameSize += frhd[i + 4] & 0x7F;
          }
        }
        
        if(strcmp(szFrameId, "TPE1") == 0)
        {
          // artist
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszArtist, unArtistSize) != 0)
          {
            break;
          }
          nFramesToRead--;
        }
        else if(strcmp(szFrameId, "TIT2") == 0)
        {
          // title
          if(Mp3ReadId3V2Text(pInFile, unFrameSize, pszTitle, unTitleSize) != 0)
          {
            break;
          }
          nFramesToRead--;
        }
        else
        {
          if(f_lseek(pInFile, f_tell(pInFile) + unFrameSize) != FR_OK)
          {
            return 1;
          }
        }
      }
    }
    if(f_lseek(pInFile, unSkip) != FR_OK)
    {
      return 1;
    }
  }
  
  return 0;
}





void RunDACsine(int sample_rate, int output_samples) {
  dac_config_t dacConfigStruct;
  DAC_GetDefaultConfig(&dacConfigStruct);
  dacConfigStruct.referenceVoltageSource = kDAC_ReferenceVoltageSourceVref2;
  dacConfigStruct.enableLowPowerMode = false;
  DAC_Init(DAC0, &dacConfigStruct);
  dac_buffer_config_t dacBufferConfigStruct;
  DAC_GetDefaultBufferConfig(&dacBufferConfigStruct);
  dacBufferConfigStruct.triggerMode = kDAC_BufferTriggerBySoftwareMode;
  DAC_SetBufferConfig(DAC0, &dacBufferConfigStruct);
  DAC_SetBufferValue(DAC0, 0U, 0x7FF); // Succeeds: quick test to output 3.3v/2
  
  // Use DMAMUX0 internal channel number 0 to connect PIT event to DMA channel 0 request
  DMAMUX_Init(DMAMUX0);
  DMAMUX_Type *pDMAMUX0 = DMAMUX0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
  DMAMUX_DisableChannel(DMAMUX0, 0/* DMAMUX channel number */); // Disable channel prior configuring it
  // DMAMUX source is unused when using periodic trigger; seems setting source unnecessary?
  // But, default source 0 is "Disable" - set to "always on" source...
  DMAMUX_SetSource          (DMAMUX0, 0/* DMAMUX channel number */, (uint8_t)kDmaRequestMux0AlwaysOn63 /*48 PDB*/);
  DMAMUX_EnablePeriodTrigger(DMAMUX0, 0/* DMAMUX channel number */);
  DMAMUX_EnableChannel      (DMAMUX0, 0/* DMAMUX channel number */);
  
  // Set up DMA channel 0 to read from data buffer and write to DAC
  DMA_Type* pDMA0 = DMA0; // expose for debugger, since "Peripherals" viewer doesn't work in Kinetis Studio
  edma_config_t edmaConfig;
  EDMA_GetDefaultConfig(&edmaConfig);
  EDMA_Init(DMA0,&edmaConfig);
  
  // Crappy FSL driver functions don't set SLAST etc; set up TCD directly, aaarrrggggg....
  DMA0->CR = 0; // default mode of operation for DMA (no minor loop mapping, etc)
  DMA0->TCD[0].SADDR = (uint32_t)(audio_buff);//SOURCE_ADDRESS); // source address
  DMA0->TCD[0].DADDR = (uint32_t)(&DAC0->DAT);  // destination address
  // Source data and destination data transfer size
  DMA0->TCD[0].ATTR = DMA_ATTR_SSIZE(kEDMA_TransferSize2Bytes) | DMA_ATTR_DSIZE(kEDMA_TransferSize2Bytes);
  assert(DMA0->TCD[0].ATTR==0x0101);
  DMA0->TCD[0].SOFF = 4; // increment source address by 2 after each 2-byte transfer
  DMA0->TCD[0].DOFF = 0; // destination address is fixed DAC; do not increment
  DMA0->TCD[0].NBYTES_MLNO = 2; // 2 bytes to DAC per transfer
  DMA0->TCD[0].SLAST = -(/*2304*/output_samples*2)*2; // decrement SADDR back to source start address after completing a major loop
  DMA0->TCD[0].DLAST_SGA = 0; // destination address (DAC) is not adjusted after completing a major loop
  DMA0->TCD[0].BITER_ELINKNO =
    DMA0->TCD[0].CITER_ELINKNO = output_samples;//2304 ; // transfers per major loop
  DMA0->SERQ = DMA_SERQ_SERQ(0/*DMA channel#*/); // last, enable hardware requests for channel 0 (enable DMA channel 0)
  
  uint8_t dmaErr = DMA0->ES;
  assert(dmaErr==0); // no errors reported by DMA module
  uint8_t dmaErq = DMA0->ERQ;
  assert((dmaErq&1)==1); // DMA channel 0 is enabled
  
  // Set up PIT timer to generate trigger at interval that yields desired frequency for given number of samples
  {
    pit_config_t pit_setup;
    PIT_GetDefaultConfig(&pit_setup);
    pit_setup.enableRunInDebug = 1;
    PIT_Init(PIT,&pit_setup);
  }
  // WARNING: Contrary to bogus Freescale documentation, PIT channel 0 (not channel 1)
  // is hardwired to DMA channel 0 trigger in DMAMUX
#define PIT_CHANNEL kPIT_Chnl_0 
  PIT_SetTimerPeriod(PIT, PIT_CHANNEL, (CLOCK_GetFreq(kCLOCK_BusClk) / ((sample_rate))));//TARGET_FREQUENCY_HZ*SOURCE_CNT)) );
  PIT_StartTimer(PIT, PIT_CHANNEL); // start the timer...
}
