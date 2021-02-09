#include <Arduino.h>
#include "key_interrupt.h"
#include "ble_kb.h"

// Button buts[NUM_KEYS] = {{0, false, &on_key_int}, {0, false, &on_key_int}, {0, false, &on_key_int}}; // k2 k4 k3
Button buts[NUM_KEYS] = {0};
uint32_t pins[NUM_KEYS] = {13, 33, 32};
char *keynames[NUM_KEYS] = {"K2", "K4", "K3"};
uint32_t error_code = 0;
TimerHandle_t timers[NUM_KEYS];

void IRAM_ATTR key_isr(void *arg)
{
  Button *s = static_cast<Button *>(arg);
  uint32_t ind = (s - buts);
  if (xTimerResetFromISR(timers[ind], pdFALSE) != pdPASS)
  {
    error_code = 1;
  }
}

void sampler_callback(TimerHandle_t xTimer)
{
  bool currentState = 0;

  uint32_t ind = (uint32_t)pvTimerGetTimerID(xTimer);
  Button *s = &buts[ind];
  currentState = !digitalRead(pins[ind]);

  if (currentState != s->state)
  {
    s->state = currentState;

    if (s->cb == nullptr)
    {
      Serial.printf("Key %s has null callback !\n", keynames[ind]);
    }
    else
    {
      uint32_t arg = (ind << 1) + (currentState ? 1 : 0); //{ind, currentState};
      // xTaskCreate(*(s->cb), "key_cb", 20000, (void*)arg, tskIDLE_PRIORITY, NULL);
      (*(s->cb))(arg);
    }

    if (currentState == true)
    {
      // pressed
      // Serial.printf("Key %s Pressed !\n", keynames[ind]);
    }
    else
    {
      // released
      // Serial.printf("Key %s Released !\n", keynames[ind]);
    }
  }
}
