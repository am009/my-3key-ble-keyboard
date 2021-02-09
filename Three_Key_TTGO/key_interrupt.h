#include <freertos/timers.h>

#define DEBOUNCE_MS 30
#define NUM_KEYS 3

typedef void (*key_func)(uint32_t p);

struct Button
{
  uint32_t lastPress;
  bool state; // true for pressed
  key_func cb;
};

extern Button buts[NUM_KEYS]; // k2 k4 k3

#define BUT1_PIN 13
#define BUT2_PIN 33
#define BUT3_PIN 32
#define BUTR_PIN 35
#define BUTL_PIN 0

extern uint32_t pins[NUM_KEYS];
extern char *keynames[NUM_KEYS];

extern TimerHandle_t timers[NUM_KEYS];

extern uint32_t error_code;

void IRAM_ATTR key_isr(void *arg);

void sampler_callback(TimerHandle_t xTimer);
