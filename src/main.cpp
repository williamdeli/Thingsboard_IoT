#include <Arduino.h>
#include <WiFi.h>
#include <Ticker.h>
#include <Wire.h>
#include "BH1750.h"
#include "DHTesp.h"
#include "ThingsBoard.h"
#include "device.h"
const char *ssid = "Lekker coffee";
const char *password = "NasiGoreng23";

// Initialize ThingsBoard client
WiFiClient konekWifi;
// Initialize ThingsBoard instance
ThingsBoard tb(konekWifi);
Ticker KirimData;
DHTesp dht;
BH1750 lightMeter;
void WifiConnect();
void kirimDataSensor();

void setup()
{
  Serial.begin(115200);
  pinMode(LED_BUILTIN, OUTPUT);
  dht.setup(PIN_DHT, DHTesp::DHT11);
  Wire.begin(PIN_SDA, PIN_SCL);
  lightMeter.begin(BH1750::CONTINUOUS_HIGH_RES_MODE, 0x23, &Wire);
  WifiConnect();
  KirimData.attach_ms(UPDATE_DATA_INTERVAL, kirimDataSensor);

  Serial.println("System ready.");

}
void WifiConnect()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }
  Serial.print("System connected with IP address: ");
  Serial.println(WiFi.localIP());
  Serial.printf("RSSI: %d\n", WiFi.RSSI());
}
void kirimDataSensor()
{
  float humidity = dht.getHumidity();
  float temperature = dht.getTemperature();
  float lux = lightMeter.readLightLevel();
  if (dht.getStatus() == DHTesp::ERROR_NONE)
  {
    Serial.printf("Temperature: %.2f C, Humidity: %.2f %%, light: %.2f\n",
                  temperature, humidity, lux);
    if (tb.connected())
    {
      tb.sendTelemetryFloat("temperature", temperature);
      tb.sendTelemetryFloat("humidity", humidity);
        tb.sendTelemetryFloat("light", lux);
    }
  }
}
void loop()
{
  if (!tb.connected())
  {
    if (tb.connect(THINGSBOARD_SERVER, THINGSBOARD_ACCESS_TOKEN))
      Serial.println("Connected to thingsboard");
    else
    {
      Serial.println("Error connected to thingsboard");
      delay(3000);
    }
  }
  tb.loop();
}
