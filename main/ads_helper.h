#ifndef ads_helper
#define ads_helper

#include "esp_log.h"
#include "ads111x.h"
#include <stdint.h>

#define ADDR_SELECT ADS111X_ADDR_GND
#define GAIN ADS111X_GAIN_4V096 // +-4.096V range?
#define ADS111X_MAX_VALUE 3.3
#define ESP32 1
#define SAMPLING_PERIOD 1

#define I2C_MASTER_NUM 0// I2C port number for master
#if ESP32
    #define I2C_MASTER_SCL_IO GPIO_NUM_22   // Set the GPIO number for SCL (adjust for your setup)
    #define I2C_MASTER_SDA_IO GPIO_NUM_21    // Set the GPIO number for SDA (adjust for your setup)
#else 
    #define I2C_MASTER_SCL_IO GPIO_NUM_9    
    #define I2C_MASTER_SDA_IO GPIO_NUM_8
#endif


extern ads111x_mux_t mux_select[4];
extern i2c_dev_t adc_device; // Declare an i2c_dev_t structure

float map_raw_to_voltage(int16_t raw, ads111x_gain_t gain);
void init_ads_adc(void);
void ads_log_mode(ads111x_mode_t mode);

#endif