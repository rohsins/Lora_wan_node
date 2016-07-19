/** 
		Hardware
*/

#include "cmsis_os.h"
#include "em_device.h"
#include "em_chip.h"
#include "em_cmu.h"
#include "em_gpio.h"
#include "em_usart.h"

USART_TypeDef *uart232 = USART0;
USART_TypeDef *uartLora = UART0;

//RingBuffer uartData;

void Initialize(void) {
	 
//  CHIP_Init();
 
  CMU_OscillatorEnable (cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet   (cmuClock_HF,    cmuSelect_HFXO);
  CMU_ClockDivSet      (cmuClock_HFPER, cmuClkDiv_1);
 
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART0, true);
	CMU_ClockEnable(cmuClock_UART0, true);
 
//  GPIO_PinModeSet(gpioPortE, 4, gpioModePushPull, 0);
//  GPIO_PinModeSet(gpioPortE, 1, gpioModePushPull, 0); 
//  GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0); 
//  GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 0);
//	
//	GPIO_PinModeSet(gpioPortD, 2, gpioModeInput, 0);
//	
//	GPIO_PinModeSet(gpioPortE, 2, gpioModePushPull, 0);
//	GPIO_PinOutSet(gpioPortE,2);
//	GPIO_PinModeSet(gpioPortE, 3, gpioModePushPull, 0);
//	GPIO_PinOutSet(gpioPortE, 3);
//	GPIO_PinModeSet(gpioPortA, 15, gpioModePushPull, 0);
//	GPIO_PinOutSet(gpioPortE, 15);		
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
	USART_InitAsync_TypeDef uartInitTypeDefLora = USART_INITASYNC_DEFAULT;
	
	USART_Enable_TypeDef uartEnableTypeDef = usartEnable;
	
	uartInitTypeDefLora.baudrate = 57600;
	uartInitTypeDef232.baudrate = 115200;
	
	USART_InitAsync(uart232, &uartInitTypeDef232);
	USART_InitAsync(uartLora, &uartInitTypeDefLora);
	
	/* uart232 */
	GPIO_PinModeSet(gpioPortE, 13, gpioModePushPull, 1); 
  GPIO_PinModeSet(gpioPortE, 12, gpioModeInput, 0);
	
	/* uartLoRa */
	GPIO_PinModeSet(gpioPortA, 3, gpioModePushPull, 1); 
  GPIO_PinModeSet(gpioPortA, 4, gpioModeInput, 0);
	
	/* uart232 */
	uart232->ROUTE |= 1 << 0 | 1 << 1 | 1 << 8 | 1 << 9;
	USART_Enable(uart232, uartEnableTypeDef);
	
	/* uartLoRa */
	uartLora->ROUTE |= 1 << 0 | 1 << 1 | 0 << 8 | 1 << 9;
	USART_Enable(uartLora, uartEnableTypeDef);
}

void uartSend(USART_TypeDef *tempType, char data[]) {
	int i = 0;
	while(data[i]) {
		USART_Tx(tempType, data[i]);
		i++;
	}
}

void ReceiveThread(void const *arg) {
	uint8_t temp;
	while (1) {
		temp = USART_Rx(uart232);
		uartSend(uart232, (char*)(&temp));
	}
}
osThreadDef(ReceiveThread, osPriorityNormal, 1, 0);

void ReceiveThread2(void const *arg) {
	uint8_t temp;
	while (1) {
		temp = USART_Rx(uartLora);
		uartSend(uart232, (char *)(&temp));
	}
}
osThreadDef(ReceiveThread2, osPriorityNormal, 1, 0);

void LoraInitialize(void) {
	uartSend(uartLora, "mac get \r\n");
}

void RunThread(void const *argument) {
	while (1) {
		LoraInitialize();
		osDelay(1000);
	}
}
osThreadDef(RunThread, osPriorityNormal, 1, 0);

int main (void) {
	
  SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000); // 1 milisecond SysTick
	
	osKernelInitialize();
	Initialize();
	uart0Initialize();
	
	osDelay(1000);
	uartSend(uart232, "System Initialize");
	
	osThreadCreate(osThread(blinky), NULL);
//	osThreadCreate(osThread(ReceiveThread), NULL);
	osThreadCreate(osThread(ReceiveThread2), NULL);
	osThreadCreate(osThread(RunThread), NULL);
	
	osKernelStart();
	
	return 0;
}
