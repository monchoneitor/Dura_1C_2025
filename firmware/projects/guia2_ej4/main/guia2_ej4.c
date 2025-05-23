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
 * |    Function  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	Entrada Analogica	 	| 	GPIO1		|
 * | 	Salida Analogica	 	| 	GPIO0		|
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
/*==================[internal data definition]===============================*/
/* Arreglo de caracteres que contiene valores que fueron muestrados de un ECG
 */

unsigned char ECG[] = {
17,17,17,17,17,17,17,17,17,17,17,18,18,18,17,17,17,17,17,17,17,18,18,18,18,18,18,18,17,17,16,16,16,16,17,17,18,18,18,17,17,17,17,
18,18,19,21,22,24,25,26,27,28,29,31,32,33,34,34,35,37,38,37,34,29,24,19,15,14,15,16,17,17,17,16,15,14,13,13,13,13,13,13,13,12,12,
10,6,2,3,15,43,88,145,199,237,252,242,211,167,117,70,35,16,14,22,32,38,37,32,27,24,24,26,27,28,28,27,28,28,30,31,31,31,32,33,34,36,
38,39,40,41,42,43,45,47,49,51,53,55,57,60,62,65,68,71,75,79,83,87,92,97,101,106,111,116,121,125,129,133,136,138,139,140,140,139,137,
133,129,123,117,109,101,92,84,77,70,64,58,52,47,42,39,36,34,31,30,28,27,26,25,25,25,25,25,25,25,25,24,24,24,24,25,25,25,25,25,25,25,
24,24,24,24,24,24,24,24,23,23,22,22,21,21,21,20,20,20,20,20,19,19,18,18,18,19,19,19,19,18,17,17,18,18,18,18,18,18,18,18,17,17,17,17,
17,17,17

} ;
/* Entero que se utilizará para recorrer el arreglo de caracteres
 */
int contEcg = 0;
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
void analogOutputEscribir()
{
	while (1)
	{
		ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
		if(contEcg >= 231)
		{
			contEcg = 0;
			AnalogOutputWrite(ECG[contEcg]);
			contEcg++;
		}
		else if(contEcg < 231)
		{
			AnalogOutputWrite(ECG[contEcg]);
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