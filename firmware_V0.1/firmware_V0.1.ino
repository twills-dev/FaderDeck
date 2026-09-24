#include <list>
#include <WiFi.h>
#include <WebSocketsClient.h>
#include <ArduinoJson.h>

//network variables
char* ssid = "";
char* password = "";
char obsServer = "";
uint16_t obsPort = 4455;

WebSocketsClient webSocket;

TaskHandle_t inputHandler; // multi-core stuff

//button & potentiometer variables
int   buttonPins[]    = {0,0,0,0,0};
bool  buttonStates[]   = {false,false,false,false,false};
int   buttonCooldwn[] = {0,0,0,0,0};

int   obsPotPins[]    = {0,0,0,0,0};
int   potValues[]     = {0,0,0,0,0};
int   lastPotValues[] = {0,0,0,0,0};
int   samples         = 32;

//setup stuff

int iArrSize(int array[]){
  return (sizeOf(array) / sizeOf(array[0]));
}
int bArrSize(bool array[]){
  return (sizeOf(array) / sizeOf(array[0]));
}

void InitWifi()
{
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.printf("Trying to connect [%s] ", WiFi.macAddress().c_str());
  while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
  }
  Serial.printf(" %s\n", WiFi.localIP().toString().c_str());
}

//websocket event handler
void WebSocketEvent(WStype_t type, uint8_t * payload, size_t length) {
    switch(type) {

        //connect & disconnect handler
        case WStype_DISCONNECTED:
            Serial.println("[WebSocket] Disconnected! Trying to reconnect...");
            break;
            
        case WStype_CONNECTED:
            Serial.println("[WebSocket] Connected to OBS server!");
            break;
            
        //ok we actually got a response - i will need to edit this if i ever need to take input
        case WStype_TEXT: {

            StaticJsonDocument<300> doc;  //make blank json doc
            DeserializationError error = deserializeJson(doc, payload);
            
            if (error) {
                return;
            }

            int op = doc["op"];
            
            // Opcode 0 is the "Hello" message from OBS
            if (op == 0) {
                Serial.println("[WebSocket] Received Hello from OBS, sending Identify...");
                
                
                StaticJsonDocument<200> identifyDoc;  //make a json doc called identifyDoc
                identifyDoc["op"] = 1; // Opcode 1 = Identify - im assuming this adds a line with op=1 to the json doc
                
                JsonObject d = identifyDoc.createNestedObject("d"); //create a nested object for reasons i do not undersand
                d["rpcVersion"] = 1; 
                d["eventSubscriptions"] = 0; // Set to 0 since we only send data out
                
                String outputJson;
                serializeJson(identifyDoc, outputJson); //formatting
                webSocket.sendTXT(outputJson);          //send it
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

void InputHandler()
{

  for(int i = 0;i < iArrSize(potPins);i++)  // for each potentiometer
  {
    potValues[i] = 0;
    for(int a = 0; i < samples; i++)        // repeat for # of samples specified
    {
      potValues[i] = potValues[i] + analogRead(potPins[i]); // running total
    }
    potValues[i] = potValues[i] / samples   // divide to get value between 0 and 409
  }

  for(int i = 0;i< iArrSize(buttonPins);i++)
  {
    if((digitalRead(buttonPins[i]) == "HIGH") && (buttonCooldwn[i] == 0)){
      buttonStates[i] = true; //this will be reset as soon as it gets sent by the ws handler
      buttonCooldwn[i] = 500; //arbitrary number
    }
    else if(buttonCooldwn[i] > 0)   //if the button is on cooldown
    {
      buttonCooldwn[i]--;           //decrement cooldown
    }
  }

}


void setup() {
  // serial recieve loop to recieve wifi credentials from connected PC. save with NVS? hardcoded for now.
  ssid = "";
  password = "";
  obsServer = "";

  InitWifi();
  webSocket.begin(obsServer,obsPort);
  webSocket.onEvent(WebSocketEvent);
  webSocket.setReconnectInterval(5000);

  //create input handler routine to run on core 1
  xTaskCreatePinnedToCore(
    InputHandler,     //function to run as this task
    "Input Handler",  //name of task
    10000,            //stack size in words
    NULL,             //input params
    0,                //priority
    &inputHandler,    //taskhandle obj from earlier
    1                 //run on core 1
  );
}

void loop() {
  // put your main code here, to run repeatedly:

}
