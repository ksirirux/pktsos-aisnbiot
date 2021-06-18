
/****************
 * Class from Mqtt Connection
 * 
 * ************************/


#include <Arduino.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>


class MQTT
{
private:
    /* data */
public:
    MQTT();
    ~MQTT();

    void init();
    void regisNode();

    void logMsg(String msg,uint8_t state);
    void sendDataToServer();
};

MQTT::MQTT(/* args */)
{
}

MQTT::~MQTT()
{
}


