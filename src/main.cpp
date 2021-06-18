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
#include "mqttParam.h"
#include <Wire.h>
#include "ClosedCube_HDC1080.h"
#include <Ultrasonic.h>
#include "global.h"

AIS_SIM7020E_API nb;
ClosedCube_HDC1080 hdc1080;
Ultrasonic ultrasonic(14, 15);

//#define NODE_ID 0010001 // id of device

String address = MQTT_SERVER;  //Your IPaddress or mqtt server url
String serverPort = MQTT_PORT; //Your server port
char *deviceID = "60c9e927643cf850d09eed66";
char *cmdChannel = "6210323";
char *deviceName = "Tester Nb-iot";
String topic = "pktsos/";
//String pubTopic = "pktsos/data"; //Your topic     < 128 characters
//String subTopic = "cmd/01-00001";
String payload;                  //Your payload   < 500 characters
String username = MQTT_USER;     //username for mqtt server, username <= 100 characters
String password = MQTT_PASSWORD; //password for mqtt server, password <= 100 characters
unsigned int subQoS = 0;
unsigned int pubQoS = 0;
unsigned int pubRetained = 0;
unsigned int pubDuplicate = 0;

//Channel Recieve
const char MQTTChannel[] = "pktsos/%s/cmd";
char CH[30];

//Channel Send
const char MQTTTopic[] = "pktsos/%s/%s";
char TOPIC[40];

char EVENT[200];

/************************************
 * Station Parameter
 *************************************/
int stationHeight = 400;
int lackLevel = 20;
int normalLevel = 80;
int warningLevel = 120;
int dangerLevel = 200;
int rainFactor = 2;

long interval = 60000; //time in millisecond
unsigned long previousMillis = 0;

int distance, beforeLevel;

//pin for device
const int lightSensorPin = 34;

void setupMQTT();
void connectStatus();
void callback(String &topic, String &payload, String &QoS, String &retained);
void setTimeReadData(int min);
void clearJson();
void sendDataToServer();
int sort_desc(const void *cmp1, const void *cmp2);
void sendLogMsg(String msg);
void regisDevice();
void printParameter();
int messureWaterLevel();

//JSON
StaticJsonDocument<400> JSONencoder;
char JSONmessageBuffer[400];
uint8_t jsonData_len = 0;
char buff[400];

DynamicJsonDocument doc(1024); //Object for convert json txt

void setup()
{
  Serial.begin(115200);

  Serial.printf("Device Name : %s \n", deviceName);
  delay(1000);
  Serial.printf("Device ID : %s \n", deviceID);
  delay(1000);
  Serial.printf("Software : %s \n", APP_NAME);
  delay(1000);
  Serial.printf("version : %s \n", APP_VERSION);

  delay(1000);
  printParameter();

  hdc1080.begin(0x40);
  nb.begin();
  setupMQTT();
  beforeLevel = messureWaterLevel();
  nb.setCallback(callback);
  regisDevice();
  delay(1000);
  sendDataToServer();

  previousMillis = millis();

}

void loop()
{
  nb.MQTTresponse();
  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval)

    sendDataToServer();
}

//=========== MQTT Function ================
void setupMQTT()
{
  if (!nb.connectMQTT(address, serverPort, deviceID, username, password))
  {
    Serial.println("\nconnectMQTT");
  }
  sprintf(CH, MQTTChannel, cmdChannel);
  nb.subscribe(CH, subQoS);
  //  nb.unsubscribe(topic);
}
void connectStatus()
{
  if (!nb.MQTTstatus())
  {
    if (!nb.NBstatus())
    {
      Serial.println("reconnectNB ");
      nb.begin();
    }
    Serial.println("reconnectMQ ");
    setupMQTT();
  }
}

enum CMD
{
  REBOOT,
  SETTIME,
  READDATA,
  SETPARAM
}; // 1 Reboot  2 SETTIME 3 ReadData
void callback(String &topic, String &payload, String &QoS, String &retained)
{
  Serial.println("-------------------------------");
  Serial.println("# Message from Topic \"" + topic + "\" : " + nb.toString(payload));
  Serial.println("# QoS = " + QoS);
  deserializeJson(doc, nb.toString(payload));
  JsonObject obj = doc.as<JsonObject>();
  int cmd = obj[String("cmd")];
  Serial.println(cmd);
  switch (cmd)
  {
  case REBOOT:
  { //0
    payload = "{\"event\":\"Rebooting\"}";
    sendLogMsg(payload);
    Serial.println("Reboot in 10 sec.");
    for (size_t i = 0; i < 10; i++)
    {
      Serial.print('.');
      delay(1000);
    }
    Serial.println('Restarting....');
    ESP.restart();
  }
  break;
  case SETTIME: //1
  {
    int time = obj[String("time")];
    sprintf(EVENT, "{\"event\":\"Set new interval %d min\",\"field\":\"duration\",\"value\":%d}", time, time);

    sendLogMsg(EVENT);
    Serial.printf("Set interval :%i min \n", time);
    setTimeReadData(time);
  }
  break;

  case READDATA:
  { //2
    Serial.println('Read Data');
    payload = "{\"event\":\"Read Station Data\"}";
    sendLogMsg(payload);
    sendDataToServer();
  }
  break;
  case SETPARAM:
  { //3

    String field = obj["field"].as<String>();
    int value = obj[String("value")];
    Serial.printf("Set Parameter %s => %d\n", field, value);
    if (field == "stationHeight")
    {
      stationHeight = value;
    }
    else if (String(field) == "lackLevel")
    {
      lackLevel = value;
    }
    else if (String(field) == "normalLevel")
    {
      normalLevel = value;
    }
    else if (String(field) == "warningLevel")
    {
      warningLevel = value;
    }
    else if (String(field) == "dangerLevel")
    {
      dangerLevel = value;
    }
    else if (String(field) == "rainFactor")
    {
      rainFactor = value;
    }
    else
    {
    }
    printParameter();
    sprintf(EVENT, "{\"event\":\"Set new parameter %s   %d\",\"field\":\"%s\",\"value\":%d}", field,value,field,value);
    sendLogMsg(EVENT);
  }
  break;

  default:
    break;
  }

  if (retained.indexOf(F("1")) != -1)
  {
    Serial.println("# Retained = " + retained);
  }
}

void sendLogMsg(String msg)
{
  connectStatus();
  Serial.printf("send log to server : %s \n", msg);

  sprintf(TOPIC, MQTTTopic, deviceID, "log");

  nb.publish(TOPIC, msg);
}

void setTimeReadData(int min)
{
  interval = min * 60000;
  Serial.println("interval set to " + String(min) + " min");
}

void sendDataToServer()
{
  //clearJson();

  connectStatus();

  Serial.println(nb.getSignal());
  int waterLevel = messureWaterLevel();

  Serial.printf("Before = %d water level : %d \n" ,beforeLevel,waterLevel);
  
  float diffRate = 0;
  if (beforeLevel-waterLevel != 0)
  {
    diffRate = (waterLevel - beforeLevel) * 3600000 / (interval*100.0); // unit in m/hr
  }
  beforeLevel = waterLevel;

  int rainCount = random(0, 20);
  Serial.printf("Rain count = %d \n",rainCount);
  float rainFall = rainCount * rainFactor / 10.0;

  String temperature = String(hdc1080.readTemperature());
  String humidity = String(hdc1080.readHumidity());
  String Light = String(analogRead(lightSensorPin));
  String status;
  if (waterLevel <= lackLevel) status = "LACK";
  else if(waterLevel > lackLevel && waterLevel <= normalLevel) status = "NORMAL";
  else if (waterLevel > normalLevel && waterLevel <= warningLevel) status = "WARNING";
  else if (waterLevel > warningLevel && waterLevel <= dangerLevel) status = "DANGER";
  else if(waterLevel> dangerLevel) status="EXTREME";



  payload = "{\"temperature\":" + temperature + ",\"humidity\":" + humidity + ",\"light\":" + Light + ",\"waterLevel\":" + waterLevel + ",\"diffRate\":" + diffRate + ",\"rainFall\":" + rainFall +",\"status\":\""+status +"\",\"rssi\":" + nb.getSignal() + "}";
  sprintf(TOPIC, MQTTTopic, deviceID, "data");
  nb.publish(TOPIC, payload);

  previousMillis = millis();
}

/**********************************
 * JSON Method
 ***********************************/

void clearJson()
{
  JSONencoder.clear();
  memset(JSONmessageBuffer, 0, strlen(JSONmessageBuffer));
}

int sort_desc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  // The comparison
  return a > b ? -1 : (a < b ? 1 : 0);
  // A simpler, probably faster way:
  //return b - a;
}

void regisDevice()
{
  sendLogMsg("{\"event\":\"Restart Device\"}");
}

void printParameter()
{
  Serial.printf("Station height : %d cm \n", stationHeight);
  Serial.printf("Station interval : %d min \n", interval / 60000);
  Serial.printf("lack Water level : %d cm \n", lackLevel);
  Serial.printf("normal water level : %d cm \n", normalLevel);
  Serial.printf("warning water level : %d cm \n", warningLevel);
  Serial.printf("danger water level : %d cm \n", dangerLevel);
  Serial.printf("Rain factor : %d  \n", rainFactor);
}


int messureWaterLevel (){

  int dist[5];
  for (size_t i = 0; i < 5; i++)
  {
    dist[i] = ultrasonic.read();
    delay(1000);
  }
  int distLength = sizeof(dist) / sizeof(dist[0]);
  qsort(dist, distLength, sizeof(dist[0]), sort_desc);

  int distance = round((dist[1] + dist[2] + dist[3]) / 3);
  
  
  int waterLevel = stationHeight - distance;
  
  return waterLevel;

}