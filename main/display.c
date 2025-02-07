#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"
#include "display.h"

static char *TAG = "SSD1306_interface";
char waiting_sig[20] = "Waiting for INPUT";
ssd1306_t lcd; 
QueueHandle_t display_queue;


void init_ssd1306_lcd(void) {
   
    lcd.height = DISPLAY_Y;
    lcd.width = DISPLAY_X;
    lcd.fb = calloc(1, SSD1306_FRAMEBUF_SIZE(&lcd));
    ESP_LOGI(TAG, "Panel is %dx%d", DISPLAY_X, DISPLAY_Y);
    ESP_ERROR_CHECK(ssd1306_init_desc(&lcd, I2C_PORT, OLED_ADDR, I2C_SDA, I2C_SCL));
    ESP_ERROR_CHECK(ssd1306_init(&lcd)); 
    
    // Set up screen defaults
    ssd1306_clear(&lcd);
    ssd1306_set_contrast(&lcd, 0xFF);
    ssd1306_set_segment_remapping_enabled(&lcd, 1); // flip horizontally
    ssd1306_set_scan_direction_fwd(&lcd, false); // flip vertically
    ESP_LOGI(TAG, "SSD1306 Initialized successfully!");
    ssd1306_draw_string(&lcd, 0, 0, waiting_sig, 1, 1);
}


void display_task(void *pvParameters) {
    SensorData recv_value;
    char buffer[20];  // Buffer for formatted output

    while (1) {
        if (xQueueReceive(display_queue, &recv_value, portMAX_DELAY) == pdTRUE) {
            //ESP_LOGI(TAG, "Q Recv: V=%.2f, I=%.2f, BV=%.2f, BI=%.2f\n",
            //recv_value.value0, recv_value.value1, recv_value.value2, recv_value.value3);
            ssd1306_clear(&lcd); 
         
            sprintf(buffer, "value0: %.2f", recv_value.value0);
            ssd1306_draw_string(&lcd, 0, 0, buffer, 1, 1);

            sprintf(buffer, "value1: %.2f", recv_value.value1);
            ssd1306_draw_string(&lcd, 0, 8, buffer, 1, 1);

            sprintf(buffer, "value2: %.2f", recv_value.value2);
            ssd1306_draw_string(&lcd, 0, 16, buffer, 1, 1);

            sprintf(buffer, "value3: %.2f", recv_value.value3);
            ssd1306_draw_string(&lcd, 0, 24, buffer,1, 1);


            ssd1306_flush(&lcd);  // Send data to OLED
        }
    }
}
