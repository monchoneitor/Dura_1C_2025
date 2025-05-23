/*! @mainpage guia2_ej2
 *
 * @section genDesc General Description
 *
 * Este programa recibe de un dispositivo emisor de ultrasonido HC_SR04 el valor de distancia hasta la pared mas cercana donde
 * rebota la onda emitida y posteriormente es recibida por el dispositio transformando el delay de llegada en un valor de
 * distancia en cm. En este caso el programa realizará sus acciones por medio de tareas e interrupciones.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	GPIO_3	 	| 	ECHO		|
 * | 	GPIO_2	 	| 	TRIGGER		|
 * | 	D1		 	| 	GPIO_20		|
 * | 	D2		 	| 	GPIO_21		|
 * | 	D3		 	| 	GPIO_22		|
 * | 	D4		 	| 	GPIO_23		|
 * | 	SEL_1	 	| 	GPIO_19		|
 * | 	SEL_2	 	| 	GPIO_18		|
 * | 	SEL_3	 	| 	GPIO_9		|
 * | 	+5V	 	| 	+5V		|
 * | 	GND	 	| 	GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 25/04/2024 | Document creation		                         |
 * | 25/04/2024 | Se completan los ejercicios                    |
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
/* Entero de 16 bits donde se almacenará la distancia obtenida por el HC_SR04
 */
uint16_t distancia = 0;
/* Booliano que almacena true si se actualiza la medicion o false si no se actualiza.
 */
bool medir = true;
/* Booliano que almacena false si se actualiza la medicion en el display o true si no se actualiza
 * la medicion en el display.
 */
bool hold = false;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn void inits()
 * @brief Funcion que inicializa todos los dispositivos necesarios para el funcionamiento del programa
 * exceptuando el timer.
 * @return
 */
void inits()
{
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	LedsInit();
	SwitchesInit();
	SwitchActivInt(SWITCH_1, &funcionTecla1, NULL);
	SwitchActivInt(SWITCH_2, &funcionTecla2, NULL);
}
/** @fn static void funcionTimerA(void *pvParameter)
 * @brief Funcion que contiene el código que se ejecuta cada vez que se actualiza el timer A,
 * el cual envia una notificación para realizar tareas.
 * @return
 */
static void funcionTimerA(void *pvParameter)
{
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}
/** @fn static void medirDistancia(void *pvParameter)
 * @brief Funcion que actualiza el valor de medición y envia una notificación a la funcion
 * mostrarDistancia().
 * @return
 */
static void medirDistancia(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskNotifyGiveFromISR(mostrar_task_handle, pdFALSE);
	}
}
/** @fn static void mostrarDistancia(void *pvParameter)
 * @brief Funcion que actualiza el valor de medición mostrado en el display lcd y enciende los leds
 * @return
 */
static void mostrarDistancia(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if (!hold)
		{
			LcdItsE0803Write(distancia);
			if (distancia < 10)
			{
				LedsOffAll();
			}
			else if (distancia > 10 && distancia < 20)
			{
				LedsOffAll();
				LedOn(LED_1);
			}
			else if (distancia > 20 && distancia < 30)
			{
				LedsOffAll();
				LedOn(LED_1);
				LedOn(LED_2);
			}
			else if (distancia > 30)
			{
				LedOn(LED_1);
				LedOn(LED_2);
				LedOn(LED_3);
			}
		}
	}
}
/** @fn static void funcionTecla1(void *pvParameter)
 * @brief Funcion que contiene la acción a ejecutar en caso de presionar el boton 1
 * @return
 */
static void funcionTecla1(void *pvParameter)
{
	medir = !medir;
}
/** @fn static void funcionTecla2(void *pvParameter)
 * @brief Funcion que contiene la acción a ejecutar en caso de presionar el boton 2
 * @return
 */
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
		.param_p = NULL};
	TimerInit(&timer_led_1);

	xTaskCreate(&medirDistancia, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &mostrar_task_handle);

	TimerStart(timer_led_1.timer);
}
/*==================[end of file]============================================*/