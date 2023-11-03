
#include "PIT.h"
#include "MK64F12.h"




void PIT_init(int ldval)
{

	// Clock Gating for PIT
	SIM->SCGC6 |= SIM_SCGC6_PIT_MASK;
	// PIT Module enable
	PIT->MCR &= ~PIT_MCR_MDIS_MASK;


	/* ===================================== */
	/* Configure timer operation when in debug mode */

        PIT->MCR &= ~PIT_MCR_FRZ_MASK;

      //  PIT->MCR |= PIT_MCR_FRZ_MASK;

    /* ===================================== */

    NVIC_EnableIRQ(PIT0_IRQn);
    NVIC_EnableIRQ(PIT1_IRQn);
    NVIC_EnableIRQ(PIT2_IRQn);
    NVIC_EnableIRQ(PIT3_IRQn);


    PIT->CHANNEL[0].LDVAL = ldval-1;
    //PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TIE_MASK;  // PIT interrupt enable (not used)
    PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;

}

void pit_enable_trig(){
	PIT->CHANNEL[0].TCTRL |= PIT_TCTRL_TEN_MASK;
}

void pit_disable_trig(){
	PIT->CHANNEL[0].TCTRL &= ~PIT_TCTRL_TEN_MASK;
}
