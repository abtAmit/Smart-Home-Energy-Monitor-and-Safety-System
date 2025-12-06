#include <Arduino.h>
#include <SHA256.h>
#include <espnow.h>
#include <ESP8266WiFi.h>
//#include <esp_wifi.h>
#include <EEPROM.h>
const uint8_t channel = 0;
//---------------------------------User Configuration Start-----------------------------------------------
#define numberOfSwitch 5                               // number of switches you want to use
const uint8_t deviceId = 101;                          // must be different for every esp8266 (must be like 101,102.......106)
char hmac_key[] = "HashkeyShouldBeExcatly32letter12";  // must change the key for better security (Should must Be Excatly 32 letter including spaces)
//  hmac_key[] should be same on all devices


#define off HIGH  // for relay using active low
#define on LOW

#define CHANNEL_COUNT 1

// -----------------------------------------------User Configuration END------------------------------------
//----------------------------------------------Advanced Configuration----------------------------------------------

// --- triac Config ---
#define TRIAC_PIN D8    
#define ZCD_PIN   D3    

#define AC_FREQUENCY 50  // 50Hz
// Shift Register
#define latchPin 5  // D1
#define clockPin 4  // D2
#define dataPin 16  // D0
//----------------------------------------------Advanced Configuration END----------------------------------------------

// time
unsigned long currentMillis = 0;
unsigned long previousMillis = 0;
unsigned long ct = 0;
unsigned long pt = 0;
unsigned long st = 0;
bool espSwitchflag = 0;
volatile bool packetReceivedFlag = false; // Flag to tell loop we have data

const unsigned int interval = 60000;  // 10seconds
uint16_t gtime = 0;
uint8_t lastonpin;  // for current cutoff
volatile bool emergencyStop = false;
// espnow

uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };
uint8_t calculated_hmac[32];
// espnow bridge
// uint8_t* espdata;
uint8_t lengt = 0;
// bool flow;
#define noOfSlave 3
uint8_t noofslavebridged = 0;
uint8_t masterNode[6];
uint8_t masterAddress[6] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };  // { 0x10, 0x51, 0xDB, 0x01, 0xED, 0x };
// uint8_t slaveAddress[noOfSlave][6];
uint8_t slaveId[noOfSlave];
// bool isBridged = false;
bool bridgeRequestFlag = false;
bool ackreceivedFlag = 0;
// uint8_t forwardMessage = 222;
// ---------------------------------------
// type 2,3,4,5,6,7,8,9 pinUpdate, scheduleUpdate, error,  pair, timming,temp,bridgerequest,bridgeack;
typedef struct channelSet {
  uint8_t type = 11;
  uint8_t channel;
  // uint8_t errcode;
  uint8_t hash[32];

} channelSet;
typedef struct pinUpdate {
  uint8_t type = 2;
  uint8_t id;
  uint8_t pin;
  uint8_t value;
  uint8_t slider;
  uint8_t state;
  uint8_t numberofswitch;
  uint8_t hash[32];
} pinUpdate;
pinUpdate pinupd;

typedef struct scheduleUpdate {
  uint8_t type = 3;
  uint8_t id;
  uint8_t pin;
  uint16_t onTime;
  uint16_t offTime;
  uint8_t hash[32];
} scheduleUpdate;
scheduleUpdate schupd;

typedef struct error {  //31 current //32 voltage
  uint8_t type = 4;
  uint8_t id;
  uint8_t err;
  uint8_t hash[32];

} error;
error err;

typedef struct heartBeat {  // r
  uint8_t type = 5;
  uint8_t id;
  uint8_t pol = 0;
  // uint8_t pmac[6];
  uint8_t hash[32];

} heartBeat;
heartBeat beat;
uint8_t pol = 0;
bool sendBeatFlag;

typedef struct __attribute__((packed)) timming {
  uint8_t type = 6;
  uint8_t id;
  uint16_t ctime;
  uint8_t hash[32];

} timming;
timming espTime;

typedef struct temp {
  uint8_t type = 7;
  uint8_t id;
  uint8_t temp;
  uint8_t humidity;
  uint8_t hash[32];

} temp;
temp temp1;
typedef struct bridge {  // bridge request  //type 8
  uint8_t type = 8;
  uint8_t id;
  uint8_t nodeId;
  //  uint8_t nodemac[6];
  //  uint8_t sendermac[6];
  uint8_t hash[32];
} bridge;
bridge bridreq;
typedef struct bridgeack {  // bridge request  //type 9
  uint8_t type = 9;
  uint8_t id;
  uint8_t nodeid;
  uint8_t hash[32];
} bridgeack;
bridgeack receiveack;
bridgeack ack;

typedef struct sendallstate {
  uint8_t type = 10;
  uint8_t id;
  uint8_t hash[32];

} sendallstate;
sendallstate sendall;
bool sendAllFlag = 0;

uint8_t dpin[] = {
  // pin map
  /*  16,  // D0
  5,   // D1
  4,   // D2
  0,   // D3*/
  2,   // D4
  14,  // D5
  12,  // D6
  13,  // D7
       // 15, // D8  ---- Boot fails if pulled HIGH
  3,   //rx
  1    //tx
};
// switch control
uint8_t pinSlider[numberOfSwitch];
uint8_t pinState[numberOfSwitch];
uint8_t pinValue[numberOfSwitch];
uint8_t switchArray[8];
uint16_t onTime[numberOfSwitch];
uint16_t offTime[numberOfSwitch];
// uint8_t shIndex[numberOfSwitch];
void hashData(void* structaddress, uint8_t length, uint8_t* hash) {  // hash data for integrety
                                                                     // Serial.println("---------------------");
  Serial.println("Hashing...........");

  // 2. Generate the HMAC signature
  SHA256 sha256;
  sha256.resetHMAC(hmac_key, 33);
  // sha256.update(address, offsetof(struct_message, hmac));
  sha256.update(structaddress, length);
  sha256.finalizeHMAC(hmac_key, 33, hash, 32);
  Serial.println("Data signature calculated.");
  //  return &hash[0];
}

//---------------------triac controll code start------------------------
// --- Debug Counters ---
volatile unsigned long debug_zcd_counter = 0;
volatile unsigned long debug_timer_counter = 0;
unsigned long last_debug_print = 0;

// --- GAMMA CORRECTION TABLE ---
// This maps 0-100% perceived brightness to 0-100% timer values.
// It creates a curve: slow changes at the bottom, fast at the top.
const uint8_t gamma_table[] = {
  0, 1, 1, 1, 1, 1, 1, 1, 1, 1,
  2, 2, 2, 2, 2, 2, 3, 3, 3, 3,
  4, 4, 4, 5, 5, 5, 6, 6, 6, 7,
  7, 8, 8, 9, 9, 10, 10, 11, 12, 12,
  13, 14, 15, 15, 16, 17, 18, 19, 20, 21,
  22, 23, 24, 25, 26, 27, 28, 29, 31, 32,
  33, 35, 36, 38, 39, 41, 42, 44, 46, 48,
  50, 52, 54, 56, 58, 60, 62, 65, 67, 70,
  72, 75, 78, 81, 84, 87, 90, 93, 96, 100,
  104, 108, 112, 116, 120, 125, 130, 135, 140, 145, 150  // Extended slightly
};

// --- Channel Structs ---
struct SmartChannel {
  uint8_t triacPin;
  bool isDimmer;
  bool state;
  uint8_t brightness;  // 0-100
};

SmartChannel g_channels[CHANNEL_COUNT];

// --- Firing Queue ---
struct FiringEvent {
  uint8_t pin;
  uint16_t delayMicros;
};

volatile FiringEvent g_fireQueue[CHANNEL_COUNT];
volatile int g_fireQueueCount = 0;
volatile int g_fireQueueIndex = 0;

// --- 1. The Executor (Timer1 ISR) ---
void ICACHE_RAM_ATTR onTimer1_ISR() {
  debug_timer_counter++;

  // 1. Fire the TRIAC
  int currentPin = g_fireQueue[g_fireQueueIndex].pin;
  digitalWrite(currentPin, HIGH);
  delayMicroseconds(100);
  digitalWrite(currentPin, LOW);

  // 2. Move to next
  g_fireQueueIndex++;

  // 3. Queue finished?
  if (g_fireQueueIndex >= g_fireQueueCount) {
    timer1_disable();
  } else {
    // 4. Re-arm timer
    uint16_t nextDelay = g_fireQueue[g_fireQueueIndex].delayMicros;
    uint16_t prevDelay = g_fireQueue[g_fireQueueIndex - 1].delayMicros;
    uint16_t delayDiff = nextDelay - prevDelay;

    timer1_write(delayDiff * 5);
  }
}

// --- 2. The Scheduler (ZCD ISR) ---
void ICACHE_RAM_ATTR onZCD_ISR() {
  debug_zcd_counter++;

  g_fireQueueCount = 0;
  g_fireQueueIndex = 0;

  for (int i = 0; i < CHANNEL_COUNT; i++) {
    // Skip if OFF or 0%
    if (g_channels[i].state == false || g_channels[i].isDimmer == false || g_channels[i].brightness == 0) {
      continue;
    }

    // --- NEW LOGIC: GAMMA CORRECTION ---
    // 1. Get gamma-corrected value (0-150 range from table)
    // We map 0-100 input to index 0-100 of the table
    int index = g_channels[i].brightness;
    if (index > 100) index = 100;

    uint8_t corrected_val = pgm_read_byte(&gamma_table[index]);

    uint16_t delay = map(g_channels[i].brightness, 1, 100, 9600, 200);

    g_fireQueue[g_fireQueueCount].pin = g_channels[i].triacPin;
    g_fireQueue[g_fireQueueCount].delayMicros = delay;
    g_fireQueueCount++;
  }

  if (g_fireQueueCount == 0) {
    timer1_disable();
    return;
  }

  // Bubble Sort
  for (int i = 0; i < g_fireQueueCount - 1; i++) {
    for (int j = 0; j < g_fireQueueCount - i - 1; j++) {
      if (g_fireQueue[j].delayMicros > g_fireQueue[j + 1].delayMicros) {
        // Manual Swap
        uint8_t tempPin = g_fireQueue[j].pin;
        uint16_t tempDelay = g_fireQueue[j].delayMicros;
        g_fireQueue[j].pin = g_fireQueue[j + 1].pin;
        g_fireQueue[j].delayMicros = g_fireQueue[j + 1].delayMicros;
        g_fireQueue[j + 1].pin = tempPin;
        g_fireQueue[j + 1].delayMicros = tempDelay;
      }
    }
  }

  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(g_fireQueue[0].delayMicros * 5);
}

//-------------------------------triac control code ends-----------------------------------
/*
//---------------------triac controll code start------------------------
// --- State Variables ---
volatile int g_brightness = 0; 
volatile bool g_state = true;   

// --- DEBUG COUNTERS ---
// We use these to see if the interrupts are actually running
volatile unsigned long debug_zcd_counter = 0;
volatile unsigned long debug_timer_counter = 0;

// --- 1. The Executor (Timer1 ISR) ---
void ICACHE_RAM_ATTR onTimer1_ISR() {
  // Fire the pulse
  digitalWrite(TRIAC_PIN, HIGH);
  delayMicroseconds(100); 
  digitalWrite(TRIAC_PIN, LOW);
  
  // Increment debug counter
  debug_timer_counter++;
}

// --- 2. The Scheduler (ZCD ISR) ---
void ICACHE_RAM_ATTR onZCD_ISR() {
  // Increment debug counter
  debug_zcd_counter++;

  // 1. Check if we should fire at all
  if (g_state == false || g_brightness == 0) {
    timer1_disable(); // Turn it off if we don't need it
    return;
  }

  // 2. Calculate delay (Map 1-100% to 8000-100us)
  uint16_t delayMicros = map(g_brightness, 1, 100, 8000, 200);

  // 3. FORCE ENABLE and Arm the timer
  // We must ensure the timer is enabled before/when writing
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE); 
  timer1_write(delayMicros * 5); 
}
//-------------------------------triac control code ends-----------------------------------
*/
void sendBeat() {
  // pol++;
  beat.id = deviceId;
  beat.pol = 1;

  hashData(&beat, offsetof(heartBeat, hash), calculated_hmac);
  memcpy(beat.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t*)&beat, sizeof(heartBeat));
  Serial.print("sending heartbeat  pol");
  Serial.println(beat.pol);
}
void turnoff(uint8_t pin, uint8_t err = 0) {  // err off due to error
  if (pinSlider[pin] == 0) {
    digitalWrite(dpin[pin], off);
    Serial.print("Gpio pin ");
    Serial.print(dpin[pin]);
    Serial.print(" ");
    Serial.print("OFF pin D");
    Serial.println(pinupd.pin);

  } else if (pinSlider[pin] == 1) {
    Serial.print("it is a slider pin value:");
    Serial.println(pinValue[pin]);
    uint8_t brightness = map(pinValue[pin], 0, 255, 0, 100);
    noInterrupts();
    //g_state = false;
    g_channels[0].state = false;
    // g_channels[0].brightness = 0;  // turn off
    interrupts();

    // need to complete // how will you turn dimmer off // saving pin state will change the dimmer
  }

  if (err == 0) {
    pinState[pin] = 0;
    EEPROM.write(pin + 16, 0);  // save 0 for pinState[pin]
    EEPROM.commit();
  }
}

void turnon(uint8_t pin) {
  if (emergencyStop) { return; }
  // digitalWrite(dpin[pin], off);
  Serial.print("Gpio pin ");
  Serial.print(dpin[pin]);
  Serial.print(" ");
  Serial.print("ON pin D");
  Serial.println(pinupd.pin);
  pinState[pin] = 1;

  if (pin != 14) {
    digitalWrite(dpin[pin], on);
  } else if (pinSlider[pin] == 14) {
    Serial.print("it is a slider pin value:");
    Serial.println(pinValue[pin]);
    uint8_t brightness = map(pinValue[pin], 0, 255, 0, 100);
    noInterrupts();
    g_channels[0].state = pinState[pin];
    g_channels[0].brightness = brightness;
    interrupts();
  }
  //else if (pinSlider[pin] == 1 and pinValue[pin] > 5) {
  // manageDimmer(pin);

  lastonpin = pin;
  // save to eeprom
}
void sendPinInfo(uint8_t pin, uint8_t type = 2) {
  pinupd.type = type;
  pinupd.id = deviceId;
  pinupd.pin = pin;
  pinupd.slider = pinSlider[pin];
  pinupd.value = pinValue[pin];
  pinupd.state = pinState[pin];
  pinupd.numberofswitch = numberOfSwitch;
  Serial.print("pin STATE ");
  Serial.println(pinupd.state);
  Serial.print("pin TYPE ");
  Serial.println(pinupd.type);

  hashData(&pinupd, offsetof(pinUpdate, hash), calculated_hmac);
  memcpy(pinupd.hash, calculated_hmac, 32);
  esp_now_send(masterAddress, (uint8_t*)&pinupd, sizeof(pinUpdate));
}
void sendAllState() {
  for (uint8_t index = 0; index < numberOfSwitch; index++) {
    sendPinInfo(index, 10);
    delay(100);
  }
}
void managepin(uint8_t pin) {
  Serial.println("managepin ");
  if (pinSlider[pin] == 0) {
    if (pinState[pin] == 1) {
      turnon(pin);

    } else {
      turnoff(pin);
    }
    sendPinInfo(pin);

    if (EEPROM.read((pin + 16)) != pinState[pin]) {
      Serial.print("manage pin ");
      Serial.print(pin);
      Serial.print(" pin States ");
      Serial.println(pinState[pin]);
      EEPROM.write(pin + 16, pinState[pin]);
      EEPROM.commit();
    }
  }
  if (pinSlider[pin] == 1) {
    if (pinState[pin] == 0) {
      noInterrupts();
      g_channels[0].state = false;
      // g_channels[0].brightness = 0;  // turn off
      interrupts();
    }
    if (pinState[pin] == 1) {
      uint8_t brightness = map(pinValue[pin], 0, 255, 0, 100);
      noInterrupts();
      g_channels[0].state = pinState[pin];
      g_channels[0].brightness = brightness;
      interrupts();
    }
  }
  //EEPROM.write(pin + 16, pinState[pin]);
}
void mgsw() {
  bool s1 = !digitalRead(12);
  bool s2 = !digitalRead(13);
  bool s3 = !digitalRead(14);

  if (s1 != switchArray[0]) {
    switchArray[0] = s1;
    pinState[0] = s1;

    managepin(0);
  }
  if (s2 != switchArray[1]) {
    switchArray[1] = s2;
    pinState[1] = s2;

    managepin(1);
  }
  if (s3 != switchArray[2]) {
    switchArray[2] = s3;
    pinState[2] = s3;

    managepin(2);
  }
}
void manageSwitch() {


  /* // 1. Read the 10-bit analog value (0-1023)
  int adcValue = analogRead(A0);

  // 2. Convert it to the single 8-bit byte (0-255)
  byte currentSwitchStates = (byte)round((adcValue / 4));
  if (lastSwitchState != currentSwitchStates) {
    Serial.println("switch state changed");
    Serial.println(lastSwitchStates, BIN);
    Serial.println(currentSwitchStates, BIN);
    lastSwitchState = currentSwitchStates;
*/
  uint8_t currentSwitch[8];
  // Step 1: Sample
  digitalWrite(latchPin, LOW);
  delayMicroseconds(5);
  digitalWrite(latchPin, HIGH);
  delayMicroseconds(5);  // for stability

  // Step 2: Shift
  // Serial.print("Bits: ");
  bool commitFlag = 0;
  for (int i = 7; i >= 0; i--) {              // fetch switch state from shift Register
    currentSwitch[i] = digitalRead(dataPin);  // gpio 3
    digitalWrite(clockPin, HIGH);             // Shift out the next bit
    delayMicroseconds(5);
    digitalWrite(clockPin, LOW);
    delayMicroseconds(5);
    /* if (bit == HIGH) {
      Serial.print("1");
    } else {
      Serial.print("0");
    }*/
  }
  for (int i = 0; i < numberOfSwitch; i++) {
    if (currentSwitch[i] != switchArray[i]) {
      commitFlag = 1;
      switchArray[i] = currentSwitch[i];
      EEPROM.write(i + 32, switchArray[i]);
      // Serial.print("button : ");
      // Serial.println(i);
      if (switchArray[i] == 1) {
        pinState[i] = 1;
        managepin(i);
        // turnon(i);
      } else {
        pinState[i] = 0;
        managepin(i);
        // turnoff(i);
      }
    }
  }

  for (int i = 0; i >= 7; i--) {
    Serial.print(currentSwitch[i]);
    if (i == 7) {
      Serial.println(" ");
    }
  }



  if (commitFlag == 1) {
    commitFlag = 0;
    EEPROM.commit();
    Serial.println("Saved in rom");
  }
}
void espSwitch() {
  uint8_t pin = pinupd.pin;
  bool commitFlag = 0;
  Serial.print("ESP switch ");
  Serial.print("pin ");
  Serial.println(pin);
  pinSlider[pin] = pinupd.slider;
  pinValue[pin] = pinupd.value;
  pinState[pin] = pinupd.state;

  managepin(pin);
  //----------- eeprom part------------
  if (EEPROM.read((pin + 8)) != pinValue[pin]) {
    EEPROM.write(pin + 8, pinValue[pin]);
    commitFlag = 1;
  }

  if (EEPROM.read((pin)) != pinSlider[pin]) {
    EEPROM.write(pin, pinSlider[pin]);
    commitFlag = 1;
  }
  if (commitFlag == 1) {
    commitFlag = 0;
    EEPROM.commit();
  }
  //-----------------------------
}
void manageDimmer(uint8_t pin) {  // not implemented yet
  Serial.print("dimmer not implemented yet ");
}
void handelerr() {
  if (err.err == 32 or err.err == 33) {
    emergencyStop = 1;
    for (uint8_t i = 0; i < numberOfSwitch; i++) {  // turnoff all devices
      if (pinSlider[i] == 0)
        turnoff(i, 1);
    }

  }

  else if (err.err == 31) {
    turnoff(lastonpin);  // turnoff
    sendPinInfo(lastonpin);
  }

  else if (err.err == 35) {
    emergencyStop = 0;
    for (uint8_t i = 0; i < numberOfSwitch; i++) {
      if (pinSlider[i] == 0 and pinState[i] == 1) {
        turnon(i);
        sendPinInfo(i);
      }
    }
  }
}
void updateSchedule() {
  // EEPROM.write((schupd.pin * 2) + 24, schupd.onTime);  // 2 byte for each variable
  //  EEPROM.write((schupd.pin * 2) + 40, schupd.offTime);
  onTime[schupd.pin] = schupd.onTime;
  offTime[schupd.pin] = schupd.offTime;
  uint16_t eepromOnTime;
  uint16_t eepromOffTime;
  EEPROM.get(((schupd.pin * 2) + 40), eepromOnTime);
  EEPROM.get(((schupd.pin * 2) + 40), eepromOffTime);
  // store value if it is different from previously stored value
  if (eepromOnTime != onTime[schupd.pin]) {
    EEPROM.put(((schupd.pin * 2) + 40), onTime[schupd.pin]);  // address of on start from 40
    EEPROM.commit();
  }
  if (eepromOffTime != offTime[schupd.pin]) {
    EEPROM.put(((schupd.pin * 2) + 56), offTime[schupd.pin]);  // address of off start from 56
    EEPROM.commit();
  }
  Serial.printf("Pin: %d onTime:%d offTime:%d", schupd.pin, onTime[schupd.pin], offTime[schupd.pin]);
}
void manageSchedule() {
  for (uint8_t u = 0; u < numberOfSwitch; u++) {
    if (onTime[u] < 1442 and onTime[u] == (gtime + 1)) {  // 9999 or value more than 1441 could never excute

      Serial.println("scheudle triggered");
      pinState[u] = 1;
      managepin(u);
      Serial.print("on time");
      Serial.println(u);
    }
    if (offTime[u] < 1442 and offTime[u] == (gtime + 1)) {
      pinState[u] = 0;
      managepin(u);
    }
  }
}
uint16_t fetchTime() {  //fetch time from espnow //incomplete
  Serial.println(" fetching time....");
  espTime.id = deviceId;
  espTime.type = 6;
  espTime.ctime = gtime;  // need to fetch
  hashData(&espTime, offsetof(timming, hash), calculated_hmac);
  memcpy(espTime.hash, calculated_hmac, 32);
  esp_now_send(masterAddress, (uint8_t*)&espTime, sizeof(timming));
  delay(200);
  return gtime;
}
void manageTime() {  // keep the time
  currentMillis = millis();
  // Serial.println(" time loop ");
  if (currentMillis - previousMillis > interval) {  // update time ever 1 minute
    previousMillis = currentMillis;
    gtime++;

    Serial.printf("TIME : %d \n", gtime);
    manageSchedule();  // call  manageSchedule() every 1 minute

    if (gtime > 1439) {
      gtime = 0;
    }

    if (gtime % 60 == 0) {  //fetch every hour or if gtime==0;
      gtime = fetchTime();
    }
  }
}
/*
bool peerAdd(uint8_t* peer) {

  // Add peer with encryption enabled via 16-byte key
  Serial.printf("paring %02X:%02X:%02X:%02X:%02X:%02X   \n",
                peer[0], peer[1], peer[2], peer[3], peer[4], peer[5]);
  if (esp_now_add_peer(peer, ESP_NOW_ROLE_COMBO, 1, LMK, 16) != 0) {
    Serial.println("Failed to add peer!");
    return 0;
  } else {
    Serial.println("Peer added with encryption.");
    return 1;
  }
  // flag = 1;
}
*/
/*
bool unencpeer(uint8_t* peer) {
  if (esp_now_add_peer(peer, ESP_NOW_ROLE_COMBO, 1, NULL, 0) != 0) {
    Serial.println("Failed to add broadcast peer!");
    return 0;
  } else {
    Serial.println("Broadcast peer added.");
    return 1;
  }
}*/
void handelBridgeRequest() {         // add encrypted peer and send ack set is bridge true
  bridgeRequestFlag = 0;             // turnoff flag so loop coesnt call it all time
  if (bridreq.nodeId == deviceId) {  // create ack message and send
                                     // peerAdd(bridreq.sendermac);      // add encrypted peer
    // memcpy(masterNode, bridreq.sendermac, 6);
    // isBridged = true;

    ack.id = bridreq.id;
    ack.nodeid = deviceId;
    ack.type = 9;
    hashData(&ack, offsetof(bridgeack, hash), calculated_hmac);
    memcpy(ack.hash, calculated_hmac, 32);
    esp_now_send(broadcastAddress, (uint8_t*)&ack, sizeof(bridgeack));
    //esp_now_send(masterNode, (uint8_t*)&ack, sizeof(bridgeack));
  }

  else if (bridreq.id == deviceId) {
    esp_now_send(broadcastAddress, (uint8_t*)&bridreq, sizeof(bridge));



    /*

    unsigned long currentMilli = millis();
    unsigned long previousMilli = currentMilli;
    peerAdd(bridreq.nodemac);

    while (!ackreceivedFlag and millis() - previousMilli < 2000) {
      esp_now_send(broadcastAddress, (uint8_t*)&bridreq, sizeof(bridge));
      delay(200);
      if (ackreceivedFlag) { break; }
    }
    uint8_t zeroMac[6] = { 0, 0, 0, 0, 0, 0 };
    if (ackreceivedFlag) {
      ackreceivedFlag = 0;                                                    // turnoff flag
      esp_now_send(masterAddress, (uint8_t*)&receiveack, sizeof(bridgeack));  // send to main master esp32
      for (uint8_t i = 0; i < noOfSlave; i++) {
        if (memcmp(slaveAddress[i], zeroMac, 6) == 0) {
          memcpy(slaveAddress[i], bridreq.nodemac, 6);
          slaveId[i] = bridreq.nodeId;
          break;
        }
      }
    }

    else {                                // optional
      esp_now_del_peer(bridreq.nodemac);  // delet added peer if peer doesnt send ack message
    }
   }
   */
  }

  /*
 bool compareHMAC(uint8_t* hmac1, uint8_t* hmac2, size_t len) {
  for (size_t i = 0; i < len; i++) {
    if (hmac1[i] != hmac2[i]) {
      return false;
    }
  }
  return true;*/
}
/*
  void frwdMsg() {  // in development
    if (flow == 0) { esp_now_send(slaveAddress[forwardMessage], (uint8_t*)espdata, lengt); }
    if (flow == 1) { esp_now_send(masterAddress, (uint8_t*)espdata, lengt); }
    // esp_now_send(masterNode, (uint8_t*)&ack, sizeof(bridgeack));
  }*/
void onDataSent(uint8_t* mac, uint8_t status) {
  Serial.printf("Sent to %02X:%02X:%02X:%02X:%02X:%02X -> %s\n",
                mac[0], mac[1], mac[2], mac[3], mac[4], mac[5],
                status == 0 ? "OK" : "FAIL");
}
uint8_t packetid = 0;
uint8_t typ = 0;
uint8_t incomingData[250];        // Buffer to store raw bytes
uint8_t len = 0;            // Store length
//uint8_t incomingMac[6];                   // Store sender MAC

void onDataRecv(uint8_t* mac, uint8_t* incomingDataPacket, uint8_t lenth) {
  // Copy the received data into the myData structure

  memcpy(incomingData, incomingDataPacket, len);
  len = lenth;
  lengt = len;
  memcpy(&typ, incomingData, 1);
  packetid = incomingData[1];
  if (typ == 11) {
    if (len != sizeof(channelSet)) {
      Serial.println("!!! ERROR: Received packet with mismatched size for type 2");
      return;  // Stop processing this packet
    }
    channelSet channel;
    memcpy(&channel, incomingData, sizeof(channel));
    hashData(&channel, offsetof(channelSet, hash), calculated_hmac);
    if (memcmp(channel.hash, calculated_hmac, 32) == 0) {
      wifi_set_channel(incomingData[1]);
    }
  }
  else {
    packetReceivedFlag = true;
  }
}
void processPacket() {
  if (packetid != deviceId) {
    for (uint8_t i = 0; i < noOfSlave; i++) {
      /* if (memcmp(mac, masterAddress, 6) == 0) {
        flow = 0;  // flow=0; -> master to node
      } else {
        flow = 1;  // flow=1 -> node to master
      } */
      if (packetid == slaveId[i]) {
        uint8_t data_buffer[len];
        esp_now_send(broadcastAddress, (uint8_t*)&data_buffer, len);
        // espdata = (uint8_t*)calloc(len, sizeof(uint8_t));
        // forwardMessage = i;  // send the message while esp in loop function
        return;
      }
    }
    return;
  }
  if (packetid == deviceId) {
    // Print a confirmation that data was received
    /*Serial.println("\n -----------------");
    Serial.print("Packet received from: ");
    // Print the MAC address of the sender
    for (int i = 0; i < 6; i++) {
      Serial.print(mac[i], HEX);
      if (i < 5) {
        Serial.print(":");
      }
    }*/
    Serial.print("TYpe");
    Serial.println(typ);
    if (typ == 2) {
      if (len != sizeof(pinUpdate)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 2");
        return;  // Stop processing this packet
      }
      memcpy(&pinupd, incomingData, sizeof(pinupd));
      hashData(&pinupd, offsetof(pinUpdate, hash), calculated_hmac);
      if (memcmp(pinupd.hash, calculated_hmac, 32) == 0) {

        Serial.print("pin ");
        Serial.println(pinupd.pin);
        Serial.print("slider ");
        Serial.println(pinupd.slider);
        Serial.print("Slider value ");
        Serial.println(pinupd.value);
        Serial.print("State ");
        Serial.println(pinupd.state);
        for (uint8_t p = 0; p < numberOfSwitch; p++) {
          if (dpin[p] == pinupd.pin) {
            pinupd.pin = p;
            break;
          }
        }
        Serial.print("pin converted to  ");
        Serial.println(pinupd.pin);

        espSwitchflag = 1;
      } else {
        Serial.println("---");
        Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
      }
    }

    else if (typ == 3) {
      if (len != sizeof(scheduleUpdate)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 3");
        return;
      }
      memcpy(&schupd, incomingData, sizeof(schupd));
      hashData(&schupd, offsetof(scheduleUpdate, hash), calculated_hmac);
      if (memcmp(schupd.hash, calculated_hmac, 32) == 0) {
        for (uint8_t p = 0; p < numberOfSwitch; p++) {
          if (schupd.pin == dpin[p]) {
            schupd.pin = p;
            break;
          }
        }
        Serial.printf(" id %d", schupd.id);
        Serial.printf(" pin %d", schupd.pin);
        Serial.printf(" onTIME %d", schupd.onTime);
        Serial.printf(" offTime %d \n", schupd.offTime);
        Serial.printf("TIME %d \n", gtime);

        updateSchedule();
      } else {
        Serial.println("---");
        Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
      }
    }

    else if (typ == 4) {
      if (len != sizeof(err)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 4");
        return;
      }
      memcpy(&err, incomingData, sizeof(err));
      hashData(&err, offsetof(error, hash), calculated_hmac);
      if (memcmp(err.hash, calculated_hmac, 32) == 0) {
        handelerr();

      } else {
        Serial.println("---");
        Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
      }
    }

    else if (typ == 5) {

      if (len != sizeof(beat)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 5");
        return;
      }
      memcpy(&beat, incomingData, sizeof(heartBeat));
      hashData(&beat, offsetof(heartBeat, hash), calculated_hmac);
      if (memcmp(beat.hash, calculated_hmac, 32) == 0) {

        sendBeat();


      } else {
        Serial.println("---");
        Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
      }
    }

    else if (typ == 6) {
      if (len != sizeof(espTime)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 6");
        return;
      }
      memcpy(&espTime, incomingData, sizeof(espTime));
      hashData(&espTime, offsetof(timming, hash), calculated_hmac);
      if (memcmp(espTime.hash, calculated_hmac, 32) == 0) {
        gtime = espTime.ctime;
        Serial.print("Time Received ");
        Serial.println(gtime);

      } else {
        Serial.println("---");
        Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
      }
    }

    else if (typ == 8) {
      if (len != sizeof(bridge)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 8");
        return;
      }
      memcpy(&bridreq, incomingData, sizeof(bridge));
      hashData(&bridreq, offsetof(bridge, hash), calculated_hmac);
      if (memcmp(bridreq.hash, calculated_hmac, 32) == 0) {
        if (bridreq.nodeId == deviceId or bridreq.id == deviceId) {
          bridgeRequestFlag = 1;
        }
      }
    }

    else if (typ == 9) {
      if (len != sizeof(bridgeack)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 9");
        return;
      }
      memcpy(&receiveack, incomingData, sizeof(bridgeack));
      hashData(&receiveack, offsetof(bridgeack, hash), calculated_hmac);
      if (memcmp(receiveack.hash, calculated_hmac, 32) == 0) {
        if (receiveack.id == deviceId and receiveack.nodeid == bridreq.id) {
          ackreceivedFlag = 1;
        }
      }
    }

    else if (typ == 10) {
      if (len != sizeof(sendallstate)) {
        Serial.println("!!! ERROR: Received packet with mismatched size for type 10");
        return;
      }
      memcpy(&sendall, incomingData, sizeof(sendallstate));
      hashData(&sendall, offsetof(sendallstate, hash), calculated_hmac);
      if (memcmp(sendall.hash, calculated_hmac, 32) == 0) {

        sendAllFlag = 1;
      }

      Serial.println(" -----------------");
    }
  }
}

void setup() {
  Serial.begin(115200);
  delay(2000);
  Serial.println("\n Hi from esp8266");
  Serial.println("pin output ");
  for (uint i = 0; i < numberOfSwitch; i++) {
    // triac pin also need to be decleared as output pin
    pinMode(dpin[i], OUTPUT);
    Serial.print(dpin[i]);
    Serial.print(", ");
    digitalWrite(dpin[i], HIGH);
  }
  Serial.println(" ");

  for (uint i = 0; i < numberOfSwitch; i++) {
    digitalWrite(dpin[i], LOW);
  }
  delay(1000);
  for (uint i = 0; i < numberOfSwitch; i++) {
    if (dpin[i] == dpin[0])
      continue;
    digitalWrite(dpin[i], HIGH);
  }


  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, INPUT);
  EEPROM.begin(128);  // Allocate 56 bytes of flash for EEPROM

  // delete after test
  /* pinMode(12, INPUT_PULLUP);

  pinMode(13, INPUT_PULLUP);

  pinMode(14, INPUT_PULLUP);
 */
  //---------------------------------------------------------
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();

  Serial.print("This device MAC: ");
  Serial.println(WiFi.macAddress());
  wifi_set_channel(channel);


  if (esp_now_init() != 0) {
    Serial.println("ESP-NOW init failed!");
    return;
  }

  esp_now_set_self_role(ESP_NOW_ROLE_COMBO);  // can send + receive
  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);

  // 🔹 Add peer (encrypted)
  /*
  if (esp_now_add_peer(masterAddress, ESP_NOW_ROLE_COMBO, 0, NULL, 16) == 0) {
    Serial.println("Peer added with encryption");
  } else {
    Serial.println("Failed to add peer");
  }*/


  fetchTime();
  //-------------------------------------------------- triac code

  pinMode(TRIAC_PIN, OUTPUT);
  digitalWrite(TRIAC_PIN, LOW);
  pinMode(ZCD_PIN, INPUT_PULLUP);

  // --- Timer Setup ---
  timer1_attachInterrupt(onTimer1_ISR);
  timer1_enable(TIM_DIV16, TIM_EDGE, TIM_SINGLE);
  timer1_write(600000); 
  timer1_disable();     

  // --- ZCD Interrupt Setup ---
  attachInterrupt(digitalPinToInterrupt(ZCD_PIN), onZCD_ISR, FALLING);
  //-----------------------------------------

  // --- Load saved switch states ---
  for (uint8_t i = 0; i < numberOfSwitch; i++) {
  EEPROM.write(i,0);} 
  uint8_t addr = 0;
  for (uint8_t i = 0; i < numberOfSwitch; i++) {  // possibly break if any array have index more than 8
    pinSlider[i] = EEPROM.read(i);
    pinValue[i] = EEPROM.read(i + 8);
    pinState[i] = EEPROM.read(i + 16);
    switchArray[i] = EEPROM.read(i + 32);
    EEPROM.get(((i * 2) + 40), onTime[i]);  //  onTime[i] = EEPROM.read(i * 2 + 24);
    EEPROM.get(((i * 2) + 56), offTime[i]);
    /*
    // for test only
    for (uint8_t i = 0; i < numberOfSwitch; i++) {
      pinSlider[i] = 0; // clear slider flag
    }
*/
    //  offTime[i] = EEPROM.read(i * 2 + 40);
    managepin(i);  // set pin to stored value
  }
  Serial.print("EEP States ");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(EEPROM.read(i + 16));
    Serial.print(" ");
  }
  Serial.println(" ");
  Serial.print("pin States ");
  for (uint8_t i = 0; i < 8; i++) {
    Serial.print(pinState[i]);
    Serial.print(" ");
  }
  Serial.println(" ");
}
void loop() {

  // Debug printer
  if (millis() - last_debug_print > 1000) {
    last_debug_print = millis();
    Serial.printf("ZCD:%lu TMR:%lu CH0:%d%%\n",
                  debug_zcd_counter, debug_timer_counter, g_channels[0].brightness);
    debug_zcd_counter = 0;
    debug_timer_counter = 0;
  }

  // Serial Input
  if (Serial.available() > 0) {
    if (isDigit(Serial.peek())) {
      int brightness = Serial.parseInt();
      while (Serial.available() > 0 && !isDigit(Serial.peek())) Serial.read();

      if (brightness >= 0 && brightness <= 100) {
        Serial.printf("Set CH0: %d%%\n", brightness);
        noInterrupts();
        g_channels[0].brightness = brightness;
        interrupts();
      }
    } else {
      Serial.read();
    }
  }
  if (packetReceivedFlag == 1) {
    packetReceivedFlag = 0;
    processPacket();
  }
  if (espSwitchflag == 1) {
    espSwitchflag = 0;
    espSwitch();
  }
  if (ackreceivedFlag == 1) {
    ackreceivedFlag = 0;
    slaveId[noofslavebridged] = bridreq.nodeId;
    noofslavebridged++;

    esp_now_send(broadcastAddress, (uint8_t*)&receiveack, sizeof(bridgeack));  // send to main master esp32
  }
  if (bridgeRequestFlag == 1) {
    handelBridgeRequest();
  }
  /* if (forwardMessage != 0) {
      frwdMsg();
      forwardMessage = 222;
    } */
  if (sendAllFlag == 1) {
    sendAllFlag = 0;
    sendAllState();
  }

  manageTime();
  ct = millis();
  if (ct - pt > 500) {
    manageSwitch();
    // mgsw();
  }

  if (ct - st > 50000) {
    st = ct;
    Serial.print("EEP States ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(EEPROM.read(i + 16));
      Serial.print(" ");
    }
    Serial.println(" ");
    Serial.print("  pin States ");
    for (uint8_t i = 0; i < 8; i++) {
      Serial.print(pinState[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
  }


  // put your main code here, to run repeatedly:
}
