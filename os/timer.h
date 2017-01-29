#ifndef _TIMER_H_INCLUDED_
#define _TIMER_H_INCLUDED_

#include "interrupt.h"

void timer_intr(softvec_type_t type, unsigned long sp);
void timer_start(int second);

#endif
