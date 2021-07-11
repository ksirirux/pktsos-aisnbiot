#define APP_NAME "PKTSOS-NBIOT"
#define APP_VERSION "v 1.01.01"
#define FW_VERSION 2021010101 //year|version xx.xx.xx
#define HWID "AISNB-01"


//DEVICE
#ifdef WATPO
    #define DEVICEID "60c9e927643cf850d09eed61"
    #define CMDCHANNEL "6210323"
    #define DEVICENAME "TEST ทดสอบ"
    #define HEIGHT 355
    #define LACK  40
    #define NORMAL 140
    #define WARN 200
    #define DANGER 250
    #define RAINFACTOR 2
#endif

#ifdef TEST
    
    #define DEVICEID "60c9e927643cf850d09eed66"
    #define CMDCHANNEL "6210361"
    #define DEVICENAME "TEST ทดสอบ"
    #define HEIGHT 355
    #define LACK   30
    #define NORMAL 140
    #define WARN 200
    #define DANGER 250
    #define RAINFACTOR 2
   
#endif

#ifdef KLONGHUAKLONG
    #define DEVICEID "60c9e927643cf850d09eed63"
    #define CMDCHANNEL "6210333"
    #define DEVICENAME "KLONGHUAKLONG"
    #define HEIGHT 357
    #define LACK   60
    #define NORMAL 180
    #define WARN 230
    #define DANGER 300
    #define RAINFACTOR 2
#endif

#ifdef KAONIYOM
    #define DEVICEID "60c9e927643cf850d09eed67"
    #define CMDCHANNEL "6210311"
    #define DEVICENAME "TEST ทดสอบ"
    #define HEIGHT 367
    #define LACK   50
    #define NORMAL 160
    #define WARN 200
    #define DANGER 240
    #define RAINFACTOR 2
#endif
#ifdef PRUGMAWAR
    #define DEVICEID "60c9e927643cf850d09eed68"
    #define CMDCHANNEL "6210368"
    #define DEVICENAME "PRUGMAWAR"
    #define HEIGHT 312
    #define LACK   20
    #define NORMAL 100
    #define WARN 150
    #define DANGER 250
    #define RAINFACTOR 2
#endif

#ifdef MONKEY
    #define DEVICEID "60c9e927643cf850d09eed75"
    #define CMDCHANNEL "6210378"
    #define DEVICENAME "MONKEY STA"
    #define HEIGHT 300
    #define LACK   10
    #define NORMAL 70
    #define WARN 100
    #define DANGER 130
    #define RAINFACTOR 2
#endif



