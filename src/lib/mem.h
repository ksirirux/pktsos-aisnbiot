#include <Arduino.h>
#include <EEPROM.h>

struct eepromData
{
    char data[30];
};

class MEM
{
private:
    /* data */
public:
    MEM();
    ~MEM();
    void init();
    eepromData readEEPROM(int address);
    void writeEEPROM(int address, String data);
    void writeBoolEEPROM(int address, bool b);
    bool readBooleanEEPROM(int address);
   
    void getDefaultValue(); //getDefault value
    void readDefaultValue();
   
    void saveBatConfig();

    //For First Run Only
    void writeDefaultValue();
};

/*MEM::mem()
{
}

MEM::~mem()
{
}*/
