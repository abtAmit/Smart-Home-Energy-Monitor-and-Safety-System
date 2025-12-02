/*
 * This is a modified sketch for an ESP8226 that serves multiple complete webpages
 * from program memory, removing the need for SPIFFS.
 */
/*
 * REQUIRED LIBRARIES:
 * - ESP8266WiFi
 * - ESP Async WebServer for ESP8266 (https://github.com/me-no-dev/ESPAsyncWebServer)
 * - ESPAsyncTCP for ESP8266 (https://github.com/me-no-dev/ESPAsyncTCP)
 * - ArduinoJson (https://arduinojson.org/)
 */
// #include "time.h"
#include <Ticker.h>
#include <SHA256.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>
#include <ESPmDNS.h>
#include <esp_now.h>
#include <esp_wifi.h>
#include "webpage.h"
#include <EEPROM.h>
//---------------------------------------------------------------User Configurable---------------------------
#define pageName "smarthome"
const uint8_t channel = 1;
const uint8_t NumberOfNodes = 6;  // enter number of node (esp8266) you want to connect
// --- Replace with your network credentials ---
const char *http_username = "admin";
const char *http_password = "home";
const char *ssid = "vivo y15";                         //"vivo Y300 5G";  // "Galaxy A21s8DB5"   //
const char *password = "12367890";                     // "alkc2049"  //"12367890";
char hmac_key[] = "HashkeyShouldBeExcatly32letter12";  // must change the key for better security (Should must Be Excatly 32 letter including spaces)


// --- !! IMPORTANT: CALIBRATION !! ---
// You MUST tune these values for your specific hardware.

// 1. Voltage Calibration (V_CAL)
// Formula: (Mains Voltage / Transformer Secondary) * (R1 + R2) / R2
#define V_CAL 201.6

// 2. Current Calibration (I_CAL)
// Formula: CT Ratio / Burden Resistor Value
#define I_CAL 90.9
// (If your CTs or burden resistors are different, make separate I_CAL values)
#define I_CAL1 90.9
#define I_CAL2 90.9
#define I_CAL3 90.9

// 3. Phase Shift Calibration (PHASE_CAL)
// This corrects for timing shifts from the transformers.
#define PHASE_CAL 1.7

//-----------------------------------------------------------END-----------------------------------------------

//-----------------------advance config---------------------
// NTP settings
const char *ntpServer = "pool.ntp.org";
const long gmtOffset_sec = 19800;  // For India (UTC +5:30) → 5*3600 + 1800
const int daylightOffset_sec = 0;  // No DST in India

// --- Define your ADC pins ---
// Make sure these are ADC-capable pins on your ESP32-C6
#define V_PIN 1  // ADC Pin for Voltage (from 12V transformer)
#define I_PIN1 2 // ADC Pin for CT 1
#define I_PIN2 3 // ADC Pin for CT 2
#define I_PIN3 4 // ADC Pin for CT 3

// --- Create three EnergyMonitor objects ---
EnergyMonitor emon1;
EnergyMonitor emon2;
EnergyMonitor emon3;





// ------------------------------------
// uint8_t LMK[16] = "ThisIsOurLMKKey";
// Ticker sendBeats;
// Ticker receiveBeats;
void requestBridge(uint8_t);
String sessionList[3] = { "", "", "" };
const uint8_t MAX_BUTTONS = 8;
uint8_t numberofswitch[NumberOfNodes];
uint8_t roomID[NumberOfNodes] = { 101, 102, 103, 104, 105, 106 };
// uint8_t nodeAddress[NumberOfNodes][6] = { { 0x40, 0x91, 0x51, 0x58, 0x5A, 0xF2 } };

// uint8_t d_pins[] = {16, 5, 4, 0, 2, 14, 12, 13};

struct DeviceState {
  bool hasSlider;
  uint8_t value;
  bool state;
};
DeviceState allDeviceStates[NumberOfNodes][MAX_BUTTONS];
uint8_t broadcastAddress[] = { 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF };


// uint8_t pinStat[NumberOfNodes][2][10];
uint8_t pinStat[NumberOfNodes][10];
uint8_t totalPins[NumberOfNodes] = { 3, 4, 6, 2, 2, 1 };  // hardcoded in esp nodes  // writing pins data for test
typedef struct nodeDetails {
  uint8_t typ;
  uint8_t DeviceID;
  uint8_t TotalPins;
  uint8_t pinStat[2][10];

} nodeDetails;
nodeDetails nodeDet;

uint8_t nodeMac[NumberOfNodes][6];

uint8_t dpins[] = {
  16,  // D0
  5,   // D1
  4,   // D2
  0,   // D3
  2,   // D4
  14,  // D5
  12,  // D6
  13,  // D7
       // 15, // D8  ---- Boot fails if pulled HIGH
  3,   //rx
  1    //tx
};
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
AsyncEventSource events("/events");  // This is the new endpoint

typedef struct swStat {
  uint8_t typ;
  uint8_t id;
  uint8_t val;
  // int hash[];
} swStat;
swStat incomingMessage;
swStat webMessage;
// --- HARDWARE CONFIGURATION MOCK ---
struct Device {
  uint8_t gpio;
  bool hasSlider;
};


// Structs and variables to hold safety settings on the ESP
struct GlobalSettings {
  uint16_t voltage = 240;
  uint8_t current = 15;
};

struct RoomSettings {
  uint8_t id;
  uint8_t current = 10;
  uint16_t voltage = 240;  // can be greater than 255
  bool override = false;
};

GlobalSettings globalSettings;
RoomSettings roomSettings[sizeof(roomID)];
typedef struct SW {  // store received data from web
  uint8_t DeviceID;
  uint8_t GPIO;
  uint8_t dpin;
  uint8_t state;
  uint8_t slider;
  uint8_t value;
  // uint8_t hash[32];
} SW;
SW upsw;
typedef struct schedule {  // store received schedule from web
  uint8_t roomId;
  uint8_t gpio;
  uint16_t onTime;
  uint16_t offTime;
  uint8_t hash[32];
} schedule;
schedule sch;
#include "espnowcomm.h"

bool state = HIGH;  // the current state of the output pin
void led(bool);
void updateswitch() {  // copy all pin data and send to node

  // Serial.println(" MAC: ");

  Serial.print("sending to node ");
  Serial.print(upsw.DeviceID);

  Serial.println(" ");


  pinupd.type = 2;
  pinupd.id = upsw.DeviceID;
  pinupd.pin = upsw.GPIO;
  pinupd.state = upsw.state;
  pinupd.value = upsw.value;
  pinupd.slider = upsw.slider;
  hashData(&pinupd, offsetof(pinUpdate, hash), calculated_hmac);
  memcpy(pinupd.hash, calculated_hmac, 32);
  uint8_t index = (upsw.DeviceID % 100) - 1;  //
  esp_now_send(broadcastAddress, (uint8_t *)&pinupd, sizeof(pinUpdate));
  // esp_now_send(nodeAddress[index], (uint8_t *)&pinupd, sizeof(pinUpdate));




  if (upsw.DeviceID == 101) {
    if (upsw.state > 0) {
      state = 1;
      led(1);
    } else {
      state = 0;
      led(0);
    }
  }
  Serial.printf("Control Command:\n Room: %d, GPIO: %d, State: %d, Value: %d\n\n", upsw.DeviceID, upsw.GPIO, upsw.state, upsw.value);
}
void ScheduleUpdate() {
  sendSchedule.type = 3;
  sendSchedule.id = sch.roomId;
  sendSchedule.pin = sch.gpio;
  sendSchedule.onTime = (sch.onTime + 1);
  sendSchedule.offTime = (sch.offTime + 1);
  hashData(&sendSchedule, offsetof(scheduleUpdate, hash), calculated_hmac);
  memcpy(sendSchedule.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&sendSchedule, sizeof(scheduleUpdate));

  Serial.printf("Schedule Update:\n Room: %d, GPIO: %d, ON: %u, OFF: %u\n\n", sch.roomId, sch.gpio, sch.onTime, sch.offTime);
}
void SettingsUpdate(uint8_t i) {
  Serial.printf("Room %u Settings Update:\n V=%u, C=%u, Override=%s\n\n", roomSettings[i].id, roomSettings[i].voltage, roomSettings[i].current, roomSettings[i].override ? "true" : "false");
  EEPROM.put(i, (roomSettings[i].override));
  EEPROM.put(i + 8, (roomSettings[i].current));
  EEPROM.put((i * 2) + 16, (roomSettings[i].voltage));
  EEPROM.commit();
}
void requestBridge(uint8_t id) {  // incomplete
}
uint8_t *mac;
// === Callbacks ===
void onDataSent(const esp_now_send_info_t *tx_info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println(" ");
  // Serial.print("This device MAC: ");
}

void onDataRecv(const esp_now_recv_info_t *recv_info, const uint8_t *incomingData, int len) {
  // mac = recv_info->src_addr;
  uint8_t packetid = 0;
  uint8_t typ = 0;
  // lengt = len;
  memcpy(&typ, incomingData, 1);
  Serial.println(" ");
  Serial.println("---------------------");
  Serial.println("Received from node ");
  if (typ == 10) {
    if (len != sizeof(pinUpdate)) {
      Serial.println("!!! ERROR: Received packet with mismatched size for type 10");
      return;  // Stop processing this packet
    }
    memcpy(&pinupd, incomingData, sizeof(pinupd));
    hashData(&pinupd, offsetof(pinUpdate, hash), calculated_hmac);
    if (memcmp(pinupd.hash, calculated_hmac, 32) == 0) {
      saveState();
    } else {
      Serial.println("---");
      Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
    }
  }

  else if (typ == 2) {
    if (len != sizeof(pinUpdate)) {
      Serial.println("!!! ERROR: Received packet with mismatched size for type 2");
      return;  // Stop processing this packet
    }
    memcpy(&pinupd, incomingData, sizeof(pinupd));
    hashData(&pinupd, offsetof(pinUpdate, hash), calculated_hmac);
    if (memcmp(pinupd.hash, calculated_hmac, 32) == 0) {
      needSwUpdate = 1;

      Serial.print("ID ");
      Serial.println(pinupd.id);
      Serial.print("pin ");
      Serial.println(pinupd.pin);
      Serial.print("slider ");
      Serial.println(pinupd.slider);
      Serial.print("Slider value ");
      Serial.println(pinupd.value);
      Serial.print("State ");
      Serial.println(pinupd.state);

    } else {
      Serial.println("---");
      Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
    }
  }

  else if (typ == 5) {
    if (len != sizeof(heartBeat)) {
      Serial.println("!!! ERROR: Received packet with mismatched size for type 5");
      return;
    }
    memcpy(&beat, incomingData, sizeof(beat));
    hashData(&beat, offsetof(heartBeat, hash), calculated_hmac);
    if (memcmp(beat.hash, calculated_hmac, 32) == 0) {
      polack[(beat.id - 101)] = beat.pol;

    } else {
      Serial.println("---");
      Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
    }
  }
  /*
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
  }*/
  else if (typ == 6) {
    if (len != sizeof(espTime)) {
      Serial.println("!!! ERROR: Received packet with mismatched size for type 6");
      return;
    }
    memcpy(&espTime, incomingData, sizeof(espTime));
    hashData(&espTime, offsetof(timming, hash), calculated_hmac);
    if (memcmp(espTime.hash, calculated_hmac, 32) == 0) {
      sendTimeFlag = 1;
    } else {
      Serial.println("---");
      Serial.print("!!! WARNING: Received packet with INVALID SIGNATURE ");
    }
  }
}

#include "functions.h"
int inPin = 9;             // the number of the input pin
int outPin = LED_BUILTIN;  // the number of the output pin


int reading;         // the current reading from the input pin
int previous = LOW;  // the previous reading from the input pin

// the follow variables are long's because the time, measured in miliseconds,
// will quickly become a bigger number than can be stored in an int.
unsigned long time1 = 0;         // the last time the output pin was toggled
unsigned long debounce = 200UL;  // the debounce time, increase if the output flickers

void setup() {
  Serial.begin(115200);
  Serial.println("\nStarting up ESP32...");
  randomSeed(analogRead(0));
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, 1);
  delay(20);
  digitalWrite(LED_BUILTIN, 0);

  pinMode(3, INPUT_PULLUP);
  /* 
  roomSettings[1].override = true;
  roomSettings[1].current = 20;
*/
  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("ESP-NOW init failed!");
    while (true) delay(1000);
  }
  // === Add broadcast peer  ===
  esp_now_peer_info_t broadcastPeer = {};
  memcpy(broadcastPeer.peer_addr, broadcastAddress, 6);
  broadcastPeer.channel = 0;
  broadcastPeer.encrypt = false;
  if (esp_now_add_peer(&broadcastPeer) != ESP_OK) {
    Serial.println("Failed to add broadcast peer!");
  } else {
    Serial.println("Broadcast peer added.");
  }
  //--------------------

  esp_now_register_send_cb(onDataSent);
  esp_now_register_recv_cb(onDataRecv);
  /*
 if (esp_now_set_pmk(LMK) != ESP_OK) {
      Serial.println("Failed to set PMK!");
      return;
  }*/


  WiFi.setSleep(false);
  WiFi.begin(ssid, password);
  channelSet channel;
  channel.channel = WiFi.channel();
  WiFi.disconnect();
  esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE);
  hashData(&channel, offsetof(channelSet, hash), calculated_hmac);
  memcpy(channel.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&channel, sizeof(channel));
  delay(100);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.print("This device MAC: ");
  Serial.println(WiFi.macAddress());
  Serial.print("Current WiFi Channel: ");
  Serial.println(WiFi.channel());


  if (!MDNS.begin(pageName)) {  // <-- 2. Start mDNS with your chosen hostname
    Serial.println("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }

  WebRequestRouter();
  server.begin();
  Serial.println("HTTP server started");
  MDNS.addService("http", "tcp", 80);  // <-- 3. (Optional) Advertise the service
                                       // allDeviceStates[0][0].hasSlider = 1;

  


  // Initialize NTP
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️  Failed to obtain time");  // yellow status
    return;
  }
  EEPROM.begin(128);
  for (size_t i = 0; i < sizeof(roomID); i++) {
    roomSettings[i].id = roomID[i];
  }
  for (uint8_t i = 0; i < NumberOfNodes; i++) {
    EEPROM.get(i, (roomSettings[i].override));
    EEPROM.get(i + 8, (roomSettings[i].current));
    EEPROM.get((i * 2) + 16, (roomSettings[i].voltage));
  }
  EEPROM.get(33, globalSettings.voltage);
  EEPROM.get(35, globalSettings.current);
  for (uint8_t i = 0; i < NumberOfNodes; i++) {
    getNodeState((101 + i));
    delay(1000);  // wait for node to send data
  }

  // heartBeat();
  // sendBeats.attach(600, setFlag10Min);

  // --- Configure Monitor 1 ---
  // All monitors share the same voltage pin and calibration
  emon1.voltage(V_PIN, V_CAL, PHASE_CAL);
  emon1.current(I_PIN1, I_CAL1); // But each has a unique current pin

  // --- Configure Monitor 2 ---
  emon2.voltage(V_PIN, V_CAL, PHASE_CAL);
  emon2.current(I_PIN2, I_CAL2);

  // --- Configure Monitor 3 ---
  emon3.voltage(V_PIN, V_CAL, PHASE_CAL);
  emon3.current(I_PIN3, I_CAL3);
}
void handle_physical_button(bool new_value) {
  uint8_t a_roomId = 101;
  uint8_t a_gpio = 16;

  // ... your logic to toggle the device ...
  // ... and get the new_value (e.g., 0 or 255) ...

  //= digitalRead(9) == HIGH ? 255 : 0;
  pinStat[0][0] = 1;
  // Broadcast the change
  sendStateUpdate(a_roomId, a_gpio, new_value, new_value, 0);
}
void led(bool v) {
 // digitalWrite(LED_BUILTIN, v);
  allDeviceStates[0][0].value = v;
}
bool f = 0;
unsigned long pre = 0;
unsigned long curr = 0;
unsigned long preHeart = 0;
bool heartbeatsentFlag = 0;
void loop() {
  emon1.calcVI(20, 2000);
  emon2.calcVI(20, 2000);
  emon3.calcVI(20, 2000);
  curr = millis();
  /* if (curr - pre > 1000) {  // checks every seconds
    pre = curr;
    for (uint8_t i = 0; i < NumberOfNodes; i++) {
      if (lostNodes[i] != 0) {
        requestBridge(lostNodes[i]);
      }
    }
  }*/

  if (curr - preHeart > 60000) {  // ten minutes
    preHeart = curr;
    heartbeat();
    heartbeatsentFlag = 1;
  }
  if (heartbeatsentFlag == 1) {
    heartbeatsentFlag = 0;
    beatReceived();
  }


  if (needSwUpdate == 1) {  // update webpage if sw physically changed
    needSwUpdate = 0;
    saveState();
    sendStateUpdate(pinupd.id, dpins[pinupd.pin], pinupd.value, pinupd.state, pinupd.slider);
  }
  if (sendTimeFlag == 1) {
    sendTimeFlag = 0;
    sendTime();
  }

  reading = digitalRead(inPin);

  // if the input just went from LOW and HIGH and we've waited long enough
  // to ignore any noise on the circuit, toggle the output pin and remember
  // the time
  if (reading == HIGH && previous == LOW && millis() - time1 > debounce) {
    state = !state;  //flip state value

    time1 = millis();
    // handle_physical_button(state);
    led(state);
    uint8_t roomid = 101;
    overCurrent(roomid);
  }


  if (digitalRead(3) == 0 and f == 0) {
    f = 1;
    overVoltage(101);
  }
  if (digitalRead(3) == 1 and f == 1) {
    faultClear(101);
    f = 0;
  }

  previous = reading;

  // Async server does not require code in loop()
}


// void serverRouter() { }
