#ifndef MEMORY_MANAGER_H
#define MEMORY_MANAGER_H

#include <Arduino.h>
#include <FS.h>
#include <LITTLEFS.h>
#include <SD.h>
#include <SPI.h>
#include <vector>

//TODO: create config file json write and reads for WifiManager and other configurations.
// TODO: Implementar manejo de errores para operaciones fallidas de archivos (por ejemplo, fallos al abrir, escribir, leer).
// TODO: Añadir soporte para otros modos de archivo en el método openFile (por ejemplo, "r" para el modo lectura).
// TODO: Asegurar la sincronización entre operaciones cuando varios procesos intenten acceder al mismo archivo (seguridad en hilos).
// TODO: Crear un método para manejar la escritura y lectura de archivos de configuración en formato JSON (writeConfig y readConfig usan actualmente formato .ini).
// TODO: Optimizar los métodos de lectura de archivos (por ejemplo, readString, readLine) para manejar archivos grandes de manera más eficiente.
// TODO: Implementar un mecanismo de tiempo de espera (timeout) en las operaciones de archivos en caso de almacenamiento no responsive (por ejemplo, al acceder a tarjetas SD corruptas).
// TODO: Añadir registro detallado de operaciones de archivo para facilitar la depuración y el seguimiento de problemas.
// TODO: Incluir métodos para gestionar archivos de configuración en diferentes formatos (por ejemplo, JSON, XML).
// TODO: Crear un mecanismo para manejar múltiples archivos abiertos (actualmente, solo se puede tener un archivo abierto a la vez).
// TODO: Asegurar la consistencia en las convenciones de nombres entre los métodos (por ejemplo, OpenFile vs openFile).
// TODO: Mejorar los mensajes de error y devolver razones detalladas para fallos en métodos como openFile, writeString y readString.

class MemoryManager {
 public:
  enum StorageType {
    LITTLE_FS,
    SD_CARD
  };
  bool begin(const String &storageType, uint8_t sck = 18, uint8_t miso = 19, uint8_t mosi = 23, uint8_t cs = 5);
  bool writeConfig(const String &section, const String &key, const String &value);
  String readConfig(const String &section, const String &key);
  String getStorageStatus();

  bool openFile(const String &filename, const String &mode);
  size_t writeOpenedFile(const String &content);
  void closeFile();

  size_t writeString(const String &filename, const String &content);
  size_t appendString(const String &filename, const String &content);
  String readString(const String &filename, size_t maxLength = 256);
  String listFiles(const String &directory = "/", const String &separator = "\n");
  String listDir(const char *dirname = "/",  uint8_t levels = 1);


  String readLine(const String &filename, size_t lineIndex);
  bool readLines(const String &filename, size_t startLine, size_t numLines, std::vector<String> &lines);

  bool OpenFile(const String &filename );
  String readNextLine();
  bool MemoryAvailable();
  bool CloseFileAlreadyOppened();
  bool CheckIfMemoryIsUsed();

  uint64_t getTotalSpace();
  uint64_t getUsedSpace();
  uint64_t getFreeSpace();

  bool exists(const String &filename);
  bool remove(const String &filename);
  bool rename(const String &oldFilename, const String &newFilename);

  size_t getSize(const String &filename);

  bool createDirectory(const String &dirname);
  bool removeDirectory(const String &dirname);

  String statusCheck();


 private:
  fs::FS *_fs;
  StorageType _storageType;
  File openedFile;
  bool MemoriaEnUso;


};

#endif // MEMORY_MANAGER_H
