

#include <Arduino.h>
#include "Mqtt.h"
#include "global.h"
#include "mem.h"
#include "mqttParam.h"

#include "AIS_SIM7020E_API.h"

AIS_SIM7020E_API nb;
MEM mMEM;

const char *mqtt_server = MQTT_SERVER;
const char *mqtt_port = MQTT_PORT;
const char *mqtt_username = MQTT_USER;
const char *mqtt_password = MQTT_PASSWORD;
unsigned int subQoS = 2;
unsigned int pubQoS = 2;
unsigned int pubRetained = 0;
unsigned int pubDuplicate = 0;

const char MQTTChannel[] = "%s/#";
char CH[30];
char TOPIC[30];

long lastMsg = 0;

//JOSN
StaticJsonDocument<400> JSONencoder;
char JSONmessageBuffer[400];
uint8_t jsonData_len = 0;
char buff[400];
DynamicJsonDocument doc(1024); //Object for convert json txt

void clearJson();
void callback(String &topic, String &payload, String &QoS, String &retained);

MQTT::MQTT()
{
}
void MQTT::init()
{
    Serial.println("Initialize MQTT ");
    nb.begin();
    nb.setCallback(callback);
}

void setupMQTT()
{
    if (!nb.connectMQTT(mqtt_server, mqtt_port, mqtt_username, mqtt_password))
    {
        Serial.println("\nconnectMQTT");
    }
    sprintf(CH, MQTTChannel, NODE_ID);
    nb.subscribe(CH, subQoS);
}

void callback(String &topic, String &payload, String &QoS, String &retained)
{
    Serial.println("-------------------------------");
    Serial.println("# Message from Topic \"" + topic + "\" : " + nb.toString(payload));
    Serial.println("# QoS = " + QoS);
    deserializeJson(doc, nb.toString(payload));

    JsonObject obj = doc.as<JsonObject>();
    int cmd = obj[String("cmd")];
    switch (cmd)
    {
    case 1:
    {
        Serial.println("Recieve 1 cmd");
        //Serial.println(obj[String("time")].as<String>());
        setTimeReadData(obj[String("time")]);
    }
    break;
    case 2:
        Serial.println("Recieve 2");
        break;
    case 9:
        Serial.println("Read Data ");
        break;

    default:
        break;
    }

    if (retained.indexOf(F("1")) != -1)
    {
        Serial.println("# Retained = " + retained);
    }
}


void setTimeReadData(int min){
  //interval = min*60000;
  Serial.println("interval set to "+String(min)+ " min");
}


void callBackCmd(String msgId,bool res){
    clearJson();
    Serial.println("Send callback to server");
   
    JSONencoder["msgId"] = msgId;
    JSONencoder["result"] = res;
    serializeJsonPretty(JSONencoder,JSONmessageBuffer, sizeof(JSONmessageBuffer));
    Serial.println(JSONmessageBuffer);
    sprintf(TOPIC,"SF/%s/callback",NODE_ID);
    Serial.println(TOPIC);


}