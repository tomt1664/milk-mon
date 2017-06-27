/*
 *  Milk monitor sketch
 *
 *  DHT11 thermometer
 *  ESP8266 WiFi
 *  2 x HX711 ADC (strain gauages)
 */
 
#include <ESP8266WiFi.h>
#include <stdlib_noniso.h>
#include "HX711.h"
#define DHTTYPE DHT11   // DHT11 sensor

#include "DHT.h"
#define DHTPIN 5     // digital pin for the DHT11 sensor
 
const char* ssid     = "XXXXXXXXXX"; // AP credentials
const char* password = "xxxxxxxx";
 
const char* host = "api.thingspeak.com"; // thingspeak host and API key
const char* APIkey   = "XXXXXXXXXXXXXX";
 
DHT dht(DHTPIN, DHTTYPE);

HX711 scale(12, 14); // strain gauage 1 
HX711 scale2(2, 4); // strain gauage 2 


//calibrated settings for the two strain gauages (0 to 1 : 0 to 4 pints water)
long oset1 = 71262;
long full1 = 597000;

long oset2 = 175219;
long full2 = 630549;  // SG 2 is the one with the thicker wires at the straight edge

//milk capacity
float fmilk1 = 0.0;
float fmilk2 = 0.0;
// temperature and humidity
float hum,temp;

//connect to AP
void connect() {
  //connecting to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
 
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
  delay(100);

  dht.begin();

  connect();

//calculate capacities
  fmilk1 = (scale.read() + oset1)*1.0/(1.0*(oset1 + full1));
  delay(500);
  fmilk2 = (scale2.read() + oset2)*1.0/(1.0*(oset2 + full2));
  delay(500);

  Serial.print("Milk capacity: ");
  Serial.print(fmilk1);
  Serial.print(" ");
  Serial.println(fmilk2);

  // read humidity
  hum = dht.readHumidity();
  // read temperature
  temp = dht.readTemperature();
  if (isnan(hum) || isnan(temp)) {
    Serial.println("Failed to read from DHT sensor!"); 
  }
  delay(1000);

  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Humidity: ");
  Serial.println(hum);
 
  char charVal[12];
  char charVal2[12];
  dtostrf(temp, 8, 2, charVal);
  dtostrf(hum, 8, 2, charVal2);
  
  Serial.print("connecting to ");
  Serial.println(host);
 
  // Use WiFiClient class to create TCP connections
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    return;
  }

    // create a URI for the request
  String url = "/update?key=";
  url += APIkey;
  url += "&field1=";
  url += fmilk1;
  url += "&field2=";
  url += fmilk2;
  url += "&field3=";
  url += charVal;
  url += "&field4=";
  url += charVal2;
 
  Serial.print("Requesting URL: ");
  Serial.println(url);
 
  // This will send the request to the server
  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
               "Host: " + host + "\r\n" +
               "Connection: close\r\n\r\n");
  delay(10);
 
  while (client.available()) {
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
 
  Serial.println();
  Serial.println("closing connection");

  // power down the two ADCs
  scale.power_down();
  scale2.power_down();

  // ESP sleep
  Serial.println("Put ESP8266 in deep sleep");
  ESP.deepSleep(30e6);
}
 
 
 
void loop() {

}
