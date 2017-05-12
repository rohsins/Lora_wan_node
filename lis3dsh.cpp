#include "lis3dsh.h"

USART_TypeDef *usartAcc = USART2;

//uint8_t accRead(uint8_t regRead) {
//	uint8_t temp = 0;
//	USART_Tx(usartAcc, (regRead | 0x80));
//	USART_Rx(usartAcc);
//	USART_Tx(usartAcc, 0x00);
//	USART_Tx(usartAcc, (regRead | 0x80));
//	USART_Rx(usartAcc);
//	USART_Tx(usartAcc, 0x00);
//	USART_Tx(usartAcc, (regRead | 0x80));
//	USART_Rx(usartAcc);
//	USART_Tx(usartAcc, 0x00);
//	USART_Tx(usartAcc, (regRead | 0x80));
//	temp = USART_Rx(usartAcc);
//	USART_Tx(usartAcc, 0x00);
//	osDelay(1);
//	return temp;
//}

uint8_t accRead(uint8_t regRead) {
	uint8_t temp = 0;
	USART_Tx(usartAcc, (regRead | 0x80));
	USART_Tx(usartAcc, 0x00);
	USART_Rx(usartAcc);
	USART_Tx(usartAcc, (regRead | 0x80));
	USART_Tx(usartAcc, 0x00);
	USART_Rx(usartAcc);
	USART_Tx(usartAcc, (regRead | 0x80));
	USART_Tx(usartAcc, 0x00);
	USART_Rx(usartAcc);
	USART_Tx(usartAcc, (regRead | 0x80));
	USART_Tx(usartAcc, 0x00);
	temp = USART_Rx(usartAcc);
	osDelay(1);
	return temp;
}

void accWrite(uint8_t regWrite, uint8_t regValue) {
	USART_Tx(usartAcc, regWrite);
	USART_Tx(usartAcc, regValue);
	osDelay(1);
}

void accConfig(void) {
	accWrite(0x20, 0x7F); //enable bdu, zen, data rate 400Hz
	accWrite(0x24, 0x80); //anti-aliasing filter 400Hz
//	accWrite(0x23, 0x88); //enable dataready interrupt
	accWrite(0x21, 0x00);
	accWrite(0x22, 0x00);
	accWrite(0x23, 0x00);
}

void accInitialize(void) {
	
	USART_InitSync_TypeDef usartInitTypeDefAcc = USART_INITSYNC_DEFAULT;
	
	usartInitTypeDefAcc.clockMode = usartClockMode3;
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

int accRead(void) {
	int8_t axisZH;
	uint8_t axisZL;
	int16_t axisZ;
	
	int whoami = accRead(0x0F);
	
	if (whoami == 0x3F) {
		axisZL = accRead(0x2C);
		axisZH = accRead(0x2D);
		axisZ = axisZH  << 8 | axisZL;
		return axisZ;
	} else {
		return whoami;
	}
}