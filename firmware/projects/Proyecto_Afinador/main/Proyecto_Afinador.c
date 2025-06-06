/*! @mainpage Proyecto Afinador
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
 * | 23/05/2025 | Document creation		                         |
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
#include "iir_filter.h"
#include "fft.h"
#include "timer_mcu.h"
#include "uart_mcu.h"
#include "analog_io_mcu.h"
/*==================[macros and definitions]=================================*/
#define BUFFER_SIZE 1650
#define FIRST_STRING_FREQ 330
#define SAMPLE_FREQ 3300                           // La frecuencia de afinacion de la primera cuerda de la guitarra (Mi agudo) es 329,63, se muestra a 10 veces la frec
#define CONFIG_REFRESH_PERIOD_DISTANCE_US_A 303    // Timer que muestrea a frec de 3300hz
#define CONFIG_REFRESH_PERIOD_DISTANCE_US_B 500000 // Timer que reinicia cada segundo
/*==================[internal data definition]===============================*/
float audio[BUFFER_SIZE] = {0};
float audio_filt[BUFFER_SIZE];
float audio_fft[BUFFER_SIZE / 2];
float vector_frec[BUFFER_SIZE / 2];
float frecPrimeraCuerda = 330;
int frecPBajo = 360;
int frecPAlto = 300;
int tolerancia = 10;
int contador = 0;
TaskHandle_t analizar_task_handle = NULL;
/*==================[internal functions declaration]=========================*/
static void micGrabar(void *pvParameter)
{
    uint16_t tension;
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

static void enviarDatos(int paramComparacion)
{
    if (paramComparacion == 0)
    {
        // printf("ajustar");
    }
    else if (paramComparacion == 1)
    {
        // printf("afinado");
    }
    else if (paramComparacion == 2)
    {
        // printf("desajustar");
    }
}

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

static void analizarAudio(void *pvParameter)
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
    LowPassInit(SAMPLE_FREQ, frecPBajo, ORDER_4);
    HiPassInit(SAMPLE_FREQ, frecPAlto, ORDER_4);
    FFTInit();

    timer_config_t timerA = {
        .timer = TIMER_A,
        .period = CONFIG_REFRESH_PERIOD_DISTANCE_US_A,
        .func_p = micGrabar,
        .param_p = NULL};
    TimerInit(&timerA);

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
        .baud_rate = 230400,
        .func_p = NULL,
        .param_p = NULL};
    UartInit(&pUart);

    ble_config_t ble_configuration = {
        "ESP_AFINADOR",
        BLE_NO_INT};
    BleInit(&ble_configuration);

    xTaskCreate(&analizarAudio, "Analizar", 2048, NULL, 5, &analizar_task_handle);

    TimerStart(TIMER_A);
}
/*==================[end of file]============================================*/