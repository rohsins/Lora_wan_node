/** 
		Hardware
*/
#ifndef _rn2483_H_
#define _rn2483_H_

#include "cmsis_os.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"
#include "RingBuffer.h"
#include "LoRa.h"
#include <stdlib.h>
#include <stdio.h>

void LoraConfig(void);
void LoraTransmit(char* port,char *arg);

#endif
