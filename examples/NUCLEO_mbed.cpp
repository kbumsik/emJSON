/**
 * emJSON example on ARM mbed platform.
 * The testing device is NUCELO-F401RE but any NUCLEO devices should work well.
 * To use this code, please go to https://developer.mbed.org/compiler/ .
 * In the mbed compiler, just copy and paste the whole code and emJSON libraries.
 */

#include "mbed.h"
#include "emJSON.h"
//------------------------------------
// Hyperterminal configuration
// 9600 bauds, 8-bit data, no parity
//------------------------------------

Serial pc(SERIAL_TX, SERIAL_RX);

DigitalOut myled(LED1);

int main() {
  // start JSON object.
  char json_input[] = "{\"message\":\"This is JSON object.\",\"time\":0.5}";
  json_t json = emJSON_init();
  emJSON_parse(&json, json_input);
  emJSON_insert_int(&json, "led", 0);

  // init variables
  char message[256];
  int led = myled;
  float timer = emJSON_get_float(&json, "time");

  pc.printf("Hello JSON! %f\n");
  while(1) {
      wait_ms(500);
      timer = timer + (float)0.5;
      myled = !myled;
      led = myled;
      emJSON_set_float(&json, "time", timer);
      emJSON_set_int(&json, "led", led);

      emJSON_strcpy(message, &json);
      pc.printf("JSON Payload: %s\n", message);
  }
  emJSON_free(&json);
}

