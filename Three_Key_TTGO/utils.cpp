#include <Arduino.h>
#include "utils.h"

int vref = 1114; // ADC VRef calibration: 1114mV
esp_adc_cal_characteristics_t adc_chars;

/*
Method to print the reason by which ESP32
has been awaken from sleep
*/
void print_wakeup_reason()
{
  esp_sleep_wakeup_cause_t wakeup_reason;

  wakeup_reason = esp_sleep_get_wakeup_cause();

  switch (wakeup_reason)
  {
  case ESP_SLEEP_WAKEUP_EXT0:
    Serial.println("Wakeup caused by external signal using RTC_IO\n");
    break;
  case ESP_SLEEP_WAKEUP_EXT1:
    Serial.println("Wakeup caused by external signal using RTC_CNTL\n");
    break;
  case ESP_SLEEP_WAKEUP_TIMER:
    Serial.println("Wakeup caused by timer\n");
    break;
  case ESP_SLEEP_WAKEUP_TOUCHPAD:
    Serial.println("Wakeup caused by touchpad\n");
    break;
  case ESP_SLEEP_WAKEUP_ULP:
    Serial.println("Wakeup caused by ULP program\n");
    break;
  default:
    Serial.printf("Wakeup was not caused by deep sleep: %d\n", wakeup_reason);
    break;
  }
}

void battery_adc_init()
{
  /*
    ADC_EN is the ADC detection enable port
    If the USB port is used for power supply, it is turned on by default.
    If it is powered by battery, it needs to be set to high level
    */
  pinMode(ADC_EN, OUTPUT);
  digitalWrite(ADC_EN, HIGH);

  esp_adc_cal_value_t val_type = esp_adc_cal_characterize(ADC_UNIT_1, ADC_ATTEN_DB_11, ADC_WIDTH_BIT_12, 1100, &adc_chars); //Check type of calibration value used to characterize ADC
  if (val_type == ESP_ADC_CAL_VAL_EFUSE_VREF)
  {
    Serial.printf("eFuse Vref:%u mV\n", adc_chars.vref);
    vref = adc_chars.vref;
  }
  else if (val_type == ESP_ADC_CAL_VAL_EFUSE_TP)
  {
    Serial.printf("Two Point --> coeff_a:%umV coeff_b:%umV\n", adc_chars.coeff_a, adc_chars.coeff_b);
  }
  else
  {
    Serial.println("Default Vref: 1100mV\n");
  }

  adc1_config_width(ADC_WIDTH_BIT_12);
  adc1_config_channel_atten(ADC1_GPIO34_CHANNEL, ADC_ATTEN_DB_11);
}

// float battery_get_voltage()
// {
//   uint16_t v = analogRead(ADC_PIN);
//   float battery_voltage = ((float)v / 4095.0) * 2.0 * 3.3 * (vref / 1000.0);
//   return battery_voltage;
// }

float battery_get_voltage()
{
  uint32_t reading = adc1_get_raw(ADC1_GPIO34_CHANNEL);
  uint32_t voltage = esp_adc_cal_raw_to_voltage(reading, &adc_chars);
  return ((float)voltage * 2 / 1000.0f);
}

float battery_percentage(float v)
{
  float square1;
  if (v >= 4.2f)
  {
    return 100.0f;
  }
  else if (v > 3.79f)
  {
    square1 = (v - 4.25f);
    return (square1 * square1 * -28.05f + 10.0f) * 10.0f;
  }
  else if (v > 3.68f)
  {
    square1 = (v - 3.673f);
    return (square1 * square1 * 217.75f + 1.0f) * 10.0f;
  }
  else if (v > 3.0f)
  {
    square1 = (v - 2.87f);
    return (square1 * square1 * 1.563f - 0.03f) * 10.0f;
  }
  else
  {
    return 0.0f;
  }
}
