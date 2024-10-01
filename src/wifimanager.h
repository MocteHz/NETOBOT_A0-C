#ifndef WiFiManager_h
#define WiFiManager_h

#include <Arduino.h>
#include <ArduinoJson.h>

/*
 * TODO:
 * 
 * 1. **Mejorar la gestión del buffer de JSON:**
 *    - Revisar el tamaño del `DynamicJsonDocument` para asegurar que puede manejar un número grande de configuraciones Wi-Fi. Considerar parametrizar el tamaño o ajustarlo dinámicamente.
 * 
 * 2. **Implementar `autoConnect()` y `startConfigPortal()`:**
 *    - Actualmente, estas funciones retornan `true` sin realizar ninguna operación. Deben implementarse adecuadamente para gestionar el modo de autoconexión y el portal de configuración.
 *
 * 3. **Añadir almacenamiento persistente del JSON (archivo de configuración):**
 *    - Implementar funciones para guardar y cargar el archivo JSON desde un sistema de archivos persistente (por ejemplo, `SPIFFS` o `LittleFS`), permitiendo que las configuraciones Wi-Fi se mantengan después de reiniciar el dispositivo.
 *
 * 4. **Agregar manejo avanzado de reconexión Wi-Fi:**
 *    - Crear un sistema para reconectar automáticamente a las redes Wi-Fi si la conexión se pierde.
 *    - Implementar un mecanismo de retardo o número máximo de intentos antes de reiniciar o abrir el portal de configuración.
 *
 * 5. **Configurar la gestión de tiempo para cada red Wi-Fi:**
 *    - Parametrizar el tiempo de espera (`connectionTimeout`) individualmente para cada red Wi-Fi, en lugar de usar un valor global.
 *
 * 6. **Mejorar la lógica de desconexión del AP cuando STA se conecta:**
 *    - Hacer que el tiempo de desconexión del AP (`APdurationOnSTACon`) sea configurable y verificar el manejo de esta condición de forma más robusta.
 * 
 * 7. **Finalizar la implementación de `pauseConnection()` y `stopConnection()`:**
 *    - Definir claramente cómo el "pausar" y "detener" la conexión Wi-Fi afectará al flujo. Implementar la reanudación de la conexión cuando sea necesario.
 * 
 * 8. **Añadir logs detallados para la depuración:**
 *    - Incluir mensajes adicionales en todas las funciones clave (especialmente en los ciclos de reconexión y gestión del AP) para facilitar la depuración y entender mejor el comportamiento del sistema en tiempo real.
 * 
 * 9. **Implementar un método para resetear completamente las conexiones Wi-Fi:**
 *    - Crear un método para borrar toda la configuración de redes Wi-Fi de la memoria y del archivo persistente (si es implementado).
 * 
 * 10. **Agregar manejo de eventos para fallos de conexión:**
 *    - Definir eventos y callbacks que se disparen cuando ocurra un fallo en la conexión, para poder tomar acciones adicionales (por ejemplo, iniciar el portal de configuración).
 */



class WiFiManager {
  public:
    typedef void (*OnInitCallback)(); // Add this line for the callback function type definition
    typedef void (*OnRunServicesCallback)(); //Esto aqui
    typedef void (*OnCloseServicesCallback)(); //Esto aqui

    WiFiManager();

    // External functions
    void onInit(OnInitCallback callback);
    void onRunServices(OnRunServicesCallback callback); // Modify this line
    void onCloseServices(OnCloseServicesCallback callback);

    //As the name claims, generates a new wifi config and is added to the internal list
    int addWiFiConfiguration(String ssid, String password, String ip = "", String subnetMask = "", bool dhcp = true);
    //Intents to connect to a given WiFi
    bool ConnectToWiFi(String ssid, String password, String ip = "", String subnetMask = "", bool dhcp = true);
    //Tries to connect to a WiFi by the index inside of the configuration File
    bool ConnectToWifiByIndex (int index);
    //Removes a WifiConfig from the list
    void removeWiFiConfiguration(String ssid);
    //This function is used for the JsonString that the webpage generates in order to actualize the internal JsonFile that contains the information
    int loadJsonString(String jsonString);         
    //Search for the SSIDs available           
    String BuscaSSID();
    //Gives the internal JsonString that contains the list of Wifis to connect
    String getJsonString();
    //Returns the number of Wifis that exist in the internal File
    size_t getNumberOfWifis();
    //Returns the SSID and Password given a Index value
    bool getWiFiCredentialsByIndex(size_t index, String &ssid, String &password);
    //This copies the file Json to the internal JsonString, good for initialization of the file
    void loadJsonFromFile(String jsonString);
    //Clears the internal file of configurations
    void resetWiFiConfigurations();
    //Returns a JsonString with the connection Status in the
    String ConnectionStatus();


    // Functions for the WiFi connection::

    bool InitWifiConnections();
    bool autoConnect(bool Auto);
    bool pauseConnection(bool Pause);
    void setTimeOutConnection(int timeOutSeconds);
    void setTimeReturnSTA(int timeReturnSeconds);

    // Configurations

    bool startConfigPortal();
    void RunConnection();

  protected:
    void triggerOnInit();
    void triggerOnRunServices();
    void triggeronCloseServices();

  private:
    DynamicJsonDocument jsonDocument{1024};
    OnInitCallback _onInitCallback;
    OnRunServicesCallback _onRunServicesCallback; 
    OnCloseServicesCallback _onCloseServicesCallback;
    bool PauseConnection = false;
    bool StopConnection = false;
};

#endif