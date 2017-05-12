#ifndef _lis3dsh_H_
#define _lis3dsh_H_

#include "cmsis_os.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "RingBuffer.h"
#include <stdlib.h>
#include <stdio.h>

uint8_t accRead(uint8_t regRead);
void accWrite(uint8_t regWrite, uint8_t regValue);
void accConfig(void);
void accInitialize(void);
int accRead(void);

#endif
