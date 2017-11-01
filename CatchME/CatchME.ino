/***********************************************************************************
************************************************************************************
* CatchME is developed by Herwono W. Wijaya < b175 [ at ] protonmail [ dot ] com > *
*       This project is based on awesome work of Sam Denty and Stefan Kremser      *
***********************************************************************************/

/***********************************************************************************
************************************************************************************
*                        https://github.com/samdenty99/Wi-PWN                      *
*                                                                                  *
*                                 (c) 2017 Sam Denty                               *
*                             https://samdd.me/projects                            *
*                                                                                  *
*----------------------------------------------------------------------------------*
*                    Wi-PWN is based on spacehuhn/esp8266_deauther                 *
*                                 (c) Stefan Kremser                               *
***********************************************************************************/
// Including libraries //
#include <Arduino.h>

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiClient.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

// Defined //
#define GPIO0_DEAUTH_BUTTON

// Number of seconds after reset during which a 
// subseqent reset will be considered a double reset
// millis / 255
#define DOUBLE_RESET_TIMEOUT 20

// Address EEPROM for the Double Reset Detector to use
#define DOUBLE_RESET_ADR 0
#define DOUBLE_RESET_FLAG_ADR 1

// HTTP server
ESP8266WebServer server(80);

// OTA Update server
ESP8266HTTPUpdateServer httpUpdater(true);

#include "data.h"
#include "NameList.h"
#include "APScan.h"
#include "ClientScan.h"
#include "Attack.h"
#include "Settings.h"
#include "SSIDList.h"

// DEBUG
const bool debug = true;

// Run-Time Variables //
String wifiMode = "";
String attackMode_deauth = "";
String attackMode_beacon = "";
String scanMode = "SCAN";

// Deauth detector
bool detecting = false;
int detectorLedState = settings.pinStateOff;
unsigned long dC = 0;
unsigned long prevTime = 0;
unsigned long curTime = 0;
int curChannel = settings.detectorChannel;

NameList nameList;
APScan apScan;
ClientScan clientScan;
Attack attack;
Settings settings;
SSIDList ssidList;

void sniffer(uint8_t *buf, uint16_t len) {
  clientScan.packetSniffer(buf, len);
}

void startWifi() {
  Serial.println("\nStarting WiFi AP:");
  WiFi.mode(WIFI_AP_STA);
  wifi_set_promiscuous_rx_cb(sniffer);
  WiFi.softAP((const char*)settings.ssid.c_str(), (const char*)settings.password.c_str(), settings.apChannel, settings.ssidHidden); //for an open network without a password change to:  WiFi.softAP(ssid);
  if (settings.wifiClient && settings.ssidClient) {
    Serial.print("Connecting to WiFi network '"+settings.ssidClient+"' using the password '"+settings.passwordClient+"' ");
    if (settings.hostname) WiFi.hostname(settings.hostname);
    WiFi.begin((const char*)settings.ssidClient.c_str(), (const char*)settings.passwordClient.c_str());
    int conAtt = 0;
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
      conAtt++;
      if (conAtt > 30) {
        Serial.println("");
        Serial.println("Failed to connect to '"+settings.ssidClient+"', skipping connection\n");
        goto startWifi;
      }
    }
    
    Serial.println(" connected!");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
    Serial.print("Netmask: ");
    Serial.println(WiFi.subnetMask());
    Serial.print("Gateway: ");
    Serial.println(WiFi.gatewayIP());
    Serial.println("");
  }
  startWifi:
  Serial.println("SSID          : '" + settings.ssid+"'");
  Serial.println("Password      : '" + settings.password+"'");

  if (settings.newUser == 1) {Serial.println("Redirecting to setup page");}
  Serial.println("-----------------------------------------------");
  if (settings.password.length() < 8) Serial.println("WARNING: password must have at least 8 characters!");
  if (settings.ssid.length() < 1 || settings.ssid.length() > 32) Serial.println("WARNING: SSID length must be between 1 and 32 characters!");
  wifiMode = "ON";
}

void stopWifi() {
  Serial.println("stopping WiFi AP");
  Serial.println("-----------------------------------------------");
  WiFi.disconnect();
  wifi_set_opmode(STATION_MODE);
  wifiMode = "OFF";
}

/** load Web Management assets **/
// load HTML assets
void loadSetupHTML() {
    server.sendHeader("Cache-Control", "no-cache, no-store, must-revalidate");
    server.sendHeader("Pragma", "no-cache");
    server.sendHeader("Expires", "0");
    sendFile(200, "text/html", data_setup_HTML, sizeof(data_setup_HTML), true);
}
void loadIndexHTML() {
  sendFile(200, "text/html", data_index_HTML, sizeof(data_index_HTML), false);
}
void loadUsersHTML() {
  sendFile(200, "text/html", data_users_HTML, sizeof(data_users_HTML), false);
}
void loadAttackHTML() {
  sendFile(200, "text/html", data_attack_HTML, sizeof(data_attack_HTML), false);
}
void loadDetectorHTML() {
  sendFile(200, "text/html", data_detector_HTML, sizeof(data_detector_HTML), false);
}
void loadSettingsHTML() {
  sendFile(200, "text/html", data_settings_HTML, sizeof(data_settings_HTML), false);
}
void load404() {
  sendFile(404, "text/html", data_error_HTML, sizeof(data_error_HTML), false);
}
void loadInfoHTML(){
  sendFile(200, "text/html", data_info_HTML, sizeof(data_info_HTML), false);
}

// load JS assets
void loadScanJS() {
  sendFile(200, "text/javascript", data_scan_JS, sizeof(data_scan_JS), false);
}
void loadUsersJS() {
  sendFile(200, "text/javascript", data_users_JS, sizeof(data_users_JS), false);
}
void loadAttackJS() {
  attack.ssidChange = true;
  sendFile(200, "text/javascript", data_attack_JS, sizeof(data_attack_JS), false);
}
void loadSettingsJS() {
  sendFile(200, "text/javascript", data_settings_JS, sizeof(data_settings_JS), false);
}
void loadInfoJS() {
  sendFile(200, "text/javascript", data_info_JS, sizeof(data_info_JS), false);
}
void loadFunctionsJS() {
  sendFile(200, "text/javascript", data_functions_JS, sizeof(data_functions_JS), false);
}

// load CSS assets
void loadStyle() {
  sendFile(200, "text/css;charset=UTF-8", data_main_CSS, sizeof(data_main_CSS), false);
}

// load page redirection
void loadRedirectHTML() {
    server.send(302, "text/html", "<meta content='0; url=http://192.168.4.1'http-equiv='refresh'>");
}


void startWiFi(bool start) {
  if (start) startWifi();
  else stopWifi();
  clientScan.clearList();
}

// AP-Scan
void startAPScan() {
  scanMode = "scanning...";

  if (apScan.start()) {
    server.send ( 200, "text/json", "true");
    attack.stopAll();
    scanMode = "SCAN";
  }
}

void sendAPResults() {
  apScan.sendResults();
}

void selectAP() {
  if (server.hasArg("num")) {
    apScan.select(server.arg("num").toInt());
    server.send( 200, "text/json", "true");
    // Remove below in a future update
    attack.stopAll();
  }
}

// Client-Scan
void startClientScan() {
  if (server.hasArg("time") && apScan.getFirstTarget() > -1 && !clientScan.sniffing) {
    server.send(200, "text/json", "true");
    clientScan.start(server.arg("time").toInt());
    attack.stopAll();
  } else server.send( 200, "text/json", "ERROR: No selected Wi-Fi networks!");
}

void sendClientResults() {
  clientScan.send();
}
void sendClientScanTime() {
  server.send( 200, "text/json", (String)settings.clientScanTime );
}

void selectClient() {
  if (server.hasArg("num")) {
    clientScan.select(server.arg("num").toInt());
    attack.stop(0);
    server.send( 200, "text/json", "true");
  }
}

void addClientFromList(){
  if(server.hasArg("num")) {
    int _num = server.arg("num").toInt();
    clientScan.add(nameList.getMac(_num));
    
    server.send( 200, "text/json", "true");
  }else server.send( 200, "text/json", "false");
}

void setClientName() {
  if (server.hasArg("id") && server.hasArg("name")) {
    if(server.arg("name").length()>0){
      nameList.add(clientScan.getClientMac(server.arg("id").toInt()), server.arg("name"));
      Serial.println("Set client: "+clientScan.getClientMac(server.arg("id").toInt()).toString());
      server.send( 200, "text/json", "true");
    }
    else server.send( 200, "text/json", "false");
  }
}

void deleteName() {
  if (server.hasArg("num")) {
    int _num = server.arg("num").toInt();
    nameList.remove(_num);
    server.send( 200, "text/json", "true");
  }else server.send( 200, "text/json", "false");
}

void clearNameList() {
  nameList.clear();
  server.send( 200, "text/json", "true" );
}

void editClientName() {
  if (server.hasArg("id") && server.hasArg("name")) {
    nameList.edit(server.arg("id").toInt(), server.arg("name"));
    server.send( 200, "text/json", "true");
  }else server.send( 200, "text/json", "false");
}

// BUGS already fixed
void addClient(){
  if(server.hasArg("mac") && server.hasArg("name")){
    String macStr = server.arg("mac");
    Serial.println("add "+macStr+" - "+server.arg("name"));
    macStr.replace(":","");
    Mac tempMac; // gunakan ini untuk memperbaiki add user manual
    if(macStr.length() == 12){
       for(int i=0;i<6;i++){
         const char* val = macStr.substring(i*2,i*2+2).c_str();
         uint8_t valByte = strtoul(val, NULL, 16);
         tempMac.setAt(valByte,i);
       }
       if(tempMac.valid()) nameList.add(tempMac,server.arg("name"));
       server.send( 200, "text/json", "true");
    } else if(macStr.length() != 12){
       server.send( 200, "text/json", "false");
    }
  }
}

// Attack
void sendAttackInfo() {
  attack.sendResults();
}

void startAttack() {
  if (server.hasArg("num")) {
    int _attackNum = server.arg("num").toInt();
    if (apScan.getFirstTarget() > -1 || _attackNum == 1 || _attackNum == 2) {
      attack.start(server.arg("num").toInt());
      server.send ( 200, "text/json", "true");
    } else server.send( 200, "text/json", "false");
  }
}

void addSSID() {
  if(server.hasArg("ssid") && server.hasArg("num") && server.hasArg("enc")){
    int num = server.arg("num").toInt();
    if(num > 0){
      ssidList.addClone(server.arg("ssid"),num, server.arg("enc") == "true");
    }else{
      ssidList.add(server.arg("ssid"), server.arg("enc") == "true" || server.arg("enc") == "1");
    }
    attack.ssidChange = true;
    server.send( 200, "text/json", "true");
  } else server.send( 200, "text/json", "false");
}

void cloneSelected(){
  if(apScan.selectedSum > 0){
    int clonesPerSSID = 48/apScan.selectedSum;
    ssidList.clear();
    for(int i=0;i<apScan.results;i++){
      if(apScan.isSelected(i)){
        ssidList.addClone(apScan.getAPName(i),clonesPerSSID, apScan.getAPEncryption(i) != "none");
      }
    }
  }
  attack.ssidChange = true;
  server.send( 200, "text/json", "true");
}

void deleteSSID() {
  ssidList.remove(server.arg("num").toInt());
  attack.ssidChange = true;
  server.send( 200, "text/json", "true");
}

void randomSSID() {
  ssidList._random();
  attack.ssidChange = true;
  server.send( 200, "text/json", "true");
}

void clearSSID() {
  ssidList.clear();
  attack.ssidChange = true;
  server.send( 200, "text/json", "true");
}

void resetSSID() {
  ssidList.load();
  attack.ssidChange = true;
  server.send( 200, "text/json", "true");
}

void saveSSID() {
  ssidList.save();
  server.send( 200, "text/json", "true");
}

void restartESP() {
  server.send( 200, "text/json", "true");
  ESP.restart();
}

void enableRandom() {
  server.send( 200, "text/json", "true");
  attack.changeRandom(server.arg("interval").toInt());
}

void startDetector() {
  Serial.println("Starting Deauth Detector...");
  server.send( 200, "text/json", "true");

  wifi_set_opmode(STATION_MODE);
  wifi_promiscuous_enable(0);
  WiFi.disconnect();
  wifi_set_promiscuous_rx_cb(dSniffer);
  wifi_set_channel(curChannel);
  wifi_promiscuous_enable(1);

  pinMode(settings.alertLedPin, OUTPUT);
  detecting = true;
}

void dSniffer(uint8_t *buf, uint16_t len) {
    if(buf[12] == 0xA0 || buf[12] == 0xC0){
      dC++;
    }
}

// Settings
void getSettings() {
  settings.send();
}

void getSysInfo() {
  settings.sendSysInfo();
}

void saveSettings() {
  server.send( 200, "text/json", "true" );
  if (server.hasArg("ssid")) settings.ssid = server.arg("ssid");
  
  if (server.hasArg("ssidHidden")) {
    if (server.arg("ssidHidden") == "false") settings.ssidHidden = false;
    else settings.ssidHidden = true;
  }
  
  if (server.hasArg("password")) settings.password = server.arg("password");
  
  if (server.hasArg("apChannel")) {
    if (server.arg("apChannel").toInt() >= 1 && server.arg("apChannel").toInt() <= 14) {
      settings.apChannel = server.arg("apChannel").toInt();
    }
  }
  
  if (server.hasArg("wifiClient")) {
    if (server.arg("wifiClient") == "false") settings.wifiClient = false;
    else settings.wifiClient = true;
  }
  
  if (server.hasArg("ssidClient")) settings.ssidClient = server.arg("ssidClient");
  if (server.hasArg("passwordClient")) settings.passwordClient = server.arg("passwordClient");
  if (server.hasArg("hostname")) settings.hostname = server.arg("hostname");
  
  if (server.hasArg("macAp")) {
    String macStr = server.arg("macAp");
    macStr.replace(":","");
    Mac tempMac; // gunakan ini untuk memperbaiki add user manual
     if(macStr.length() == 12){
       for(int i=0;i<6;i++){
         const char* val = macStr.substring(i*2,i*2+2).c_str();
         uint8_t valByte = strtoul(val, NULL, 16);
         tempMac.setAt(valByte,i);
       }
       if(tempMac.valid()) settings.macAP.set(tempMac);
     } else if(macStr.length() == 0){
       settings.macAP.set(settings.defaultMacAP);
     }
  }
  
  if (server.hasArg("randMacAp")) {
    if (server.arg("randMacAp") == "false") settings.isMacAPRand = false;
    else settings.isMacAPRand = true;
  }

  if (server.hasArg("apScanHidden")) {
    if (server.arg("apScanHidden") == "false") settings.apScanHidden = false;
    else settings.apScanHidden = true;
  }

  if (server.hasArg("deauthReason")) settings.deauthReason = server.arg("deauthReason").toInt();
  if (server.hasArg("timeout")) settings.attackTimeout = server.arg("timeout").toInt();
  if (server.hasArg("packetRate")) settings.attackPacketRate = server.arg("packetRate").toInt();
  if (server.hasArg("scanTime")) settings.clientScanTime = server.arg("scanTime").toInt();
  
  if (server.hasArg("beaconInterval")) {
    if (server.arg("beaconInterval") == "false") settings.beaconInterval = false;
    else settings.beaconInterval = true;
  }
  
  if (server.hasArg("useLed")) {
    if (server.arg("useLed") == "false") settings.useLed = false;
    else settings.useLed = true;
    attack.refreshLed();
  }

  if (server.hasArg("ledPin")) settings.setLedPin(server.arg("ledPin").toInt());
  
  if (server.hasArg("useAlertLed")) {
    if (server.arg("useAlertLed") == "false") settings.useAlertLed = false;
    else settings.useAlertLed = true;
  }

  if (server.hasArg("alertLedPin")) settings.setAlertLedPin(server.arg("alertLedPin").toInt());
  
  if (server.hasArg("multiAPs")) {
    if (server.arg("multiAPs") == "false") settings.multiAPs = false;
    else settings.multiAPs = true;
  }
  
  if (server.hasArg("multiAttacks")) {
    if (server.arg("multiAttacks") == "false") settings.multiAttacks = false;
    else settings.multiAttacks = true;
  }
  
  if(server.hasArg("macInterval")) settings.macInterval = server.arg("macInterval").toInt();

  if (server.hasArg("cache")) {
    if (server.arg("cache") == "false") settings.cache = false;
    else settings.cache = true;
  }

  if (server.hasArg("serverCache")) settings.serverCache = server.arg("serverCache").toInt();

  if (server.hasArg("detectorChannel")) settings.detectorChannel = server.arg("detectorChannel").toInt();

  if (server.hasArg("detectorAllChannels")) {
    if (server.arg("detectorAllChannels") == "false") settings.detectorAllChannels = false;
    else settings.detectorAllChannels = true;
  }

  if (server.hasArg("detectorScanTime")) settings.detectorScanTime = server.arg("detectorScanTime").toInt();

  if (server.hasArg("newUser")) {
    if (server.arg("newUser") == "false") settings.newUser = false;
    else settings.newUser = true;
  }

  if (server.hasArg("mode")) {
    if (server.arg("mode") == "false") settings.modeOps = false;
    else settings.modeOps = true;
  }
  
  settings.save();
}

void resetSettings() {
  settings.reset();
  server.send( 200, "text/json", "true" );
}

void drState(long m) {
  int maxTime = DOUBLE_RESET_TIMEOUT;
  int runTime = m / 255;
  
  if(runTime < 30) {
    // check millis time and write to EEPROM for Flag detection
    if(runTime >= maxTime) {
      EEPROM.write(DOUBLE_RESET_ADR, maxTime);
      EEPROM.commit();
    } else {
      EEPROM.write(DOUBLE_RESET_ADR, runTime);
      EEPROM.commit();
    }
  
    // set double reset Flag
    int drStateVal = EEPROM.read(DOUBLE_RESET_ADR);
    if(drStateVal < maxTime) {
      EEPROM.write(DOUBLE_RESET_FLAG_ADR, 1);
      EEPROM.commit();
    } else {
      EEPROM.write(DOUBLE_RESET_FLAG_ADR, 0);
      EEPROM.commit();
    }
  }
  
  if(EEPROM.read(DOUBLE_RESET_ADR) < maxTime) {
    delay(1500); // delay 2ms for proper EEPROM write and detect Flag before 5ms
  }
  
}

// detect double reset press botton
bool drDetect() {
  if(EEPROM.read(DOUBLE_RESET_FLAG_ADR) == 1){
    return true;
  } else {
    return false;
  }
}

void setup() {
  randomSeed(os_random());
  
  Serial.begin(115200);

  attackMode_deauth = "START";
  attackMode_beacon = "START";

  EEPROM.begin(4096);
  SPIFFS.begin();
  
  settings.load();
  if (debug) settings.info();
  settings.syncMacInterface();
  nameList.load();
  ssidList.load();

  attack.refreshLed();

  delay(500); // Prevent bssid leak

  startWifi();
  attack.stopAll();
  attack.generate();

  // Web Server
  if (settings.newUser == 1) {
    /* LOAD CatchME - Setup */
    /* HTML */
    server.onNotFound(loadRedirectHTML);
    server.on ("/", loadSetupHTML);
    server.on ("/index", loadSetupHTML);

    /* CSS */
    server.on ("/main.css", loadStyle);

    /* JS */
    server.on("/js/functions.js", loadFunctionsJS);

    /* JSON */
    server.on("/ClientScanTime.json", sendClientScanTime);
    server.on("/settingsSave.json", saveSettings);
    server.on("/restartESP.json", restartESP);
    server.on("/settingsReset.json", resetSettings);
  } else if (settings.modeOps == 1) {
    /* LOAD CatchME Detector Mode */ 
    /* HTML */
    server.onNotFound(loadRedirectHTML);
    server.on("/", loadDetectorHTML);
    server.on("/index", loadDetectorHTML);

    /* CSS */
    server.on ("/main.css", loadStyle);

    /* JS */
    server.on("/js/functions.js", loadFunctionsJS);

    /* JSON */
    server.on("/settings.json", getSettings);
    server.on("/sysinfo.json", getSysInfo);
    server.on("/detectorStart.json", startDetector);
    server.on("/ClientScanTime.json", sendClientScanTime);
    server.on("/settingsSave.json", saveSettings);
    server.on("/restartESP.json", restartESP);
  } else {
    /* LOAD CatchME Attack Mode */
    /* HTML */
    server.onNotFound(load404);
    server.on("/", loadIndexHTML);
    server.on("/index", loadIndexHTML);
    server.on("/users", loadUsersHTML);
    server.on("/attack", loadAttackHTML);
    server.on("/settings", loadSettingsHTML);
    server.on("/info", loadInfoHTML);
  
    /* JS */
    server.on("/js/functions.js", loadFunctionsJS);
    server.on("/js/scan.js", loadScanJS);
    server.on("/js/users.js", loadUsersJS);
    server.on("/js/attack.js", loadAttackJS);
    server.on("/js/settings.js", loadSettingsJS);
    server.on("/js/info.js", loadInfoJS);
  
    /* CSS */
    server.on ("/main.css", loadStyle);
  
    /* JSON */
    server.on("/APScanResults.json", sendAPResults);
    server.on("/APScan.json", startAPScan);
    server.on("/APSelect.json", selectAP);
    server.on("/ClientScan.json", startClientScan);
    server.on("/ClientScanResults.json", sendClientResults);
    server.on("/ClientScanTime.json", sendClientScanTime);
    server.on("/clientSelect.json", selectClient);
    server.on("/setName.json", setClientName);
    server.on("/addClientFromList.json", addClientFromList);
    server.on("/attackInfo.json", sendAttackInfo);
    server.on("/attackStart.json", startAttack);
    server.on("/settings.json", getSettings);
    server.on("/sysinfo.json", getSysInfo);
    server.on("/settingsSave.json", saveSettings);
    server.on("/settingsReset.json", resetSettings);
    server.on("/deleteName.json", deleteName);
    server.on("/clearNameList.json", clearNameList);
    server.on("/editNameList.json", editClientName);
    server.on("/addSSID.json", addSSID);
    server.on("/cloneSelected.json", cloneSelected);
    server.on("/deleteSSID.json", deleteSSID);
    server.on("/randomSSID.json", randomSSID);
    server.on("/clearSSID.json", clearSSID);
    server.on("/resetSSID.json", resetSSID);
    server.on("/saveSSID.json", saveSSID);
    server.on("/restartESP.json", restartESP);
    server.on("/addClient.json", addClient);
    server.on("/enableRandom.json", enableRandom);
  }
  
  // OTA Update
  httpUpdater.setup(&server);
  
  server.begin();

  if(debug){
    Serial.println("\nStarting...\n");
  }

  // setup for double RESET button press detection and do configuration reset
  if(drDetect()){
    Serial.println("Reseting Configuration...");
    // blink builtin LED 10 times to indicated reset configuration
    // Led in NodeMCU at pin GPIO16 (D0)
    pinMode(settings.alertLedPin, OUTPUT);
    
    for(int i=1; i <= 10; i++){
      digitalWrite(settings.alertLedPin, settings.pinStateOff);
      delay(80);
      digitalWrite(settings.alertLedPin, !settings.pinStateOff);
      delay(80);
      digitalWrite(settings.alertLedPin, settings.pinStateOff);
    }
    
    EEPROM.write(DOUBLE_RESET_FLAG_ADR, 0);
    EEPROM.commit();

    // reset the configuration
    resetSettings();

    delay(3000);
    restartESP();
  } else {
    Serial.println("Normal Reboot !");
    pinMode(settings.ledPin, OUTPUT);
    
    for(int i=1; i <= 2; i++){
      digitalWrite(settings.ledPin, settings.pinStateOff);
      delay(350);
      digitalWrite(settings.ledPin, !settings.pinStateOff);
      delay(350);
      digitalWrite(settings.ledPin, settings.pinStateOff);
    }
  }
  
}

void loop() {
  // detect double RESET button press to reset the configuration
  // double reset loop
  long m = millis();
  drState(m);
  
  if (detecting) {
    curTime = millis();
    if(curTime - prevTime >= settings.detectorScanTime){
      prevTime = curTime;
      Serial.println((String)dC+" - channel "+(String)curChannel);
      
      if(dC >= 2) {
        digitalWrite(settings.ledPin, settings.pinStateOff);
        digitalWrite(settings.alertLedPin, !settings.pinStateOff);
      } else {
        if (detectorLedState == settings.pinStateOff) {
          detectorLedState = !settings.pinStateOff;
        } else {
          detectorLedState = settings.pinStateOff;
        }

        digitalWrite(settings.alertLedPin, settings.pinStateOff);
        digitalWrite(settings.ledPin, detectorLedState);
      }
      
      dC = 0;
      if(settings.detectorAllChannels){
        curChannel++;
        if(curChannel > 14) curChannel = 1;
        wifi_set_channel(curChannel);
      }
    }
  } else if (settings.newUser == 1) {
    server.handleClient();
  } else {
    if (clientScan.sniffing) {
      if (clientScan.stop()) startWifi();
    } else {
      server.handleClient();
      attack.run();
    }

    #ifdef GPIO0_DEAUTH_BUTTON
      // Long-press  = triple LED blink + deauth all
      // Short-press = LED blink + toggle deauth attack on networks selected
      //               If no networks are selected, then deauth all
      // Make sure the device has been powered on for at least 10 seconds (prevents bootloop issue)
      if(digitalRead(0) == LOW && millis() > 10000) {
        Serial.println("FLASH button (GPIO0) pressed!");
        if(apScan.selectedSum == 0) {
          Serial.println("No networks selected... selecting & deauthing all networks");
          digitalWrite(settings.ledPin, !settings.pinStateOff);
          delay(50);
          digitalWrite(settings.ledPin, settings.pinStateOff);
          apScan.start();
          apScan.select(-1);
          attack.start(0);
        } else {
          int button_delay = 0;
          while (digitalRead(0) == LOW && millis() > 4000){
           button_delay++;
           delay(100);
           if(button_delay == 10){
              Serial.println("Attack / Flash button held down... selecting & deauthing all networks");
              digitalWrite(settings.ledPin, settings.pinStateOff);
              delay(50);
              digitalWrite(settings.ledPin, !settings.pinStateOff);
              delay(100);
              digitalWrite(settings.ledPin, settings.pinStateOff);
              delay(100);
              digitalWrite(settings.ledPin, !settings.pinStateOff);
              delay(100);
              digitalWrite(settings.ledPin, settings.pinStateOff);
              delay(100);
              digitalWrite(settings.ledPin, !settings.pinStateOff);
              delay(100);
              digitalWrite(settings.ledPin, settings.pinStateOff);
              apScan.start();
              apScan.select(-1);
              attack.start(0);
              break;
           }
          }
          if(button_delay < 10) {
            digitalWrite(settings.ledPin, !settings.pinStateOff);
            delay(50);
            digitalWrite(settings.ledPin, settings.pinStateOff);
            Serial.println("Attack / Flash button quickly pressed... toggling deauth attack");
            attack.start(0);
          }
        }
        delay(400);
      }

    #endif
  }
}
