#include <BleKeyboard.h>

extern BleKeyboard bleKeyboard;
void on_key_int(uint32_t p);

extern QueueHandle_t key_queue;

extern uint32_t last_use;

#define SLEEP_DELAY_MS (4 * 60 * 1000)
#define UPDATE_DELAY_MS (20 * 1000)
