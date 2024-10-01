#ifndef COMMAND_MANAGER_H
#define COMMAND_MANAGER_H

#include <Arduino.h>
#include <WiFiManager.h>
#include "MemoryManager.h"
#include "ReproductionLibrary.h"

class CommandManager {
 public:
  CommandManager(WiFiManager &wifiManager, MemoryManager &memoryManager, ReproductionLibrary &reproductionLibrary);
  String processCommand(const String &command);
  String UploadHandle (const String &data );
  uint32_t rgbOut;
  uint8_t Brillo;

 private:
  WiFiManager &_wifiManager;
  MemoryManager &_memoryManager;
  ReproductionLibrary &_reproductionLibrary;
  String FileUploadName;
  long filesize;
  long AcumuladoUpload;
};

#endif // COMMAND_MANAGER_H
