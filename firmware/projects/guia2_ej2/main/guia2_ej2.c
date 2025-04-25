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
 * | 	GPIO_3	 	| 	ECHO		|
 * | 	GPIO_2	 	| 	TRIGGER		|
 * | 	+5V	 	| 	+5V		|
 * | 	GND	 	| 	GND		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/04/2024 | Document creation		                         |
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
#include "switch.h"
#include "timer_mcu.h"
#include "hc_sr04.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_REFRESH_PERIOD_DISTANCE_US 1000000
/*==================[internal data definition]===============================*/
uint16_t distancia = 0;
bool medir = true;
bool hold = false;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;

/*==================[internal functions declaration]=========================*/
void inits()
{
	HcSr04Init(GPIO_3,GPIO_2);
	LcdItsE0803Init();
	LedsInit();
	SwitchesInit();
	SwitchActivInt(SWITCH_1, &funcionTecla1, NULL);
	SwitchActivInt(SWITCH_2, &funcionTecla2, NULL);
}

static void funcionTimerA(void *pvParameter)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

static void medirDistancia(void *pvParameter)
{
	while(1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
	}
}

static void mostrarDistancia(void *pvParameter)
{	
	while(1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(!hold)
		{
			LcdItsE0803Write(distancia);	
		}
		if(distancia < 10)
		{
			LedsOffAll();
		}
			else if(distancia > 10 && distancia < 20)
			{
				LedsOffAll();
				LedOn(LED_1);
			}	
			else if(distancia > 20 && distancia < 30)
			{
				LedsOffAll();
				LedOn(LED_1);
				LedOn(LED_2);
			}	
			else if(distancia > 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
		}		
	}
}

static void funcionTecla1(void *pvParameter)
{
	medir = !medir;
}

static void funcionTecla2(void *pvParameter)
{
	hold = !hold;
}
/*==================[external functions definition]==========================*/
void app_main(void)
{
	inits();
	
	timer_config_t timer_led_1 = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH_PERIOD_DISTANCE_US,
        .func_p = funcionTimerA,
        .param_p = NULL
    };
    TimerInit(&timer_led_1);

	xTaskCreate(&medirDistancia, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
	
	TimerStart(timer_led_1.timer);
}
/*==================[end of file]============================================*/