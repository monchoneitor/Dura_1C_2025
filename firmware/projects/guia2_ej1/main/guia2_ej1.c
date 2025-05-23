/*! @mainpage guia2_ej1
 *
 * @section genDesc General Description
 *
 * Este programa recibe de un dispositivo emisor de ultrasonido HC_SR04 el valor de distancia hasta la pared mas cercana donde
 * rebota la onda emitida y posteriormente es recibida por el dispositio transformando el delay de llegada en un valor de
 * distancia en cm.
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
 * | 11/04/2024 | Document creation		                         |
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
#include "hc_sr04.h"
#include "lcditse0803.h"
/*==================[macros and definitions]=================================*/

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
/* Entero de 8 bits que se utilizará para reconocer el accionamiento de los botones.
 */
uint8_t teclas;
TaskHandle_t medir_task_handle = NULL;
TaskHandle_t mostrar_task_handle = NULL;
TaskHandle_t teclas_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn void inits()
 * @brief Funcion que inicializa todos los dispositivos necesarios para el funcionamiento del programa
 * @return
 */
void inits()
{
	HcSr04Init(GPIO_3, GPIO_2);
	LcdItsE0803Init();
	LedsInit();
	SwitchesInit();
}

/** @fn static void medirDistancia(void *pvParameter)
 * @brief Funcion que actualiza el valor de medición
 * @return
 */
static void medirDistancia(void *pvParameter)
{
	while (1)
	{
		if (medir)
		{
			distancia = HcSr04ReadDistanceInCentimeters();
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}
/** @fn static void mostrarDistancia(void *pvParameter)
 * @brief Funcion que actualiza el valor de medición mostrado en el display lcd y enciende
 * @return
 */
static void mostrarDistancia(void *pvParameter)
{
	while (1)
	{
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
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/** @fn static void funcionTeclas(void *pvParameter)
 * @brief Funcion que reconoce la pulsación de las teclas y modifica variables: en caso de presionar el
 * boton 1 se cambia modifica el valor del booliano medir al inverso y en caso de presionar el boton 2
 * se modifica el valor del booliano hold al inverso.
 * @return
 */
static void funcionTeclas(void *pvParameter)
{
	while (1)
	{
		teclas = SwitchesRead();
		switch (teclas)
		{
		case (SWITCH_1):
			medir = !medir;
			break;
		case (SWITCH_2):
			hold = !hold;
			break;
		}
		vTaskDelay(1000 / portTICK_PERIOD_MS);
	}
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	inits();
	xTaskCreate(&medirDistancia, "Medir", 2048, NULL, 5, &medir_task_handle);
	xTaskCreate(&mostrarDistancia, "Mostrar", 512, NULL, 5, &mostrar_task_handle);
	xTaskCreate(&funcionTeclas, "Teclas", 512, NULL, 5, &teclas_task_handle);
}
/*==================[end of file]============================================*/