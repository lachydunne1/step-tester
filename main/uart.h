#ifndef uart
#define uart

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "driver/gpio.h"

#define UART_SELECT UART_NUM_0
#define BUF_SIZE (1024)
#define RD_BUF_SIZE (BUF_SIZE)
#define PATTERN_CHR_NUM    (3)    

//QueueHandle_t uart0_queue;

void init_uart(void);
void uart_event_task(void *pvParameters);
void toggle_gpio(uint8_t *uart_data);
#endif // uart.h