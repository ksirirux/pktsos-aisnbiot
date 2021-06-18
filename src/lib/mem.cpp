#include "mem.h"
#include "global.h"
#include "EEPROM_writeall.h"


float batLow ;
float batFull;

struct battLevel{
    float batLow;
    float batFull;

}BattLevel;

struct wifiConfig{
    char ssid;
    char password;
}WifiConfig;

MEM::MEM(){

};

void MEM::init(){
    EEPROM.begin(EEPROM_SIZE);
}

eepromData MEM::readEEPROM(int address){
    unsigned char k;
    uint8_t len = 0;
    eepromData  v;
    k = EEPROM.read(address);
    while(k !='\0' && len<30){
        k = EEPROM.read(address+len);
        v.data[len]= k;
        len++;
    }
    v.data[len]='\0';
    return v;
}

void MEM::writeEEPROM(int address,String data){
    int _size = data.length();
    for (size_t i = 0; i < _size; i++)  {
        EEPROM.write(i+address,data[i]);
    }
    EEPROM.write(address+_size,'\0');
    EEPROM.commit();
}

void MEM::writeBoolEEPROM(int address,bool b){
    EEPROM.write(address,b);
    EEPROM.commit();
}

bool MEM::readBooleanEEPROM(int address){
    return EEPROM.read(address);
    
}
