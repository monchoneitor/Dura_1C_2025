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
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
int8_t  convertToBcdArray (uint32_t data, uint8_t digits, uint8_t * bcd_number){
	digits--;
	for(int i=0; i<=digits; i++)
	{
		bcd_number[digits-i] = data%10;
		data = data/10;
	}
	return 0;
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t numBCD [3] = {0};
	uint32_t num32B = 123;
	uint8_t digitos = 3;

	convertToBcdArray( num32B, digitos, &numBCD);
	printf("%d",numBCD[0]);
	printf("%d",numBCD[1]);
	printf("%d",numBCD[2]);

}
/*==================[end of file]============================================*/