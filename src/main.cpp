/*
 Supported DEVIO NB-DEVKIT I Board 
    |  Do not use PIN   |
    |      16 TX        |
    |      17 RX        |
    |      4 EINT       |
    |   26 power key    |
    |     27 reset      |

    If you have any questions, please see more details at https://www.facebook.com/AISDEVIO
*/
#include <Arduino.h>
#include <ArduinoJson.h>

#include "AIS_SIM7020E_API.h"
String address    = "206.189.145.76";               //Your IPaddress or mqtt server url
String serverPort = "1883";               //Your server port
String clientID   = "pkt-01-00001";               //Your client id < 120 characters
String pubTopic      = "/data/01-00001";               //Your topic     < 128 characters
String subTopic =  "/cmd/01-00001";
String payload    = "HelloWorld!";    //Your payload   < 500 characters
String username   = "khomkrit";               //username for mqtt server, username <= 100 characters
String password   = "tonkla0709";               //password for mqtt server, password <= 100 characters 
unsigned int subQoS       = 2;
unsigned int pubQoS       = 2;
unsigned int pubRetained  = 0;
unsigned int pubDuplicate = 0;


long interval = 5000;           //time in millisecond 
unsigned long previousMillis = 0;
int cnt = 0;

AIS_SIM7020E_API nb;

void setupMQTT();
void connectStatus();
void callback(String &topic,String &payload, String &QoS,String &retained);
void setTimeReadData(int min);

DynamicJsonDocument doc(1024); //Object for convert json txt
 

void setup() {
   Serial.begin(115200);
  
  nb.begin();
  setupMQTT();
  nb.setCallback(callback);   
  previousMillis = millis();            
}


void loop() {   
  nb.MQTTresponse();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
        cnt++;
        connectStatus();
        nb.publish(pubTopic,payload+String(cnt));      
//      nb.publish(topic, payload, pubQoS, pubRetained, pubDuplicate);      //QoS = 0, 1, or 2, retained = 0 or 1, dup = 0 or 1
        previousMillis = currentMillis;  
  }
}

//=========== MQTT Function ================
void setupMQTT(){
  if(!nb.connectMQTT(address,serverPort,clientID,username,password)){ 
     Serial.println("\nconnectMQTT");
  }
    nb.subscribe(subTopic,subQoS);
//  nb.unsubscribe(topic); 
}
void connectStatus(){
    if(!nb.MQTTstatus()){
        if(!nb.NBstatus()){
           Serial.println("reconnectNB ");
           nb.begin();
        }
       Serial.println("reconnectMQ ");
       setupMQTT();
    }   
}
void callback(String &topic,String &payload, String &QoS,String &retained){
  Serial.println("-------------------------------");
  Serial.println("# Message from Topic \""+topic+"\" : "+nb.toString(payload));
  Serial.println("# QoS = "+QoS);
  deserializeJson(doc, nb.toString(payload));
  JsonObject obj = doc.as<JsonObject>();
  int cmd =  obj[String("cmd")];
  switch (cmd)
  {
  case 1 : {
          Serial.println("Recieve 1 cmd"); 
          //Serial.println(obj[String("time")].as<String>());
          setTimeReadData(obj[String("time")]);
    }break;
  case 2 : Serial.println("Recieve 2" ); break;
  case 9 : Serial.println("Read Data "); break;
  
  default:
    break;
  }

  
  if(retained.indexOf(F("1"))!=-1){
    Serial.println("# Retained = "+retained);
  }
}

void setTimeReadData(int min){
  interval = min*60000;
  Serial.println("interval set to "+String(min)+ " min");
}
