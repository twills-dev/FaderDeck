#include <list>


TaskHandle_t Task1;

std::list<int> buttonPins      = {0,0,0,0,0};
std::list<int> buttonCooldown  = {0,0,0,0,0};
std::list<int> potPins         = {0,0,0,0,0};
std::list<int> potValues       = {0,0,0,0,0};
int samples = 32;   //32 samples per potentiometer reading

void setup() 
{
  xTaskCreatePinnedToCore(
      InputHandler, /* Function to implement the task */
      "Input Handler",      /* Name of the task */
      10000,        /* Stack size in words */
      NULL,         /* Task input parameter */
      0,            /* Priority of the task */
      &Task1,       /* Task handle. */
      0);           /* Core where the task should run */
}

void InputHandler(void *i_need_this){
  for(;;){  //inf loop


    //handle all the shit
    for(int i=0;i<5; i++){
      int value = 0;
      for(int a=0; a<samples; a++){
        value = value + analogRead(i);  //running total of values 
      }
      potValues[i] = (value / samples);//take an average

      
    }

  }
}

void loop() {
  // put your main code here, to run repeatedly:
  
}
