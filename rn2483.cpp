/** 
		Hardware
*/

#include "rn2483.h"

USART_TypeDef *uartLora = UART0;

void LoraConfig(void) {
	LoRa wlora;
//	char incoming[32];
	
	wlora.uartLoraInitialize();
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
	wlora.mac.set(adr, on); //enable the adaptive data rate(ADR)
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
	
	osDelay(2000);
	
//	while (1) {
////		wlora.mac.tx(uncnf, (char *)"200", (char *)"I");
//		LoraTransmit((char *)"200", (char *)test);
//		osDelay(3000);
//	}
}

void LoraTransmit(char* port,char *arg) {
	LoRa wlora;
	wlora.mac.tx(cnf, port, arg);
}

//void LoraReceive(void const *arg) {
//	LoRa wlora;
//	
//	wlora.radio.set(mod, lora);
////	wlora.radio.set(freq, (char *)"865295000");
//	wlora.radio.set(freq, (char *)"867500000");
////	wlora.radio.set(freq, (char *)"868500000");
//	wlora.radio.set(pwr, (char *)"14");
//	wlora.radio.set(sf, (char *)"sf12");
//	wlora.radio.set(afcbw, (char *)"125");
//	wlora.radio.set(rxbw, (char *)"125");
//	wlora.radio.set(fdev, (char *)"5000");
//	wlora.radio.set(prlen, (char *)"8");
//	wlora.radio.set(crc, on);
//	wlora.radio.set(cr, (char *)"4/7");
//	wlora.radio.set(wdt, (char *)"0");
//	wlora.radio.set(sync, (char *)"12");
//	wlora.radio.set(bw, (char *)"250");
//	
//	wlora.mac.pause();
//	
//	while (1) {
//		wlora.radio.rx((char *)"0");
//		osDelay(1000);
//		wlora.radio.get(snr);
//		osDelay(1000);
//	}
//}
//osThreadDef(LoraReceive, osPriorityNormal, 1, 0);
