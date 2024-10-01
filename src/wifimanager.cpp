#include "WiFiManager.h"
#include <ArduinoJson.h>
#include <WiFi.h>


int connectionTimeout = 10;
bool APCreated = false;
unsigned long APdurationOnSTACon = 500;

WiFiManager::WiFiManager() : jsonDocument(1024) {
  jsonDocument.clear();
  JsonObject root = jsonDocument.to<JsonObject>();
  root["type"] = "WiFiConfig";
  JsonArray wifis = root.createNestedArray("wifis");
}

void WiFiManager::removeWiFiConfiguration(String ssid) {
  JsonObject root = jsonDocument.as<JsonObject>();
  JsonArray wifis = root["wifis"];

  for (int i = 0; i < wifis.size(); i++) {
    if (strcmp(wifis[i]["ssid"].as<const char*>(), ssid.c_str()) == 0) {
      wifis.remove(i);
      break;
    }
  }
}

int WiFiManager::loadJsonString(String jsonString) {
  DynamicJsonDocument inputJson(1024);
  DeserializationError error = deserializeJson(inputJson, jsonString);
  int index = -1;

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return index;
  }

  JsonObject inputObject = inputJson.as<JsonObject>();
  String ssid;
  String password;
  // Check if the JSON object has the required fields
  if (inputObject.containsKey("type") && inputObject["type"].as<String>() == "AddWifi" && inputObject.containsKey("ssid") && inputObject.containsKey("password")) {
    ssid = inputObject["ssid"].as<String>();
    password = inputObject["password"].as<String>();
    index = addWiFiConfiguration(ssid, password);
  } else {
    // Check if the JSON object has the required fields
    if (inputObject.containsKey("type") && inputObject["type"].as<String>() == "ConnectTo" && inputObject.containsKey("ssid") && inputObject.containsKey("password")) {
      ssid = inputObject["ssid"].as<String>();
      password = inputObject["password"].as<String>();
      index = addWiFiConfiguration(ssid, password);
    } else {
      Serial.println(F("Invalid JSON format or missing required fields."));
    }
  }
  return index;
}

String WiFiManager::getJsonString() {
  JsonObject root = jsonDocument.as<JsonObject>();
  String jsonString;
  serializeJson(root, jsonString);
  return jsonString;
}

// New method: Get the number of Wi-Fi configurations
size_t WiFiManager::getNumberOfWifis() {
  JsonObject root = jsonDocument.as<JsonObject>();
  JsonArray wifis = root["wifis"];
  return wifis.size();
}

// New method: Get SSID and password by index
bool WiFiManager::getWiFiCredentialsByIndex(size_t index, String &ssid, String &password) {
  JsonObject root = jsonDocument.as<JsonObject>();
  JsonArray wifis = root["wifis"];

  if (index < wifis.size()) {
    ssid = wifis[index]["ssid"].as<String>();
    password = wifis[index]["password"].as<String>();
    return true;  // Successfully retrieved the SSID and password
  } else {
    return false; // Index out of bounds
  }
}

String WiFiManager::BuscaSSID() {
  String values = "{";// "{\"SSIDDisp\":\"";
  int n = WiFi.scanNetworks(); //String(n)  + \",
  values +=  "\"SSIDList\": [ ";
  for (int i = 0; i < n; ++i)
  {
    // Print SSID and RSSI for each network found
    values += "{\"SSID\":\"" + WiFi.SSID(i) + "\",\"RSSI\":\"" + WiFi.RSSI(i) + "\",\"ENC\":\"";
    switch (WiFi.encryptionType(i))
    {
    case WIFI_AUTH_OPEN:
      values += "OPEN";
      break;
    case WIFI_AUTH_WEP:
      values += "WEP";
      break;
    case WIFI_AUTH_WPA_PSK:
      values += "WPA";
      break;
    case WIFI_AUTH_WPA2_PSK:
      values += "WPA2";
      break;
    case WIFI_AUTH_WPA_WPA2_PSK:
      values += "WPA+WPA2";
      break;
    case WIFI_AUTH_WPA2_ENTERPRISE:
      values += "WPA2-EAP";
      break;
    case WIFI_AUTH_WPA3_PSK:
      values += "WPA3";
      break;
    case WIFI_AUTH_WPA2_WPA3_PSK:
      values += "WPA2+WPA3";
      break;
    case WIFI_AUTH_WAPI_PSK:
      values += "WAPI";
      break;
    default:
      values += "UNKNOWN";
    }
    values += "\"}";
    if (i < n - 1)
    {
      values += ",";
    }
  }
  values += "]}";
  return values;
}

void WiFiManager::loadJsonFromFile(String jsonString) {
  DynamicJsonDocument inputJson(1024);
  DeserializationError error = deserializeJson(inputJson, jsonString);

  if (error) {
    Serial.print(F("deserializeJson() failed: "));
    Serial.println(error.c_str());
    return;
  }

  JsonObject inputObject = inputJson.as<JsonObject>();
  if (inputObject.containsKey("type") && inputObject["type"].as<String>() == "WiFiConfig" && inputObject.containsKey("wifis")) {
    jsonDocument = inputJson;
  } else {
    Serial.println(F("Invalid JSON format or missing required fields."));
  }
}


void WiFiManager::resetWiFiConfigurations() {
  jsonDocument.clear();
  JsonObject root = jsonDocument.to<JsonObject>();
  root["type"] = "WiFiConfig";
  JsonArray wifis = root.createNestedArray("wifis");
}


int WiFiManager::addWiFiConfiguration(String ssid, String password, String ip, String subnetMask, bool dhcp) {
  JsonObject root = jsonDocument.as<JsonObject>();
  JsonArray wifis = root["wifis"];

  JsonObject existingConfig = JsonObject();
  int index = 0;

  // Check if the SSID already exists
  for (JsonObject wifi : wifis) {
    if (wifi["ssid"].as<String>() == ssid) {
      existingConfig = wifi;
      break;
    }
    index++;
  }

  // If the SSID does not exist, create a new Wi-Fi configuration
  if (existingConfig.isNull()) {
    JsonObject wifiConfig = wifis.createNestedObject();
    wifiConfig["ssid"] = ssid;
    wifiConfig["password"] = password;

    if (ip != "") {
      wifiConfig["ip"] = ip;
    }
    if (subnetMask != "") {
      wifiConfig["subnetMask"] = subnetMask;
    }
    wifiConfig["dhcp"] = dhcp;

    index = wifis.size() - 1;
  } else {
    // If the SSID exists and any of the fields are different, update the existing configuration
    bool updated = false;

    if (existingConfig["password"].as<String>() != password) {
      existingConfig["password"] = password;
      updated = true;
    }

    if (ip != "" && existingConfig["ip"].as<String>() != ip) {
      existingConfig["ip"] = ip;
      updated = true;
    }

    if (subnetMask != "" && existingConfig["subnetMask"].as<String>() != subnetMask) {
      existingConfig["subnetMask"] = subnetMask;
      updated = true;
    }

    if (existingConfig["dhcp"].as<bool>() != dhcp) {
      existingConfig["dhcp"] = dhcp;
      updated = true;
    }

    if (updated) {
      Serial.println(F("Updated existing Wi-Fi configuration."));
    } else {
      Serial.println(F("Wi-Fi configuration is unchanged."));
    }
  }
  return index;
}


bool WiFiManager::InitWifiConnections() {
  Serial.println("Starting WiFi connection...");
  
  // Get number of Wi-Fi configurations
  size_t numberOfWifis = getNumberOfWifis();
  
  WiFi.mode(WIFI_AP_STA);
    // Check if there are any Wi-Fi configurations
  //TODO check if when there is not files it will create a 0 here
  if (numberOfWifis == 0) {
    Serial.println("No Wi-Fi configurations found.");
    // Start the configuration portal
  } else {
    Serial.print("Found ");
    Serial.print(numberOfWifis);
    Serial.println(" Wi-Fi configurations.");
    Serial.print("WiFi status: ");
    Serial.println(ConnectionStatus());
    // Try to connect to Wi-Fi
    for (size_t i = 0; i < numberOfWifis; i++) {
      String ssid;
      String password;
      
      if (getWiFiCredentialsByIndex(i, ssid, password)) {
        Serial.print("Trying to connect to SSID: ");
        Serial.print(ssid);
        Serial.print("...");
        WiFi.begin(ssid.c_str(), password.c_str());
        
        // Wait for Wi-Fi to connect
        bool isConnected = ConnectToWiFi(ssid, password);

        if (isConnected) {
          Serial.println("");
          Serial.print("Successfully connected to SSID: ");
          Serial.println(ssid);
          Serial.print("STA IP address: ");
          Serial.println(WiFi.localIP());
          Serial.println("Starting Init services");
          triggerOnInit();
          return true;
        } else {
          Serial.println("");
          Serial.print("Failed to connect to SSID: ");
          Serial.println(ssid);
          WiFi.disconnect();
          delay(100);
        }
      }
    }
    // If no Wi-Fi configuration works, start the configuration portal
    Serial.println("No working Wi-Fi configurations found., starting AP");
  }
// Set the access point SSID and password
  String apSsid = "Netobot-AP";
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAP(apSsid.c_str());
  Serial.print("WiFi status: ");
  Serial.println(WiFi.status());

  // Get the IP address of the access point
  IPAddress apIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(apIP);
  APCreated = true;
  Serial.println("APCreated true, starting services");
  triggerOnInit();
  return false;
}

bool WiFiManager::pauseConnection(bool Pause){
  PauseConnection = Pause;
}

void WiFiManager::onInit(OnInitCallback callback) {
  _onInitCallback = callback;
}

void WiFiManager::onRunServices(OnRunServicesCallback callback) {
  _onRunServicesCallback = callback;
}

void WiFiManager::triggerOnInit() {
  if (_onInitCallback) {
    _onInitCallback();
  }
}

void WiFiManager::triggerOnRunServices() {
  if (_onRunServicesCallback) {
    _onRunServicesCallback();
  }
}

void WiFiManager::onCloseServices(OnCloseServicesCallback callback){
  _onCloseServicesCallback = callback;
}

void WiFiManager::triggeronCloseServices(){
  if (_onCloseServicesCallback) {
    _onCloseServicesCallback();
  }
}

bool WiFiManager::ConnectToWiFi(String ssid, String password, String ip, String subnetMask, bool dhcp){
  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect(true);
  }
  // Configure DHCP and static IP settings
  if (!dhcp) {
    IPAddress localIP, subnet;
    if (localIP.fromString(ip) && subnet.fromString(subnetMask)) {
      WiFi.config(localIP, IPAddress(0, 0, 0, 0), subnet);
    } else {
      // Invalid IP or subnet mask provided
      return false;
    }
  } else {
    // Enable DHCP (optional, it's the default setting)
    //WiFi.config(INADDR_NONE, INADDR_NONE, INADDR_NONE);
  }

  // Connect to the WiFi network
  WiFi.begin(ssid.c_str(), password.c_str());
  // Wait for connection

  int timeout = connectionTimeout;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    timeout--;
  }
  // Connection successful
  return WiFi.status() == WL_CONNECTED;
}

bool WiFiManager::ConnectToWifiByIndex(int index) {
  String ssid, password, ip, subnetMask;
  bool dhcp;

  if (!getWiFiCredentialsByIndex(index, ssid, password)) {
    // Invalid index or failed to get the WiFi credentials
    Serial.println("Valio madres");
    return false;
  }

  if (WiFi.status() == WL_CONNECTED) {
    WiFi.disconnect(true);
  }

  // Connect to the WiFi network
  Serial.println("Conectando a: SSID:");
  Serial.println(ssid);
  WiFi.mode(WIFI_AP_STA);

  WiFi.begin(ssid.c_str(), password.c_str());
  // Wait for connection

  int timeout = connectionTimeout;
  while (WiFi.status() != WL_CONNECTED && timeout > 0) {
    delay(1000);
    timeout--;
  }

  // Connection successful
  if(WiFi.status() == WL_CONNECTED){
    Serial.println("Conectado");
  }else {
    Serial.println("Desconectado");
  }
  return WiFi.status() == WL_CONNECTED;
}

String WiFiManager::ConnectionStatus() {
  // Create a DynamicJsonDocument to store the data
  DynamicJsonDocument doc(512);

  // Add the requested data to the JSON object
  doc["type"] = "StatusConfig";
  doc["SSID"] =  WiFi.SSID(); // Replace with the actual SSID value
  doc["ENC"] = true; // Replace with the actual encryption status
  doc["IP"] = WiFi.localIP().toString();  // Replace with the actual IP address value
  doc["SUBMASK"] =WiFi.subnetMask().toString(); // Replace with the actual subnet mask value

  // Convert the JSON object to a string and return it
      // Get the connection mode
  wifi_mode_t mode = WiFi.getMode();
  String connectionMode;
  if (mode == WIFI_STA) {
      connectionMode = "STA";
  } else if (mode == WIFI_AP) {
      connectionMode = "AP";
  } else if (mode == WIFI_AP_STA) {
      connectionMode = "STAAP";
  } else {
      connectionMode = "Unknown";
  }
  doc["connectionMode"] = connectionMode;

  if (WiFi.status() == WL_CONNECTED) {
        doc["Status"] = "Connected";
    } else {
        doc["Status"] = "Disconnected";
    }
  String jsonString;
  serializeJson(doc, jsonString);
  return jsonString;
}

int TimeOut ;
int TimeReturn ;

bool WiFiManager::autoConnect(bool Auto) {
  return true;
}
void WiFiManager::setTimeOutConnection(int timeOutSeconds){
  connectionTimeout = timeOutSeconds;
}
void WiFiManager::setTimeReturnSTA(int timeReturnSeconds){
  TimeReturn = timeReturnSeconds;
}
bool WiFiManager::startConfigPortal(){
  return true;
}

unsigned long Timestamp = 0;
unsigned long Elapsed = 0;
bool FirstAPandSTA = false;


void WiFiManager::RunConnection(){

  if (PauseConnection == true) {

  } 

  if (StopConnection == true) {

  }

  if (WiFi.status() != WL_CONNECTED && APCreated == false) {
    // WiFi is not connected, call the initialization again
    triggeronCloseServices();
    InitWifiConnections();    //Este se debe de llamar en ciertas circunstancias, solo cuando se deconecte

  } 
  if(APCreated == true){
    triggerOnRunServices();

  }
  if (WiFi.status() == WL_CONNECTED ) {
    triggerOnRunServices();

  }
  if(APCreated == true && WiFi.status() == WL_CONNECTED){
    //Hay que apagar el AP
    if ( FirstAPandSTA == false){
      FirstAPandSTA = true;
      Timestamp = millis();
    }
    Elapsed = millis() - Timestamp;
    if(Elapsed/1000 > APdurationOnSTACon){
      WiFi.softAPdisconnect();
      APCreated = false;
      FirstAPandSTA = false;
    }
  }
}
