#include "fsl_debug_console.h"
#include "board.h"
#include "fsl_ftm.h"

#include "pin_mux.h"
#include "clock_config.h"
#include "ir_capture.h"


#define BOARD_FTM_BASEADDR FTM0

/* FTM channel used for input capture */
#define BOARD_FTM_INPUT_CAPTURE_CHANNEL kFTM_Chnl_0

/* Interrupt number and interrupt handler for the FTM base address used */
#define FTM_INTERRUPT_NUMBER FTM0_IRQn
#define FTM_INPUT_CAPTURE_HANDLER FTM0_IRQHandler

/* Interrupt to enable and flag to read */
#define FTM_CHANNEL_INTERRUPT_ENABLE kFTM_Chnl0InterruptEnable
#define FTM_CHANNEL_FLAG kFTM_Chnl0Flag

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK CLOCK_GetFreq(kCLOCK_BusClk)

volatile bool ftmIsrFlag = false;
volatile   uint32_t captureVal;

volatile uint16_t ir_buff[25];

volatile uint16_t rc5_data;
volatile uint8_t ir_buff_index;
volatile uint8_t ir_data_found;
volatile uint16_t ir_data;
volatile uint8_t v_ir_data_available;

void FTM_INPUT_CAPTURE_HANDLER(void)
{
  static int prev_tick, prev_value;
  int value;
  
    
  if ((FTM_GetStatusFlags(BOARD_FTM_BASEADDR) & FTM_CHANNEL_FLAG) == FTM_CHANNEL_FLAG)
  {
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, FTM_CHANNEL_FLAG);
  }
  
  

  captureVal = BOARD_FTM_BASEADDR->CONTROLS[BOARD_FTM_INPUT_CAPTURE_CHANNEL].CnV;

  
  if( get_tick( ) > prev_tick + 150 ) {
    ir_buff_index = 0;
    goto end;
  }

  
  value = (int)captureVal - prev_value;
  if(value < 0 ) {
    value = 0xffff - prev_value + captureVal;
  }
  

  ir_buff[ ir_buff_index++ ] = value;
  if(  ir_buff_index >= sizeof(ir_buff)/sizeof(int16_t) ) {
    ir_buff_index--;
  }
  
  int8_t ir_index = 0;
  uint8_t rising = 1;
  uint16_t checkValue = 0;//1700;
  if( ir_buff_index == 23 ) {
    ir_data = 0;
    for(int i = 0; i < 23; i++) {
      rising ^= 1;
      checkValue += ir_buff[ i ];
      if( checkValue > 3000 && checkValue < 3600) {
        if( rising ) {
          ir_data |= 1<<(12 - ir_index);
        }
        
        ir_index ++;          
        v_ir_data_available = 1;
        ir_data = ir_data & 0x3f;

        if(ir_index >= 13) 
          goto end;
        
        checkValue = 0;
      } else if( checkValue < 1000 || checkValue > 4000 ) {
        
        v_ir_data_available = 0;
        goto end;
      }      
    }
    
    
    
    //    if( ir_data & 1<<12 ) {

//   }
    
  }
  
end:
  prev_value = captureVal;
  prev_tick = get_tick();
  
}


uint8_t ir_data_available(void)
{
  return v_ir_data_available;
}

uint8_t read_ir_data(void)
{
  v_ir_data_available = 0;
  return (uint8_t)ir_data;  
}

void ir_capture_init(void)
{
    ftm_config_t ftmInfo;

   

    /* Print a note to terminal */
    PRINTF("\r\nFTM input capture example\r\n");
    PRINTF("\r\nOnce the input signal is received the input capture value is printed\r\n");

    FTM_GetDefaultConfig(&ftmInfo);
    /* Initialize FTM module */
    ftmInfo.prescale = kFTM_Prescale_Divide_32;
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /* Setup dual-edge capture on a FTM channel pair */
    FTM_SetupInputCapture(BOARD_FTM_BASEADDR, BOARD_FTM_INPUT_CAPTURE_CHANNEL, kFTM_RiseAndFallEdge, 0);

    /* Set the timer to be in free-running mode */
    BOARD_FTM_BASEADDR->MOD = 0xFFFF;

    /* Enable channel interrupt when the second edge is detected */
    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, FTM_CHANNEL_INTERRUPT_ENABLE);

    /* Enable at the NVIC */
    EnableIRQ(FTM_INTERRUPT_NUMBER);
 
    NVIC_SetPriority(FTM0_IRQn, 1);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);

}
