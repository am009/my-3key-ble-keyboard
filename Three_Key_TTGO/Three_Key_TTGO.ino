#include <Arduino.h>
#include "key_interrupt.h"
#include "ble_kb.h"
#include <driver/rtc_io.h>
#include "utils.h"

#include <TFT_eSPI.h>
#include <SPI.h>

QueueHandle_t key_queue;
uint32_t last_use = 0; // 蓝牙发送时更新，用于loop中判断是否需要休眠
uint32_t last_update = 0;
float battery_voltage = 0;
float percentage = 0;
bool butr_pressed = false;
bool butl_pressed = false;

// 负责处理队列里的蓝牙发送请求
void ble_send_task(void *p)
{
  if (key_queue == NULL)
  {
    Serial.printf("ERROR: ble_send_task started before queue init.\n");
    return;
  }
  uint8_t c = 0;

  do
  {
    if (xQueueReceive(key_queue, &c, portMAX_DELAY) != pdTRUE)
    {
      Serial.printf("ERROR: key_queue Receive failed.\n");
      continue;
    }
    if (bleKeyboard.isConnected())
    {
      bleKeyboard.write(c);
    }
    last_use = millis();
  } while (true);
}

// ("TTGO 3-Key Mechanical Keyboard", "warrenwjk", 100)
BleKeyboard bleKeyboard("TTGO 3-Key Mechanical Keyboard", "warrenwjk", 76);
TFT_eSPI tft = TFT_eSPI(135, 240); // Invoke custom library
static uint8_t ble2key[NUM_KEYS] = {KEY_RETURN, '1', '1'}; // 在这里修改按键值
void on_key_int(uint32_t p)
{
  bool current_state = (((uint32_t)p % 2 == 1) ? true : false);
  uint32_t ind = ((uint32_t)p >> 1);
  // Serial.printf("key pressed p: %d.\n", (uint32_t)p);

  if (current_state != 1)
  {
    return;
  }

  if (ind >= NUM_KEYS)
  {
    Serial.printf("key press index out of bound.\n");
    return;
  }

  // Serial.printf("key %c pressed\n", ble2key[ind]);
  if (xQueueSend(key_queue, &ble2key[ind], 0) != pdPASS)
  {
    Serial.printf("key enqueue failed.\n");
  }
}

void IRAM_ATTR butr_isr(void *arg)
{
  butr_pressed = true;
}

void IRAM_ATTR butl_isr(void *arg)
{
  butl_pressed = true;
}

void setup()
{
  butl_pressed = false;
  butr_pressed = false;
  Serial.begin(115200);
  print_wakeup_reason();

  for (uint32_t i = 0; i < NUM_KEYS; i++)
  {
    timers[i] = xTimerCreate("KeyStateSampler", pdMS_TO_TICKS(30), pdFALSE, (void *)(i), sampler_callback);
    if (timers[i] == NULL)
    {
      Serial.printf("KeyStateSampler timer create failed!\n");
    }
    buts[i].cb = on_key_int;
  }
  for (uint32_t i = 0; i < NUM_KEYS; i++)
  {
    rtc_gpio_deinit((gpio_num_t)(pins[i]));
  }
  pinMode(BUT1_PIN, INPUT_PULLUP);
  attachInterruptArg(BUT1_PIN, key_isr, &buts[0], CHANGE);
  pinMode(BUT2_PIN, INPUT_PULLUP);
  attachInterruptArg(BUT2_PIN, key_isr, &buts[1], CHANGE);
  pinMode(BUT3_PIN, INPUT_PULLUP);
  attachInterruptArg(BUT3_PIN, key_isr, &buts[2], CHANGE);
  pinMode(BUTR_PIN, INPUT);
  attachInterruptArg(BUTR_PIN, butr_isr, NULL, FALLING);
  pinMode(BUTL_PIN, INPUT_PULLUP);
  attachInterruptArg(BUTL_PIN, butl_isr, NULL, FALLING);

  Serial.printf("Interrupt setup finished at %d ms.\n", millis());

  bleKeyboard.begin();
  key_queue = xQueueCreate(10, sizeof(uint8_t));
  if (key_queue == NULL)
  {
    Serial.printf("key_queue create failed!\n");
  }
  if (xTaskCreate(ble_send_task, "ble_send_task", 10000, NULL, tskIDLE_PRIORITY, NULL) != pdPASS)
  {
    Serial.printf("ble_send_task create failed!\n");
  }

  // Wait for stable voltage.
  vTaskDelay(100 / portTICK_PERIOD_MS);
  battery_adc_init();
  battery_voltage = battery_get_voltage();
  percentage = battery_percentage(battery_voltage);
  Serial.printf("Battery voltage is %f v. %f%%\n", battery_voltage, percentage);
  bleKeyboard.setBatteryLevel(roundf(percentage));
  
  Serial.println("BLE started.\n");
}

void loop()
{
  if (error_code != 0)
  {
    Serial.printf("error\n");
  }
  uint32_t current = millis();
  if (current - last_use > SLEEP_DELAY_MS || butr_pressed)
  {
    esp_err_t sleep_res;
    #if 1
    sleep_res = esp_sleep_enable_ext1_wakeup(GPIO_SEL_35, ESP_EXT1_WAKEUP_ALL_LOW);
    if (sleep_res != ESP_OK) {
      Serial.println("[Error] esp_sleep_enable_ext1_wakeup Error!");
    }
    // 完全不需要 rtc_gpio_pullup_en(GPIO_NUM_35); ，因为自带上拉电阻。
    #else
    // 使用三个键同时按下作为唤醒方式。
    sleep_res = esp_sleep_enable_ext1_wakeup(GPIO_SEL_13 | GPIO_SEL_32 | GPIO_SEL_33, ESP_EXT1_WAKEUP_ALL_LOW);
    if (sleep_res != ESP_OK) {
      Serial.println("[Error] esp_sleep_enable_ext1_wakeup Error!");
    }
    sleep_res = esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    if (sleep_res != ESP_OK) {
      Serial.println("[Error] esp_sleep_pd_config Error!");
    }
    for (uint32_t i = 0; i < NUM_KEYS; i++)
    {
      sleep_res = rtc_gpio_pullup_en((gpio_num_t)(pins[i]));
      if (sleep_res != ESP_OK) {
        Serial.println("[Error] rtc_gpio_pullup_en Error!");
      }
    }
    #endif
    Serial.println("Entering deep sleep...");
    bleKeyboard.end();
    vTaskDelay(200 / portTICK_PERIOD_MS);
    esp_deep_sleep_start(); // not return
  }
  if (current - last_update > UPDATE_DELAY_MS)
  {
    last_update = current;
    // 更新电池电量
    battery_voltage = battery_get_voltage();
    float temp_percent = battery_percentage(battery_voltage);
    if (temp_percent != 0.0f)
    {
      percentage = temp_percent;
      bleKeyboard.setBatteryLevel(roundf(percentage));
    }
    else
    {
      Serial.printf("Anomalous battery voltage auto-update: %f v. %f%%\n", battery_voltage, percentage);
    }
  }
  if (butl_pressed)
  {
    battery_voltage = battery_get_voltage();
    float temp_percent = battery_percentage(battery_voltage);
    if (temp_percent != 0.0f)
    {
      percentage = temp_percent;
      bleKeyboard.setBatteryLevel(roundf(percentage));
    }
    else
    {
      Serial.printf("Anomalous battery voltage but-press: %f v. %f%%\n", battery_voltage, percentage);
    }
    tft.init();
    tft.setRotation(1); //横
    String voltage = String(battery_voltage) + "V";
    String p = String(percentage) + "%";
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(7);
    tft.drawString(voltage, tft.width() / 2, 0);
    tft.setTextSize(5);
    tft.drawString(p, tft.width() / 2, 75);
    vTaskDelay(3000 / portTICK_PERIOD_MS);
    digitalWrite(TFT_BL, LOW);
    tft.writecommand(TFT_DISPOFF);
    tft.writecommand(TFT_SLPIN);

    bleKeyboard.setBatteryLevel(roundf(percentage));

    butl_pressed = false;
  }
  vTaskDelay(300 / portTICK_PERIOD_MS);
}
