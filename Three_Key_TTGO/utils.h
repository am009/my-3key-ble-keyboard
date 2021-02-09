#include "esp_adc_cal.h"

void print_wakeup_reason();
void battery_adc_init();
float battery_get_voltage();
float battery_percentage(float v);

#define ADC_EN 14  // ADC_EN is the ADC detection enable port
#define ADC_PIN 34 // Battery ADC

extern int vref; // ADC VRef calibration: 1114mV
extern esp_adc_cal_characteristics_t adc_chars;