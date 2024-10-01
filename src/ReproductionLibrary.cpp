#include "ReproductionLibrary.h"
#include "pin_out.h"
#include "GyverPlanner2.h"

Stepper<STEPPER2WIRE> stepper1(XSTEP, XDIR);
Stepper<STEPPER2WIRE> stepper2(YSTEP, YDIR);
GPlanner2<STEPPER2WIRE, 2> planner;

ReproductionLibrary::ReproductionLibrary(MemoryManager &sdManager) : _sdManager(sdManager), _random(false), _bucle(false) {
}
bool ReproductionLibrary::Reproduce(){
  _PlayActive = true;
  return true;
}
bool ReproductionLibrary::begin() {
   // Check if totalFileList exists, if not, create it
  if (!_sdManager.exists( "/" + _totalFileListName)) {
    _sdManager.writeString( "/" + _totalFileListName, ""); // Changed to writeString()
  }
  // Check if likedFileList exists, if not, create it
  if (!_sdManager.exists( "/" + _likedFileListName)) {
    _sdManager.writeString( "/" + _likedFileListName, ""); // Changed to writeString()
  }
  // Check if the folders for images, files, and playlists exist, if not, create them
  if (!_sdManager.exists( "/Images")) {
    _sdManager.createDirectory( "/Images");
  }
  if (!_sdManager.exists( "/Files")) {
    _sdManager.createDirectory( "/Files");
  }
  if (!_sdManager.exists( "/Playlists")) {
    _sdManager.createDirectory( "/Playlists");
  }

  // Load all files and liked files
  _loadAllFiles();
  _loadLikedFiles();
  getNextFile();
  printAllFiles();
  
  pinMode(Enable, OUTPUT);
  digitalWrite(Enable, LOW);

  // añadimos los motores paso a paso a los ejes
  planner.addStepper(0, stepper1);  // eje 0
  planner.addStepper(1, stepper2);  // eje 1

  _Pause = false;
  _NuevoArchivoAbierto = true; 
  _ReadyToFeed = true; 
  _PlayActive = true;

  SpeedMotors = 5000;
  
  // establecemos la aceleración y la velocidad
  planner.setAcceleration(800);
  planner.setMaxSpeed(SpeedMotors);      //Max 5000 

  return true;
}

bool ReproductionLibrary::uploadFileToBase(const FileProperties &fileProperties) {
  // Open the file with the provided name in the imagePath
  String filePath =  "";

  if(fileProperties.ext == "thr" ){
    filePath =  _filePath + "/" + fileProperties.name;
    bool success = addFileToRoot(fileProperties);
  }
  if(fileProperties.ext == "png" ){
    filePath =  _imagePath + "/" + fileProperties.name;
  }
  Serial.println("File Path:");
  Serial.println(filePath);
  // Check if file already exists
  if (_sdManager.exists(filePath)) {
    // Remove existing file and thumbnail
    _sdManager.remove(filePath);
    // Remove existing file information from totalFileList
    String fileListContents = _sdManager.readString(_basePath + _totalFileListName);
    String fileLine = fileProperties.toString();
    int lineIndex = fileListContents.indexOf(fileLine);

    if (lineIndex != -1) {
      fileListContents.remove(lineIndex, fileLine.length() + 1); // +1 to remove the newline character
      _sdManager.writeString(_basePath + _totalFileListName, fileListContents);
    }
  }
  _sdManager.openFile(filePath, "a"); // Create it if it does not exist
  
  // Update the _allFiles vector
  _loadAllFiles();
  return true;
}

void ReproductionLibrary::printAllFiles() {
  Serial.println("Archivos en _allFiles:");

  if (_allFiles.empty()) {
    Serial.println("No hay archivos cargados.");
    return;
  }

  for (size_t i = 0; i < _allFiles.size(); i++) {
    Serial.print(i + 1);
    Serial.print(": ");
    Serial.println(_allFiles[i]);
  }
}

String ReproductionLibrary::handleUploadFileToBase(const String &Data) {

  _sdManager.writeOpenedFile(Data);


  return "";
}

bool ReproductionLibrary::addFileToRoot(const FileProperties &fileProperties) {
  // Open the _totalFileList.txt in append mode
  if (_sdManager.openFile( _totalFileListName, "a")) {
    // Prepare a comma-separated entry with the FileProperties values
    String fileEntry = fileProperties.name + "," + fileProperties.date + "," + fileProperties.author + "," +
                       String(fileProperties.length) + "," + String(fileProperties.reversible) + "," +
                       String(fileProperties.origin) + "\n";

    // Append the file entry to _totalFileList.txt
    _sdManager.appendString(_totalFileListName, fileEntry);
    // Close the file
    _sdManager.closeFile();
    // Update the _allFiles vector
    _allFiles.push_back(fileProperties.name);

    return true;
  }
  return false;
}

bool ReproductionLibrary::uploadImageToBase(const FileProperties &fileProperties) {
  // Open the file with the provided name in the imagePath
  _sdManager.openFile(_imagePath + fileProperties.name, ""); // Create it if it does not exist
  
  // Call addFileToRoot to fill the control file
  bool success = addFileToRoot( fileProperties);

  return success;
}

String ReproductionLibrary::handleUploadImageToBase(const String &Data) {
  return "";
}

// Remove a file from the root file list
bool ReproductionLibrary::removeFileFromRoot(const String &fileName) {
  // Implement the logic for removing a file from the root file list
  // Update the _totalFileList.txt to remove the specified file entry
  // Return true if successful, false otherwise
   return false;
}

// Clear the root file list
bool ReproductionLibrary::clearRootFileList() {
  // Implement the logic for clearing the root file list
  // Delete all entries in the _totalFileList.txt
  // Return true if successful, false otherwise
  return false;
}

// Add a file to the liked file list
bool ReproductionLibrary::addFileToLiked(const String &fileName) {
  // Implement the logic for adding a new file to the liked file list
  // Update the _likedFileList.txt with the new file entry (comma-separated)
  // Return true if successful, false otherwise
   return false;
}

// Remove a file from the liked file list
bool ReproductionLibrary::removeFileFromLiked(const String &fileName) {
  // Implement the logic for removing a file from the liked file list
  // Update the _likedFileList.txt to remove the specified file entry
  // Return true if successful, false otherwise
   return false;
}

// Clear the liked file list
bool ReproductionLibrary::clearLikedFileList() {
  // Implement the logic for clearing the liked file list
  // Delete all entries in the _likedFileList.txt
  // Return true if successful, false otherwise
   return false;
}


bool ReproductionLibrary::createPlaylist(const String &playlistName) {
  String playlistPath = _getPlaylistPath(playlistName);
  if (!_sdManager.exists(playlistPath)) {
    return _sdManager.createDirectory(playlistPath);
  }
  return false;
}

bool ReproductionLibrary::addFileToPlaylist(const String &fileName, const String &playlistName) {
  String playlistPath = _getPlaylistPath(playlistName);
  if (_sdManager.exists(playlistPath)) {
    String playlistFile = playlistPath + "/playlist.txt";
    String fileEntry = fileName + "\n";
    _sdManager.writeString(playlistFile, fileEntry);
    return true;
  }
  return false;
}

bool ReproductionLibrary::removeFileFromPlaylist(const String &fileName, const String &playlistName) {
  
  String playlistPath = _getPlaylistPath(playlistName);
  if (_sdManager.exists(playlistPath)) {
    String playlistFile = playlistPath + "/playlist.txt";
    std::vector<String> lines;
    _sdManager.readLines(playlistFile, 0, -1, lines);

    auto it = std::find(lines.begin(), lines.end(), fileName);
    if (it != lines.end()) {
      lines.erase(it);
      _sdManager.remove(playlistFile);
      for (const String &line : lines) {
        _sdManager.writeString(playlistFile, line + "\n");
      }
      return true;
    }
  }
  return false;
}
/*
void ReproductionLibrary::_loadAllFiles() {
  String allFilesString = _sdManager.readString(_totalFileListName);
  Serial.println("Listado de archivos de memoria para load files");
  
  _allFiles.clear();
  size_t start = 0;
  size_t end = allFilesString.indexOf('\n');
  while (end != -1) {
    String Salida = allFilesString.substring(start, end);
    Serial.println(Salida);
    _allFiles.push_back(Salida);
    start = end + 1;
    end = allFilesString.indexOf('\n', start);
  }
}
*/

void ReproductionLibrary::_loadAllFiles() {
  String allFilesString = _sdManager.readString(_totalFileListName);
  Serial.println("Listado de archivos de memoria para load files");
  
  _allFiles.clear();
  size_t start = 0;
  size_t end = allFilesString.indexOf('\n');
  
  while (end != -1) {
    String line = allFilesString.substring(start, end);
    
    // Extraemos el nombre del archivo hasta la primera coma
    size_t commaPos = line.indexOf(',');
    if (commaPos != -1) {
      line = "/" + line.substring(0, commaPos); // Cortamos la cadena hasta la primera coma
    }
    
    Serial.println(line); // Imprime solo el nombre del archivo
    _allFiles.push_back(line); // Almacenamos el nombre del archivo
    
    start = end + 1;
    end = allFilesString.indexOf('\n', start);
  }
}

void ReproductionLibrary::_loadLikedFiles() {
  String likedFilesString = _sdManager.readString(_likedFileListName);
  _likedFiles.clear();
  size_t start = 0;
  size_t end = likedFilesString.indexOf('\n');
  while (end != -1) {
    _likedFiles.push_back(likedFilesString.substring(start, end));
    start = end + 1;
    end = likedFilesString.indexOf('\n', start);
  }
}

bool ReproductionLibrary::likeFile(const String &fileName) {
  if (std::find(_likedFiles.begin(), _likedFiles.end(), fileName) == _likedFiles.end()) {
    _likedFiles.push_back(fileName);
    String likedFilesString = _sdManager.readString(_basePath + "/" + _likedFileListName);
    likedFilesString += fileName + '\n';
    _sdManager.writeString(_basePath + "/" + _likedFileListName, likedFilesString);
    return true;
  }
  return false;
}

bool ReproductionLibrary::unlikeFile(const String &fileName) {
  auto it = std::find(_likedFiles.begin(), _likedFiles.end(), fileName);
  if (it != _likedFiles.end()) {
    _likedFiles.erase(it);
    String likedFilesString = "";
    for (const String &file : _likedFiles) {
      likedFilesString += file + '\n';
    }
    _sdManager.writeString(_basePath + "/" + _likedFileListName, likedFilesString);
    return true;
  }
  return false;
}

String ReproductionLibrary::getFileList() {

  String fileListContents = _sdManager.readString( _totalFileListName);
  String fileListJson = "{\"type\":\"allFilesPL\",\"payload\":[";

  size_t start = 0;
  size_t end = fileListContents.indexOf('\n');
  while (end != -1) {
    String line = fileListContents.substring(start, end);
    fileListJson += "\"" + line + "\"";
    
    start = end + 1;
    end = fileListContents.indexOf('\n', start);
    
    if (end != -1) {
      fileListJson += ",";
    }
  }

  fileListJson += "]}";
  return fileListJson;
}

String ReproductionLibrary::getPlaylistFileList(const String &playlistName) {
  String playlistPath = _getPlaylistPath(playlistName);
  if (_sdManager.exists(playlistPath)) {
    return _sdManager.readString(playlistPath);
  }
  return "";
}

String ReproductionLibrary::getLikedFileList() {

  String fileListContents = _sdManager.readString( _totalFileListName);
  String fileListJson = "{\"type\":\"allFilesPL\",\"payload\":[";

  size_t start = 0;
  size_t end = fileListContents.indexOf('\n');
  while (end != -1) {
    String line = fileListContents.substring(start, end);
    fileListJson += "\"" + line + "\"";
    
    start = end + 1;
    end = fileListContents.indexOf('\n', start);
    
    if (end != -1) {
      fileListJson += ",";
    }
  }

  fileListJson += "]}";
  return fileListJson;
}

FileProperties ReproductionLibrary::getFileProperties(const String &fileName) {
  FileProperties properties;
  String propertiesString = _sdManager.readString(_basePath + "/Files/" + fileName);
  properties.name = fileName;
  properties.date = propertiesString.substring(0, propertiesString.indexOf(','));
  properties.author = propertiesString.substring(propertiesString.indexOf(',') + 1, propertiesString.indexOf(',', propertiesString.indexOf(',') + 1));
  properties.length = propertiesString.substring(propertiesString.indexOf(',', propertiesString.indexOf(',') + 1) + 1, propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',') + 1) + 1)).toInt();
  properties.reversible = propertiesString.substring(propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',') + 1) + 1) + 1, propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',') + 1) + 1) + 1)) == "1";
  properties.origin = propertiesString.substring(propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',', propertiesString.indexOf(',') + 1) + 1) + 1) + 1) == "1";
  return properties;
}

bool ReproductionLibrary::parseLine(const String& line, float& num1, float& num2) {
  // Separar la línea en tokens
  int firstSpace = 0;
  int secondSpace = line.indexOf(' ', firstSpace + 1);

  if (firstSpace == -1 || secondSpace == -1) {
    // No se encontraron dos números
    return false;
  }

  // Obtener los dos números como strings
  String num1Str = line.substring(firstSpace, secondSpace);
  String num2Str = line.substring(secondSpace + 1, line.length());
  // Convertir a flotantes
  num1 = num1Str.toFloat();
  num2 = num2Str.toFloat();

  return true;
}



/*La version vieja 
bool ReproductionLibrary::HandleRepLib() {
  //Manejo de operaciones
  planner.tick();
  if(_PlayActive == false){
    if ( _sdManager.CheckIfMemoryIsUsed() == true) {
      _sdManager.CloseFileAlreadyOppened();
    }
    return false;
  }
  
  //Caso de nueva reproducción 
  if ( _NuevoArchivoAbierto == true) {
    _NuevoArchivoAbierto = false;
    if( _sdManager.CheckIfMemoryIsUsed() == true){  //Si ya estaba reproduciendo hay un archivo abierto
      _sdManager.CloseFileAlreadyOppened();         //Cieralo para arbrir el nuevo
    }
    _sdManager.OpenFile(_FilePlaying);  
    planner.start();            
    _ReadyToFeed = true;
  }
  
  //Maneja el control de pausa
  if ( _Pause == true ){
    planner.stop();
    _ReadyToFeed == false;
  }
  if ( _Pause == false ){
    planner.resume();
    _ReadyToFeed == true;
  }
  //Alimenta el planner con nuevas lineas de código
  if ( _ReadyToFeed == true) {  
    String line = "";
    if (planner.available()) {
      float num1, num2;
      if (_sdManager.MemoryAvailable() == true){
        line = _sdManager.readNextLine();  
        //Serial.print(line);
        if (line.charAt(0) == '#') {
        // Descartar la línea
          Serial.println("Línea descartada");
        } else {
          // Separar y convertir los números
          if (parseLine(line, num1, num2)) {
            // Guardar los números en el registro
            int32_t Array[1][2];
          
            Array[0][0] = (int32_t) (num1 * 1000);
            Array[0][1] = (int32_t) (num2 * 10000);
            Serial.print("Número 1: ");
            Serial.print(Array[0][0]);
            Serial.print(", Número 2: ");
            Serial.println(Array[0][1]);
            planner.addTarget(Array[0], 0);
          
          } else {
            Serial.println("Error al parsear la línea");
          }
        }
      }
      // Verificar si la línea comienza con #
      
    }
  }
  return true;
}*/

String ReproductionLibrary::getPlayerStatus() {
  String status = "{\n";

  // Archivo actual en reproducción
  status += "  \"archivoEnReproduccion\": \"" + (_FilePlaying != "" ? _FilePlaying : "Ninguno") + "\",\n";

  // Estado de reproducción
  status += "  \"estado\": \"" + String(_PlayActive ? (_Pause ? "Pausado" : "Reproduciendo") : "Detenido") + "\",\n";

  // Modo de reproducción (bucle, random)
  status += "  \"modoRandom\": " + String(_randomEnabled ? "true" : "false") + ",\n";
  status += "  \"modoBucle\": " + String(_bucleEnabled ? "true" : "false") + ",\n";

  // Velocidad de reproducción
  status += "  \"velocidad\": " + String(SpeedMotors) + ",\n";

  // Progreso (ejemplo, puedes agregar lógica adicional para calcular el progreso)
  // status += "  \"progreso\": " + String(actualProgress) + ",\n"; // Descomentar si tienes lógica de progreso

  status += "}";

  return status;
}


bool ReproductionLibrary::HandleRepLib() {
  // Tick del planner para avanzar su ejecución
  planner.tick();

  // Si la reproducción está desactivada, cerramos cualquier archivo abierto y detenemos el planner
  if (_PlayActive == false) {
    if (_sdManager.CheckIfMemoryIsUsed() == true) {
      _sdManager.CloseFileAlreadyOppened();
      planner.stop(); // Detenemos el planner solo si no hay reproducción activa
    }
    return false;
  }
  
  // Manejo de nueva reproducción
  if (_NuevoArchivoAbierto == true) {
    _NuevoArchivoAbierto = false;
    if (_sdManager.CheckIfMemoryIsUsed() == true) {  // Si había un archivo abierto
      _sdManager.CloseFileAlreadyOppened();          // Cerramos el archivo actual
      planner.stop();                                // Detenemos el planner y limpiamos los buffers solo en cambio de archivo
    }
    _FilePlaying = _filePath + _FilePlaying;
    if (_sdManager.OpenFile(_FilePlaying)) {
      Serial.println("Se abrió");
    }
    else {
      Serial.println("Archivo no existe");
    }
    //Serial.println("Abriendo archivo:");
    //Serial.println(_FilePlaying);
    planner.start();                    // Iniciamos el planner para la nueva reproducción
    _ReadyToFeed = true;                // Permitimos que se alimenten nuevas líneas
  }
  // Checar si el planner terminó su tarea
  if (planner.getStatus() == 0) {  
    // Cuando el planner termina, cerramos el archivo si está abierto
    if (_sdManager.CheckIfMemoryIsUsed() == true) {
      _sdManager.CloseFileAlreadyOppened();  // Cerramos el archivo
    }
    planner.stop(); // Detenemos el planner solo si ya terminó la reproducción
    // Aquí verificamos si hay más archivos en la lista de reproducción
    if (_PlayActive) {
      _FilePlaying = getNextFile();  // Obtener el siguiente archivo (considerando bucle, random, etc.)
      if (_FilePlaying != "") {      // Si hay un siguiente archivo, lo abrimos
        _NuevoArchivoAbierto = true;
      } else {
        _PlayActive = false; // Si no hay más archivos, detenemos la reproducción
      }
    }
    return true;
  }
  // Maneja el control de pausa
  if (_Pause == true) {
    planner.stop();
    _ReadyToFeed = false;
  } else {
    planner.resume();
    _ReadyToFeed = true;
  }

  // Manejo de "Next" y "Previous"
  if (_NextFileTriggered) {  // Si se activó la señal de siguiente archivo
    _NextFileTriggered = false;
    planner.stop();  // Detenemos el planner y limpiamos los buffers antes de cambiar de archivo
    if (_sdManager.CheckIfMemoryIsUsed() == true) {
      _sdManager.CloseFileAlreadyOppened();
    }
    _FilePlaying = getNextFile();  // Obtener el siguiente archivo
    if (_FilePlaying != "") {
      _NuevoArchivoAbierto = true;  // Indicamos que hay un nuevo archivo para reproducir
    }
  }

  if (_PreviousFileTriggered) {  // Si se activó la señal de archivo anterior
    _PreviousFileTriggered = false;
    planner.stop();  // Detenemos el planner y limpiamos los buffers antes de cambiar de archivo
    if (_sdManager.CheckIfMemoryIsUsed() == true) {
      _sdManager.CloseFileAlreadyOppened();
    }
    _FilePlaying = getPreviousFile();  // Obtener el archivo anterior
    if (_FilePlaying != "") {
      _NuevoArchivoAbierto = true;  // Indicamos que hay un nuevo archivo para reproducir
    }
  }

  // Alimenta el planner con nuevas líneas de código
  if (_ReadyToFeed == true) {
    String line = "";
    if (planner.available()) {
      float num1, num2;
      if (_sdManager.MemoryAvailable() == true) {
        line = _sdManager.readNextLine();  
        
        // Verificar si llegamos al final del archivo (palabra "Fin")
        line.trim();  // Elimina espacios y saltos de línea
        
        //Serial.print(line);
        //Serial.print("  ");

        if (line.charAt(0) == '#') {
          // Descartar la línea
          //Serial.println("Línea descartada");
        } else {
          // Separar y convertir los números
          if (parseLine(line, num1, num2)) {
            // Guardar los números en el registro
            int32_t Array[1][2];
            Array[0][0] = (int32_t) (num1 * 1000);
            Array[0][1] = (int32_t) (num2 * 10000);

            //Serial.print("Número 1: ");
            //Serial.print(Array[0][0]);
            //Serial.print(", Número 2: ");
            //Serial.println(Array[0][1]);
            planner.addTarget(Array[0], 0);
          } else {
            Serial.println("Error al parsear la línea");
          }
        }
      }
      else {  //Se acabó el archivo
        //Serial.println("Archivo terminado.");
          if (_sdManager.CheckIfMemoryIsUsed()) {
            _sdManager.CloseFileAlreadyOppened();  // Cerramos el archivo
          }
          planner.stop();
          // Pasar al siguiente archivo si está disponible
          _FilePlaying = getNextFile();
          if (_FilePlaying != "") {
            _NuevoArchivoAbierto = true;
          } else {
            _PlayActive = false;  // Si no hay más archivos, detener reproducción
          }
          return true;
      }
    }
  }
  
  return true;
}


bool ReproductionLibrary::SetSpeedRM ( int Speed){

  float Velocidad = Speed * 50;
  if( Velocidad > 5000 ){
    Velocidad = 5000;
  }
  if(Velocidad < 100){
    Velocidad = 100;
  }
  planner.stop();
  planner.setSpeed(0, Velocidad);      //Max 5000 
  planner.setSpeed(1, Velocidad);
  planner.start();
  return true;
}


bool ReproductionLibrary::play(const String &fileName) {
  _FilePlaying = _filePath + fileName;
  _NuevoArchivoAbierto = true;
  _PlayActive = true;
  // Not implemented
  // Implement the actual playback logic using the hardware and the method for playback
  return false;
}

bool ReproductionLibrary::pause() {
  // Not implemented
  // Implement the actual pause logic using the hardware and the method for playback
  return false;
}

bool ReproductionLibrary::forward() {
 // Not implemented
  // Implement the actual forward logic using the hardware and the method for playback
  return false;
}

bool ReproductionLibrary::backward() {
  // Not implemented
  // Implement the actual backward logic using the hardware and the method for playback
  return false;
}

bool ReproductionLibrary::setRandom(bool enabled) {
  _randomEnabled = enabled;
  return true;
}

bool ReproductionLibrary::setBucle(bool enabled) {
  _bucleEnabled = enabled;
  return true;
}
/*
String ReproductionLibrary::getCurrentFile() {
  // Not implemented
  // Return the current file being played or an empty string if no file is being played
  return "";
}

String ReproductionLibrary::getNextFile() {
  // Not implemented
  // Return the next file to be played based on the current playback order (random or sequential)
  return "";
}

String ReproductionLibrary::getPreviousFile() {
  // Not implemented
  // Return the previous file that was played based on the current playback order (random or sequential)
  return "";
}*/
String ReproductionLibrary::getCurrentFile() {
  // Devuelve el archivo que está siendo reproducido actualmente
  return _FilePlaying;
}

String ReproductionLibrary::getNextFile() {
  if (_allFiles.empty()) {
    //Serial.println("No hay archivos disponibles");
    return ""; // No hay archivos disponibles
  }
  /*
  Serial.println("---- Depuración de getNextFile() ----");
  Serial.print("Archivo actual reproduciéndose (_FilePlaying): ");
  Serial.println(_FilePlaying);
  
  // Imprimir todos los archivos para depuración
  Serial.println("Archivos disponibles en _allFiles:");
  for (const auto& file : _allFiles) {
    Serial.println(file);
  }
  */
  // Eliminar el prefijo "/Files" de _FilePlaying para la búsqueda
  String currentFileWithoutPrefix = _FilePlaying;
  if (currentFileWithoutPrefix.startsWith("/Files")) {
    currentFileWithoutPrefix.remove(0, strlen("/Files"));
  }

  // Encontrar el índice del archivo actual en la lista
  auto it = std::find_if(_allFiles.begin(), _allFiles.end(), 
    [currentFileWithoutPrefix](const String& file) {
      return file.equalsIgnoreCase(currentFileWithoutPrefix);  // Comparación sin sensibilidad a mayúsculas
    });
  /*
  Serial.print("Iterador encontrado (posición): ");
  Serial.println(std::distance(_allFiles.begin(), it));
  */
  // Si no se encuentra el archivo actual en la lista, reproducimos el primero
  if (it == _allFiles.end()) {
    //Serial.println("Archivo actual no encontrado en la lista, reproduciendo el primero.");
    _FilePlaying = _allFiles.front();
    return _FilePlaying;
  }

  // Obtener el índice del archivo actual en la lista
  size_t currentIndex = std::distance(_allFiles.begin(), it);
  //Serial.print("Índice del archivo actual: ");
  //Serial.println(currentIndex);

  // Obtener el siguiente archivo
  if (currentIndex + 1 < _allFiles.size()) {
    _FilePlaying = _allFiles[currentIndex + 1];   // Nombre del siguiente archivo
    //Serial.print("Reproduciendo siguiente archivo: ");
    //Serial.println(_FilePlaying);
  } else if (_bucle) {
    _FilePlaying = _allFiles.front();             // Volver al primero si el bucle está activado
    //Serial.println("Se ha alcanzado el final de la lista, volviendo al primer archivo por bucle.");
  } else {
    _FilePlaying = ""; // No más archivos y no está en bucle
    //Serial.println("No hay más archivos para reproducir y el bucle está desactivado.");
  }

  //Serial.println("---- Fin de la depuración ----");
  return _FilePlaying;
}

String ReproductionLibrary::getPreviousFile() {
  if (_allFiles.empty()) {
    return ""; // No hay archivos disponibles
  }

  // Encontrar el índice del archivo actual en la lista
  auto it = std::find(_allFiles.begin(), _allFiles.end(), _FilePlaying);

  // Si no se encuentra el archivo actual en la lista, reproducimos el último
  if (it == _allFiles.end()) {
    _FilePlaying = _allFiles.back();
    return _FilePlaying;
  }

  // Obtener el archivo anterior, si estamos en el primero, volver al último si bucle está activado
  size_t currentIndex = std::distance(_allFiles.begin(), it);
  if (currentIndex > 0) {
    _FilePlaying = _allFiles[currentIndex - 1];
  } else if (_bucle) {
    _FilePlaying = _allFiles.back();
  } else {
    _FilePlaying = ""; // No más archivos hacia atrás y no está en bucle
  }

  return _FilePlaying;
}

String ReproductionLibrary::_getPlaylistPath(const String &playlistName) {
  // Implement the function that returns the path of the playlist based on the playlistName
  // For example:
  return _basePath + "/" + playlistName;
}

String ReproductionLibrary::listPlaylists() {
  String playlists = _sdManager.listFiles(_basePath + "/Playlists", "\n");
  return playlists;
}
void ReproductionLibrary::checkAndFixFileList() {
  // List files in the /Files folder
  String fileListString = _sdManager.listFiles(_filePath);

  // Print fileListString for debugging
  Serial.println("File names in the filepath:");
  Serial.println(fileListString);

  // Split the file list into a vector of file names
  std::vector<String> fileList;
  size_t start = 0;
  size_t end = fileListString.indexOf('\n');
  while (end != -1) {
    fileList.push_back(fileListString.substring(start, end));
    start = end + 1;
    end = fileListString.indexOf('\n', start);
  }

  // Read the content of the totalFileList
  String totalFileListContent = _sdManager.readString(_basePath + "/" + _totalFileListName);

  // Print totalFileListContent for debugging
  Serial.println("File names in the totalFileListName:");
  Serial.println(totalFileListContent);

  // Check each file in the fileList
  for (const String &file : fileList) {
    // Check if the file exists in the totalFileList
    bool fileInList = _isFileInTotalFileList(file, totalFileListContent);

    // If the file is not in the totalFileList, add it with default properties
    if (!fileInList) {
      FileProperties defaultProperties(file, "unknown", "unknown", 0, false, false);
      _addFileToTotalFileList(defaultProperties);
    }
  }
}
bool ReproductionLibrary::_isFileInTotalFileList(const String &filename, const String &totalFileListContent) {
  // Split the totalFileListContent into a vector of file lines
  std::vector<String> fileListLines;
  size_t start = 0;
  size_t end = totalFileListContent.indexOf('\n');
  while (end != -1) {
    fileListLines.push_back(totalFileListContent.substring(start, end));
    start = end + 1;
    end = totalFileListContent.indexOf('\n', start);
  }

  // Check if the file exists in the totalFileList
  for (const String &fileLine : fileListLines) {
    int commaIndex = fileLine.indexOf(',');
    String currentFileName = fileLine.substring(0, commaIndex);
    if (currentFileName == filename) {
      return true;
    }
  }
  return false;
}

bool ReproductionLibrary::_addFileToTotalFileList(const FileProperties &fileProperties) {
  String fileLine = fileProperties.toString();
  String currentContent = _sdManager.readString(_basePath + "/" + _totalFileListName);
  currentContent += fileLine + '\n';
  _sdManager.writeString(_basePath + "/" + _totalFileListName, currentContent);
  return true;
}

bool ReproductionLibrary::_removeFileFromTotalFileList(const String &fileName) {
  String currentContent = _sdManager.readString(_basePath + "/" + _totalFileListName);
  int fileStart = currentContent.indexOf(fileName);
  if (fileStart == -1) {
    return false;
  }
  int fileEnd = currentContent.indexOf('\n', fileStart);
  String newContent = currentContent.substring(0, fileStart) + currentContent.substring(fileEnd + 1);
  _sdManager.writeString(_basePath + "/" + _totalFileListName, newContent);
  return true;
}

std::vector<String> ReproductionLibrary::_parseTotalFileList(const String &content) {
  std::vector<String> lines;
  size_t startPos = 0;
  size_t endPos = content.indexOf('\n');

  while (endPos != -1) {
    lines.push_back(content.substring(startPos, endPos));
    startPos = endPos + 1;
    endPos = content.indexOf('\n', startPos);
  }

  return lines;
}

