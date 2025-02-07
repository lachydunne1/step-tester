#include "driver/gpio.h"
#include "gpio.h"

void init_gpio(){
    gpio_set_direction(CONTROL_GPIO, GPIO_MODE_INPUT_OUTPUT);
}
