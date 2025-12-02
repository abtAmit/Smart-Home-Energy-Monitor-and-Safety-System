/*
bool addPeer(const uint8_t *mac_addr, const uint8_t *lmk_key) {
  esp_now_peer_info_t peer;
  memset(&peer, 0, sizeof(peer));  // Clear the peer info structure

  // Copy the peer's MAC address
  memcpy(peer.peer_addr, mac_addr, 6);

  // Set the Wi-Fi channel (0 means current channel)
  peer.channel = 0;
  peer.ifidx = WIFI_IF_STA;
  // Set the LMK for this peer
  memcpy(peer.lmk, lmk_key, ESP_NOW_KEY_LEN);

  // Enable encryption
  peer.encrypt = true;

  // Add the peer
  esp_err_t addStatus = esp_now_add_peer(&peer);

  if (addStatus == ESP_OK) {
    Serial.println("Peer Added Successfully");
    return true;
  } else if (addStatus == ESP_ERR_ESPNOW_EXIST) {
    Serial.println("Peer Already Exists");
    return true;  // Already exists is not a failure for this function
  } else {
    Serial.print("Failed to add peer, error: ");
    Serial.println(esp_err_to_name(addStatus));
    return false;
  }
} */
// Callback function for when data is sent
void OnDataSent(const esp_now_send_info_t *info, esp_now_send_status_t status) {
  Serial.print("\r\nLast Packet Send Status:\t");
  if (status == ESP_NOW_SEND_SUCCESS) {
    Serial.println("Delivery Success");
  } else {
    Serial.println("Delivery Fail");
  }
}
void printIncomingData() {
  Serial.println("\r\n--- Message Received ---");
  Serial.print("From MAC: ");
  // Print the sender's MAC address

  Serial.printf("%02X:%02X:%02X:%02X:%02X:%02X\r\n", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  // Print the data from the structure
  Serial.print("typ: ");
  Serial.println(incomingMessage.typ);
  Serial.print("Char: ");
  Serial.println(incomingMessage.id);
  Serial.print("Int: ");
  Serial.println(incomingMessage.val);
  Serial.println("------------------------");
}

void sendStateUpdate(uint8_t roomId, uint8_t gpio, uint8_t value, bool state, bool slider) {
  JsonDocument doc;
  doc["roomId"] = roomId;
  doc["gpio"] = gpio;
  doc["state"] = state;  // <-- This is the key translation
  doc["slider"] = slider;
  doc["value"] = value;

  String jsonResponse;
  serializeJson(doc, jsonResponse);

  // "stateChange" MUST match your JavaScript's addEventListener
  events.send(jsonResponse.c_str(), "stateChange", millis());
}
/*
 * HANDLER 1: /get-config-maps
 * create the room and pin maps on web. This is fast.
 // send dpin array to web to map
 */
void handleGetConfigMaps(AsyncWebServerRequest *request) {
  // Small document, this is a small message
  DynamicJsonDocument doc(2048);

  // The root is no longer just two simple arrays.
  // It will contain ONE array called "rooms".
  JsonArray roomsArray = doc.createNestedArray("rooms");

  // Loop through each room
  for (int r = 0; r < NumberOfNodes; r++) {

    //
    JsonObject roomObj = roomsArray.createNestedObject();

    roomObj["id"] = roomID[r];
    if (numberofswitch[r] == 0) { numberofswitch[r] = 4; }
    JsonArray pinsArray = roomObj.createNestedArray("pins");
    for (int b = 0; b < numberofswitch[r]; b++) {

      pinsArray.add(dpins[b]);  // Assuming roomPins is your 2D pin array
    }
  }
/*
  JsonArray pins = doc.createNestedArray("pins");
  for (int b = 0; b < MAX_BUTTONS; b++) {
    pins.add(dpins[b]);
  }
  */

  String responseString;
  serializeJson(doc, responseString);
  request->send(200, "application/json", responseString);
}

/*
 * HANDLER 2: /get-raw-states
 * Sends the 48-item state array. This is also fast.
 * sends status of each button
 */
void handleGetRawStates(AsyncWebServerRequest *request) {
  // Larger doc, 48 objects
  DynamicJsonDocument doc(2048);
  Serial.println("looped in states");
  JsonArray states = doc.to<JsonArray>();
  for (int r = 0; r < NumberOfNodes; r++) {
    if (numberofswitch[r] == 0) { numberofswitch[r] = 4; }
    for (int b = 0; b < numberofswitch[r]; b++) {
      JsonObject s = states.createNestedObject();
      s["v"] = allDeviceStates[r][b].value;      // "v" for value
      s["s"] = allDeviceStates[r][b].hasSlider;  // "s" for slider
      s["p"] = allDeviceStates[r][b].state;
    }
  }

  String responseString;
  serializeJson(doc, responseString);
  request->send(200, "application/json", responseString);
}
// --- Helper Function (NEW) ---
// This checks if the user's cookie is in our "guest list"
bool isAuthenticated(AsyncWebServerRequest *request) {
  if (!request->hasHeader("Cookie")) return false;  // No cookie at all

  String cookie = request->getHeader("Cookie")->value();

  // Loop through our 3-person list
  for (int i = 0; i < 3; i++) {
    if (sessionList[i] == "") continue;  // Skip empty slots

    // Check if the user's cookie contains a valid session ID
    if (cookie.indexOf("session=" + sessionList[i]) != -1) {
      return true;  // Found a match!
    }
  }

  return false;  // No match found in the list
}
void WebRequestRouter() {
  // --- WEB SERVER ROUTES ---
  server.on("/login", HTTP_GET, [](AsyncWebServerRequest *request) {
    request->send_P(200, "text/html", login_html);
  });
  // --- 4. The Logout Handler ---
  // NEW /logout HANDLER
  server.on("/logout", HTTP_GET, [](AsyncWebServerRequest *request) {
    // Find and clear this user's session from the list
    if (request->hasHeader("Cookie")) {
      String cookie = request->getHeader("Cookie")->value();
      for (int i = 0; i < 3; i++) {
        if (sessionList[i] == "") continue;
        if (cookie.indexOf("session=" + sessionList[i]) != -1) {
          sessionList[i] = "";  // Found it, clear the slot
          break;
        }
      }
    }

    // Send the response to delete the cookie and redirect
    AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Found");
    response->addHeader("Location", "/login");
    response->addHeader("Set-Cookie", "session=; Path=/; Expires=Thu, 01 Jan 1970 00:00:00 GMT");
    request->send(response);
  });
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
      request->redirect("/login");
    } else {
      request->send_P(200, "text/html", HTML_PROGMEM);
    }
  });

  server.on("/get-config-maps", HTTP_GET, handleGetConfigMaps);  // load first time
  server.on("/get-raw-states", HTTP_GET, handleGetRawStates);

  server.on("/power", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
      request->redirect("/login");
    } else {
      request->send_P(200, "text/html", POWER_HTML_PROGMEM);
    }
  });

  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
      request->redirect("/login");
    } else {
      request->send_P(200, "text/html", SETTINGS_HTML_PROGMEM);
    }
  });
  // --- MODIFIED: Graph endpoints now use room IDs and no longer send colors ---
  server.on("/graph/today", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["totalConsumption"] = 17.5 + (random(0, 50) / 10.0);
    JsonArray rooms = doc["rooms"].to<JsonArray>();

    for (size_t i = 0; i < sizeof(roomID); i++) {
      JsonObject room = rooms.add<JsonObject>();
      room["id"] = roomID[i];
      room["consumption"] = (random(10, 60) / 10.0);
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/graph/details", HTTP_GET, [](AsyncWebServerRequest *request) {
    String roomIdStr = "Total";
    String range = "daily";

    if (request->hasParam("roomId")) roomIdStr = request->getParam("roomId")->value();
    if (request->hasParam("range")) range = request->getParam("range")->value();

    JsonDocument doc;
    JsonObject chart = doc["chart"].to<JsonObject>();
    JsonArray chart_labels = chart["labels"].to<JsonArray>();
    JsonArray chart_data = chart["data"].to<JsonArray>();

    uint8_t num_points = 7;
    if (range == "hourly") {
      num_points = 8;
      const char *labels[] = { "12am", "3am", "6am", "9am", "12pm", "3pm", "6pm", "9pm" };
      for (uint8_t i = 0; i < num_points; i++) chart_labels.add(labels[i]);
    } else if (range == "daily") {
      num_points = 7;
      const char *labels[] = { "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun" };
      for (uint8_t i = 0; i < num_points; i++) chart_labels.add(labels[i]);
    } else if (range == "monthly") {
      num_points = 6;
      const char *labels[] = { "Jan", "Feb", "Mar", "Apr", "May", "Jun" };
      for (uint8_t i = 0; i < num_points; i++) chart_labels.add(labels[i]);
    } else {
      num_points = 3;
      const char *labels[] = { "2023", "2024", "2025" };
      for (uint8_t i = 0; i < num_points; i++) chart_labels.add(labels[i]);
    }
    for (uint8_t i = 0; i < num_points; i++) { chart_data.add(random(10, 50)); }

    JsonArray list = doc["List"].to<JsonArray>();  // Renamed for clarity in JS
    if (roomIdStr == "Total") {
      for (size_t i = 0; i < sizeof(roomID); i++) {
        JsonObject item = list.add<JsonObject>();
        item["id"] = roomID[i];
        item["consumption"] = (random(10, 60) / 10.0);
      }
    } else {
      JsonObject item1 = list.add<JsonObject>();
      item1["name"] = "Main Appliance";
      item1["consumption"] = (random(10, 30) / 10.0);
      item1["uptime"] = String(random(1, 12)) + "h " + String(random(1, 60)) + "m";
    }

    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });


  server.on("/graph", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (!isAuthenticated(request)) {
      request->redirect("/login");
    } else {
      request->send_P(200, "text/html", GRAPH_HTML_PROGMEM);
    }
  });

  server.on("/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    doc["voltage"] = 220.0 + (random(0, 200) / 10.0) - 10.0;
    doc["pf"] = 0.95 + (random(0, 6) / 100.0) - 0.03;
    doc["wattage"] = 300.0 + random(0, 250);
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/roompower", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    float voltage = 225.0;
    for (size_t i = 0; i < sizeof(roomID); i++) {
      JsonObject room = array.add<JsonObject>();
      float power = 50.0 + random(0, 2000) / 10.0;
      room["id"] = roomID[i];
      room["power"] = power;
      room["ampere"] = power / voltage;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/rooms", HTTP_GET, [](AsyncWebServerRequest *request) {
    JsonDocument doc;
    JsonArray array = doc.to<JsonArray>();
    for (size_t i = 0; i < sizeof(roomID); i++) {
      JsonObject room = array.add<JsonObject>();
      room["id"] = roomID[i];
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });

  server.on("/devices", HTTP_GET, [](AsyncWebServerRequest *request) {
    if (request->hasParam("roomId")) {
      uint8_t roomId = request->getParam("roomId")->value().toInt();
      JsonDocument doc;
      JsonArray array = doc.to<JsonArray>();
      Device *devices;
      size_t count = 0;
      Serial.println("looped in devices");
      uint8_t node = (roomId % 100) - 1;
      for (size_t i = 0; i < totalPins[node]; i++) {
        JsonObject dev = array.add<JsonObject>();
        dev["gpio"] = dpins[i];
        dev["hasSlider"] = pinStat[node][i];
      }
      String jsonResponse;
      serializeJson(doc, jsonResponse);
      request->send(200, "application/json", jsonResponse);
    } else {
      request->send(400, "text/plain", "Missing roomId parameter");
    }
  });

  server.on("/getsettings", HTTP_GET, [](AsyncWebServerRequest *request) {
    DynamicJsonDocument doc(2048);
    JsonObject global = doc["global"].to<JsonObject>();
    global["voltage"] = globalSettings.voltage;
    global["current"] = globalSettings.current;
    JsonArray rooms = doc["rooms"].to<JsonArray>();
    for (size_t i = 0; i < sizeof(roomID); i++) {
      JsonObject room = rooms.add<JsonObject>();
      room["id"] = roomSettings[i].id;
      room["voltage"] = roomSettings[i].voltage;
      room["current"] = roomSettings[i].current;
      room["override"] = roomSettings[i].override;
    }
    String jsonResponse;
    serializeJson(doc, jsonResponse);
    request->send(200, "application/json", jsonResponse);
  });


  // --- POST HANDLERS ---

  // NEW /login POST HANDLER
  server.on("/login", HTTP_POST, [](AsyncWebServerRequest *request) {
    if (request->hasParam("username", true) && request->hasParam("password", true)) {
      String user = request->getParam("username", true)->value();
      String pass = request->getParam("password", true)->value();

      if (user == http_username && pass == http_password) {
        // --- SUCCESS! Find an empty slot ---
        int emptySlot = -1;
        for (int i = 0; i < 3; i++) {
          if (sessionList[i] == "") {
            emptySlot = i;
            break;
          }
        }

        if (emptySlot != -1) {
          // --- Found a slot ---
          String newSessionId = String(random(0xffffffff), HEX);
          sessionList[emptySlot] = newSessionId;  // Add to guest list

          AsyncWebServerResponse *response = request->beginResponse(302, "text/plain", "Found");
          response->addHeader("Location", "/");
          response->addHeader("Set-Cookie", "session=" + newSessionId + "; Path=/");
          request->send(response);
          return;
        } else {
          // --- No slots left! ---
          request->redirect("/login?error=full");
          return;
        }
      }
    }
    // --- FAILURE (Bad password) ---
    request->redirect("/login?error=1");
  });


  server.on(
    "/control", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      if (deserializeJson(doc, (const char *)data, len) == DeserializationError::Ok) {
        upsw.DeviceID = doc["roomId"];
        upsw.GPIO = doc["gpio"];
        upsw.state = doc["state"];
        upsw.value = doc["value"];
        upsw.slider = doc["slider"];
        for (uint8_t i = 0; i < sizeof(dpins); i++) {
          if (upsw.GPIO == dpins[i]) {
            upsw.dpin = i;  // upsw.GPIO;
            break;
          }
        }

        updateswitch();
        // sendStateUpdate(upsw.DeviceID, upsw.GPIO, upsw.value, upsw.state,upsw.slider);
      }
    });

  server.on(
    "/schedule", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      if (deserializeJson(doc, (const char *)data, len) == DeserializationError::Ok) {
        sch.roomId = doc["roomId"];
        sch.gpio = doc["gpio"];
        sch.onTime = doc["onTime"];
        sch.offTime = doc["offTime"];
        ScheduleUpdate();
      }
    });

  server.on(
    "/setsetting", HTTP_POST, [](AsyncWebServerRequest *request) {
      request->send(200);
    },
    NULL, [](AsyncWebServerRequest *request, uint8_t *data, size_t len, size_t index, size_t total) {
      JsonDocument doc;
      if (deserializeJson(doc, (const char *)data, len) == DeserializationError::Ok) {
        uint8_t roomId = doc["roomId"];
        if (roomId == 222) {
          globalSettings.voltage = doc["voltage"];
          globalSettings.current = doc["current"];
          EEPROM.put(33,globalSettings.voltage);
          EEPROM.put(35,globalSettings.current);
          EEPROM.commit();
          Serial.printf("Global Settings Update:\n Voltage: %d, Current: %d\n\n", globalSettings.voltage, globalSettings.current);
        } else {
          for (size_t i = 0; i < sizeof(roomID); i++) {
            if (roomSettings[i].id == roomId) { // check index of room
              roomSettings[i].voltage = doc["voltage"];
              roomSettings[i].current = doc["current"];
              roomSettings[i].override = doc["override"];
              SettingsUpdate(i);
              break;
            }
          }
        }
      }
    });
  // This attaches the /events endpoint to your web server
  server.addHandler(&events);
  server.onNotFound([](AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Not found.");
  });
}