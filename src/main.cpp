#include <Arduino.h>



#include "Magellan_SIM7020E.h"

Magellan_SIM7020E magel;          

String dimmer_state="0";
String dimmer="0";
String payload;
const int ledPin = 2;

// Setting PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;

void setup() 
{
  Serial.begin(115200);
    
  magel.begin();            //Init Magellan LIB

  payload="{\"dimmer\":0}"; //Please provide payload with json format
  magel.report(payload);    //Initial dimmer data to Magellan 

  ledcSetup(ledChannel, freq, resolution);
  ledcAttachPin(ledPin, ledChannel);
  ledcWrite(ledChannel, 255);
}

void loop() 
{
  /*
    Example control the LED on board from Magellan IoT Platform
  */
  dimmer_state=magel.getControl("dimmer"); //Get dimmer data from Magellan

  if(dimmer_state.indexOf("40300")==-1)   //Response data is not 40300
  {
    dimmer=dimmer_state;
  }

  Serial.print("dimmer ");
  Serial.println(dimmer);
  ledcWrite(ledChannel, dimmer.toInt());   //Control the LED on board
}