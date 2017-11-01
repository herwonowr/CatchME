#ifndef Settings_h
#define Settings_h

#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "Mac.h"
#include "MacList.h"
#include "NameList.h"

extern "C" {
  #include "user_interface.h"
}

extern void sendBuffer();
extern void sendToBuffer(String str);
extern void sendHeader(int code, String type, size_t _size);

extern const bool debug;
extern String data_getVendor(uint8_t first, uint8_t second, uint8_t third);
extern void eepromWriteInt(int adr, int val);
extern int eepromReadInt(int adr);
extern NameList nameList;

// EEPROM address allocation
#define ssidLenAdr 1024
#define ssidAdr 1025 // 32 byte
#define ssidHiddenAdr 1058
#define passwordLenAdr 1059
#define passwordAdr 1060 // 32 byte
#define apChannelAdr 1093
#define wifiClientAdr 1094
#define ssidClientLenAdr 1195
#define ssidClientAdr 1196 // 32 byte
#define passwordClientLenAdr 1229
#define passwordClientAdr 1230 // 32 byte
#define hostnameLenAdr 1263 
#define hostnameAdr 1264 // 32 byte
#define macAPAdr 1297 // 6 byte
#define isMacAPRandAdr 1304
#define apScanHiddenAdr 1305
#define deauthReasonAdr 1306
#define attackTimeoutAdr 1307 // 2 byte
#define attackPacketRateAdr 1310
#define clientScanTimeAdr 1311
#define beaconIntervalAdr 1312
#define useLedAdr 1313
#define ledPinAdr 1314
#define useAlertLedAdr 1315
#define alertLedPinAdr 1316
#define multiAPsAdr 1317
#define multiAttacksAdr 1318
#define macIntervalAdr 1319 // 2 byte
#define cacheAdr 1322
#define serverCacheAdr 1323 // 2 byte
#define detectorChannelAdr 1326
#define detectorAllChannelsAdr 1327
#define detectorScanTimeAdr 1328 // 2 byte
#define newUserAdr 1331
#define modeOpsAdr 1332

#define checkNumAdr 3000
#define checkNum 16

class Settings
{
  public:
    Settings();
    void load();
    void reset();
    void save();
    void send();
    void sendSysInfo();
    void info();

    int ssidLen;
    String ssid = "";
    bool ssidHidden;
    int passwordLen;
    String password = "";
    int apChannel;
    
    bool wifiClient;
    int ssidClientLen;
    String ssidClient = "";
    int passwordClientLen;
    String passwordClient = "";
    int hostnameLen;
    String hostname = "";
    Mac defaultMacAP;
    Mac macAP;

    bool isMacAPRand;
    bool apScanHidden;
    uint8_t deauthReason;
    unsigned int attackTimeout;
    int attackPacketRate;
    int clientScanTime;
    bool beaconInterval;
    bool useLed;
    int ledPin = 0;
    bool useAlertLed;
    int alertLedPin = 16;
    bool multiAPs;
    bool multiAttacks;
    int macInterval;
    bool cache;
    int serverCache;
    int detectorChannel;
    bool detectorAllChannels;
    int detectorScanTime;
    bool newUser;
    bool modeOps; // baru
     
    int prevLedPin = 0;
    int prevAlertLedPin = 16;

    void syncMacInterface();
    void setLedPin(int newLedPin);
    void setAlertLedPin(int newAlertLedPin); // baru fungsi belum ada
    
    bool pinStateOff = true;  // PIN state is HIGH mean led off
    
    bool isSettingsLoaded = 0;
    
    unsigned long deauthpackets = 0;
    unsigned long beaconpackets = 0;

  private:
    size_t getSize();
    size_t getSysInfoSize();
};

#endif
