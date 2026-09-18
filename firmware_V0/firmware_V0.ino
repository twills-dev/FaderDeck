#include <list>


TaskHandle_t Task1;

int buttonPins[5]       = {0,0,0,0,0};
bool buttonState[5]     = {false,false,false,false,false};
int  buttonCooldown[5]  = {0,0,0,0,0};

//only 1 for testing purposes
int  potPins[1]         = {32};
int  potValues[1]       = {0};
int  lastPotValues[1]   = {0};
int samples = 32;   //32 samples per potentiometer reading


#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>



const char* ssid = "";
const char* password = "";
const char* obsServer = "192.168.0.102"; 
uint16_t obsPort = 4455;
char* sourceNames[3] = {"Game Capture", "Mic/Aux", "Discord"};

const int sendInterval = 50;
int lastSendTime = 0;

WebSocketsClient webSocket;

void webSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Disconnected! Trying to reconnect...");
            break;
            
        case WStype_CONNECTED:
            Serial.println("[WebSocket] Connected to OBS server!");
            break;
            
        case WStype_TEXT: {
            StaticJsonDocument<300> doc;
            DeserializationError error = deserializeJson(doc, payload);
            
            if (error) {
                return;
            }

            int op = doc["op"];
            
            // Opcode 0 is the "Hello" message from OBS
            if (op == 0) {
                Serial.println("[WebSocket] Received Hello from OBS, sending Identify...");
                
                StaticJsonDocument<200> identifyDoc;
                identifyDoc["op"] = 1; // Opcode 1 = Identify
                
                JsonObject d = identifyDoc.createNestedObject("d");
                d["rpcVersion"] = 1; 
                d["eventSubscriptions"] = 0; // Set to 0 since we only send data out
                
                String outputJson;
                serializeJson(identifyDoc, outputJson);
                webSocket.sendTXT(outputJson);
            }
            // Opcode 2 means OBS successfully accepted our Identify packet!
            else if (op == 2) {
                Serial.println("[WebSocket] Successfully Identified with OBS!");
            }
            break;
        }
        default:
            break;
    }
}

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

void setup() 
{
  //xTaskCreatePinnedToCore(

      //run input handler on core 1
      //InputHandler, /* Function to implement the task */
      //"Input Handler",      /* Name of the task */
      //10000,        /* Stack size in words */
      //NULL,         /* Task input parameter */
      //0,            /* Priority of the task */
      //&Task1,       /* Task handle. */
      //1);           /* Core where the task should run */


    //set all button pins appropriately
    //for(int i=0;i<sizeof(buttonPins); i++){
    //  pinMode(buttonPins[i], INPUT_PULLUP);
    //}
    Serial.begin(115200);
    initWiFi();
    webSocket.begin(obsServer, obsPort);
    webSocket.onEvent(webSocketEvent);
    webSocket.setReconnectInterval(5000);
}
/*
void InputHandler(void *i_need_this){
  for(;;){  //inf loop


    
    for(int i=0;i<sizeof(potPins); i++)   //for each potentiometer
    {  
      int value = 0;

      for(int a=0; a<samples; a++)      //loop for # of samples
      {
        value = value + analogRead(potPins[i]);  //running total of values - dumb average but it should be fine
      }
      potValues[i] = (value / samples); //take an average and write that to somewhere that core 0 can access :3
    }
    
    
    for(int i=0;i<sizeof(buttonPins);i++)   //for each button
    {
      if(digitalRead(buttonPins[i]) == "HIGH" && buttonCooldown[i] == 0)
      {
        buttonState[i] = true;
        buttonCooldown[i] = 500;
      }
      else if(buttonCooldown[i] > 0)
      {
        buttonCooldown[i]--;
      }
    }

  }
}
*/


void loop() {
  webSocket.loop();

    // Throttle checks to prevent flooding the websocket
    if (millis() - lastSendTime > sendInterval) {
        lastSendTime = millis();

        //for(int i=0;i<sizeof(potPins); i++)   //for each potentiometer
        //{  
          int value = 0;

          for(int a=0; a<samples; a++)      //loop for # of samples
          {
            value = value + analogRead(32);  //running total of values - dumb average but it should be fine
          }
          potValues[0] = (value / samples); //take an average and write that to somewhere that core 0 can access :3
          //Serial.println(potValues[0]);
        //}

        for(int i=0;i < (sizeof(potPins) / sizeof(potPins[0])); i++)
        {

        // Only send if the potentiometer value changed significantly (to reduce noise)
        if (abs(potValues[i] - lastPotValues[i]) > 3) 
        {
            lastPotValues[i] = potValues[i];

            // Map potentiometer (0-4095) to volume multiplier (0.0 to 1.0)
            float volumeMultiplier = (float)potValues[i] / 4095.0;

            // Create JSON document for OBS WebSocket v5 request
            StaticJsonDocument<250> doc;
            doc["op"] = 6; // Opcode 6 corresponds to a Request in OBS WebSocket v5

            JsonObject d = doc.createNestedObject("d");
            d["requestType"] = "SetInputVolume";
            d["requestId"] = "req_01"; // <-- Required by OBS v5 for requests

            JsonObject requestData = d.createNestedObject("requestData");
            requestData["inputName"] = sourceNames[i];
            requestData["inputVolumeMul"] = volumeMultiplier;

            String jsonString;
            serializeJson(doc, jsonString);

            // Send payload over WebSocket
            webSocket.sendTXT(jsonString);
        }
        }
        
    }
}
