#ifndef REPRODUCTION_LIBRARY_H
#define REPRODUCTION_LIBRARY_H

#include <Arduino.h>
#include "MemoryManager.h"
#include <vector>
#include <algorithm>
#include <ctime>

struct FileProperties {
  String name;
  String date;
  String author;
  uint32_t length;
  bool reversible;
  bool origin;
  String ext;

  FileProperties(
    const String &name, const String &date, const String &author,
    uint32_t length, bool reversible, bool origin)
    : name(name), date(date), author(author), length(length),
      reversible(reversible), origin(origin) {}
    FileProperties() = default;
  String toString() const {
    return name + "," + date + "," + author + "," + String(length) + "," + String(reversible) + "," + String(origin);
  }
};

class ReproductionLibrary {
 public:
  ReproductionLibrary(MemoryManager &sdManager);  //Necesitamos sdManager para leer/escribir la memoria SD

  bool begin();                                   //Inicia 
  bool HandleRepLib();
  String getPlayerStatus();
  void printAllFiles();

  bool Reproduce();
  //Manejo de playlist
  bool createPlaylist(const String &playlistName);
  bool addFileToPlaylist(const String &fileName, const String &playlistName);
  bool removeFileFromPlaylist(const String &fileName, const String &playlistName);

  //Manejo de archivos base
  bool addFileToRoot(const FileProperties &fileProperties);
  bool removeFileFromRoot(const String &fileName);
  bool clearRootFileList();

  //Manejo de Likes
  bool addFileToLiked(const String &fileName);
  bool removeFileFromLiked(const String &fileName);
  bool clearLikedFileList();
  bool likeFile(const String &fileName);
  bool unlikeFile(const String &fileName);

  //Manejo de Uploads
  bool uploadFileToBase(const FileProperties &fileProperties);    //Inicia el proceso de subida del archivo
  String handleUploadFileToBase(const String &Data);              //Continúa con el proceso de subida de archivo

  bool uploadImageToBase(const FileProperties &fileProperties);   //Iniica el proceso de subir una imagen
  String handleUploadImageToBase(const String &Data);             //Continúa con el proceso de subir las imagenes

  //Manejo de archivos y listas
  String getFileList();
  String getPlaylistFileList(const String &playlistName);
  String getLikedFileList();
  String listPlaylists();
  String getCurrentFile();
  String getNextFile();
  String getPreviousFile();

  //Manejo de operación
  FileProperties getFileProperties(const String &fileName);
  bool parseLine(const String& line, float& num1, float& num2);
  bool play(const String &fileName);
  bool pause();
  bool forward();
  bool backward();
  bool setRandom(bool enabled);
  bool setBucle(bool enabled);



  //Configuración de velocidad
  bool SetSpeedRM(int Speed);

  //Este no me acuerdo porqué lo hice
  void checkAndFixFileList();

 private:
  MemoryManager &_sdManager;
  String _currentFile;
  std::vector<String> _playlistFiles;
  std::vector<String> _likedFiles;
  std::vector<String> _allFiles;
  bool _random;
  bool _bucle;
  int SpeedMotors;

  String _basePath = "/";
  String _filePath = "/Files";
  String _imagePath = "/Images";
  String _totalFileListName = "/totalFileList.txt";
  String _likedFileListName = "/likedFileList.txt";
  String _FilePlaying = "";
  String _FileList = "";

  void _loadAllFiles();
  void _loadLikedFiles();
  void _saveLikedFiles();

  bool _randomEnabled;      //Habilitar Random
  bool _bucleEnabled;       //Habilitar Bucle
  bool _PlayActive;         //Estas corriendo?
  bool _Pause;              //Estado pausa
  bool _NuevoArchivoAbierto = true;//Archivo abierto
  bool _ReadyToFeed;        //No me acuerdo
  bool _NextFileTriggered;             // Bit que indica si se debe cambiar al siguiente archivo
  bool _PreviousFileTriggered;         // Bit que indica si se debe cambiar al archivo anterior

  String _getPlaylistPath(const String &playlistName);

  bool _isFileInTotalFileList(const String &fileName, const String &totalFileListContent);
  bool _addFileToTotalFileList(const FileProperties &fileProperties);
  bool _removeFileFromTotalFileList(const String &fileName);
  
  std::vector<String> _parseTotalFileList(const String &content); // Add this method declaration

};

#endif // REPRODUCTION_LIBRARY_H

