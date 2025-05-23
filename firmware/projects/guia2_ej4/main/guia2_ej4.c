/*! @mainpage guia2_ej1
 *
 * @section genDesc General Description
 *
 * Este programa recibe un valor de una entrada analógica y la transforma en una digital almacenando su valor en mV, posteriormente
 * este valor es enviado a travez de formato UART para ser leido por un graficador de puerto serie y observar el graficador, funcionando
 * asi el programa como osciloscopio
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	CH1	 	| 	GPIO21		|
 * | 	CH0	 	| 	GPIO20		|
 * | 	+3,3V	 	| 	+3,3V		|
 * | 	GND	 	| 	GND		|
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 09/05/2024 | Document creation		                         |
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
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def CONFIG_REFRESH_PERIOD_DISTANCE_US
 * @brief Valor del periodo de refresco del timer A.
 */
#define CONFIG_REFRESH_PERIOD_DISTANCE_US 2000
/** @def CONFIG_REFRESH_PERIOD_DISTANCE_US2
 * @brief Valor del periodo de refresco del timer B.
 */
#define CONFIG_REFRESH_PERIOD_DISTANCE_US2 2000
/** @def BUFFER_SIZE
 * @brief Valor que contiene el tamaño del arreglo de caracteres que contiene el ECG muestreado.
 */
#define BUFFER_SIZE 231
/*==================[internal data definition]===============================*/
/* Arreglo de caracteres que contiene valores que fueron muestrados de un ECG
 */
const char ecg[BUFFER_SIZE] = {
    76, 77, 78, 77, 79, 86, 81, 76, 84, 93, 85, 80,
    89, 95, 89, 85, 93, 98, 94, 88, 98, 105, 96, 91,
    99, 105, 101, 96, 102, 106, 101, 96, 100, 107, 101,
    94, 100, 104, 100, 91, 99, 103, 98, 91, 96, 105, 95,
    88, 95, 100, 94, 85, 93, 99, 92, 84, 91, 96, 87, 80,
    83, 92, 86, 78, 84, 89, 79, 73, 81, 83, 78, 70, 80, 82,
    79, 69, 80, 82, 81, 70, 75, 81, 77, 74, 79, 83, 82, 72,
    80, 87, 79, 76, 85, 95, 87, 81, 88, 93, 88, 84, 87, 94,
    86, 82, 85, 94, 85, 82, 85, 95, 86, 83, 92, 99, 91, 88,
    94, 98, 95, 90, 97, 105, 104, 94, 98, 114, 117, 124, 144,
    180, 210, 236, 253, 227, 171, 99, 49, 34, 29, 43, 69, 89,
    89, 90, 98, 107, 104, 98, 104, 110, 102, 98, 103, 111, 101,
    94, 103, 108, 102, 95, 97, 106, 100, 92, 101, 103, 100, 94, 98,
    103, 96, 90, 98, 103, 97, 90, 99, 104, 95, 90, 99, 104, 100, 93,
    100, 106, 101, 93, 101, 105, 103, 96, 105, 112, 105, 99, 103, 108,
    99, 96, 102, 106, 99, 90, 92, 100, 87, 80, 82, 88, 77, 69, 75, 79,
    74, 67, 71, 78, 72, 67, 73, 81, 77, 71, 75, 84, 79, 77, 77, 76, 76,
};
/* Entero de 16 bits que se utilizará para recorrer el arreglo de caracteres
 */
uint16_t contEcg = 0;
/* Entero de 16 bits donde se almacenará el valor de tension en mV obtenidos por el CH1
 */
uint16_t tension = 0;
TaskHandle_t analogleer_task_handle = NULL;
TaskHandle_t analogesc_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn static void funcionTimerA(void *pvParameter)
 * @brief Funcion que contiene el código que se ejecuta cada vez que se actualiza el timer A,
 * el cual envia una notificación para realizar tareas.
 * @return
 */
static void funcionTimerA(void *pvParameter)
{
	//vTaskNotifyGiveFromISR(analogesc_task_handle, pdFALSE);
	vTaskNotifyGiveFromISR(analogleer_task_handle, pdFALSE);
}
/** @fn static void funcionTimerB(void *pvParameter)
 * @brief Funcion que contiene el código que se ejecuta cada vez que se actualiza el timer B,
 * el cual envia una notificación para realizar tareas.
 * @return
 */
static void funcionTimerB(void *pvParameter)
{
	vTaskNotifyGiveFromISR(analogesc_task_handle, pdFALSE);
}
/** @fn void funcionUARTRead(void *pvParameter)
 * @brief Función que lee del dispositivo que se este enviado y recibiendo valores en formato UART
 * un caracter.
 * @return
 */
void funcionUARTRead(void *pvParameter)
{
	uint8_t caracter;
	UartReadByte(UART_PC, &caracter);
}
/** @fn static void analogInputLeer(void *pvParameter)
 * @brief Funcion que lee el valor de tensión recibido a travez de CH1 en mV y envia este valor en formato
 * UART
 * @return
 */
static void analogInputLeer(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		AnalogInputReadSingle(CH1, &tension);
		
		UartSendString(UART_PC, "$");
		UartSendString(UART_PC, (char *)UartItoa(tension, 10));
		UartSendString(UART_PC, ";");
	}
}
/** @fn static void analogOutputEscribir(void *pvParameter)
 * @brief Funcion que envía a travez de CH0 el valor contECG del arreglo de caracteres ecg convertido a mV
 * @return
 */
static void analogOutputEscribir(void *pvParameter)
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(contEcg == BUFFER_SIZE)
		{
			contEcg = 0;
			AnalogOutputWrite(ecg[contEcg]);
			contEcg++;
		}
		else
		{
			AnalogOutputWrite(ecg[contEcg]);
			contEcg++;
		}
			
	}
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

	timer_config_t timerA = {
		.timer = TIMER_A,
		.period = CONFIG_REFRESH_PERIOD_DISTANCE_US,
		.func_p = funcionTimerA,
		.param_p = NULL};
	TimerInit(&timerA);

	timer_config_t timerB = {
		.timer = TIMER_B,
		.period = CONFIG_REFRESH_PERIOD_DISTANCE_US2,
		.func_p = funcionTimerB,
		.param_p = NULL};
	TimerInit(&timerB);

	analog_input_config_t input_Analog = {
		.input = CH1,
		.mode = ADC_SINGLE,
		.func_p = NULL,
		.param_p = NULL,
		.sample_frec = 0,};
	AnalogInputInit(&input_Analog);

	AnalogOutputInit();

	serial_config_t pUart = {
		.port = UART_PC,
		.baud_rate = 115200,
		.func_p = funcionUARTRead,
		.param_p = NULL};
	UartInit(&pUart);



	xTaskCreate(&analogInputLeer, "Medir_Analog", 1024, NULL, 5, &analogleer_task_handle);
	xTaskCreate(&analogOutputEscribir, "Escribir_Analog", 1024, NULL, 5, &analogesc_task_handle);

	TimerStart(timerA.timer);
	TimerStart(timerB.timer);
}
/*==================[end of file]============================================*/