/*! @mainpage guia1_ej5
 *
 * @section genDesc General Description
 *
 * Este programa a partir de un valor numerico de 32 bits definido en el main lo modifica en un arreglo de numeros
 * BCD y lo levanta en un display LCD *
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	D1		 	| 	GPIO_20		|
 * | 	D2		 	| 	GPIO_21		|
 * | 	D3		 	| 	GPIO_22		|
 * | 	D4		 	| 	GPIO_23		|
 * | 	SEL_1	 	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V		 	| 	+5V			|
 * | 	GND		 	| 	GND			|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 04/05/2024 | Document creation		                         |
 * | 04/05/2024 | Se crearon las funciones para convertir el 	 |
 * |			| numero de 32 bits a un arreglo BCD y para 	 |	
 * |			| levantar el mismo en el display LCD			 |
 *
 * @author Simon Pedro Dura (sipedura@gmail.com)
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
/** @fn int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number)
 * @brief Funcion que recibe un entero de 32 bits y lo transforma en un numero BCD de 8 bits almacenado cifra a cifra
 * en un array
 * @param data Valor uint32_t que contiene el entero de 32 bits
 * @param digits Valor uint8_t que almacena la cantidad de digitos del entero de 32 bits
 * @param bcd_number Puntero a un arreglo de valores uint8_t donde se almacenara el numero BCD
 * @return 0
 */
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

/** @fn void activateGPIO(uint8_t bcd_number, gpioConf_t *gpioBCD
 * @brief Funcion que recibe un numero en formato BCD de 8 bits y modifica los pines del GPIO en alto y bajo  
 * dependiendo del valor BCD
 * @param bcd_number Valor uint8_t que almacena el numero BCD
 * @param gpioBCD Puntero gpioConf_t que contiene la direccion de memoria donde se almacenan los pines a poner en alto o bajo
 * @return 
 */
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

/** @fn void showNumbers(uint32_t data, uint8_t digits, uint8_t *bcd_number, gpioConf_t *gpioBCD, gpioConf_t *gpioPOS)
 * @brief Funcion que levanta en el display un numero de 32 bits que fue transformado a BCD
 * @param data Valor uint32_t que alamacena el numero de 32 bits
 * @param digits Valor uint8_t que almacena la cantidad de digitos del numero de 32 bits
 * @param bcd_number Puntero uint8_t que almacena la direccion de memoria del numero BCD
 * @param gpioBCD Puntero gpioConf_t que contiene la direccion de memoria donde se almacenan los pines a poner en 
 * alto o bajo
 * @param gpioPOS Puntero gpioConf_t que contiene la dirección de memoria de los pines que eligen la seccion del LCD
 * @return 
 */
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
	uint32_t num32B = 135;
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
}
/*==================[end of file]============================================*/