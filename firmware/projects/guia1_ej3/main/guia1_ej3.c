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
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
#define CONFIG_BLINK_PERIOD 1000

struct leds
{
    uint8_t mode;       //OFF (0), ON (1), TOGGLE(3)
	uint8_t n_led;      //indica el número de led a controlar
	uint8_t n_ciclos; 	//indica la cantidad de ciclos de encendido/apagado
	uint16_t periodo;   //indica el tiempo de cada ciclo
} my_leds;
/*==================[internal data definition]===============================*/

/*==================[internal functions declaration]=========================*/

/*==================[external functions definition]==========================*/
void app_main(void){
	uint8_t teclas;
	LedsInit();
	SwitchesInit();
	struct leds sLed;
	sLed.mode = 0;
	sLed.n_led = 0;
    
	while(1)    {
    	teclas  = SwitchesRead();
    	switch(teclas){
    		case SWITCH_1:
				if(sLed.n_led < 2)
				{
					sLed.n_led++;
				}
				else 
				{
					sLed.n_led = 0;
				}
    		break;

    		case SWITCH_2:
				if(sLed.mode < 2)
				{
					sLed.mode++;
				}
				else 
				{
					sLed.mode = 0;
				}
			break;

			case SWITCH_1 & SWITCH_2:
				if(sLed.n_led == 0)
				{
					if(sLed.mode == 0)
					{
						LedOff(LED_1);
					}
					else if(sLed.mode == 1)
					{
						LedOn(LED_1);
					}
					else if(sLed.mode == 2)
					{
						LedToggle(LED_1);
					}
				}
				else if(sLed.n_led == 1)
				{
					if(sLed.mode == 0)
					{
						LedOff(LED_2);
					}
					else if(sLed.mode == 1)
					{
						LedOn(LED_2);
					}
					else if(sLed.mode == 2)
					{
						LedToggle(LED_2);
					}
				}
				else if(sLed.n_led == 2)
				{
					if(sLed.mode == 0)
					{
						LedOff(LED_3);
					}
					else if(sLed.mode == 1)
					{
						LedOn(LED_3);
					}
					else if(sLed.mode == 2)
					{
						LedToggle(LED_3);
					}
				}				
			break;
    	}
		vTaskDelay(CONFIG_BLINK_PERIOD / portTICK_PERIOD_MS);
	}

	
}
/*==================[end of file]============================================*/