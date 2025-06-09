/*! @mainpage Proyecto Estacion de monitoreo.
 *
 * @section genDesc Descripcion General
 *
 * Este programa trabaja como una estación de monitoreo de temperatura, humedad y radiación. El mismo obtiene
 * los datos de temperatura y humedad a traves de un dispositivo DHT11 y la radiación a traves de un sensor
 * que devuelve un valor de tension entre 0V y 3,3V dependiendo de la intensidad de la radiación avisando si 
 * puede ocurrir una nevada o la radiación es elevada. El programa tiene un botón de encendido y apagado el 
 * cual activa o desactiva las funciones mencionadas.
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	DHT11	 	| 	GPIO_20		|
 * | 	Entrada Analogica	 	| 	GPIO_1		|
 * | 	DHT11	 	| 	GPIO_20		|
 * | 	+5V	 	| 	+5V		|
 * | 	GND	 	| 	GND		|
 * 
 * 
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/06/2025 | Document creation		                         |
 * | 		    | Se agregan las funciones que miden las variables|
 * | 		    | y las interrupciones que permiten que los botones|
 * | 		    | inicien o detengan el programa.				|
 * 
 * @author Simon Pedro Dura (sipedura@gmail.ar)
 *
 */

/*==================[inclusions]=============================================*/
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
#include "dht11.h"
#include "led.h"
#include "switch.h"
/*==================[macros and definitions]=================================*/
/** @def CONFIG_REFRESH_PERIOD_DISTANCE_US
 * @brief Valor del periodo de refresco del timer A (1 segundo).
 */
#define CONFIG_REFRESH_PERIOD_DISTANCE_US 1000000
/*==================[internal data definition]===============================*/
/* Entero que se utilizará para contar 5 segundos y reiniciarse.
 */
int cont5s = 0;
/* Entero que se utilizará para almacenar el valor de humedad.
 */
float humedad = 0;
/* Entero que se utilizará para almacenar el valor de temperatura.
 */
float temperatura = 0;
/* Entero que se utilizará para almacenar el valor de radiacion.
 */
float radiacion = 0;
/* Entero que se utilizará para almacenar el valor de tension obtenido del medidor de radiacion.
 */
float tension = 0;
/* Declaración de la funcionTimerA, que ejecutará el mismo
*/
void funcionTimerA();
/* timer_config_t que posee la configuración del timer A a utilizar en la aplicacion.
 */
 timer_config_t timerA = {
	.timer = TIMER_A,
	.period = CONFIG_REFRESH_PERIOD_DISTANCE_US,
	.func_p = funcionTimerA,
	.param_p = NULL};
/* TaskHandle_t que se utilizará para notificar la tarea medir.
 */
TaskHandle_t medir_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn void funcionTimerA()
 * @brief Funcion que contiene el código que se ejecuta cada vez que se actualiza el timer A,
 * el cual envia una notificación para realizar tareas y aumenta en uno el contador.
 * @return
 */
void funcionTimerA()
{
	cont5s++;
	vTaskNotifyGiveFromISR(medir_task_handle, pdFALSE);
}

/** @fn  void enviarVariables()
 * @brief Funcion que envia los mensajes que contienen los valores de temperatura, humedad y radiación junto
 * con una alerta en caso de que exista alerta de peligro de nevada o radiación elevada.
 * @return
 */
void enviarVariables()
{
	UartSendString(UART_PC, "Temperatura: ");
	UartSendString(UART_PC, (char *)UartItoa(temperatura, 10));
	UartSendString(UART_PC, "ºC - Humedad: ");
	UartSendString(UART_PC, (char *)UartItoa(humedad, 10));
	UartSendString(UART_PC, "%");

	if (temperatura <= 2 && humedad >= 85)
	{
		UartSendString(UART_PC, " - RIESGO DE NEVADA");
	}

	UartSendString(UART_PC, "\n");

	if (cont5s == 5)
	{
		cont5s = 0;
		UartSendString(UART_PC, "Radiación: ");
		UartSendString(UART_PC, (char *)UartItoa(radiacion, 10));
		UartSendString(UART_PC, "mR/h");
		if (radiacion >= 50)
		{
			UartSendString(UART_PC, " - RADIACION ELEVADA");
		}
		UartSendString(UART_PC, "\n");
	}
}

/** @fn void medirVariables()
 * @brief Funcion que toma las mediciones de los valores de temperatura, humedad y radiación obtenidos de
 * los sensores.
 * @return
 */
void medirVariables()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		dht11Read(&humedad, &temperatura);

		if (cont5s == 5)
		{
			AnalogInputReadSingle(CH1, &tension);
			radiacion = tension * 100 / 3.3;
		}

		enviarVariables();

		if (radiacion < 50 && temperatura > 2 && humedad < 85)
		{
			LedOn(LED_1);
		}
		else
		{
			LedsOffAll();
		}
	}
}

/** @fn void funcionBoton1()
 * @brief Funcion que contiene el codigo a ejecutar al presionar el boton 1, el cual incia el timer y por ende
 * el programa.
 * @return
 */
void funcionBoton1()
{
	TimerStart(timerA.timer);
}

/** @fn void funcionBoton2()
 * @brief Funcion que contiene el codigo a ejecutar al presionar el boton 2, el cual detiene el timer y por ende
 * el programa.
 * @return
 */
void funcionBoton2()
{
	TimerStop(timerA.timer);
}

/*==================[external functions definition]==========================*/
void app_main(void)
{
	LedsInit();

	dht11Init(GPIO_20);

	SwitchesInit();
	SwitchActivInt(SWITCH_1, &funcionBoton1, NULL);
	SwitchActivInt(SWITCH_2, &funcionBoton2, NULL);

	analog_input_config_t input_Analog = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0,
	};
	AnalogInputInit(&input_Analog);

	serial_config_t pUart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = NULL,
		.param_p = NULL};
	UartInit(&pUart);

	TimerInit(&timerA);

	xTaskCreate(&medirVariables, "Medir", 2048, NULL, 5, &medir_task_handle);

}
/*==================[end of file]============================================*/