
#include "PWM.h"


static uint16_t PWM_duty    = 74;//5000-1;
static ftm_config_t modPWM_FTM;
static uint16_t aux  = 0;

void pwmInit(void)
{
	FTM_Init();
	//Inicializo el FTM usando el FTM1 para PWM
	modPWM_FTM.mode = FTM_mPulseWidthModulation;

	//Channel n output is forced high at the counter overflow when de CNTIN register value
	// is loaded into the FTM counter, and it is forced low at the channel (n) match
	// FTM counter == CnV
	modPWM_FTM.logic = FTM_lAssertedLow;

	/// Reminder:BusClock=sysclk/2= 50MHz
	/// Set prescaler = divx1 => timer clock = 1 x (1/BusClock)= 1/50MHz= 20 nseg
	modPWM_FTM.prescaler = FTM_PSC_x1;

	//Period T = MOD - CNTIN + 1
	modPWM_FTM.CNTIN_ = 0x00;
	modPWM_FTM.MOD_ = 74;	//Para tener Fs 800 kbps

	//Duty Cycle D = CnV + CNTIN --> CnV 0xXXXX
	modPWM_FTM.duty = PWM_duty;
	modPWM_FTM.channel = 0;
	//ftm0 CH0 --> ptc1
	//ftm2 ch0 ptb18
	//ftm3 CH0 --> ptd0

	FTM_Start(FTM_ID_2, modPWM_FTM, &PWM_FTM_IRQ);
	
}

uint16_t pwmUpdateData(uint16_t data2update)
{
	return data2update * modPWM_FTM.MOD_ / 4095;
}

void PWM_FTM_IRQ(void)
{
	//yo uso FTM2
	if((aux))
	{
		aux--;
		FTM_SetCounter(FTM2, modPWM_FTM.channel, 40);
	}
	else
	{
		FTM_SetCounter(FTM2, modPWM_FTM.channel, 0);
	}
	PWM_duty %= modPWM_FTM.MOD_;
}
