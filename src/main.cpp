#include <Arduino.h>
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include "ESPAsyncWebServer.h"
#include "FS.h"
#include <LITTLEFS.h>
#include <WebSocketsServer.h>
#include "wifimanager.h"
#include "pin_out.h"
#include "memorymanager.h"
#include "ReproductionLibrary.h"
#include "commandManager.h"
#include "esp_task_wdt.h"
#include <FastLED.h>

#define NUM_LEDS 50
#define DATA_PIN 14
CRGBArray<NUM_LEDS> leds;
uint32_t rgb =0;
uint8_t BrilloOut = 0;

MemoryManager SDManager;
MemoryManager FSManager;
WiFiManager wifiManager;
ReproductionLibrary RepManager(SDManager);

DNSServer dnsServer;
AsyncWebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

CommandManager Command(wifiManager, SDManager, RepManager );

bool name_received = false;
bool proficiency_received = false;
bool TrytoConnectWifi = false;
bool wasConnected = false;
long FileSizeUpload = 0;
long AcumuladoUpload = 0;

String FileUpload = "";
File file ;

class CaptiveRequestHandler : public AsyncWebHandler
{
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request)
  {
    // request->addInterestingHeader("ANY");

    return true;
  }
  //exclusivo para portal captivo
  void handleRequest(AsyncWebServerRequest *request)
  {
    String path = request->url();
    Serial.println(path);
    if (path == "/") {
      request->send(LITTLEFS, "/captive_.html", "text/html", false);
    }
    if (path == "/vue.js") {
      request->send(LITTLEFS, "/vue.js", "application/javascript", false);
    }
    if (path == "/fonts.css") {
      request->send(LITTLEFS, "/fonts.css", "text/css", false);
    }
    
  }
};

void setupServer() {
  server.onNotFound([](AsyncWebServerRequest *request) {
    
    String path = request->url();
    Serial.println(path);
    if (path == "/") path = "/index.html"; // Special case for root URL
    Serial.println("Quiere página");
    // Determine the content type
    String contentType;
    if (path.endsWith(".html")) contentType = "text/html";
    else if (path.endsWith(".css")) contentType = "text/css";
    else if (path.endsWith(".js")) contentType = "application/javascript";
    else if (path.endsWith(".svg")) contentType = "image/svg+xml";
    else if (path.endsWith(".png")) contentType = "image/png";
    else if (path.endsWith(".jpg") || path.endsWith(".jpeg")) contentType = "image/jpeg";
    else if (path.endsWith(".gif")) contentType = "image/gif";
    else contentType = "text/plain";

    if (path.startsWith("/Images")) {
      // Serve the file from the SD card
      Serial.println("Entrando a imagenes: Tipo de contenido");
      Serial.println(contentType);
      if (SDManager.exists(path)) {
        request->send(SD, path, contentType);
      } else {
        request->send(404, "text/plain", "File not found");
      }
    } else {
      // Serve the file from LittleFS
      if (LITTLEFS.exists(path)) {
        request->send(LITTLEFS, path, contentType);
      } else {
        request->send(404, "text/plain", "File not found");
      }
    }
  });
}

void hexdump(const void *mem, uint32_t len, uint8_t cols = 16){
  const uint8_t *src = (const uint8_t *)mem;
  Serial.printf("\n[HEXDUMP] Address: 0x%08X len: 0x%X (%d)", (ptrdiff_t)src, len, len);
  for (uint32_t i = 0; i < len; i++)
  {
    if (i % cols == 0)
    {
      Serial.printf("\n[0x%08X] 0x%08X: ", (ptrdiff_t)src, i);
    }
    Serial.printf("%02X ", *src);
    src++;
  }
  Serial.printf("\n");
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length){

  switch (type)
  {
  case WStype_DISCONNECTED:
    Serial.printf("[%u] Disconnected!\n", num);
    break;
  case WStype_CONNECTED:
    {
      IPAddress ip = webSocket.remoteIP(num);
      Serial.printf("[%u] Connected from %d.%d.%d.%d url: %s\n", num, ip[0], ip[1], ip[2], ip[3], payload);

      // send message to client
      /*
      webSocket.sendTXT(num, "Connected");
      String response = wifiManager.getJsonString();
      webSocket.sendTXT(num, response);
      response = BuscaSSID();
      webSocket.sendTXT(num, response);*/
      break;
    }
  case WStype_TEXT:
    {
      Serial.printf("[%u] get Text: %s\n", num, payload);

      String DataIn = ""; //Convierte el dato a String para manejarlo fácil
      for (int i = 0; i < length; i++) {
        DataIn += (char)payload[i];
      }
      Serial.println(DataIn);
      String Response = Command.processCommand(DataIn);
      //Command.RunLed();
      Serial.println(Response);
      webSocket.sendTXT(num, Response);
      rgb = Command.rgbOut;
      BrilloOut = Command.Brillo;
      break;
    }
    case WStype_BIN:
    {
      Serial.printf("[WSc] get binary length: %u\n", length);
      String DataIn = ""; //Convierte el dato a String para manejarlo fácil
      for (int i = 0; i < length; i++) {
        DataIn += (char)payload[i];
      }
      AcumuladoUpload += length;
      String jsonString = "{\"type\":\"upload-progress\",\"loaded\":" + String(AcumuladoUpload) + ",\"total\":" + String(FileSizeUpload) + "}";
      String Response = Command.UploadHandle( DataIn);  
      Serial.println(Response);
      webSocket.sendTXT(num, Response);

    }
  }
}

void initServices()  {
  // Custom implementation here
  //log_e("Error de prueba");
  Serial.println("Initializing Server, DNS server, Websocket server");
  setupServer();
  dnsServer.start(53, "*", WiFi.softAPIP());
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER); // only when requested from AP
  // more handlers...
  server.begin();
  //WebSockets
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void runServices() {
  dnsServer.processNextRequest();
  webSocket.loop();
}

void closeServices(){
  server.end();
  webSocket.close();
  dnsServer.stop();
  Serial.println("Stopping services");
}

// Declaración de la tarea
void Task1(void *pvParameters);
void Task2(void *pvParameters);

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  FastLED.addLeds<WS2812, DATA_PIN, GRB>(leds, NUM_LEDS);  // GRB ordering
  FastLED.setBrightness(  32 );
  
  Serial.println("Start");

  esp_task_wdt_init(5, true);  //WDT es 5 segundos


  //Manejo de memorias
  bool sdInitSuccess = SDManager.begin("SD", 18, 19, 23, 5);

  if (sdInitSuccess) {
    Serial.println("SD card initialized successfully.");

    String InfoMemoria = SDManager.statusCheck();
    Serial.println(InfoMemoria);

  } else {
    Serial.println("Failed to initialize SD card.");
  }


  RepManager.checkAndFixFileList();
  /*
  String fname = "/Files/TurnStar.thr";
  SDManager.openFile(fname, "a");
  SDManager.writeOpenedFile("Prueba");
  SDManager.closeFile();
  */
 Serial.println("RepManager Start");
  if (!RepManager.begin()) {
    Serial.println("Error initializing the library.");
    return;
  } else {
    Serial.println("Correctly Initialized RepManager");
  }
  Serial.println(SDManager.listDir());
  bool FSInitSuccess = FSManager.begin("LittleFS");
  if (FSInitSuccess) {
    Serial.println("FS initialized successfully.");
  } else {
    Serial.println("Failed to initialize FS.");
  }
  Serial.println(FSManager.statusCheck());
  Serial.println(FSManager.listDir("/", 1));

  //Manejo de servidores
  //Inicialización del manejo de redes
  
  wifiManager.onInit(initServices);           //Services that need to start
  wifiManager.onRunServices(runServices);     //Services that will be called in the loop
  wifiManager.onCloseServices(closeServices); //Services that must stop when there is no connection
  wifiManager.setTimeOutConnection(7);        //TimeOut to 7seg

  FSManager.removeDirectory("/WifiConfig.txt");
  Serial.println("Escribiendo Archivo");
  String JsonConfig = "{\"type\":\"WiFiConfig\",\"wifis\":[{\"ssid\":\"INFINITUM6128\",\"password\":\"chinguense\",\"dhcp\":true}]}";
  Serial.println("String a escribir");
  Serial.println(JsonConfig);
  FSManager.openFile("/WifiConfig.txt", "w");
  FSManager.writeOpenedFile(JsonConfig);
  FSManager.closeFile();


  Serial.println("Configuración inicial en memoria de WifiConfig");
  String InitialConfigJson = FSManager.readString("/WifiConfig.txt", 300);
  Serial.println(InitialConfigJson);
  wifiManager.loadJsonFromFile(InitialConfigJson);

  // Read WiFi configuration from the file
  //wifiManager.addWiFiConfiguration("INFINITUM6128", "chinguense");

  wifiManager.InitWifiConnections();

  //RepManager.play("/StarClean.thr");

  //RepManager.Reproduce();
  Serial.println("Configuración del rep manager:");
  Serial.println(RepManager.getPlayerStatus());


  xTaskCreatePinnedToCore(
    Task1,          // Función de la tarea
    "Task1",        // Nombre de la tarea (para depuración)
    10000,          // Tamaño de la pila de la tarea
    NULL,           // Parámetros de entrada
    1,              // Prioridad de la tarea
    NULL,           // Manejador de la tarea
    0);             // Núcleo en el que se ejecutará la tarea (0 o 1)

  xTaskCreatePinnedToCore(
    Task2,          // Función de la tarea
    "Task2",        // Nombre de la tarea (para depuración)
    10000,          // Tamaño de la pila de la tarea
    NULL,           // Parámetros de entrada
    1,              // Prioridad de la tarea
    NULL,           // Manejador de la tarea
    1);             // Núcleo en el que se ejecutará la tarea (0 o 1)
}

//Este while es el que corre en el procesador 2, maneja el movimiento.
void Task2(void *pvParameters) {
  while (true) {
    RepManager.HandleRepLib();
  }
}
int contador = 0;

//While de manejo del procesador 1, maneja la operación de comunicaciones y otros.
void Task1(void *pvParameters) {
  while (true) {
    wifiManager.RunConnection();
    leds[contador] = rgb ;
    FastLED.show();

    contador++;
    if (contador >= NUM_LEDS){
      contador = 0;
    }
    FastLED.setBrightness(  BrilloOut );
    vTaskDelay(1); 
  }
  
  
}


void loop() {
  // put your main code here, to run repeatedly:
  
/*
  static uint8_t hue;
  for(int i = 0; i < NUM_LEDS/2; i++) {   
    // fade everything out
    leds.fadeToBlackBy(50);

    // let's set an led value
    leds[i] = 0xd5b59c ;

    // now, let's first 20 leds to the top 20 leds, 
    leds(NUM_LEDS/2,NUM_LEDS-1) = leds(NUM_LEDS/2 - 1 ,0);
    FastLED.show();
  }
*/

  
}