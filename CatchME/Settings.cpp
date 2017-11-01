#include "Settings.h"

Settings::Settings() {
  uint8_t tempMAC[6];
  defaultMacAP.set(WiFi.softAPmacAddress(tempMAC));
  if(!defaultMacAP.valid()) defaultMacAP.randomize();
}

void Settings::syncMacInterface(){
  if(debug) Serial.println("Trying to sync the MAC addr with settings");
  if(isSettingsLoaded){
    Mac macToSync;
    if(isMacAPRand){
      macToSync.randomize();
      wifi_set_macaddr(SOFTAP_IF, macToSync._get());
      if(debug) Serial.println("Synced with a random mac addr : " + macToSync.toString());
    }else if(macAP.valid()){
      macToSync = macAP;
      wifi_set_macaddr(SOFTAP_IF, macToSync._get());
      if(debug) Serial.println("Synced with saved mac addr : " + macToSync.toString());
    }else{
      if(debug) Serial.println("Could not sync because of invalid settings !");
    }
  }else{
    if(debug) Serial.println("Could not sync because settings are not loaded !");
  }
}

void Settings::setLedPin(int newLedPin) {
  prevLedPin = ledPin;
  if(newLedPin > 0 && newLedPin != prevLedPin){
    ledPin = newLedPin;
    pinMode(ledPin, OUTPUT);
    if(!prevLedPin == 0){
      digitalWrite(ledPin, digitalRead(prevLedPin));
      digitalWrite(prevLedPin, pinStateOff);
      pinMode(prevLedPin, INPUT);
    }else{
      digitalWrite(ledPin, pinStateOff);
    }
  }
}

void Settings::setAlertLedPin(int newAlertLedPin) {
  prevAlertLedPin = alertLedPin;
  if(newAlertLedPin > 0 && newAlertLedPin != prevAlertLedPin){
    alertLedPin = newAlertLedPin;
    pinMode(alertLedPin, OUTPUT);
    if(!prevAlertLedPin == 0){
      digitalWrite(alertLedPin, digitalRead(prevAlertLedPin));
      digitalWrite(prevAlertLedPin, pinStateOff);
      pinMode(prevAlertLedPin, INPUT);
    }else{
      digitalWrite(alertLedPin, pinStateOff);
    }
  }
}

void Settings::load() {
  if (EEPROM.read(checkNumAdr) != checkNum) {
    Serial.println("");
    Serial.println("Resetting settings, checknum=" + (String)checkNum + " but it should equal" + (String)EEPROM.read(checkNumAdr));
    Serial.println("");
    reset();
    return;
  }

  ssidLen = EEPROM.read(ssidLenAdr);
  ssid = "";
  for (int i = 0; i < ssidLen; i++) ssid += (char)EEPROM.read(ssidAdr + i);

  ssidHidden = (bool)EEPROM.read(ssidHiddenAdr);
  
  passwordLen = EEPROM.read(passwordLenAdr);
  password = "";
  for (int i = 0; i < passwordLen; i++) password += (char)EEPROM.read(passwordAdr + i);
  
  if (ssidLen < 1 || ssidLen > 32 || passwordLen < 8 && passwordLen != 0  || passwordLen > 32) {
    reset();
    return;
  }

  if ((int)EEPROM.read(apChannelAdr) >= 1 && (int)EEPROM.read(apChannelAdr) <= 14) {
    apChannel = (int)EEPROM.read(apChannelAdr);
  } else {
    apChannel = 1;
  }
  
  wifiClient = (bool)EEPROM.read(wifiClientAdr);
  
  ssidClientLen = EEPROM.read(ssidClientLenAdr);
  for (int i = 0; i < ssidClientLen; i++) ssidClient += (char)EEPROM.read(ssidClientAdr + i);
  
  passwordClientLen = EEPROM.read(passwordClientLenAdr);
  for (int i = 0; i < passwordClientLen; i++) passwordClient += (char)EEPROM.read(passwordClientAdr + i);
  
  hostnameLen = EEPROM.read(hostnameLenAdr);
  for (int i = 0; i < hostnameLen; i++) hostname += (char)EEPROM.read(hostnameAdr + i);
  
  
  for(int i=0; i<6; i++){
    macAP.setAt((uint8_t)EEPROM.read(macAPAdr+i),i);
  }
  
  if(!macAP.valid()) macAP.set(defaultMacAP);
  isMacAPRand = (bool)EEPROM.read(isMacAPRandAdr);

  apScanHidden = (bool)EEPROM.read(apScanHiddenAdr);
  deauthReason = EEPROM.read(deauthReasonAdr);
  attackTimeout = eepromReadInt(attackTimeoutAdr);
  attackPacketRate = EEPROM.read(attackPacketRateAdr);
  clientScanTime = EEPROM.read(clientScanTimeAdr);
  beaconInterval = (bool)EEPROM.read(beaconIntervalAdr);
  useLed = (bool)EEPROM.read(useLedAdr);
  setLedPin((int)EEPROM.read(ledPinAdr));
  useAlertLed = (bool)EEPROM.read(useAlertLedAdr);
  setAlertLedPin((int)EEPROM.read(alertLedPinAdr));
  multiAPs = (bool)EEPROM.read(multiAPsAdr);
  multiAttacks = (bool)EEPROM.read(multiAttacksAdr);
  macInterval = eepromReadInt(macIntervalAdr);
  isSettingsLoaded = 1;
  cache = (bool)EEPROM.read(cacheAdr);
  serverCache = (int)EEPROM.read(serverCacheAdr);
  detectorChannel = (int)EEPROM.read(detectorChannelAdr);
  detectorAllChannels = (bool)EEPROM.read(detectorAllChannelsAdr);
  detectorScanTime = (int)EEPROM.read(detectorScanTimeAdr);
  newUser = (bool)EEPROM.read(newUserAdr);
  modeOps = (bool)EEPROM.read(modeOpsAdr);
}

void Settings::reset() {
  Serial.println("Resetting settings! checknum=" + (String)checkNum + ", EEPROM=" + (String)EEPROM.read(checkNumAdr));
  if (debug) Serial.print("reset settings...");

  ssid = "CATCH-ME";
  ssidHidden = false;
  password = "";
  apChannel = 1;
  wifiClient = false;
  ssidClient = "";
  passwordClient = "";
  hostname = "CATCH-ME";
  macAP = defaultMacAP;
  isMacAPRand = 0;
  apScanHidden = true;
  deauthReason = 0x01;
  attackTimeout = 0;
  attackPacketRate = 10;
  clientScanTime = 15;
  beaconInterval = false;
  useLed = true;
  ledPin = 2;
  useAlertLed = true;
  alertLedPin = 16;
  multiAPs = true;
  multiAttacks = true;
  macInterval = 4;
  cache = true;
  serverCache = 194;
  detectorChannel = 1;
  detectorAllChannels = true;
  detectorScanTime = 200;
  newUser = true;
  modeOps = false;

  save();
}

void Settings::save() {
  ssidLen = ssid.length();
  EEPROM.write(ssidLenAdr, ssidLen);
  for (int i = 0; i < ssidLen; i++) EEPROM.write(ssidAdr + i, ssid[i]);
  
  EEPROM.write(ssidHiddenAdr, ssidHidden);
  
  passwordLen = password.length();
  EEPROM.write(passwordLenAdr, passwordLen);
  for (int i = 0; i < passwordLen; i++) EEPROM.write(passwordAdr + i, password[i]);

  EEPROM.write(apChannelAdr, apChannel);
  EEPROM.write(wifiClientAdr, wifiClient);
  
  ssidClientLen = ssidClient.length();
  EEPROM.write(ssidClientLenAdr, ssidClientLen);
  for (int i = 0; i < ssidClientLen; i++) EEPROM.write(ssidClientAdr + i, ssidClient[i]);

  passwordClientLen = passwordClient.length();
  EEPROM.write(passwordClientLenAdr, passwordClientLen);
  for (int i = 0; i < passwordClientLen; i++) EEPROM.write(passwordClientAdr + i, passwordClient[i]);

  hostnameLen = hostname.length();
  EEPROM.write(hostnameLenAdr, hostnameLen);
  for (int i = 0; i < hostnameLen; i++) EEPROM.write(hostnameAdr + i, hostname[i]);

  for(int i=0; i<6; i++){
    EEPROM.write(macAPAdr+i, macAP._get(i));
  }

  EEPROM.write(isMacAPRandAdr, isMacAPRand);
  EEPROM.write(apScanHiddenAdr, apScanHidden);
  EEPROM.write(deauthReasonAdr, deauthReason);
  eepromWriteInt(attackTimeoutAdr, attackTimeout);
  EEPROM.write(attackPacketRateAdr, attackPacketRate);
  EEPROM.write(clientScanTimeAdr, clientScanTime);
  EEPROM.write(beaconIntervalAdr, beaconInterval);
  EEPROM.write(useLedAdr, useLed);
  EEPROM.write(ledPinAdr, ledPin);
  EEPROM.write(useAlertLedAdr, useAlertLed);
  EEPROM.write(alertLedPinAdr, alertLedPin);
  EEPROM.write(multiAPsAdr, multiAPs);
  EEPROM.write(multiAttacksAdr, multiAttacks);
  eepromWriteInt(macIntervalAdr, macInterval);
  EEPROM.write(cacheAdr, cache);
  EEPROM.write(serverCacheAdr, serverCache);
  EEPROM.write(detectorChannelAdr, detectorChannel);
  EEPROM.write(detectorAllChannelsAdr, detectorAllChannels);
  EEPROM.write(detectorScanTimeAdr, detectorScanTime);
  EEPROM.write(modeOpsAdr, modeOps);
  EEPROM.write(newUserAdr, newUser);
  
  EEPROM.write(checkNumAdr, checkNum);
  EEPROM.commit();

  if (debug) {
    info();
    Serial.println("settings saved!");
  }
}

//belum di benerin
void Settings::info() {
  Serial.println();
  Serial.println("Firmware Information: CatchME v1.0.0-beta");
  Serial.println();
  /*
  Serial.println("CatchME - Settings:");
  Serial.println("  SSID            : '" + ssid + "'\t\t\t|  characters='" + (String)ssidLen + "'\t\t|  hidden='"+(String)ssidHidden+"'\t\t|  channel='"+(String)apChannel+"'\t\t|");
  Serial.println("  Password        : '" + password + "'\t\t|  characters='" + (String)passwordLen + "'\t\t|");
  Serial.println("  Scan            : hidden-networks='" + (String)apScanHidden + "'\t\t|  client-scan-time='" + (String)clientScanTime + "'\t|");
  Serial.println("  Attack          : timeout='" + (String)attackTimeout + "'\t\t\t|  packet_rate='" + (String)attackPacketRate + "'\t\t|  deauth_reason='" + (String)(int)deauthReason + "'");
  Serial.println("  Interface       : dark-mode='" + (String)darkMode + "'\t\t|  new-user='" + (String)newUser + "'\t\t\t|  cache='" + (String)cache + " ("+(String)serverCache+"s)'");
  Serial.println("  LED Indicator   : enable='" + (String)useLed + "'\t\t\t|  pin='" + (String)ledPin + "'\t\t\t|");
  Serial.println("  MAC AP          : default='" + defaultMacAP.toString()+"'\t|  saved='" + macAP.toString()+"'\t|  random='" + (String)isMacAPRand + "'");
  Serial.println("  Beacons         : mac-change-interval='" + (String)multiAttacks + "'\t|  " + "1s-interval='" + (String)beaconInterval + "");
  Serial.println("  Deauth Detector : all-channels='" + (String)detectorAllChannels + "'\t\t|  channel='" + (String)detectorChannel + "'\t\t\t|  alert-pin='" + (String)alertPin + "'\t|  invert-pin='" + (String)invertAlertPin + "'\t|  scan-time='" + (String)detectorScanTime + "'");
  Serial.println("  Other           : channel-hopping='" + (String)channelHop + "'\t\t|  multiple-aps='" + (String)multiAPs + "'\t\t|  multiple-attacks='" + (String)multiAttacks + "'");
  Serial.println("");
  */
}

size_t Settings::getSize() {
  String json = "{";
  size_t jsonSize = 0;

  json += "\"ssid\":\"" + ssid + "\",";
  json += "\"ssidHidden\":" + (String)ssidHidden + ",";
  json += "\"password\":\"" + password + "\",";
  json += "\"apChannel\":" + (String)apChannel + ",";
  json += "\"wifiClient\":" + (String)wifiClient + ",";
  json += "\"ssidClient\":\"" + (String)ssidClient + "\",";
  json += "\"passwordClient\":\"" + (String)passwordClient + "\",";
  json += "\"hostname\":\"" + (String)hostname + "\",";
  json += "\"macAp\":\"" + macAP.toString() + "\",";
  json += "\"randMacAp\":" + (String)isMacAPRand + ",";
  json += "\"apScanHidden\":" + (String)apScanHidden + ",";
  json += "\"deauthReason\":" + (String)(int)deauthReason + ",";
  json += "\"attackTimeout\":" + (String)attackTimeout + ",";
  json += "\"attackPacketRate\":" + (String)attackPacketRate + ",";
  json += "\"clientScanTime\":" + (String)clientScanTime + ",";
  json += "\"beaconInterval\":" + (String)beaconInterval + ",";
  json += "\"useLed\":" + (String)useLed + ",";
  json += "\"ledPin\":" + (String)ledPin + ",";
  json += "\"useAlertLed\":" + (String)useAlertLed + ",";
  json += "\"alertLedPin\":" + (String)alertLedPin + ",";
  json += "\"multiAPs\":" + (String)multiAPs + ",";
  json += "\"multiAttacks\":" + (String)multiAttacks + ",";
  json += "\"macInterval\":" + (String)macInterval + ",";
  json += "\"cache\":" + (String)cache + ",";
  json += "\"serverCache\":" + (String)serverCache + ",";
  json += "\"detectorChannel\":" + (String)detectorChannel + ",";
  json += "\"detectorAllChannels\":" + (String)detectorAllChannels + ",";
  json += "\"detectorScanTime\":" + (String)detectorScanTime + ",";
  json += "\"newUser\":" + (String)newUser + ",";
  json += "\"mode\":" + (String)modeOps + "}";
  jsonSize += json.length();

  return jsonSize;
}

void Settings::send() {
  if (debug) Serial.println("getting settings json");
  sendHeader(200, "text/json", getSize());

  String json = "{";
  json += "\"ssid\":\"" + ssid + "\",";
  json += "\"ssidHidden\":" + (String)ssidHidden + ",";
  json += "\"password\":\"" + password + "\",";
  json += "\"apChannel\":" + (String)apChannel + ",";
  json += "\"wifiClient\":" + (String)wifiClient + ",";
  json += "\"ssidClient\":\"" + (String)ssidClient + "\",";
  json += "\"passwordClient\":\"" + (String)passwordClient + "\",";
  json += "\"hostname\":\"" + (String)hostname + "\",";
  json += "\"macAp\":\"" + macAP.toString() + "\",";
  json += "\"randMacAp\":" + (String)isMacAPRand + ",";
  json += "\"apScanHidden\":" + (String)apScanHidden + ",";
  json += "\"deauthReason\":" + (String)(int)deauthReason + ",";
  json += "\"attackTimeout\":" + (String)attackTimeout + ",";
  json += "\"attackPacketRate\":" + (String)attackPacketRate + ",";
  json += "\"clientScanTime\":" + (String)clientScanTime + ",";
  json += "\"beaconInterval\":" + (String)beaconInterval + ",";
  json += "\"useLed\":" + (String)useLed + ",";
  json += "\"ledPin\":" + (String)ledPin + ",";
  json += "\"useAlertLed\":" + (String)useAlertLed + ",";
  json += "\"alertLedPin\":" + (String)alertLedPin + ",";
  json += "\"multiAPs\":" + (String)multiAPs + ",";
  json += "\"multiAttacks\":" + (String)multiAttacks + ",";
  json += "\"macInterval\":" + (String)macInterval + ",";
  json += "\"cache\":" + (String)cache + ",";
  json += "\"serverCache\":" + (String)serverCache + ",";
  json += "\"detectorChannel\":" + (String)detectorChannel + ",";
  json += "\"detectorAllChannels\":" + (String)detectorAllChannels + ",";
  json += "\"detectorScanTime\":" + (String)detectorScanTime + ",";
  json += "\"newUser\":" + (String)newUser + ",";
  json += "\"mode\":" + (String)modeOps + "}";
  sendToBuffer(json);
  sendBuffer();

  if (debug) Serial.println("\ndone");

}

size_t Settings::getSysInfoSize() {
  String json = "{";
  size_t jsonSize = 0;

  json += "\"freememory\":\"" + (String)ESP.getFreeHeap() + "\",";
  json += "\"deauthpackets\":\"" + (String)deauthpackets + "\",";
  json += "\"beaconpackets\":\"" + (String)beaconpackets + "\",";
  json += "\"uptime\":\"" + (String)millis() + "\",";
  json += "\"ipaddress\":\"" + (String)WiFi.localIP().toString() + "\",";
  json += "\"gateway\":\"" + (String)WiFi.gatewayIP().toString() + "\",";
  json += "\"bootmode\":\"" + (String)ESP.getBootMode() + "\",";
  json += "\"bootversion\":\"" + (String)ESP.getBootVersion() + "\",";
  json += "\"sdkversion\":\"" + (String)ESP.getSdkVersion() + "\",";
  json += "\"chipid\":\"" + (String)ESP.getChipId() + "\",";
  json += "\"flashchipid\":\"" + (String)ESP.getFlashChipId() + "\",";
  json += "\"flashchipsize\":\"" + (String)ESP.getFlashChipSize() + "\",";
  json += "\"flashchiprealsize\":\"" + (String)ESP.getFlashChipRealSize() + "\"}";
  jsonSize += json.length();

  return jsonSize;
}

void Settings::sendSysInfo() {
  if (debug) Serial.println("getting sysinfo json");
  sendHeader(200, "text/json", getSysInfoSize());
  
  String json = "{";
  json += "\"freememory\":\"" + (String)ESP.getFreeHeap() + "\",";
  json += "\"deauthpackets\":\"" + (String)deauthpackets + "\",";
  json += "\"beaconpackets\":\"" + (String)beaconpackets + "\",";
  json += "\"uptime\":\"" + (String)millis() + "\",";
  json += "\"ipaddress\":\"" + (String)WiFi.localIP().toString() + "\",";
  json += "\"gateway\":\"" + (String)WiFi.gatewayIP().toString() + "\",";
  json += "\"bootmode\":\"" + (String)ESP.getBootMode() + "\",";
  json += "\"bootversion\":\"" + (String)ESP.getBootVersion() + "\",";
  json += "\"sdkversion\":\"" + (String)ESP.getSdkVersion() + "\",";
  json += "\"chipid\":\"" + (String)ESP.getChipId() + "\",";
  json += "\"flashchipid\":\"" + (String)ESP.getFlashChipId() + "\",";
  json += "\"flashchipsize\":\"" + (String)ESP.getFlashChipSize() + "\",";
  json += "\"flashchiprealsize\":\"" + (String)ESP.getFlashChipRealSize() + "\"}";
  sendToBuffer(json);
  sendBuffer();

  if (debug) Serial.println("\ndone");

}
