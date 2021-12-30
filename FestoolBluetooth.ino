/*
   Festool Bluetooth Remote Emulator
*/

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>
#include <BLEAddress.h>
#include <BLEUUID.h>
#include <EEPROM.h>
#include <M5Atom.h>
#include <Preferences.h>
#include <nvs_flash.h>


#define MAX_REMOTES 5
String FESTOOL_UUID = "0000fe39-0000-1000-8000-00805f9b34fb";

//globals
int scanTime = 5; //In seconds
BLEScan* pBLEScan;
Preferences prefs;
long timeAtStart = 0; //for measuring button presses
long lastTime = 0; //for LED control
long factoryResetTime = 0; //for factory reset

bool LED_STATE = false;
bool RELAY_STATE = false;
String remote_addresses[MAX_REMOTES];
int numOfRemotes = 0;
int MODE = 0; // 0 = init, 1=pairing, 2=factory reset, 3=normal



bool isRemoteAddressStored(String addr) {
  for (int i = 0; i < numOfRemotes; i++) {
    if (addr == remote_addresses[i]) {
      return true;
    }
  }
  return false;
}

void addRemoteAddress(String addr) {
  loadRemoteAddresses();
  if (!isRemoteAddressStored(addr)) {
    numOfRemotes = prefs.getInt("numOfRemotes", 0);
    int currRemoteIndex = prefs.getInt("currRemoteIndex", 0);;

    prefs.remove("Remote_Address_" + currRemoteIndex);
    prefs.putString("Remote_Address_" + currRemoteIndex, addr);

    if (currRemoteIndex < MAX_REMOTES) {
      currRemoteIndex++;
      if (numOfRemotes < MAX_REMOTES) {
        numOfRemotes = currRemoteIndex;
        prefs.remove("numOfRemotes");
        prefs.putInt("numOfRemotes", numOfRemotes);
      }

      prefs.remove("currRemoteIndex");
      prefs.putInt("currRemoteIndex", currRemoteIndex);
    } else {
      currRemoteIndex = 0;
      prefs.remove("currRemoteIndex");
      prefs.putInt("currRemoteIndex", currRemoteIndex);
    }
  }

}

void printRemoteAddresses() {
  loadRemoteAddresses();
  for (int i = 0; i < numOfRemotes; i++) {
    Serial.println(remote_addresses[i]);
  }
}

void loadRemoteAddresses() {
  numOfRemotes = prefs.getInt("numOfRemotes", 0);

  if (numOfRemotes > 0) {
    for (int i = 0; i < numOfRemotes; i++) {
      remote_addresses[i] = prefs.getString("Remote_Address_" + i);
    }
  }
}



class PairingDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      BLEUUID uuid  = BLEUUID("0000fe39-0000-1000-8000-00805f9b34fb");
      if (advertisedDevice.getServiceUUID().equals(uuid)) {
        //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
        Serial.println(advertisedDevice.getAddress().toString().c_str());
        pBLEScan->stop();
        addRemoteAddress(advertisedDevice.getAddress().toString().c_str()); //add address;
        //MODE = 0; //reset device
        ESP.restart();
      }
    }
};

void toggleVacuum() {
  pBLEScan->stop();
  if (RELAY_STATE) {
    digitalWrite(23, LOW);
    M5.dis.drawpix(0, 0x000000);
    RELAY_STATE = false;
  } else {
    digitalWrite(23, HIGH);
    M5.dis.drawpix(0, CRGB::Red);
    RELAY_STATE = true;
  }

}

class FestoolDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      BLEAddress addr  = BLEAddress("ea:65:1a:a2:36:fa");
      //Serial.printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
      if (isRemoteAddressStored(advertisedDevice.getAddress().toString().c_str())){
     // if (advertisedDevice.getAddress().equals(addr)) {
        
        Serial.printf("Received Button click from:  %s \n", advertisedDevice.getAddress().toString().c_str());
        pBLEScan->stop();
        toggleVacuum();

      }
    }
};


bool pair() {
  int currTime = millis();
  int endTime = currTime + 30000;
  bool done = false;

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new PairingDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value
  BLEScanResults foundDevices = pBLEScan->start(1, false);

}

bool listen() {

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new FestoolDeviceCallbacks());
  pBLEScan->setActiveScan(false); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  
  pBLEScan->setWindow(99);  // less or equal setInterval value
  BLEScanResults foundDevices = pBLEScan->start(0, false);

}

void setup() {
  Serial.begin(115200);
  Serial.println("Scanning...");
  M5.begin(true, false, true);
  prefs.begin("Festool", false);


  pinMode(23, OUTPUT);
  pinMode(39, INPUT);


  Serial.println("Printing stored remote addresses:");
  printRemoteAddresses();

  
  timeAtStart = millis();
}


void checkForButtonPress() {
 // Serial.println(MODE);

  int buttonstate = digitalRead(39);

  if (buttonstate == 0) {
    long currTime = millis();
    if (MODE == 1 && (currTime - 6000) >= timeAtStart) {
      MODE = 2; //set to factory reset
      factoryResetTime = millis();
    } else if (MODE == 0 && (currTime - 3000) >= timeAtStart) {
      MODE = 1; //set mode to pairing
    }
  } else {
    if (MODE == 0) {
      MODE = 3; //if we are still in init, set mode to Normal when button is released;
    }
  }
}

void handleLED() {
  //lastTime
  if (MODE == 3) { //we are in normal mode--LED should be off.

  } else {
    int currTime = millis();
    if (currTime - 500 >= lastTime && LED_STATE == true) {
      M5.dis.drawpix(0, CRGB::Black);
      lastTime = currTime;
      LED_STATE = false;
    } else if (LED_STATE == false && currTime - 500 >= lastTime && MODE == 1) {
      M5.dis.drawpix(0, CRGB::Blue);
      lastTime = currTime;
      LED_STATE = true;
    } else if (LED_STATE == false && currTime - 500 >= lastTime && MODE == 2) {
      M5.dis.drawpix(0, CRGB::White);
      lastTime = currTime;
      LED_STATE = true;
    }
  }
}

void factoryReset() {
  long currTime = millis();
  if (currTime - 5000 >= factoryResetTime) { //flash the white light for 5 seconds, reset the Flash, and restart
    nvs_flash_erase(); // erase the NVS partition and...
    nvs_flash_init();
    MODE = 0;
    ESP.restart();
  }
}

void loop() {

  checkForButtonPress();
  handleLED();
  
  
  if(MODE == 3){
    listen();
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
    delay(1000);
  }if(MODE == 1){
    pair();
    pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  }else if(MODE == 2){
    factoryReset();
  }
  // listen();

  //  delay(2000);
}
