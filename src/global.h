#define APP_NAME "PKTSOS-NBIOT"
#define APP_VERSION "v 1.01.01"
#define FW_VERSION 2021010101 //year|version xx.xx.xx
#define HWID "AISNB-01"

/*****************************
 * EEPROM Parametre
 *  ADS =>Address
 * ****************************/
#define EEPROM_SIZE 512
#define ADS_DEVICEID 0
#define ADS_INTERVAL 15




//DEVICE
#ifdef WATPO
    #define DEVICEID "60c9e927643cf850d09eed66"
    #define CMDCHANNEL "6210323"
#endif

#ifdef TEST
    #define DEVICEID "60c9e927643cf850d09eed61"
    #define CMDCHANNEL "6210361"
#endif

#ifdef KLONGHUAKLONG
    #define DEVICEID "60c9e927643cf850d09eed63"
    #define CMDCHANNEL "6210333"
#endif

#ifdef KAONIYOM
    #define DEVICEID "60c9e927643cf850d09eed67"
    #define CMDCHANNEL "6210311"
#endif
#ifdef PRUGMAWAR
    #define DEVICEID "60c9e927643cf850d09eed68"
    #define CMDCHANNEL "6210368"
#endif

#ifdef MONKEY
    #define DEVICEID "60c9e927643cf850d09eed75"
    #define CMDCHANNEL "6210378"
#endif



