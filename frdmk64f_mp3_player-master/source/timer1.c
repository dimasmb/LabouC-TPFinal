#include "board.h"
#include "fsl_ftm.h"

  
#define BOARD_FTM_BASEADDR FTM1

/* Interrupt number and interrupt handler for the FTM instance used */
#define BOARD_FTM_IRQ_NUM FTM1_IRQn
#define BOARD_FTM_HANDLER FTM1_IRQHandler

/* Get source clock for FTM driver */
#define FTM_SOURCE_CLOCK (CLOCK_GetFreq(kCLOCK_BusClk)/4)


ftm_config_t ftmInfo;
volatile uint32_t uTick;

void timer1_init(void)
{


    FTM_GetDefaultConfig(&ftmInfo);
    
    /* Divide FTM clock by 4 */
    ftmInfo.prescale = kFTM_Prescale_Divide_4;
    
    /* Initialize FTM module */
    FTM_Init(BOARD_FTM_BASEADDR, &ftmInfo);

    /*
     * Set timer period.
    */
    FTM_SetTimerPeriod(BOARD_FTM_BASEADDR, USEC_TO_COUNT(1000U, FTM_SOURCE_CLOCK));

    FTM_EnableInterrupts(BOARD_FTM_BASEADDR, kFTM_TimeOverflowInterruptEnable);

    EnableIRQ(BOARD_FTM_IRQ_NUM);

    FTM_StartTimer(BOARD_FTM_BASEADDR, kFTM_SystemClock);


}


void BOARD_FTM_HANDLER(void)
{
    /* Clear interrupt flag.*/
    FTM_ClearStatusFlags(BOARD_FTM_BASEADDR, kFTM_TimeOverflowFlag);
    uTick++;
}


uint32_t get_tick(void)
{
  return uTick;
}
  
void delay(uint32_t d)
{
  volatile uint32_t t = get_tick() + d;
  while(! (get_tick() > t) ); 
}
