uint8_t calculated_hmac[32];
volatile bool needSwUpdate = 0;
volatile bool sendTimeFlag = 0;
struct tm timeinfo;  // store time
uint8_t polack[NumberOfNodes];
uint8_t lostNodes[NumberOfNodes];
bool gotack = 0;

// espnow data structure
typedef struct channelSet {  
  uint8_t type = 11;
  uint8_t channel;
 // uint8_t errcode;
  uint8_t hash[32];

} channelSet;


typedef struct pinUpdate {  // r
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


typedef struct scheduleUpdate {  // only send
  uint8_t type = 3;
  uint8_t id;
  uint8_t pin;
  uint16_t onTime;
  uint16_t offTime;
  uint8_t hash[32];
} scheduleUpdate;
scheduleUpdate sendSchedule;

typedef struct error {  //31 current //32 voltage  // only send
  uint8_t type = 4;
  uint8_t id;
  uint8_t errcode;
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
//pair pairreq;

typedef struct __attribute__((packed)) timming {  // r
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
// temp temp1;
typedef struct bridge {  // bridge request  //type 8
  uint8_t type = 8;
  uint8_t id;
  uint8_t nodeId;
  uint8_t nodemac[6];
  uint8_t sendermac[6];
  uint8_t hash[32];
} bridge;
// bridge bridreq;
typedef struct bridgeack {  // bridge request  //type 9 // r
  uint8_t type = 9;
  uint8_t id;
  uint8_t nodeid;
  uint8_t hash[32];
} bridgeack;
// bridgeack receiveack;
// bridgeack ack;
typedef struct sendallstate {
  uint8_t type = 10;
  uint8_t id;
  uint8_t hash[32];

} sendallstate;
sendallstate sendall;
bool sendAllFlag = 0;
void saveState() {
  uint8_t index = (pinupd.id % 100) - 1;
  allDeviceStates[index][pinupd.pin].value = pinupd.value;
  // allDeviceStates[index][pinupd.pin].hasSlider=pinupd.slider;  // node cant change slider, so no need to update slider
  allDeviceStates[index][pinupd.pin].state = pinupd.state;
  numberofswitch[index] = pinupd.numberofswitch;
}

void hashData(void *structaddress, uint8_t length, uint8_t *hash) {  // hash data for integrety
  Serial.println("----");
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

void addPeer(uint8_t *peerMac) {  // === Add encrypted unicast peer ===

  esp_now_peer_info_t peer = {};
  memcpy(peer.peer_addr, peerMac, 6);
  peer.channel = 0;
  peer.encrypt = true;
  memcpy(peer.lmk, LMK, 16);
  if (esp_now_add_peer(&peer) != ESP_OK) {
    Serial.println("Failed to add encrypted peer!");
  } else {
    Serial.println("Encrypted peer added.");
  }
}
void overCurrent(uint8_t roomId) {
  err.id = roomId;
  err.errcode = 31;  // overcurrent code
  Serial.println("over current protection engaged");

  hashData(&err, offsetof(error, hash), calculated_hmac);
  memcpy(err.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&err, sizeof(error));
}
void overVoltage(uint8_t roomId) {
  err.id = roomId;
  err.errcode = 32;  // over voltage code
  Serial.println("over voltage protection engaged");

  hashData(&err, offsetof(error, hash), calculated_hmac);
  memcpy(err.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&err, sizeof(error));
}
void faultClear(uint8_t roomId) {
  err.id = roomId;
  err.errcode = 35;  // No fault code
  Serial.println("----------------FaultClear---------------");

  hashData(&err, offsetof(error, hash), calculated_hmac);
  memcpy(err.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&err, sizeof(error));
}
void getNodeState(uint8_t id) {
  sendall.id = id;
  hashData(&sendall, offsetof(sendallstate, hash), calculated_hmac);
  memcpy(sendall.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&sendall, sizeof(sendallstate));
}
void sendTime() {
  if (!getLocalTime(&timeinfo)) {
    Serial.println("⚠️  Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "📅 %A, %d %B %Y ⏰ %H:%M:%S");
  uint8_t sec = 0;
  if (timeinfo.tm_sec > 30) { sec = 1; }

  uint16_t gtime = (timeinfo.tm_hour * 60) + timeinfo.tm_min + sec;

  espTime.ctime = gtime;  // need to fetch
  hashData(&espTime, offsetof(timming, hash), calculated_hmac);
  memcpy(espTime.hash, calculated_hmac, 32);
  esp_now_send(broadcastAddress, (uint8_t *)&espTime, sizeof(timming));
}

void heartbeat() {
  uint8_t i = 0;
  for (uint8_t b = 0; b < NumberOfNodes; b++) {
    // for (uint8_t t = 0; t < 3; t++) {
    beat.id = (101 + b);
    polack[b] = 0;  // node will set pol 1
    hashData(&beat, offsetof(heartBeat, hash), calculated_hmac);
    memcpy(beat.hash, calculated_hmac, 32);
    esp_now_send(broadcastAddress, (uint8_t *)&beat, sizeof(heartBeat));
    // delay(2000);
  }
  // }

}
void beatReceived() {
  for (uint8_t i = 0; i < NumberOfNodes; i++) {
    if (polack[i] != 1) { lostNodes[i] = beat.id; }
  }
  for (uint8_t i = 0; i < NumberOfNodes; i++) {
    if (lostNodes[i] > 0) {
      requestBridge(lostNodes[i]);
      Serial.printf("Node lost ID: %d \n",lostNodes[i]);
      lostNodes[i] = 0;  // clear id from lost node
    }
  }
}