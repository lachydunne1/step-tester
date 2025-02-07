#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "ads_helper.h"
#include "uart.h"
#include "display.h"
#include "gpio.h"

/** 
    @brief: Provide functionality to perform unit step tests and record responses
            via accurate ADC interface.

            The user is to:
                -define their own recording software (adc task processes, equations) at the bottom of this src.
                - [OPTIONAL] : construct their own load switch: control gpio is customised in uart.h (switching gpio)
                - [OPTIONAL] : source their own external ADC and display.
                
*/
static const char *TAG = "unit_step";

static void unit_step_task(void *pvParameters);
void toggle_gpio(uint8_t *uart_data);
float calculate_value0(float adc_value);
float calculate_value1(float adc_value);
float calculate_value2(float adc_value);
float calculate_value3(float adc_value);

void app_main(void){
    //setup ADC
    esp_err_t error_check;
    ESP_ERROR_CHECK(i2cdev_init()); //INIT LIBRARY
    init_uart();
    init_ads_adc();
    init_ssd1306_lcd();
    init_gpio();

    error_check = gpio_set_direction(CONTROL_GPIO, GPIO_MODE_INPUT_OUTPUT); //NOTE: IF PULLDOWN/UP NOT CONFIGURED THIS GPIO SPAZZES
    if (error_check != ESP_OK){
        ESP_LOGE(TAG, "Error in GPIO_SET_DIRECTION");
    }
    /*
    error_check = gpio_pulldown_en(CONTROL_GPIO);
    if (error_check != ESP_OK){
        ESP_LOGE(TAG, "Error in GPIO_PULLDOWN_ENABLE");
    }
    */
    display_queue = xQueueCreate(DISPLAY_QUEUE_LENGTH, sizeof(SensorData));

    //Create a task to handler UART event from ISR
    xTaskCreate(uart_event_task, "uart_event_task", 2048, NULL, 12, NULL);
    
    //Create a task for unit_step on to be triggered on sempahore give, which is in turn toggled on gpio
    //rising edge
    xTaskCreate(unit_step_task, "unit_step_task", 2048*2, NULL, 13, NULL);
    xTaskCreate(display_task, "display_task", 2048, NULL, 14, NULL);

}



static void unit_step_task(void *pvParameters){

    int16_t raw = 1;
    
    // variables to track oversampling
    int os_rate = 20;
    int os_idx = 0;
    float value0 = 0.0;
    float value1 = 0.0;
    float value2 = 0.0;
    float value3 = 0.0;
    float os_value0 = 0.0;
    float os_value1 = 0.0;
    float os_value2 = 0.0;
    float os_value3 = 0.0;
    float voltage0 = 0.0;
    float voltage1 = 0.0;
    float voltage2 = 0.0;
    float voltage3 = 0.0;

    while(1){

        if (gpio_get_level(CONTROL_GPIO) ==1){

            for (int i = 0; i < 4; i++) {
                // Set the input mux to the correct channel
                ESP_ERROR_CHECK(ads111x_set_input_mux(&adc_device, mux_select[i]));
                ads111x_start_conversion(&adc_device);

                // Wait for conversion to complete
                bool busy;
                do {
                    ads111x_is_busy(&adc_device, &busy);
                } while (busy);

                // Read ADC value
                if (ads111x_get_value(&adc_device, &raw) == ESP_OK) {
                    float voltage = map_raw_to_voltage(raw, GAIN);
                    //ESP_LOGI(TAG, "Voltage Read: %f", voltage);
                    // Store voltage readings in corresponding variables
                    switch (i) {
                        case 0:
                            voltage0 += voltage;
                            value0 = calculate_value0(voltage);
                            os_value0 += value0;
                            break;
                        case 1:
                            voltage1 += voltage;
                            value1 = calculate_value1(voltage);
                            os_value1 += value1;
                            break;
                        case 2:
                            voltage2 += voltage;
                            value2 = calculate_value2(voltage);
                            os_value2 += value2;
                            break;
                        case 3:
                            voltage3 += voltage;
                            value3 = calculate_value3(voltage);
                            os_value3 += value3;
                            break;
                    }
                    //ESP_LOGI(TAG, "Raw Read: %i from channel: %i", raw, i);
                    //ESP_LOGI(TAG, "Voltage Read: %f from channel: %d", voltage, i);
          

                } else {
                    ESP_LOGE(TAG, "Failed to read value from channel %d", i);
                }
                
                os_idx+=1;
                if (os_idx == os_rate){
                    /* complete oversampling and send values to be updated in ssd1306:
                        hence the refresh rate of the display is 1/(SAMPLING_PERIOD*os_rate);
                    */
                    //ESP_LOGI(TAG, "Accumulated: V=%.2f, I=%.2f, BV=%.2f, BI=%.2f\n",
                    //os_value0, os_value1, os_value2, os_value3);

                    SensorData values;

                    // acutal values read from ADC
                    voltage0 = (4*voltage0) / (float)os_rate;
                    voltage1 = (4*voltage1) / (float)os_rate;
                    voltage2 = (4*voltage2) / (float)os_rate;
                    voltage3 = (4*voltage3) / (float)os_rate;
                    // calculated values
                    values.value0 = (4*os_value0) / (float)os_rate;
                    values.value1 = (4*os_value1) / (float)os_rate;
                    values.value2 = (4*os_value2) / (float)os_rate;
                    values.value3 = (4*os_value3) / (float)os_rate;
                    ESP_LOGI("","%f,%f,%f,%f", values.value0, values.value1,voltage0, voltage1);
                    xQueueSend(display_queue, &values, portMAX_DELAY);

                    os_value0 =os_value1=os_value2=os_value3=0.0;
                    voltage0 = voltage1 = voltage2 = voltage3 =0.0;
                    os_idx = 0;  // Reset index
                } 

                //vTaskDelay(pdMS_TO_TICKS(SAMPLING_PERIOD));
            }
        }else { 
            ssd1306_draw_string(&lcd, 0, 0, waiting_sig, 1, 1); 
            vTaskDelay(pdMS_TO_TICKS(1000));
        } //waiting to start test

    } 
    
}


/* setup your own relations here*/
float calculate_value0(float adc_voltage){
    return (0);
}

float calculate_value1(float adc_voltage){
    return (0);
}
float calculate_value2(float adc_voltage){
    return (0);
}
float calculate_value3(float adc_voltage){
    return (0);
}
