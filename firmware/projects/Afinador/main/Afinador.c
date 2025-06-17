/*! @mainpage Proyecto Afinador
 *
 * @section genDesc General Description
 *
 * Este programa recibe una señal de audio de una cuerda de guitarra mediante un microfono analógico por medio de su conversor analógico digital,
 * guarda la señal recibida, la analiza, aplica Transformada Rapida de Fourier, compara con el valor de frecuencia a la que deberia de estar afinada
 * la misma y enciende un led en la app bluetooth electronics dependiendo de si se debe ajustar o desajustar o si se encuentra afinada.-
 *
 * <a href="https://drive.google.com/...">Operation Example</a>
 *
 * @section hardConn Hardware Connection
 *
 * |    Peripheral  |   ESP32   	|
 * |:--------------:|:--------------|
 * | 	+5V	 	| 	+5V		|
 * | 	+3,3V	 	| 	+3,3V		|
 * | 	GND	 	| 	GND		|
 * | 	Entrada Analogica	 	| 	GPIO_1		|
 *
 *
 * @section changelog Changelog
 *
 * |   Date	    | Description                                    |
 * |:----------:|:-----------------------------------------------|
 * | 23/05/2025 | Document creation		                         |
 * | 30/05/2025 | Se agregan las funciones correspondientes al   |
 * | 30/05/2025 | muestreo, análisis y comparación del audio     |
 * | 13/06/2025 | Se agregan las funciones para interactuar      |
 * | 13/06/2025 | con el dispositivo bluetooth                   |
 * | 16/06/2025 | Se agregan comentadas lineas para transmision  |
 * | 13/06/2025 | de datos por UART en caso de fallar Bluetooth  |
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
#include <iir_filter.h>
#include <fft.h>
#include "uart_mcu.h"
#include "timer_mcu.h"
#include "analog_io_mcu.h"
#include "ble_mcu.h"
/*==================[macros and definitions]=================================*/
/** @def BUFFER_SIZE
 * @brief Tamaño del buffer donde se almacena el audio muestreado.
 */
#define BUFFER_SIZE 1650
/** @def SAMPLE_FREQ
 * @brief Valor del periodo de la frecuencia de muestreo.
 */
#define SAMPLE_FREQ 3300
/** @def CONFIG_REFRESH_PERIOD_DISTANCE_US_A
 * @brief Valor del periodo de refresco del timer A que muestrea a 3300 Hz.
 */
#define CONFIG_REFRESH_PERIOD_DISTANCE_US_A 303
/*==================[internal data definition]===============================*/
/* Arreglo de flotantes donde se almacena el audio muestreado.
 */
float audio[BUFFER_SIZE] = {0};
/* Arreglo de flotantes donde se almacena el audio filtrado.
 */
float audio_filt[BUFFER_SIZE];
/* Arreglo de flotantes donde se almacena la FFT del audio filtrado.
 */
float audio_fft[BUFFER_SIZE / 2];
/* Arreglo de flotantes donde se almacena el vector de frecuencias del audio filtrado.
 */
float vector_frec[BUFFER_SIZE / 2];
/* Valor flotante que corresponde a la frecuencia de afinado de la primera cuerda.
 */
float frecPrimeraCuerda = 330;
/* Valor entero que corresponde a la frecuencia del filtro pasa bajo.
 */
int frecPBajo = 360;
/* Valor entero que corresponde a la frecuencia del filtro pasa alto.
 */
int frecPAlto = 300;
/* Valor entero que corresponde a la tolerancia de afinado.
 */
int tolerancia = 10;
/* Valor entero que se utilizará para contar las muestras tomadas.
 */
int contador = 0;
/* Valor uint16_t que se utiliza para obtener el valor de tension del CAD en el CH1.
 */
uint16_t tension = 0;
/* TaskHandle_t que se utilizará para notificar la tarea analizar.
 */
TaskHandle_t analizar_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
/** @fn static void micGrabar()
 * @brief Funcion que se encarga de muestrear de la señal analógica recibida el audio y posteriormente
 * notifica a la funcion analizar para iniciar el análisis de la señal deteniendo el timer.
 * @return
 */
static void micGrabar()
{
    if (contador < BUFFER_SIZE)
    {
        AnalogInputReadSingle(CH1, &tension);
        audio[contador] = tension;
        contador++;
    }
    else if (contador == BUFFER_SIZE)
    {
        vTaskNotifyGiveFromISR(analizar_task_handle, pdFALSE);
        TimerStop(TIMER_A);
    }
}

/** @fn static void enviarDatos(int paramComparacion)
 * @brief Funcion que envia al receptor bluetooth un mensaje que enciende un led dependiendo el parámetro
 * de comparación recibido.
 * @param paramComparacion Valor entero que se utilizará para decidir que led encender.
 * @return
 */
static void enviarDatos(int paramComparacion)
{
    //char msg[25];
    //for(int16_t i=0; i<825; i++){
    //        /* Formato de datos para que sean graficados en la aplicación móvil */
    //        sprintf(msg, "*HX%2.2fY%2.2f*\n", vector_frec[i], audio_fft[i]);
    //        BleSendString(msg);
    //    }
    if (paramComparacion == 0)
    {
        /* Enciende el led Amarillo y apaga el resto.
         */
        BleSendString("*Amarillo");
        BleSendString("*AR255G255B0");
        BleSendString("*VR0G0B0");
        BleSendString("*RR0G0B0");
    }
    else if (paramComparacion == 1)
    {
        /* Enciende el led Verde y apaga el resto.
         */
        BleSendString("*Verde");
        BleSendString("*AR0G0B0");
        BleSendString("*VR0G255B0");
        BleSendString("*RR0G0B0");
    }
    else if (paramComparacion == 2)
    {
        /* Enciende el led Rojo y apaga el resto.
         */
        BleSendString("*Rojo");
        BleSendString("*AR0G0B0");
        BleSendString("*VR0G0B0");
        BleSendString("*RR255G0B0");
    }
}

/** @fn static void compararMagnitud()
 * @brief Funcion que compara las frecuencias obtenidas de la FFT del audio filtrado con la frecuencia de afinación esperada
 * para la primer cuerda de la guitarra y posteriormente llama a la funcion para enviar los datos al receptor bluetooth.
 * @return
 */
static void compararMagnitud()
{
    float frecMax = vector_frec[0];
    float magnitudMax = audio_fft[0];
    for (int i = 0; i < BUFFER_SIZE / 2; i++)
    {
        if (magnitudMax < audio_fft[i])
        {
            frecMax = vector_frec[i];
            magnitudMax = audio_fft[i];
        }
    }

    if (frecMax < (frecPrimeraCuerda - tolerancia))
    {
        enviarDatos(0);
    }
    else if (frecMax > (frecPrimeraCuerda - tolerancia) && frecMax < (frecPrimeraCuerda + tolerancia))
    {
        enviarDatos(1);
    }
    else if (frecMax > (frecPrimeraCuerda + tolerancia))
    {
        enviarDatos(2);
    }
}

/** @fn static void analizarAudio()
 * @brief Funcion que aplica el fitro pasa bajo y pasa alto, obtiene el vector de frecuencias y las magnitudes por medio de la FFT y las almacena para
 * posteriormente llamar a la función compararMagnitud, reiniciar el contador e iniciar el timer.
 * @return
 */
static void analizarAudio()
{
    while (1)
    {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
        LowPassFilter(audio, audio_filt, BUFFER_SIZE);
        HiPassFilter(audio_filt, audio_filt, BUFFER_SIZE);
        FFTFrequency(SAMPLE_FREQ, BUFFER_SIZE, vector_frec);
        FFTMagnitude(audio_filt, audio_fft, BUFFER_SIZE);
        compararMagnitud();
        contador = 0;
        TimerStart(TIMER_A);
    }
}
/*==================[external functions definition]==========================*/
void app_main(void)
{

    timer_config_t timerA = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH_PERIOD_DISTANCE_US_A,
        .func_p = micGrabar,
        .param_p = NULL};

    analog_input_config_t input_Analog = {
        .input = CH1,
        .mode = ADC_SINGLE,
        .func_p = NULL,
        .param_p = NULL,
        .sample_frec = 0};

    ble_config_t ble_configuration = {
        "Afinador_ESP",
        BLE_NO_INT};

    serial_config_t pUart = {
        .port = UART_PC,
        .baud_rate = 230400,
        .func_p = NULL,
        .param_p = NULL};

    BleInit(&ble_configuration);

    UartInit(&pUart);

    TimerInit(&timerA);

    AnalogInputInit(&input_Analog);

    LowPassInit(SAMPLE_FREQ, frecPBajo, ORDER_4);
    HiPassInit(SAMPLE_FREQ, frecPAlto, ORDER_4);
    FFTInit();

    xTaskCreate(&analizarAudio, "Analizar", 8192, NULL, 5, &analizar_task_handle);

    TimerStart(TIMER_A);
}
/*==================[end of file]============================================*/