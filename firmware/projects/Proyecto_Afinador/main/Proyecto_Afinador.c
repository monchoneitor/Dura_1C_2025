/*! @mainpage Proyecto Afinador
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
 * | 23/05/2025 | Document creation		                         |
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
#include "iir_filter.h"
#include "fft.h"
/*==================[macros and definitions]=================================*/
#define SAMPLE_FREQ	200
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	
}
/*==================[end of file]============================================*/