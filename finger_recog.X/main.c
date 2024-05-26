/**
  Generated main.c file from MPLAB Code Configurator

  @Company
    Microchip Technology Inc.

  @File Name
    main.c

  @Summary
    This is the generated main.c using PIC24 / dsPIC33 / PIC32MM MCUs.

  @Description
    This source file provides main entry point for system initialization and application code development.
    Generation Information :
        Product Revision  :  PIC24 / dsPIC33 / PIC32MM MCUs - 1.171.4
        Device            :  PIC24FJ128GA705
    The generated drivers are tested against the following:
        Compiler          :  XC16 v2.10
        MPLAB 	          :  MPLAB X v6.05
*/

/*
    (c) 2020 Microchip Technology Inc. and its subsidiaries. You may use this
    software and any derivatives exclusively with Microchip products.

    THIS SOFTWARE IS SUPPLIED BY MICROCHIP "AS IS". NO WARRANTIES, WHETHER
    EXPRESS, IMPLIED OR STATUTORY, APPLY TO THIS SOFTWARE, INCLUDING ANY IMPLIED
    WARRANTIES OF NON-INFRINGEMENT, MERCHANTABILITY, AND FITNESS FOR A
    PARTICULAR PURPOSE, OR ITS INTERACTION WITH MICROCHIP PRODUCTS, COMBINATION
    WITH ANY OTHER PRODUCTS, OR USE IN ANY APPLICATION.

    IN NO EVENT WILL MICROCHIP BE LIABLE FOR ANY INDIRECT, SPECIAL, PUNITIVE,
    INCIDENTAL OR CONSEQUENTIAL LOSS, DAMAGE, COST OR EXPENSE OF ANY KIND
    WHATSOEVER RELATED TO THE SOFTWARE, HOWEVER CAUSED, EVEN IF MICROCHIP HAS
    BEEN ADVISED OF THE POSSIBILITY OR THE DAMAGES ARE FORESEEABLE. TO THE
    FULLEST EXTENT ALLOWED BY LAW, MICROCHIP'S TOTAL LIABILITY ON ALL CLAIMS IN
    ANY WAY RELATED TO THIS SOFTWARE WILL NOT EXCEED THE AMOUNT OF FEES, IF ANY,
    THAT YOU HAVE PAID DIRECTLY TO MICROCHIP FOR THIS SOFTWARE.

    MICROCHIP PROVIDES THIS SOFTWARE CONDITIONALLY UPON YOUR ACCEPTANCE OF THESE
    TERMS.
*/

/**
  Section: Included Files
*/
#include "mcc_generated_files/system.h"
#include "mcc_generated_files/adc1.h"
#include "mcc_generated_files/drivers/timeout.h"
#include <stdio.h>

#define LIGHT_TRIGGER_THRES     (50) // This value is appropriate when using a mobile phone flashlight
// #define ENABLE_DEBUG

typedef enum {
    APP_STATE_IDLE,
    APP_STATE_DETECTED_NEGA,
    APP_STATE_DETECTED_POS
} appStates_e;

static appStates_e appState = APP_STATE_IDLE;
static uint16_t light1 = 0;
static uint16_t light2 = 0;
static float light_prev1 = 0.;
static int count = 0;

uint16_t get_light_sensor(void);
void main_app(void);
static uint32_t periodic_handler(void);

uint16_t get_light_sensor(void)
{
    int i;
    uint16_t conversion;

    ADC1_Enable();
    ADC1_ChannelSelect(LIGHT_SENSOR);
    ADC1_SoftwareTriggerEnable();
    //Provide Delay
    for(i = 0; i < 1000; i++){}
    ADC1_SoftwareTriggerDisable();
    while(!ADC1_IsConversionComplete(LIGHT_SENSOR));
    conversion = ADC1_ConversionResultGet(LIGHT_SENSOR);
    ADC1_Disable();

    return conversion;
}


/*
    Main application
 */
int main(void)
{
    // initialize the device
    SYSTEM_Initialize();
    timerStruct_t periodic_timer = {periodic_handler, NULL};
    timeout_create(&periodic_timer, 0xF423);

    while (1)
    {
        timeout_callNextCallback();
        main_app();
    }

    return 1;
}

void main_app(void)
{
    float delta;
    float light;
    
    light1 = get_light_sensor();
    light2 = get_light_sensor();
    light = (light1 + light2) / 2;
    delta = light - light_prev1;
    light_prev1 = light;

#ifdef ENABLE_DEBUG
    printf("light: %.1f, delta: %.1f, state: %d, un: %d\n", light, delta, appState, light_unchanged);
#endif

    switch(appState)
    {
        case APP_STATE_IDLE:
        {
            if (delta < -LIGHT_TRIGGER_THRES)
            {
                appState = APP_STATE_DETECTED_NEGA;
            }
            break;
        }

        case APP_STATE_DETECTED_NEGA:
        {
            if (delta > LIGHT_TRIGGER_THRES)
            {
                count++;
                appState = APP_STATE_DETECTED_POS;
            }
            break;
        }

        case APP_STATE_DETECTED_POS:
        {
            if (delta < -LIGHT_TRIGGER_THRES)
            {
                appState = APP_STATE_DETECTED_NEGA;
            }
            break;
        }
    }
}

static uint32_t periodic_handler(void)
{
    printf("Finger count: %d\n", count);
    appState = APP_STATE_IDLE;
    count = 0;

    return 0xF423;
}

/**
 End of File
*/
