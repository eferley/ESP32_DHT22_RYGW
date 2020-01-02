#include <Arduino.h>
/* 
 * Temperature and Humidity sensor on ESP32 with Web Server
 */
#include "DHT.h"

#define LED_RED_PIN       0
#define LED_YELLOW_PIN    2
#define LED_GREEN_PIN     16
#define LED_WHITE_PIN     5

#define LED_ON            HIGH
#define LED_OFF           LOW

#define DHTPIN            23
#define DHTTYPE           DHT22
#define DHTMEASURETIME    2000

#define TEMP_RED          22.0        // Red LED above this temp.
#define TEMP_YELLOW       21.0        // Yellow LED above this temp.
#define TEMP_GREEN        19.5        // Green LED above / White LED under this temp.

DHT dhtSensor(DHTPIN, DHTTYPE);       // Temp./Humidity sensor module

float fTmp;               // Temperature (Celcius)
float fHum;               // Humidity (percent)
float fHtIdx;             // Heat Index (Celcius)
float fSndSpd;            // Sound Speed (m/s)

unsigned long ulTime;     // current time (milliseconds)
unsigned long ulT0;       // last measurement time (milliseconds)

void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_WHITE_PIN, OUTPUT);

  dhtSensor.begin();

  digitalWrite(LED_RED_PIN, LED_OFF);
  digitalWrite(LED_YELLOW_PIN, LED_OFF);
  digitalWrite(LED_GREEN_PIN, LED_OFF);
  digitalWrite(LED_WHITE_PIN, LED_OFF);

  ulT0 = millis();
} // void setup()

void loop()
{
  ulTime = millis();

  if (ulTime - ulT0 > DHTMEASURETIME)       // Take a measurement every DHTMEASURETIME milliseconds
  {
    fTmp = dhtSensor.readTemperature(false);
    fHum = dhtSensor.readHumidity();
    // Get Heat Index
    fHtIdx = dhtSensor.computeHeatIndex(fTmp, fHum, false);
    // Calculate the Speed of Sound in m/s
    fSndSpd = 331.4 + (0.606 * fTmp) + (0.0124 * fHum);

    ulT0 = ulTime;

    Serial.print("Temp.  : "); Serial.print(fTmp, 1); Serial.print(" C");
    Serial.print(" - Humid. : " ); Serial.print(fHum, 1); Serial.print(" %");
    Serial.print(" - Heat Idx. : " ); Serial.print(fHtIdx, 1); Serial.print(" C");
    Serial.print( " - Snd.Sp.: " ); Serial.print(fSndSpd, 1); Serial.print( " m/s " );
    Serial.println();

    if (fTmp >= TEMP_RED) {
      Serial.println(" => Red");
      digitalWrite(LED_RED_PIN, LED_ON);
      digitalWrite(LED_YELLOW_PIN, LED_OFF);
      digitalWrite(LED_GREEN_PIN, LED_OFF);
      digitalWrite(LED_WHITE_PIN, LED_OFF);
    }
    else if (fTmp >= TEMP_YELLOW) {
      Serial.println(" => Yellow");
      digitalWrite(LED_RED_PIN, LED_OFF);
      digitalWrite(LED_YELLOW_PIN, LED_ON);
      digitalWrite(LED_GREEN_PIN, LED_OFF);
      digitalWrite(LED_WHITE_PIN, LED_OFF);
    }
    else if (fTmp >= TEMP_GREEN) {
      Serial.println(" => Green");
      digitalWrite(LED_RED_PIN, LED_OFF);
      digitalWrite(LED_YELLOW_PIN, LED_OFF);
      digitalWrite(LED_GREEN_PIN, LED_ON);
      digitalWrite(LED_WHITE_PIN, LED_OFF);
    }
    else {
      Serial.println(" => White");
      digitalWrite(LED_RED_PIN, LED_OFF);
      digitalWrite(LED_YELLOW_PIN, LED_OFF);
      digitalWrite(LED_GREEN_PIN, LED_OFF);
      digitalWrite(LED_WHITE_PIN, LED_ON);
    }
  } // if (ulTime - ulT0 > DHTMEASURETIME)

  delayMicroseconds(100);
} // void loop()
