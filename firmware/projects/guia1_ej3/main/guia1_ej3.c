/*! @mainpage guia1_ej3
 *
 * @section genDesc General Description
 *
 * El programa genera que cuando se presione el Boton 1 el Led 1 se encienda, cuando se presione el
 * Boton 2 el Led 2 se apague y cuando se presionen el Boton 1 y Boton 2 al mismo tiempo el Led 3 
 * parpadee durante un periodo de tiempo.
 *
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
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 100

/**
 * @def leds
 * @brief Struct que almacena los datos correspondiente a un led, los mismos son su modo,
 *  numero de led, numero de ciclos de encendido/apagado y el periodo del ciclo. 
 */
struct leds
{
    uint8_t mode;       //OFF (0), ON (1), TOGGLE(3)
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos; 	//indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;   //indica el tiempo de cada ciclo
} my_leds;


/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/
/** @fn void ledWork(struct leds)
 * @brief Funcion que recibe un struct led y dependiendo el modo del mismo enciende,
 * apaga o hace parpadear el led
 * @param leds struct led que almacena las variables del led.
 * @return
 */
void ledWork(struct leds);

/*==================[external functions definition]==========================*/
void app_main(void)
{
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
	struct leds pLed1, pLed2, pLed3;

	pLed1.n_ciclos = 5;
	pLed1.n_led = 0;
	pLed1.mode = 0;	
	pLed2.n_ciclos = 5;
	pLed2.n_led = 1;
	pLed2.mode = 1;	
	pLed3.n_ciclos = 10;
	pLed3.n_led = 2;
	pLed3.mode = 2;	
	pLed3.periodo = 250;

	struct leds* puntLed = &pLed2;

	while(1)
	{
		teclas = SwitchesRead();
		switch(teclas)
		{
			case SWITCH_1:
				LedsOffAll();
				printf("Led1Task\n");
				ledWork(pLed1);
			break;
			case SWITCH_2:
				LedsOffAll();
				printf("Led2Task\n");
				ledWork(pLed2);
			break;
			case SWITCH_1 | SWITCH_2:
				LedsOffAll();
				printf("Led3Task\n");
				ledWork(pLed3);
			break;
		}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}
}

void ledWork(struct leds pLed)
{
	led_t puntLed = LED_1;
	if(pLed.n_led == 0)
	{
		puntLed = LED_1;
	}
	else if(pLed.n_led == 1)
	{
		puntLed = LED_2;
	}
	else if(pLed.n_led == 2)
	{
		puntLed = LED_3;
	}

	if (pLed.mode == 0)
	{
		LedOn(puntLed);
	}
	else if(pLed.mode == 1)
	{
		LedOff(puntLed);
	}
	else if(pLed.mode == 2)
	{
		for(int i=0; i<( 2 * pLed.n_ciclos );i++)
		{
			LedToggle(puntLed);
			vTaskDelay(pLed.periodo / portTICK_PERIOD_MS);
		}
		LedOff(puntLed);
	}
}
/*==================[end of file]============================================*/