#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

//#include <AsyncTCP.h>
//#include <ESPAsyncWebServer.h>

const char* ssid = "";
const char* password = "";
const char* obsPassword = "password";
const char*




void initWiFi() {
  delay(500);
  Serial.println("");
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  initWiFi();
}

void loop() {
  // put your main code here, to run repeatedly:

}
