/*
 Supported DEVIO NB-DEVKIT I Board 
    |  Do not use PIN   |
    |      16 TX        |
    |      17 RX        |
    |      4 EINT       |
    |   26 power key    |
    |     27 reset      |
    13 FOR REED PIN RAIN GUAGE
    14 TRIG ULTRA SONIC 
    15 ECHO ULTRA SONIC

    33 TOUCH TO READ DATA
    4 TOUCH TO SEND DATA
    12 TOUCH TO RESTART MODULE

    If you have any questions, please see more details at https://www.facebook.com/AISDEVIO
*/


#define KAONIYOM2 1  //DEVICE ID 
#define WRITESTATE 1 //0 not write 1 write


#include <WiFi.h>
#include <Arduino.h>
#include <ArduinoJson.h>
#include <EEPROM.h>
#include "AIS_SIM7020E_API.h"
#include "mqttParam.h"
#include <Wire.h>
#include "ClosedCube_HDC1080.h"
#include <Ultrasonic.h>
#include "eepromAddress.h"
#include "global.h"
#include "EEPROM_writeall.h"
#include <esp_bt.h>


//For interupt
#ifdef ESP8266 || ESP32
#define ISR_PREFIX ICACHE_RAM_ATTR
#else
#define ISR_PREFIX
#endif

AIS_SIM7020E_API nb;
ClosedCube_HDC1080 hdc1080;
Ultrasonic ultrasonic(14, 15);

//#define NODE_ID 0010001 // id of device

String address = MQTT_SERVER;  //Your IPaddress or mqtt server url
String serverPort = MQTT_PORT; //Your server port
char *deviceID ;//= "60c9e927643cf850d09eed66";
char *cmdChannel ;//= "6210323";
char *deviceName = DEVICENAME;
//String topic = "pktsos/";
//String pubTopic = "pktsos/data"; //Your topic     < 128 characters
//String subTopic = "cmd/01-00001";
String payload;                  //Your payload   < 500 characters
String username = MQTT_USER;     //username for mqtt server, username <= 100 characters
String password = MQTT_PASSWORD; //password for mqtt server, password <= 100 characters

unsigned int subQoS =0;
unsigned int pubQoS = 0;
unsigned int pubRetained = 0;
unsigned int pubDuplicate = 0;

int deviceState = 0; // 0 test 1 real
//Channel Recieve
const char MQTTChannel[] = "pktsos/%s/cmd/#";
char CH[100];

//Channel Send
const char MQTTTopic[] = "pktsos/%s/%s";
char TOPIC[100];
const char MQTTResponse[]="pktsos/%s/response/%s";
char EVENT[200];

/************************************
 * Station Parameter
 *************************************/
int stationHeight = 355;
int lackLevel = 40;
int normalLevel =140;
int warnLevel = 200;
int dangerLevel = 250;
int rainFactor = 2;

int interval = 10; //in minute
#define SECONDS_DS(seconds) ((seconds)*1000UL);

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
void sendLogMsg(String logType,String msg);
void sendResponse(String msgId);
void regisDevice();
void printParameter();
int messureWaterLevel();

void readDefaultParam();

struct myMillis {
  unsigned long offset = 0;
  unsigned long get() { return millis() - offset; }
  void reset() { offset = millis(); }
  void set(unsigned long value) { offset = millis() - value; }
};
myMillis MM;

struct eepromData {
  char data[30];
};




void writeEEPROM(int address, String data);
eepromData readEEPROM(int address) ;

//JSON
StaticJsonDocument<400> JSONencoder;
char JSONmessageBuffer[400];
uint8_t jsonData_len = 0;
char buff[400];

DynamicJsonDocument doc(1024); //Object for convert json txt

#ifndef REED_PIN
#define REED_PIN 13 // for rainguage
#endif

int rainCount;

ISR_PREFIX void rainCountFunc()
{
  rainCount = rainCount + 1;
  Serial.print("raincount");
  Serial.println(rainCount);
}

//sleep mode 
#define TIME_MICRO_SECONDS 1000000ULL 


void writeDefaultParam() {

  Serial.println("WRITE DEFAULT PARAM");
  writeEEPROM(ADS_DEVICEID, DEVICEID);
  delay(1000);
  writeEEPROM(ADS_CMDCHANNEL, CMDCHANNEL);
  delay(500);
  EEPROM_writeAnything(ADS_INTERVAL, 10);
  delay(200);
  EEPROM_writeAnything(ADS_STATIONHEIGHT, HEIGHT);
  delay(200);
  EEPROM_writeAnything(ADS_LACKLEVEL, LACK);
  delay(200);
  EEPROM_writeAnything(ADS_NORMALLEVEL, NORMAL);
  delay(200);
  EEPROM_writeAnything(ADS_WARNLEVEL, WARN);
  delay(200);
  EEPROM_writeAnything(ADS_DANGERLEVEL, DANGER);
  delay(200);
  EEPROM_writeAnything(ADS_RAINFACTOR, RAINFACTOR);
  delay(200);
  EEPROM_writeAnything(ADS_DEVICESTATE,0);
  delay(200);
 
}

void setup()
{
  Serial.begin(115200);
  EEPROM.begin(512);
  delay(1000);
  if (WRITESTATE){
    writeDefaultParam();
  }
    
  delay(1000);
  readDefaultParam();
  delay(1000);
  deviceID = readEEPROM(ADS_DEVICEID).data;
  Serial.printf("DEVICE ID : %s\n", deviceID);
  delay(1000);

  cmdChannel = readEEPROM(ADS_CMDCHANNEL).data;
  delay(500);
  Serial.printf("Channel :%s \n", cmdChannel);
  Serial.printf("Device Name : %s \n", deviceName);
  delay(1000);
  Serial.printf("Software : %s \n", APP_NAME);
  delay(1000);
  Serial.printf("version : %s \n", APP_VERSION);

  

  pinMode(REED_PIN, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(REED_PIN), rainCountFunc, FALLING);

  delay(1000);
  printParameter();

  //beforeLevel = messureWaterLevel();

  hdc1080.begin(0x40);
  nb.begin();
  setupMQTT();
  beforeLevel = messureWaterLevel();
  nb.setCallback(callback);
  regisDevice();
  delay(1000);
  sendDataToServer();

  previousMillis = millis();
  unsigned long duration = SECONDS_DS(interval*60);
  //esp_sleep_enable_timer_wakeup(duration); 
   
  //  WiFi.disconnect(true);
  //  btStop();
  
}

void loop()
{
  nb.MQTTresponse();
  unsigned long duration = SECONDS_DS(interval*60);
   if(MM.get() > duration){
       
       sendDataToServer();
        
        MM.reset();
     // 
      }
 
    if(touchRead(33) <2){
      messureWaterLevel();
    }
   // esp_light_sleep_start();
    
   //Serial.println(nb.getSignal());
   //delay(1000);
    
      

}

//=========== MQTT Function ================
void setupMQTT()
{
  if (!nb.connectMQTT(address, serverPort, deviceID, username, password))
  {
    Serial.println("\nconnectMQTT");
  }
  cmdChannel = readEEPROM(ADS_CMDCHANNEL).data;
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
  SETPARAM,
  SETSTATE,
  SENDPARAM,
}; 
void callback(String &topic, String &payload, String &QoS, String &retained)
{
  Serial.println("-------------------------------");
  Serial.println("# Message from Topic \"" + topic + "\" : " + nb.toString(payload));
  

  Serial.println("# QoS = " + QoS);
  Serial.println("# QoS = " + retained);
  deserializeJson(doc, nb.toString(payload));
  JsonObject obj = doc.as<JsonObject>();
  int cmd = obj[String("cmd")];
  Serial.println(cmd);
  switch (cmd)
  {
  case REBOOT:
  { //0
    payload = "{\"event\":\"Rebooting\"}";
    sendLogMsg("log",payload);
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
    interval = obj[String("time")];
    if (interval ==0) return;
    sprintf(EVENT, "{\"event\":\"Set new interval %d min\",\"field\":\"intervel\",\"value\":%d}",interval,interval);

    sendLogMsg("log",EVENT);
    Serial.printf("Set interval :%i min \n",interval);
    EEPROM_writeAnything(ADS_INTERVAL,interval);
    //setTimeReadData(interval);
  }
  break;

  case READDATA:
  { //2
    Serial.println('Read Data');
    payload = "{\"event\":\"Read Station Data\"}";
    sendLogMsg("log",payload);
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
    else if (String(field) == "warnLevel")
    {
      warnLevel = value;
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
    sprintf(EVENT, "{\"event\":\"Set new parameter %s = %d\",\"field\":\"%s\",\"value\":%d}", field, value,field,value);

    sendLogMsg("log",EVENT);
  }
  break;
  case SETSTATE : {
    deviceState = obj[String("state")];
    sprintf(EVENT, "{\"event\":\"Set device state %s \",\"field\":\"state\",\"value\":%d}",
           deviceState==0 ? "test":"real",deviceState);
    Serial.println(EVENT);
    
    sendLogMsg("log",EVENT);
    EEPROM_writeAnything(ADS_DEVICESTATE,deviceState);
  }break;
  case SENDPARAM :{ //5
      sprintf(EVENT, "{\"event\":\"send Device Parameter \"}");
      sendLogMsg("log",EVENT);
      delay(1000);
      sprintf(EVENT,"{\"height\":%d,\"normalLevel\":%d,\"lackLevel\":%d,\"warnLevel\":%d,\"dangerLevel\":%d,\"rainFactor\":%d,\"interval\":%d,\"deviceState\":%d}",stationHeight,normalLevel,lackLevel,warnLevel,dangerLevel,rainFactor,interval,deviceState);
      sendLogMsg("param",EVENT);


  }break;
  default:{
     sprintf(EVENT, "{\"event\":\"Unknow Command\"}");
    sendLogMsg("log",EVENT);
    }break;
  }
  
  int idxPro =topic.indexOf('/');
  int idxChannel = topic.indexOf('/',idxPro+1);
  int idxCmd = topic.indexOf('/',idxChannel+1);
  
  String msgId = topic.substring(idxCmd+1);
  sendResponse(msgId);

  

  if (retained.indexOf(F("1")) != -1)
  {
    Serial.println("# Retained = " + retained);
  }
}

void sendResponse(String msgId){
  connectStatus();
  deviceID = readEEPROM(ADS_DEVICEID).data;
  sprintf(TOPIC,MQTTResponse,deviceID,msgId);
  nb.publish(TOPIC,"{\"result\":true}",pubQoS,pubRetained,pubDuplicate);
  delay(2000);
}

void sendLogMsg(String logType,String msg)
{
  connectStatus();
  deviceID = readEEPROM(ADS_DEVICEID).data;
  Serial.printf("send log to server : %s \n", msg);
  if (logType == "log")
  sprintf(TOPIC, MQTTTopic, deviceID, "log");
  else if (logType=="param")
  sprintf(TOPIC, MQTTTopic, deviceID, "param");

  nb.publish(TOPIC, msg,pubQoS,pubRetained,pubDuplicate);
  delay(2000);
  
}

/*void setTimeReadData(int min)
{
  interval = min * 60000;
  Serial.println("interval set to " + String(min) + " min");
}*/

void sendDataToServer()
{
  //clearJson();

  connectStatus();

  Serial.println(nb.getSignal());
  int waterLevel = messureWaterLevel();

  Serial.printf("Before = %d water level : %d \n", beforeLevel, waterLevel);

  float diffRate = 0;
  if (beforeLevel - waterLevel != 0)
  {
    diffRate = (waterLevel - beforeLevel) * 60 / (interval * 100.0); // unit in m/hr
  }
  beforeLevel = waterLevel;

  Serial.printf("Rain count = %d \n", rainCount);
  float rainFall = rainCount * rainFactor / 10.0;

  String temperature = String(hdc1080.readTemperature());
  String humidity = String(hdc1080.readHumidity());
  String Light = String(analogRead(lightSensorPin));
  String status;
  if (waterLevel <= lackLevel)
    status = "LACK";
  else if (waterLevel > lackLevel && waterLevel <= normalLevel)
    status = "NORMAL";
  else if (waterLevel > normalLevel && waterLevel <= warnLevel)
    status = "WARNING";
  else if (waterLevel > warnLevel && waterLevel <= dangerLevel)
    status = "DANGER";
  else if (waterLevel > dangerLevel)
    status = "EXTREME";

  if (deviceState==0)
    payload = "{\"test\":true,\"temperature\":" + temperature + ",\"humidity\":" + humidity + ",\"light\":" + Light + ",\"waterLevel\":" + waterLevel + ",\"diffRate\":" + diffRate + ",\"rainFall\":" + rainFall + ",\"status\":\"" + status + "\",\"rssi\":" + nb.getSignal() + "}";
  else
    payload = "{\"temperature\":" + temperature + ",\"humidity\":" + humidity + ",\"light\":" + Light + ",\"waterLevel\":" + waterLevel + ",\"diffRate\":" + diffRate + ",\"rainFall\":" + rainFall + ",\"status\":\"" + status + "\",\"rssi\":" + nb.getSignal() + "}";
  deviceID = readEEPROM(ADS_DEVICEID).data;
  sprintf(TOPIC, MQTTTopic, deviceID, "data");
  nb.publish(TOPIC, payload,pubQoS,pubRetained,pubDuplicate);
  delay(2000);
  rainCount = 0;
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
  sendLogMsg("log","{\"event\":\"Restart Device\"}");
}

void printParameter()
{
  Serial.printf("Station height : %d cm \n", stationHeight);
  Serial.printf("Station interval : %d min \n", interval );
  Serial.printf("lack Water level : %d cm \n", lackLevel);
  Serial.printf("normal water level : %d cm \n", normalLevel);
  Serial.printf("warning water level : %d cm \n", warnLevel);
  Serial.printf("danger water level : %d cm \n", dangerLevel);
  Serial.printf("Rain factor : %d  \n", rainFactor);
  Serial.printf("Device state : %s \n",deviceState ==0 ? "test":"real");
}

int messureWaterLevel()
{

  int dist[10];
  for (size_t i = 0; i < 10; i++)
  {
    dist[i] = ultrasonic.read();
    Serial.printf("Distance = %d\n",dist[i]);
    delay(1000);
  }
  int distLength = sizeof(dist) / sizeof(dist[0]);
  qsort(dist, distLength, sizeof(dist[0]), sort_desc);

  int distance = round((dist[2] + dist[3] + dist[4]+dist[5] + dist[6] + dist[7]) / 6);

  int waterLevel = stationHeight - distance;

  return waterLevel;
}



void readDefaultParam() {
  Serial.println("SET DEFAULT PARAM");

  EEPROM_readAnything(ADS_STATIONHEIGHT, stationHeight);
  delay(100);
  EEPROM_readAnything(ADS_LACKLEVEL, lackLevel);
  delay(100);
  EEPROM_readAnything(ADS_NORMALLEVEL, normalLevel);
  delay(100);
  EEPROM_readAnything(ADS_WARNLEVEL, warnLevel);
  delay(100);
  EEPROM_readAnything(ADS_DANGERLEVEL, dangerLevel);
  delay(100);
  EEPROM_readAnything(ADS_RAINFACTOR, rainFactor);
  delay(100);
  EEPROM_readAnything(ADS_INTERVAL, interval);
  delay(100);
  EEPROM_readAnything(ADS_DEVICESTATE,deviceState);
  delay(100);
 
}

eepromData readEEPROM(int address) {

  unsigned char k;
  uint8_t len = 0;
  eepromData v;
  k = EEPROM.read(address);
  while (k != '\0' && len < 30) {
    k = EEPROM.read(address + len);
    v.data[len] = k;
    len++;
  }
  v.data[len] = '\0';
  return v;
}




/**********************************
 * Write EEPROM
* https://circuits4you.com/2018/10/16/arduino-reading-and-writing-string-to-eeprom/
***********************************/
void writeEEPROM(int address, String data) {
  int _size = data.length();
  for (int i = 0; i < _size; i++) {
    EEPROM.write(i + address, data[i]);
  }
  EEPROM.write(address + _size, '\0');
  EEPROM.commit();
}
