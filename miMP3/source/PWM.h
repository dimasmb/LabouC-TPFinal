#ifndef MODULADORPWM_H_
#define MODULADORPWM_H_

#include <stdio.h>
#include "FTM.h"

void pwmInit(void);

void PWM_FTM_IRQ(void);

uint16_t pwmUpdateData(uint16_t data2update);

#endif
