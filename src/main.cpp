#include <Arduino.h>

#include "DHT.h"

#define LED_RED_PIN       0
#define LED_YELLOW_PIN    2
#define LED_GREEN_PIN     16
#define LED_WHITE_PIN     5


void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_WHITE_PIN, OUTPUT);
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);

  digitalWrite(LED_RED_PIN, HIGH);
  digitalWrite(LED_YELLOW_PIN, HIGH);
  digitalWrite(LED_GREEN_PIN, HIGH);
  digitalWrite(LED_WHITE_PIN, HIGH);

  Serial.print("LED_BUILTIN="); Serial.print(LED_BUILTIN);
  Serial.print(" Status="); Serial.println(digitalRead(LED_BUILTIN) == HIGH ? "HIGH" : "LOW");
  // wait for a second
  delay(2000);
  // turn the LED off by making the voltage LOW
  digitalWrite(LED_BUILTIN, LOW);

  digitalWrite(LED_RED_PIN, LOW);
  digitalWrite(LED_YELLOW_PIN, LOW);
  digitalWrite(LED_GREEN_PIN, LOW);
  digitalWrite(LED_WHITE_PIN, LOW);

  Serial.print("LED_BUILTIN="); Serial.print(LED_BUILTIN);
  Serial.print(" Status="); Serial.println(digitalRead(LED_BUILTIN) == HIGH ? "HIGH" : "LOW");
   // wait for a second
  delay(1000);
}
