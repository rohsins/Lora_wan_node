/** 
		Hardware
*/

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
#include "rn2483.h"
#include "lis3dsh.h"

USART_TypeDef *uart232 = USART0;
//USART_TypeDef *uartLora = UART0;
//SART_TypeDef *usartAcc = USART2;
extern USART_TypeDef *uartLora;
extern USART_TypeDef *usartAcc;

float adcResult = 404.0;
char iotBuffer[16];
char transitory[32];
uint32_t adcResultwhat = 0;
int16_t AxisZ;


void Initialize(void) {
  CMU_OscillatorEnable (cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet   (cmuClock_HF,    cmuSelect_HFXO);
  CMU_ClockDivSet      (cmuClock_HFPER, cmuClkDiv_1);
 
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART0, true);
	CMU_ClockEnable(cmuClock_UART0, true);
	CMU_ClockEnable(cmuClock_USART2, true);
	CMU_ClockEnable(cmuClock_ADC0, true);		
}

void adcFunc(void) {
	//enable battery measure
	GPIO_PinModeSet(gpioPortE, 1, gpioModePushPull, 1);
	GPIO_PinOutSet(gpioPortE, 1);
	
	ADC0->CTRL = (24 << 16) | (4 << 8);
	ADC0->IEN = 0;
	ADC0->CMD = 1 << 0;
	ADC0->SINGLECTRL = (2 << 16) | (4 << 8) | (0 << 4); // VDD reference, adcn_ch4, 12bit resolution
	
	while (! (ADC0->STATUS & ( 1 << 16 ))){};
	adcResult = ADC0->SINGLEDATA;
	ADC0->CMD = 1 << 0;
	adcResult = (((adcResult/4095)*(5*3.3)/3)*1000);
	adcResultwhat = adcResult;
//	GPIO_PinOutClear(gpioPortE, 1);
}

void blinky(void const* arg) {
	
	GPIO_PinModeSet(gpioPortA, 7, gpioModePushPull, 0);
	
	while(1) {
		GPIO_PinOutSet(gpioPortA, 7);
		osDelay(1000);
		GPIO_PinOutClear(gpioPortA, 7);
		osDelay(50);
	}
}
osThreadDef(blinky, osPriorityNormal, 1, 0);

void uart0Initialize(void) {
	USART_InitAsync_TypeDef uartInitTypeDef232 = USART_INITASYNC_DEFAULT;
	USART_Enable_TypeDef uartEnableTypeDef = usartEnable;
	uartInitTypeDef232.baudrate = 115200;
	USART_InitAsync(uart232, &uartInitTypeDef232);
	
	/* uart232 */
	GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1); 
  GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);
	
	/* uart232 */
	uart232->ROUTE |= 1 << 0 | 1 << 1 | 1 << 8 | 1 << 9;
	USART_Enable(uart232, uartEnableTypeDef);
}

void uartSend(USART_TypeDef *tempType, char data[]) {
	int i = 0;
	while(data[i]) {
		USART_Tx(tempType, data[i]);
//		USART_Tx(uart232, data[i]); //for debug
		i++;
	}
}

void ReceiveThread(void const *arg) {
	uint8_t temp;
	while (1) {
		temp = USART_Rx(uartLora);
		uartSend(uart232, (char *)(&temp));
	}
}
osThreadDef(ReceiveThread, osPriorityNormal, 1, 0);

void LoraTransmit(void const *arg) {
	LoraConfig();
	accInitialize();
	while (1) {
		adcFunc();
		AxisZ = accRead();
		sprintf(iotBuffer,"%04x%04x", AxisZ, adcResultwhat);
		LoraTransmit((char *)"200", (char *)iotBuffer);
		uartSend(uart232, (char *)iotBuffer);
		uartSend(uart232, (char *) "\n\n");
		osDelay(10000);
	}
}
osThreadDef(LoraTransmit, osPriorityNormal, 1, 0);

int main (void) {
	
	SysTick_Config(SystemCoreClock/1000); // 1 milisecond SysTick
	
	osKernelInitialize();
	Initialize();
	uart0Initialize();
	
	uartSend(uart232, (char *) "\r\nSystem Initialize\r\n");
	osThreadCreate(osThread(blinky), NULL);
	osThreadCreate(osThread(ReceiveThread), NULL);
	osThreadCreate(osThread(LoraTransmit), NULL);
//	osThreadCreate(osThread(LoraReceive), NULL);
	
	osKernelStart();
	
	return 0;
}
