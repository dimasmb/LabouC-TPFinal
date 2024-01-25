#include "FTM.h"
#include "board.h"

static ftm_callback_t ftmCallbacks[FTM_CANT];
FTM_Type* FTMptr[] = { FTM0, FTM1, FTM2, FTM3 };

__ISR__ FTM0_IRQHandler(void)
{
	FTM_ClearOverflowFlag (FTM0);
	(*ftmCallbacks[0])();
}

__ISR__ FTM1_IRQHandler(void)
{
	FTM_ClearOverflowFlag (FTM1);
	(*ftmCallbacks[1])();
}

__ISR__ FTM2_IRQHandler(void)
{
	//FTM_ClearInterruptFlag(FTM2, FTM_CH_0);
	FTM_ClearOverflowFlag (FTM2);
	(*ftmCallbacks[2])();
}

__ISR__ FTM3_IRQHandler(void)
{
#ifdef MEASURE_IRQ_TIME
	gpioWrite(MEASURE_IRQ_FTM3,HIGH);
#endif

	FTM_ClearInterruptFlag(FTM3, FTM_CH_5);
	(*ftmCallbacks[3])();

#ifdef MEASURE_IRQ_TIME
	gpioWrite(MEASURE_IRQ_FTM3,LOW);
#endif
}

void FTM_Init (void)
{
	SIM->SCGC6 |= SIM_SCGC6_FTM0_MASK;
	SIM->SCGC6 |= SIM_SCGC6_FTM1_MASK;
	SIM->SCGC6 |= SIM_SCGC6_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM2_MASK;
	SIM->SCGC3 |= SIM_SCGC3_FTM3_MASK;

	NVIC_EnableIRQ(FTM0_IRQn);
	NVIC_EnableIRQ(FTM1_IRQn);
	NVIC_EnableIRQ(FTM2_IRQn);
	NVIC_EnableIRQ(FTM3_IRQn);

	FTM0->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM1->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM2->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;
	FTM3->PWMLOAD = FTM_PWMLOAD_LDOK_MASK | 0x0F;

}

void FTM_Start(ftm_id_t id, ftm_config_t configs , ftm_callback_t ftmCallback)
{
	FTM_SetPrescaler(FTMptr[id], configs.prescaler);
	FTM_SetModulus(FTMptr[id],configs.MOD_);
	FTMptr[id]->CNTIN = configs.CNTIN_;

	ftmCallbacks[id] = ftmCallback;

	switch(configs.mode)
	{
	case FTM_mInputCapture:
	{
		FTMptr[id]->MODE = (FTMptr[id]->MODE & ~FTM_MODE_FTMEN_MASK) | FTM_MODE_FTMEN(1);

		FTM_SetInputCaptureEdge(FTMptr[id],configs.channel, configs.edge);
		FTM_SetInterruptMode(FTMptr[id],configs.channel, true);
		FTM_SetWorkingMode(FTMptr[id],configs.channel,configs.mode);

		FTM_SetInterruptMode(FTMptr[id], configs.channel, true);

		//PTC9 as Input Compare (FTM3-CHANNEL5)
		PORTC->PCR[9] = PORT_PCR_DSE(1) | PORT_PCR_MUX(0b011) | PORT_PCR_IRQC(0);
	}
	break;

	case FTM_mOutputCompare:
	{
		FTM_SetOutputCompareEffect(FTMptr[id],configs.channel, configs.effect);
		FTM_SetInterruptMode(FTMptr[id],configs.channel, true);
		FTM_SetWorkingMode(FTMptr[id],configs.channel,configs.mode);

		//Set PTC1 as Output Compare (FTM0-CHANNEL0)
		PORTC->PCR[1] = PORT_PCR_DSE(1) | PORT_PCR_MUX(0b100) | PORT_PCR_IRQC(0);
	}
	break;

	case FTM_mOverflow:

	{	//Enable Timer advanced modes (FTMEN=1)
		FTMptr[id]->MODE=(FTMptr[id]->MODE & ~FTM_MODE_FTMEN_MASK) | FTM_MODE_FTMEN(1);

		///Enable Timer Overflow interrupt
		FTMptr[id]->SC = (FTM0->SC & ~FTM_SC_TOIE_MASK) | FTM_SC_TOIE(1);
	}
	break;

	case FTM_mPulseWidthModulation:
	{
		switch(id)
		{
			case 0:
			{
				//FTM0 --> ch0 PTC1
				PORTC->PCR[1] = 0; //clear
				PORTC->PCR[1] |= PORT_PCR_DSE(1) | PORT_PCR_MUX(0b011) | PORT_PCR_IRQC(0);
			}break;
			case 1:
			{
			//No disponible en kinetis
			}break;
			case 2:
			{
			//FTM2 --> ch0 PTB18
				PORTB->PCR[18] = 0; //clear
				PORTB->PCR[18] |= PORT_PCR_DSE(1) | PORT_PCR_MUX(0b011) | PORT_PCR_IRQC(0) | PORT_PCR_PE(1) |PORT_PCR_PS(0) ;
			}break;
			case 3:
			{
			//FTM3 --> ch0 PTD0
			//Set PTD0 as PWM
				PORTD->PCR[0] = 0; //clear
				PORTD->PCR[0] |= PORT_PCR_DSE(1) | PORT_PCR_MUX(0b100) | PORT_PCR_IRQC(0);
			}break;
		}
		//configs
		//Interrupt mode
		FTM_SetInterruptMode(FTMptr[id],configs.channel, true);
		//FTM_SetOverflowMode(FTMptr[id], true);

		FTM_SetWorkingMode(FTMptr[id],configs.channel,configs.mode);
		FTM_SetPulseWidthModulationLogic(FTMptr[id],configs.channel,configs.logic);

		FTM_SetCounter(FTMptr[id],configs.channel,configs.duty);

		//enable DMA
		FTM_SetDMA(FTMptr[id], configs.channel, true);

	}
		break;
	}
	//FTM_StartClock(FTMptr[id]);
}


// Setters

void FTM_SetPrescaler (FTM_t ftm, FTM_Prescal_t data)
{
	ftm->SC = (ftm->SC & ~FTM_SC_PS_MASK) | FTM_SC_PS(data);
}

void FTM_SetModulus (FTM_t ftm, FTMData_t data)
{
	ftm->CNTIN = 0X00;
	ftm->CNT = 0X00;
	ftm->MOD = FTM_MOD_MOD(data);
}

FTMData_t FTM_GetModulus (FTM_t ftm)
{
	return ftm->MOD & FTM_MOD_MOD_MASK;
}

void FTM_StartClock (FTM_t ftm)
{
	ftm->SC |= FTM_SC_CLKS(0x01);
}

void FTM_StopClock (FTM_t ftm)
{
	ftm->SC &= ~FTM_SC_CLKS(0x01);
}

void FTM_SetOverflowMode (FTM_t ftm, bool mode)
{
	ftm->SC = (ftm->SC & ~FTM_SC_TOIE_MASK) | FTM_SC_TOIE(mode);
}

bool FTM_IsOverflowPending (FTM_t ftm)
{
	return ftm->SC & FTM_SC_TOF_MASK;
}

void FTM_ClearOverflowFlag (FTM_t ftm)
{
	ftm->SC &= ~FTM_SC_TOF_MASK;
}

void FTM_SetWorkingMode (FTM_t ftm, FTMChannel_t channel, FTMMode_t mode)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK)) |
			                      (FTM_CnSC_MSB((mode >> 1) & 0X01) | FTM_CnSC_MSA((mode >> 0) & 0X01));
}

FTMMode_t FTM_GetWorkingMode (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_MSB_MASK | FTM_CnSC_MSA_MASK)) >> FTM_CnSC_MSA_SHIFT;
}

void FTM_SetInputCaptureEdge (FTM_t ftm, FTMChannel_t channel, FTMEdge_t edge)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((edge >> 1) & 0X01) | FTM_CnSC_ELSA((edge >> 0) & 0X01));
}

void FTM_SetDMA(FTM_t ftm, FTMChannel_t channel, bool mode)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_DMA_MASK)) | (FTM_CnSC_DMA(mode));
}


FTMEdge_t FTM_GetInputCaptureEdge (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetOutputCompareEffect (FTM_t ftm, FTMChannel_t channel, FTMEffect_t effect)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((effect >> 1) & 0X01) | FTM_CnSC_ELSA((effect >> 0) & 0X01));
}

FTMEffect_t FTM_GetOutputCompareEffect (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetPulseWidthModulationLogic (FTM_t ftm, FTMChannel_t channel, FTMLogic_t logic)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~(FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) |
				                  (FTM_CnSC_ELSB((logic >> 1) & 0X01) | FTM_CnSC_ELSA((logic >> 0) & 0X01));
}

FTMLogic_t FTM_GetPulseWidthModulationLogic (FTM_t ftm, FTMChannel_t channel)
{
	return (ftm->CONTROLS[channel].CnSC & (FTM_CnSC_ELSB_MASK | FTM_CnSC_ELSA_MASK)) >> FTM_CnSC_ELSA_SHIFT;
}

void FTM_SetCounter (FTM_t ftm, FTMChannel_t channel, FTMData_t data)
{
	ftm->CONTROLS[channel].CnV = FTM_CnV_VAL(data);
}

FTMData_t FTM_GetCounter (FTM_t ftm, FTMChannel_t channel)
{
	return ftm->CONTROLS[channel].CnV & FTM_CnV_VAL_MASK;
}

void FTM_SetInterruptMode (FTM_t ftm, FTMChannel_t channel, bool mode)
{
	ftm->CONTROLS[channel].CnSC = (ftm->CONTROLS[channel].CnSC & ~FTM_CnSC_CHIE_MASK) | FTM_CnSC_CHIE(mode);
}

bool FTM_IsInterruptPending (FTM_t ftm, FTMChannel_t channel)
{
	return ftm->CONTROLS[channel].CnSC & FTM_CnSC_CHF_MASK;
}

void FTM_ClearInterruptFlag (FTM_t ftm, FTMChannel_t channel)
{
	ftm->CONTROLS[channel].CnSC &= ~FTM_CnSC_CHF_MASK;
}
