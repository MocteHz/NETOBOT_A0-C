#include "memorymanager.h"

bool MemoryManager::begin(const String &storageType, uint8_t sck, uint8_t miso, uint8_t mosi, uint8_t cs) {
  if (storageType == "SD") {
    _storageType = SD_CARD;
  } else {
    _storageType = LITTLE_FS;
  }

  if (storageType == "LittleFS") {
    if (!LITTLEFS.begin()) {
      return false;
    }
    _fs = &LITTLEFS;
  } else if (storageType == "SD") {
    SPI.begin(sck, miso, mosi, cs);
    if (!SD.begin(cs)) {
      return false;
    }
    _fs = &SD;
  } else {
    return false;
  }
  return true;
}

bool MemoryManager::writeConfig(const String &section, const String &key, const String &value) {
  String filename = "/" + section + ".ini";
  if (!exists(filename)) {
    if (!openFile(filename, "w")) {
      return false;
    }
  }

  String content = readString(filename);
  String newLine = key + "=" + value + "\n";

  int keyIndex = content.indexOf(key + "=");
  if (keyIndex != -1) {
    int endIndex = content.indexOf('\n', keyIndex);
    content = content.substring(0, keyIndex) + newLine + content.substring(endIndex + 1);
  } else {
    content += newLine;
  }

  return writeString(filename, content) > 0;
}

String MemoryManager::readConfig(const String &section, const String &key) {
  String filename = "/" + section + ".ini";
  if (!exists(filename)) {
    return "";
  }

  String content = readString(filename);
  int keyIndex = content.indexOf(key + "=");
  if (keyIndex != -1) {
    int endIndex = content.indexOf('\n', keyIndex);
    return content.substring(keyIndex + key.length() + 1, endIndex);
  }

  return "";
}

String MemoryManager::getStorageStatus() {
  uint64_t totalSpace = getTotalSpace();
  uint64_t usedSpace = getUsedSpace();
  uint64_t freeSpace = getFreeSpace();

  String status = "Total: " + String(totalSpace) + " bytes\n";
  status += "Used: " + String(usedSpace) + " bytes\n";
  status += "Free: " + String(freeSpace) + " bytes\n";

  return status;
}

bool MemoryManager::openFile(const String &filename, const String &mode) {
  String openMode = mode;
  if (openMode == "") {
    openMode = "a";
  }

  if (mode == "a" ) {
    openedFile = _fs->open(filename, FILE_APPEND);
  } else if (mode == "w") {
    openedFile = _fs->open(filename, FILE_WRITE);
  } else {
    return false;
  }
  return openedFile;
}


size_t MemoryManager::writeOpenedFile(const String &content) {
  if (!openedFile) {
    return 0;
  }
  return openedFile.print(content);
}

void MemoryManager::closeFile() {
  if (openedFile) {
    openedFile.close();
  }
}


size_t MemoryManager::writeString(const String &filename, const String &content) {
  if (!exists(filename)) {
    if (!openFile(filename, "w")) {
      return 0;
    }
  }

  File file;
  if (_storageType == LITTLE_FS) {
    file = LITTLEFS.open(filename, "w");
  } else if (_storageType == SD_CARD) {
    file = SD.open(filename, "w");
  }

  if (!file) {
    return 0;
  }

  size_t written = file.print(content);
  file.close();
  return written;
}


size_t MemoryManager::appendString(const String &filename, const String &content) {
  if (!exists(filename)) {
    if (!openFile(filename, "w")) {
      return 0;
    }
  }

  File file = _fs->open(filename, "a"); // Change "w" to "a" for append mode
  if (!file) {
    return 0;
  }

  size_t written = file.print(content);
  file.close();
  return written;
}

String MemoryManager::readString(const String &filename, size_t maxLength) {
  if (!exists(filename)) {
    return "";
  }

  File file = _fs->open(filename, "r");
  if (!file) {
    return "";
  }

  String content = "";
  size_t bytesRead = 0;
  while (file.available() && bytesRead < maxLength) {
    content += static_cast<char>(file.read());
    bytesRead++;
  }
  
  file.close();
  return content;
}

String MemoryManager::listFiles(const String &directory, const String &separator) {
  String fileList = "";
  File dir = _fs->open(directory);
  if (!dir) {
    return "";
  }

  File entry = dir.openNextFile();
  while (entry) {
    fileList += entry.name();
    fileList += separator;
    entry = dir.openNextFile();
  }
  
  return fileList;
}

String MemoryManager::listDir(const char *dirname, uint8_t levels) {
  String output = "Listing directory: " + String(dirname) + "\r\n";

  File root = _fs->open(dirname);
  if (!root) {
    output += "- failed to open directory\r\n";
    return output;
  }
  if (!root.isDirectory()) {
    output += " - not a directory\r\n";
    return output;
  }

  File file = root.openNextFile();
  while (file) {
    String fileName = String(file.name());
    if (!fileName.startsWith("/")) {
      fileName = "/" + fileName;
    }

    if (file.isDirectory()) {
      output += "  DIR : " + fileName + "\r\n";
      if (levels) {
        output += listDir(fileName.c_str(), levels - 1);
      }
    } else {
      output += "  FILE: " + fileName + "\tSIZE: " + String(file.size()) + "\r\n";
    }
    file = root.openNextFile();
  }
  return output;
}


String MemoryManager::readLine(const String &filename, size_t lineIndex) {
  if (!exists(filename)) {
    return "";
  }

  File file = _fs->open(filename, "r");
  if (!file) {
    return "";
  }

  size_t currentLine = 0;
  String line = "";
  while (file.available()) {
    line = file.readStringUntil('\n');
    if (currentLine == lineIndex) {
      break;
    }
    currentLine++;
  }

  file.close();
  return (currentLine == lineIndex) ? line : "";
}


bool MemoryManager::CheckIfMemoryIsUsed (){
  return MemoriaEnUso;
}

bool MemoryManager::OpenFile ( const String &filename){

  MemoriaEnUso = true;
  if (!exists(filename)) {
    return false;
  }
  openedFile = _fs->open(filename, "r");
  if (!openedFile) {
    return false;
  } 
  else {
    return true;
  }
  
}


bool MemoryManager::CloseFileAlreadyOppened(){
  openedFile.close();
  return true;
}

bool MemoryManager::MemoryAvailable(){
  return openedFile.available();
}

String MemoryManager::readNextLine ( ){
  String line;
  if (MemoriaEnUso == false){
    return "";
  }
  if (openedFile.available() > 0){
    line = openedFile.readStringUntil('\n') + '\n';
    return line;
  }
  else {
    openedFile.close();
    MemoriaEnUso = false;
    return "Fin";
  }
  return "";
}


bool MemoryManager::readLines(const String &filename, size_t startLine, size_t numLines, std::vector<String> &lines) {
  if (!exists(filename)) {
    return false;
  }

  File file = _fs->open(filename, "r");
  if (!file) {
    return false;
  }

  size_t currentLine = 0;
  size_t linesRead = 0;
  String line;
  while (file.available() && linesRead < numLines) {
    line = file.readStringUntil('\n');
    if (currentLine >= startLine) {
      lines.push_back(line);
      linesRead++;
    }
    currentLine++;
  }

  file.close();
  return linesRead > 0;
}

uint64_t MemoryManager::getTotalSpace() {
  if (_fs == &LITTLEFS) {
    return LITTLEFS.totalBytes();
  } else if (_fs == &SD) {
    return SD.totalBytes();
  } else {
    return 0;
  }
}

uint64_t MemoryManager::getUsedSpace() {
  if (_fs == &LITTLEFS) {
    return LITTLEFS.usedBytes();
  } else if (_fs == &SD) {
    return SD.usedBytes();
  } else {
    return 0;
  }
}

uint64_t MemoryManager::getFreeSpace() {
  uint64_t totalSpace = getTotalSpace();
  uint64_t usedSpace = getUsedSpace();
  return totalSpace - usedSpace;
}

bool MemoryManager::exists(const String &filename) {
  return _fs->exists(filename);
}

bool MemoryManager::remove(const String &filename) {
  return _fs->remove(filename);
}

bool MemoryManager::rename(const String &oldFilename, const String &newFilename) {
  return _fs->rename(oldFilename, newFilename);
}

size_t MemoryManager::getSize(const String &filename) {
  if (!exists(filename)) {
    return 0;
  }

  File file = _fs->open(filename, "r");
  if (!file) {
    return 0;
  }

  size_t fileSize = file.size();
  file.close();
  return fileSize;
}
bool MemoryManager::createDirectory(const String &dirname) {
  if (_storageType == LITTLE_FS) {
    return LITTLEFS.mkdir(dirname);
  } else if (_storageType == SD_CARD) {
    return SD.mkdir(dirname);
  }
  return false;
}

bool MemoryManager::removeDirectory(const String &dirname) {
  if (_storageType == LITTLE_FS) {
    return LITTLEFS.rmdir(dirname);
  } else if (_storageType == SD_CARD) {
    return SD.rmdir(dirname);
  }
  return false;
}

String MemoryManager::statusCheck() {
  String storageType;
  String initStatus;
  uint64_t totalSpace = 0;
  uint64_t usedSpace = 0;
  uint64_t freeSpace = 0;

  if (_fs == &LITTLEFS) {
    storageType = "LittleFS";
    initStatus = "Initialized";
    totalSpace = getTotalSpace();
    usedSpace = getUsedSpace();
    freeSpace = getFreeSpace();
  } else if (_fs == &SD) {
    storageType = "SD";
    initStatus = "Initialized";
    totalSpace = getTotalSpace();
    usedSpace = getUsedSpace();
    freeSpace = getFreeSpace();
  } else {
    initStatus = "Not initialized";
  }

  String statusJson = "{";
  statusJson += "\"storageType\": \"" + storageType + "\", ";
  statusJson += "\"initStatus\": \"" + initStatus + "\", ";
  statusJson += "\"totalSpace\": " + String(totalSpace) + ", ";
  statusJson += "\"usedSpace\": " + String(usedSpace) + ", ";
  statusJson += "\"freeSpace\": " + String(freeSpace);
  statusJson += "}";

  return statusJson;
}
