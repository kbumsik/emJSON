/**
 *  emJSON Arduino Uno example
 * This example code is tested with Arduino Uno hardware.
 * However, any kinds of Arduino boards should work too.
 * Just copy all header and source file of emJSON in the
 * Arduino folder and run it.
 */

#include "emJSON.h"

// JSON object.
char json_input[] = "{\"message\":\"This is JSON object.\",\"time\":0.5}";
json_t json;

// init variables
char message[128];
int count = 0;
float timer;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Hello World!\n");

  // init JSON object
  json = emJSON_init();
  emJSON_parse(&json, json_input);
  emJSON_insert_int(&json, "count", 0);
}

void loop() {
  delay(500);
  timer = emJSON_get_float(&json, "time") + 0.5;
  count = emJSON_get_int(&json, "count") + 1;
  emJSON_set_float(&json, "time", timer);
  emJSON_set_int(&json, "count", count);
    
  emJSON_strcpy(message, &json);
  Serial.print(message);
  Serial.print('\n');
}
