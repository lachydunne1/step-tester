#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "ssd1306.h"

#define DISPLAY_X 128
#define DISPLAY_Y 32
// Define your I2C pins
#define I2C_PORT        I2C_NUM_0
#define I2C_SDA         GPIO_NUM_21
#define I2C_SCL         GPIO_NUM_22
#define OLED_ADDR       0x3C  // Default SSD1306 I2C address
#define DISPLAY_QUEUE_LENGTH 10

typedef struct {
    float value0;
    float value1;
    float value2;
    float value3;
}SensorData;

extern QueueHandle_t display_queue;
extern char waiting_sig[20];

extern int centre;
extern ssd1306_t lcd; 

void init_ssd1306_lcd(void);
void display_task(void *pvParameters);