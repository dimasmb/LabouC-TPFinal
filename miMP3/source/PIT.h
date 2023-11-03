
#ifndef PIT_H
#define PIT_H

#include <stdbool.h>
#include <stdint.h>

#include "hardware.h"


void PIT_init(int ldval);
void pit_enable_trig();
void pit_disable_trig();


#endif /* PIT_H */
