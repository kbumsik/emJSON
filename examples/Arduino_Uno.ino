/**
 *  emJSON Arduino Uno example
 * This example code is tested with Arduino Uno hardware.
 * However, any kinds of Arduino boards should work too.
 * Just copy all header and source file of emJSON in the
 * Arduino folder and run it.
 */

#include "emJSON.h"

// JSON object.
char json_input[] = "{\"message\":\"This is JSON object.\"}";
json_t json;

// init variables
char message[128];
int timer = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  Serial.print("Hello World!\n");

  // init JSON object
  json = emJSON_init();
  emJSON_parse(&json, json_input);
  emJSON_insert_int(&json, "timer", 0);
}

void loop() {
  delay(1000);
  timer++;
  emJSON_set_int(&json, "timer", timer);
    
  emJSON_strcpy(message, &json);
  Serial.print(message);
  Serial.print('\n');
}
