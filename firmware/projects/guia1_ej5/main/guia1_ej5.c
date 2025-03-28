/*! @mainpage Template
 *
 * @section genDesc General Description
 *
 * This section describes how the program works.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	PIN_X	 	| 	GPIO_X		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 12/09/2023 | Document creation		                         |
 *
 * @author Albano Peñalva (albano.penalva@uner.edu.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gpio_mcu.h"
/*==================[macros and definitions]=================================*/
typedef struct
{
	gpio_t pin; /*!< GPIO pin number */
	io_t dir;	/*!< GPIO direction '0' IN;  '1' OUT*/
} gpioConf_t;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t convertToBcdArray(uint32_t data, uint8_t digits, uint8_t *bcd_number)
{
	digits--;
	for (int i = 0; i <= digits; i++)
	{
		bcd_number[digits - i] = data % 10;
		data = data / 10;
	}
	return 0;
}

void activateGPIO(uint8_t bcd_number, gpioConf_t *gpioBCD)
{
	for (int i = 0; i < 4; i++)
	{
		if (bcd_number & 1 << i)
		{
			GPIOOn(gpioBCD[i].pin);
		}
		else
		{
			GPIOOff(gpioBCD[i].pin);
		}
	}
}

void showNumbers(uint32_t data, uint8_t digits, uint8_t *bcd_number, gpioConf_t *gpioBCD, gpioConf_t *gpioPOS)
{
	convertToBcdArray(data, digits, bcd_number);
	for (int i = 0; i < 3; i++)
	{
		activateGPIO(bcd_number[i], gpioBCD);
		GPIOOn(gpioPOS[i].pin);
		GPIOOff(gpioPOS[i].pin);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t numBCD[3] = {0};
	uint32_t num32B = 666;
	uint8_t digitos = 3;

	gpioConf_t GPIOBCD[5] = {
		{GPIO_20, GPIO_OUTPUT},
		{GPIO_21, GPIO_OUTPUT},
		{GPIO_22, GPIO_OUTPUT},
		{GPIO_23, GPIO_OUTPUT},
	};

	gpioConf_t GPIOPOS[3] = {
		{GPIO_19, GPIO_OUTPUT},
		{GPIO_18, GPIO_OUTPUT},
		{GPIO_9, GPIO_OUTPUT},
	};

	for (int i = 0; i < 4; i++)
	{
		GPIOInit(GPIOBCD[i].pin, GPIOBCD[i].dir);
	}

	for (int i = 0; i < 3; i++)
	{
		GPIOInit(GPIOPOS[i].pin, GPIOPOS[i].dir);
	}

	showNumbers(num32B,digitos,&numBCD,&GPIOBCD,&GPIOPOS);
	//	convertToBcdArray(num32B, digitos, &numBCD);

	//	activateGPIO(numBCD[0], GPIOBCD);
}
/*==================[end of file]============================================*/