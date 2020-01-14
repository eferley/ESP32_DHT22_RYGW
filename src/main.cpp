#include <Arduino.h>
/*
 * Temperature and Humidity sensor on ESP32 with Async Web Server
 * inspired from https://randomnerdtutorials.com/esp32-dht11-dht22-temperature-humidity-web-server-arduino-ide/
 */
#include <WiFi.h>
#include <FS.h>                     // Required for ESP Async WebServer
#include "DHT.h"
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#define LED_RED_PIN       0
#define LED_YELLOW_PIN    4
#define LED_GREEN_PIN     16
#define LED_WHITE_PIN     5

#define LED_ON            HIGH
#define LED_OFF           LOW

#define DHTPIN            23
#define DHTTYPE           DHT22
#define DHTMEASURETIME    5000

#define TEMP_RED          22.0        // Red LED above this temp.
#define TEMP_YELLOW       21.0        // Yellow LED above this temp.
#define TEMP_GREEN        19.5        // Green LED above / White LED under this temp.

// Temp./Humidity sensor object
DHT dhtSensor(DHTPIN, DHTTYPE);

float fTmp;                       // Temperature (Celcius)
float fHum;                       // Humidity (percent)
float fHtIdx;                     // Heat Index (Celcius)
float fSndSpd;                    // Sound Speed (m/s)
String ulMeasureTime;             // measurement time (HH:MM:SS)

// WIFI and AsyncWebServer object
const char *pWifiSsid = "BF53newSSID";
const char *pWifiPassword = "nEwPaSs12!/";

// Create AsyncWebServer object on port 80
AsyncWebServer oWebServer(80);

// Create UDP client for NTP
WiFiUDP ntpUDP;

// You can specify the time server pool and the offset (in seconds, can be
// changed later with setTimeOffset() ). Additionaly you can specify the
// update interval (in milliseconds, can be changed using setUpdateInterval() ).
NTPClient clientNTP(ntpUDP, "europe.pool.ntp.org", 3600, 60000);

unsigned long ulTime;     // current time (milliseconds)
unsigned long ulT0;       // last measurement time (milliseconds)


const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML><html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <link rel="stylesheet" href="https://use.fontawesome.com/releases/v5.7.2/css/all.css" integrity="sha384-fnmOCqbTlWIlj8LyTjo7mOUStjsKC4pOpQbqyi7RrhN7udi9RwhKkMHpvLbHG9Sr" crossorigin="anonymous">
  <style>
    html {
     font-family: Arial;
     display: inline-block;
     margin: 0px auto;
     text-align: center;
    }
    h2 { font-size: 3.0rem; }
    p { font-size: 3.0rem; }
    .units { font-size: 1.2rem; }
    .dht-labels{
      font-size: 1.5rem;
      vertical-align:middle;
      padding-bottom: 15px;
    }
  </style>
</head>
<body>
  <h2>Y FAIT BEAU, Y FAIT CHAUD !</h2>
  <p>
    <span id="thetime">%THETIME%</span>
  </p>
  <p>
    <i class="fas fa-thermometer-half" style="color:#059e8a;"></i>
    <span class="dht-labels">Temp. : </span>
    <span id="temperature">%TEMPERATURE%</span>
    <sup class="units">&deg;C</sup>
  </p>
  <p>
    <i class="fas fa-tint" style="color:#00add6;"></i>
    <span class="dht-labels">Taux Humid. : </span>
    <span id="humidity">%HUMIDITY%</span>
    <sup class="units">%</sup>
  </p>
</body>
<script>
setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("thetime").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/thetime", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("temperature").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/temperature", true);
  xhttp.send();
}, 10000 ) ;

setInterval(function ( ) {
  var xhttp = new XMLHttpRequest();
  xhttp.onreadystatechange = function() {
    if (this.readyState == 4 && this.status == 200) {
      document.getElementById("humidity").innerHTML = this.responseText;
    }
  };
  xhttp.open("GET", "/humidity", true);
  xhttp.send();
}, 10000 ) ;
</script>
</html>)rawliteral";

// Function prototypes
String processOutput(const String& var);
String outputTemperature();
String outputHumidity();
String outputTime();


void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  Serial.begin(115200);

  pinMode(LED_RED_PIN, OUTPUT);
  pinMode(LED_YELLOW_PIN, OUTPUT);
  pinMode(LED_GREEN_PIN, OUTPUT);
  pinMode(LED_WHITE_PIN, OUTPUT);

  dhtSensor.begin();

  digitalWrite(LED_BUILTIN, LED_ON);

  digitalWrite(LED_RED_PIN, LED_ON);
  digitalWrite(LED_YELLOW_PIN, LED_ON);
  digitalWrite(LED_GREEN_PIN, LED_ON);
  digitalWrite(LED_WHITE_PIN, LED_ON);

  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(pWifiSsid, pWifiPassword);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }

  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());

  // Initialize NTP client
  clientNTP.begin();
  clientNTP.update();

  Serial.println();
  Serial.print("Ready ! Current time : "); Serial.println(clientNTP.getFormattedTime());
  Serial.println();

  // Route for root / web page
  oWebServer.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/html", index_html, processOutput);
  });
  oWebServer.on("/temperature", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", outputTemperature().c_str());
  });
  oWebServer.on("/humidity", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", outputHumidity().c_str());
  });
  oWebServer.on("/thetime", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send_P(200, "text/plain", outputTime().c_str());
  });

  // Start server
  oWebServer.begin();

  digitalWrite(LED_BUILTIN, LED_OFF);

  digitalWrite(LED_RED_PIN, LED_OFF);
  digitalWrite(LED_YELLOW_PIN, LED_OFF);
  digitalWrite(LED_GREEN_PIN, LED_OFF);
  digitalWrite(LED_WHITE_PIN, LED_OFF);

  ulT0 = millis();
} // void setup()
//-------------------------------------

void loop()
{
  ulTime = millis();

  if (ulTime - ulT0 > DHTMEASURETIME)       // Take a measurement every DHTMEASURETIME milliseconds
  {
    // Serial.println();
    digitalWrite(LED_BUILTIN, LED_ON);
    clientNTP.update();
    ulMeasureTime = clientNTP.getFormattedTime();

    // Get readings from sensor
    fTmp = dhtSensor.readTemperature(false);
    fHum = dhtSensor.readHumidity();
    // Get Heat Index
    fHtIdx = dhtSensor.computeHeatIndex(fTmp, fHum, false);
    // Calculate the Speed of Sound in m/s
    fSndSpd = 331.4 + (0.606 * fTmp) + (0.0124 * fHum);

    ulT0 = ulTime;

    Serial.print(ulMeasureTime); Serial.print (" - ");
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
    else if (!isnan(fTmp)) {
      Serial.println(" => White");
      digitalWrite(LED_RED_PIN, LED_OFF);
      digitalWrite(LED_YELLOW_PIN, LED_OFF);
      digitalWrite(LED_GREEN_PIN, LED_OFF);
      digitalWrite(LED_WHITE_PIN, LED_ON);
    }

    digitalWrite(LED_BUILTIN, LED_OFF);
  } // if (ulTime - ulT0 > DHTMEASURETIME)

  delayMicroseconds(100);
  // Serial.print(".");
} // void loop()
//-------------------------------------

// Replaces placeholder with DHT values
String processOutput(const String& var)
{
  //Serial.println(var);
  if(var == "TEMPERATURE"){
    return outputTemperature();
  }
  else if(var == "HUMIDITY"){
    return outputHumidity();
  }
  else if(var == "THETIME"){
    return outputTime();
  }
  return String();
} // String processOutput(const String& var)
//-------------------------------------

String outputTemperature()
{
  // Check if any reads failed and exit early (to try again).
  if (isnan(fTmp)) {
    Serial.println("Failed to get Temperature from DHT sensor!");
    return "--";
  }
  else {
    // Serial.println(fTmp, 1);
    return String(fTmp);
  }
} // String outputTemperature()
//-------------------------------------

String outputHumidity()
{
  // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
  if (isnan(fHum)) {
    Serial.println("Failed to get Humidity from DHT sensor!");
    return "--";
  }
  else {
    // Serial.println(fHum, 1);
    return String(fHum);
  }
} // String outputHumidity()
//-------------------------------------

String outputTime()
{
  return ulMeasureTime;
} // String outputTime()
//-------------------------------------
