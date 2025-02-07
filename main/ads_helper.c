#include "ads_helper.h"
#include "esp_log.h"
#include "ads111x.h"
#include "ads_helper.h"
#include <stdint.h>

ads111x_mux_t mux_select[4] = {
    ADS111X_MUX_0_GND,   //!< positive = AIN0, negative = GND
    ADS111X_MUX_1_GND,   //!< positive = AIN1, negative = GND
    ADS111X_MUX_2_GND,   //!< positive = AIN2, negative = GND
    ADS111X_MUX_3_GND,   //!< positive = AIN3, negative = GND
};
ads111x_mode_t mode; 
i2c_dev_t adc_device; // Declare an i2c_dev_t structure

const static char *TAG = "ADS111x_Interface";

void init_ads_adc(void){
 
    ESP_LOGI(TAG, "Initializing ADS111x descriptor");
    ESP_ERROR_CHECK(ads111x_init_desc(&adc_device, ADDR_SELECT,I2C_MASTER_NUM, I2C_MASTER_SDA_IO, I2C_MASTER_SCL_IO));
    //adc_device->cfg.scl_pullup_en = true;   
    ESP_LOGI(TAG, "Setting ADS111x data rate");
    ESP_ERROR_CHECK(ads111x_set_data_rate(&adc_device, ADS111X_DATA_RATE_32));
    ESP_LOGI(TAG, "Setting ADS111x input mux");
    ESP_ERROR_CHECK(ads111x_set_input_mux(&adc_device, ADS111X_MUX_0_GND));  // positive = AIN0, negative = GND
    ESP_LOGI(TAG, "Setting ADS111x gain");  
    ESP_ERROR_CHECK(ads111x_set_gain(&adc_device, GAIN));

}

void ads_log_mode(ads111x_mode_t mode) {
    const char *mode_str;

    // Map enum to string
    switch (mode) {
        case ADS111X_MODE_CONTINUOUS:
            mode_str = "Continuous conversion mode";
            break;
        case ADS111X_MODE_SINGLE_SHOT:
            mode_str = "Single-shot mode (default)";
            break;
        default:
            mode_str = "Unknown mode";
            break;
    }

    // LOG the mode
    ESP_LOGI(TAG, "ADS111x Operational Mode: %s\n", mode_str);
}

float map_raw_to_voltage(int16_t raw, ads111x_gain_t gain) {
    float full_scale_voltage = 0;

    // Determine the full-scale voltage based on the gain
    switch (gain) {
        case ADS111X_GAIN_6V144:
            full_scale_voltage = 6.144;
            break;
        case ADS111X_GAIN_4V096:
            full_scale_voltage = 4.096;
            break;
        case ADS111X_GAIN_2V048:
            full_scale_voltage = 2.048;
            break;
        case ADS111X_GAIN_1V024:
            full_scale_voltage = 1.024;
            break;
        case ADS111X_GAIN_0V512:
            full_scale_voltage = 0.512;
            break;
        case ADS111X_GAIN_0V256:
            full_scale_voltage = 0.256;
            break;
        default:
            full_scale_voltage = 2.048; // Default to ±2.048V
            break;
    }

    // Map the raw 16 bit ADC value to a voltage
    return ((float)raw / 32768.0) * full_scale_voltage;
}

