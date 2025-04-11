/*! @mainpage guia2_ej1
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
 * | 11/04/2024 | Document creation		                         |
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
#include "led.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/

/*==================[internal data definition]===============================*/
uint16_t distancia = 0;
bool medir = true;
bool hold = false;
TaskHandle_t medir_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
void inits()
{
	HcSr04Init(GPIO_3,GPIO_2);
	LcdItsE0803Init();
}

static void medirDistancia(void *pvParameter)
{
	while(1)
	{
		distancia = HcSr04ReadDistanceInCentimeters();
		printf("Distancia: ");
		printf("%d",distancia);
		printf("\n");
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

static void mostrarDistancia(void *pvParameter)
{	
	//while(hold != true)
	//{
	//	LcdItsE0803Write(distancia);
	//	vTaskDelay(1000 / portTICK_PERIOD_MS);
	//}	
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	inits();
	xTaskCreate(&medirDistancia, "Medir", 512, NULL, 5, &medir_task_handle);
}
/*==================[end of file]============================================*/