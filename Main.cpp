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
USART_TypeDef *usartAcc = USART2;

float adcResult = 404.0;
char iotBuffer[16];
char transitory[32];
uint32_t adcResultwhat = 0;
int16_t axisZ;
int16_t axisY;
int whoami;
float zGravity;
float yGravity;
float xGravity;

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

uint8_t accRead(uint8_t regRead) {
	uint8_t temp = 0;
	USART_Tx(usartAcc, (regRead | 0x80));
	USART_Tx(usartAcc, 0x00);
	temp = USART_Rx(usartAcc);
	osDelay(1);
	return temp;
}

uint8_t accWrite(uint8_t regWrite, uint8_t regValue) {
	uint8_t temp = 0;
	USART_Tx(usartAcc, regWrite);
	USART_Tx(usartAcc, regValue);
	temp = USART_Rx(usartAcc);
	osDelay(1);
	return temp;
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

void accConfig(void) {
	accWrite(0x20, 0x7F); //enable bdu, zen, data rate 400Hz
	accWrite(0x24, 0x80); //anti-aliasing filter 400Hz
//	accWrite(0x23, 0x88); //enable dataready interrupt
	accWrite(0x21, 0x00);
	accWrite(0x22, 0x00);
	accWrite(0x23, 0x00);
}

void spiInitialize(void) {
	
	USART_InitSync_TypeDef usartInitTypeDefAcc = USART_INITSYNC_DEFAULT;
	
	usartInitTypeDefAcc.clockMode = usartClockMode0;
	usartInitTypeDefAcc.msbf = true;
	
	usartInitTypeDefAcc.baudrate = 400000;
	usartInitTypeDefAcc.databits = usartDatabits8;
	
	USART_Enable_TypeDef usartEnableTypeDef = usartEnable;
		
	USART_InitSync(usartAcc, &usartInitTypeDefAcc);
	
	/* usartAcc */
	GPIO_PinModeSet(gpioPortB, 3, gpioModePushPull, 1); /*MOSI*/
  GPIO_PinModeSet(gpioPortB, 4, gpioModeInput, 0); /*MISO*/
	GPIO_PinModeSet(gpioPortB, 5, gpioModePushPull, 0); /*CLOCK*/
  GPIO_PinModeSet(gpioPortB, 6, gpioModePushPull, 1); /*CS*/
	
	/* usartAcc */
	usartAcc->ROUTE |= 1 << 0 | 1 << 1 | 1 << 2 | 1 << 3 | 1 << 8 | 0 << 9;
	usartAcc->CTRL |= USART_CTRL_AUTOCS;
	
	USART_Enable(usartAcc, usartEnableTypeDef);
	
	accConfig();
}

void uartSend(USART_TypeDef *tempType, char data[]) {
	int i = 0;
	while(data[i]) {
		USART_Tx(tempType, data[i]);
//		USART_Tx(uart232, data[i]); //for debug
		i++;
	}
}

void spi_thread(void const* arg) {
	uint8_t spiRead;
	while(1) {
		USART_Tx(usartAcc, 0x8F);
		USART_Tx(usartAcc, 0x00);
		spiRead = USART_Rx(usartAcc);
		USART_Tx(uart232, spiRead); //for debug
		osDelay(2900);
	}
}
osThreadDef(spi_thread, osPriorityNormal, 1, 0);

void ReceiveThread(void const *arg) {
	uint8_t temp;
	while (1) {
		temp = USART_Rx(uartLora);
		uartSend(uart232, (char *)(&temp));
	}
}
osThreadDef(ReceiveThread, osPriorityNormal, 1, 0);

void LoraTransmit(void const *arg) {
	LoRa wlora;
	
	char incoming[32];
	
	wlora.sys.reset();
	
	osDelay(1000);
	wlora.mac.set(devaddr, DEVADDR);
	wlora.mac.set(deveui, DEVEUI);
	wlora.mac.set(appeui, APPEUI);
	wlora.mac.set(nwkskey, NWKSKEY);
	wlora.mac.set(appskey, APPSKEY);
	wlora.mac.set(appkey, APPKEY);
	
//	wlora.mac.set(devaddr, DEVADDR2);
//	wlora.mac.set(deveui, DEVEUI2);
//	wlora.mac.set(appeui, APPEUI2);
//	wlora.mac.set(nwkskey, NWKSKEY2);
//	wlora.mac.set(appskey, APPSKEY2);
//	wlora.mac.set(appkey, APPKEY2);
	
	wlora.mac.set(pwridx, (char *)"1"); // set the tx output power to 14dBm
	wlora.mac.set(dr, (char *)"5"); //datarate 125kHz
	wlora.mac.set(adr, on); //disable the adaptive data rate(ADR)
	wlora.mac.set(bat, (char *)"127"); // battery is set to 50 %
	wlora.mac.set(retx, (char *)"1"); // the number of retransmission made for a an uplink confirmed packet is set to 1
	wlora.mac.set(linkchk , (char *)"1000"); // the module will attempt a link check process at 1000 second intervals
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
		adcFunc();
		wlora.mac.tx(uncnf, (char *)"200", (char *)iotBuffer);
		uartSend(uart232, (char *)iotBuffer);
		uartSend(uart232, (char *) "\n\n");
		osDelay(10123);
	}

	GPIO_PinModeSet(gpioPortA, 0, gpioModeInput, 1);
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
//	uint16_t axisZ;
	int8_t axisZH;
	uint8_t axisZL;
	int8_t axisYH;
	uint8_t axisYL;
	int8_t axisXH;
	uint8_t axisXL;
//	accWrite(0x20, 0x17);
	whoami = accRead(0x0F);
	while (1) {	
		
		axisYL = accRead(0x2A);
		axisYH = accRead(0x2B);
		
		axisZL = accRead(0x2C);
		axisZH = accRead(0x2D);
		
		axisZ = axisZH  << 8 | axisZL;
		axisY = axisYH  << 8 | axisYL;
		
		zGravity = ((axisZ*2.0)/32768.0);
		yGravity = ((axisY*2.0)/32768.0);
		
		sprintf(transitory,"z-axis data: %d \n",axisZ);
		uartSend(uart232, transitory);
		sprintf(transitory,"y-axis data: %d \n\n",axisY);
		uartSend(uart232, transitory);
		
		osDelay(1000);
	}
}
osThreadDef(RunThread, osPriorityNormal, 1, 0);

int main (void) {
	
	SysTick_Config(SystemCoreClock/1000); // 1 milisecond SysTick
	
	osKernelInitialize();
	Initialize();
	uart0Initialize();
	spiInitialize();
	
	osDelay(1000);
	
	uartSend(uart232, (char *) "\r\nSystem Initialize\r\n");
	osThreadCreate(osThread(blinky), NULL);
//	osThreadCreate(osThread(ReceiveThread), NULL);
	osDelay(3000);
	osThreadCreate(osThread(RunThread), NULL);
//	osThreadCreate(osThread(spi_thread), NULL);
	
//	osThreadCreate(osThread(LoraTransmit), NULL);
//	osThreadCreate(osThread(LoraReceive), NULL);
	
	osKernelStart();
	
	return 0;
}
