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

USART_TypeDef *uart232 = USART0;
USART_TypeDef *uartLora = UART0;

float adcResult = 404.0;
char iotBuffer[16];
uint32_t adcResultwhat = 0;

void Initialize(void) {
  CMU_OscillatorEnable (cmuOsc_HFXO, true, true);
  CMU_ClockSelectSet   (cmuClock_HF,    cmuSelect_HFXO);
  CMU_ClockDivSet      (cmuClock_HFPER, cmuClkDiv_1);
 
  CMU_ClockEnable(cmuClock_HFPER, true);
  CMU_ClockEnable(cmuClock_GPIO, true);
  CMU_ClockEnable(cmuClock_USART0, true);
	CMU_ClockEnable(cmuClock_UART0, true);
	CMU_ClockEnable(cmuClock_ADC0, true);		
}

void adcFunc(void) {	
	
	ADC0->CTRL = (24 << 16) | (4 << 8);
	ADC0->IEN = 0;
	ADC0->CMD = 1 << 0;
	ADC0->SINGLECTRL = (2 << 16) | (4 << 8) | (0 << 4); // VDD reference, adcn_ch4, 12bit resolution
	
	while (! (ADC0->STATUS & ( 1 << 16 )));
		adcResult = ADC0->SINGLEDATA;
		ADC0->CMD = 1 << 0;
	  adcResult = (((adcResult/4095)*(5*3.3)/3)*1000);
	  adcResultwhat = adcResult;
	  sprintf(iotBuffer,"%x",adcResultwhat);
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
//		USART_Tx(uart232, data[i]); //for debug
		i++;
	}
}

//void ReceiveThread(void const *arg) {
//	uint8_t temp;
//	while (1) {
//		temp = USART_Rx(uart232);
//		uartSend(uart232, (char*)(&temp));
//	}
//}
//osThreadDef(ReceiveThread, osPriorityNormal, 1, 0);

void ReceiveThread2(void const *arg) {
	uint8_t temp;
	while (1) {
		temp = USART_Rx(uartLora);
		uartSend(uart232, (char *)(&temp));
	}
}
osThreadDef(ReceiveThread2, osPriorityNormal, 1, 0);

void LoraTransmit(void const *arg) {
	LoRa wlora;
	
	char incoming[32];
	
	wlora.sys.reset();
	
	osDelay(1000);
//	wlora.mac.set(devaddr, DEVADDR);
//	wlora.mac.set(deveui, DEVEUI);
//	wlora.mac.set(appeui, APPEUI);
//	wlora.mac.set(nwkskey, NWKSKEY);
//	wlora.mac.set(appskey, APPSKEY);
//	wlora.mac.set(appkey, APPKEY);
	
	wlora.mac.set(devaddr, DEVADDR2);
	wlora.mac.set(deveui, DEVEUI2);
	wlora.mac.set(appeui, APPEUI2);
	wlora.mac.set(nwkskey, NWKSKEY2);
	wlora.mac.set(appskey, APPSKEY2);
	wlora.mac.set(appkey, APPKEY2);
	
	wlora.mac.set(pwridx, (char *)"1"); // set the tx output power to 14dBm
	wlora.mac.set(dr, (char *)"5"); //datarate 125kHz
	wlora.mac.set(adr, on); //enable the adaptive data rate(ADR)
	wlora.mac.set(bat, (char *)"127"); // battery is set to 50 %
	wlora.mac.set(retx, (char *)"1"); // the number of retransmission made for a an uplink confirmed packet is set to 1
	wlora.mac.set(linkchk , (char *)"100"); // the module will attempt a link check process at 100 second intervals
	wlora.mac.set(rxdelay1, (char *) "1000"); //set the delay between the transmission and the first Receive window to 1000 ms.
	wlora.mac.set(ar, on); //enables the automatic reply process inside the module
	wlora.mac.set(rx2,(char *) "3", (char *) "868500000"); //receive window 2 is configured with sf9/125kHz data rate with a center frequency of 865MHz.
//	wlora.mac.set(ch, freq, (char *) "8", (char *) "864000000"); // define frequency for channel 8 to be 864.
//	wlora.mac.set(ch, dcycle, (char *) "8", (char *) "9"); // defines duty cycle for channel 8 to be 10%. Since(100/10) -1 = 9, the parameter that gets configured is 9.
//	wlora.mac.set(ch, drrange, (char *) "8", (char *) "0", (char *) "2"); // on channel 8 the data rate can range from 0 (sf12/125kHz) to 2 (sf10/125) as required.
	wlora.mac.save();
	
	osDelay(3000);
	
	wlora.mac.get(devaddr);
	wlora.mac.get(deveui);
	wlora.mac.get(appeui);
	osDelay(1000);
	
	wlora.mac.join(abp);

	while (1) {
//		wlora.mac.rx((char *)"214", (char *)"AC");
		adcFunc();
		osDelay(10000);
		wlora.mac.tx(uncnf, (char *)"218", (char *)iotBuffer);
		
	}
}
osThreadDef(LoraTransmit, osPriorityNormal, 1, 0);

void LoraReceive(void const *arg) {
	LoRa wlora;
	
	wlora.radio.set(mod, lora);
//	wlora.radio.set(freq, (char *)"865295000");
	wlora.radio.set(freq, (char *)"867500000");
//	wlora.radio.set(freq, (char *)"868500000");
	wlora.radio.set(pwr, (char *)"14");
	wlora.radio.set(sf, (char *)"sf12");
	wlora.radio.set(afcbw, (char *)"125");
	wlora.radio.set(rxbw, (char *)"125");
	wlora.radio.set(fdev, (char *)"5000");
	wlora.radio.set(prlen, (char *)"8");
	wlora.radio.set(crc, on);
	wlora.radio.set(cr, (char *)"4/7");
	wlora.radio.set(wdt, (char *)"0");
	wlora.radio.set(sync, (char *)"12");
	wlora.radio.set(bw, (char *)"250");
	
	wlora.mac.pause();
	
	while (1) {
		wlora.radio.rx((char *)"0");
		osDelay(1000);
		uartSend(uart232, (char *)"snr: ");
		wlora.radio.get(snr);
		osDelay(1000);
	}
}
osThreadDef(LoraReceive, osPriorityNormal, 1, 0);

void RunThread(void const *argument) {	
//	LoraInitialize();
//	LoraConfiguration();
//	while (1) {
//	}
}
osThreadDef(RunThread, osPriorityNormal, 1, 0);

int main (void) {
	
  SystemCoreClockUpdate();
	SysTick_Config(SystemCoreClock/1000); // 1 milisecond SysTick
	
	osKernelInitialize();
	Initialize();
	uart0Initialize();
	
	osDelay(1000);
	
	uartSend(uart232, (char *) "\r\nSystem Initialize\r\n");
	osThreadCreate(osThread(blinky), NULL);
//	osThreadCreate(osThread(ReceiveThread), NULL);
	osThreadCreate(osThread(ReceiveThread2), NULL);
//	osThreadCreate(osThread(RunThread), NULL);
	
	osThreadCreate(osThread(LoraTransmit), NULL);
//	osThreadCreate(osThread(LoraReceive), NULL);
	
	osKernelStart();
	
	return 0;
}
