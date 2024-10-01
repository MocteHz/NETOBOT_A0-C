
#include "CommandManager.h"

CommandManager::CommandManager(WiFiManager &wifiManager, MemoryManager &memoryManager, ReproductionLibrary &reproductionLibrary) :
  _wifiManager(wifiManager), _memoryManager(memoryManager), _reproductionLibrary(reproductionLibrary) {
  // Constructor body (if needed)
  rgbOut = 0;

}

String CommandManager::processCommand(const String &command) {
  String response = "";
  // AddWifi
  if ( command.indexOf("AddWifi") >= 0) {
        response = _wifiManager.getJsonString(); 
      }
  if (command.indexOf("addWiFiConfiguration") >= 0) {
    // Extract and process necessary parameters from the command
    String ssid = ""; // Replace with the actual SSID
    String password = ""; // Replace with the actual password
    _wifiManager.addWiFiConfiguration(ssid, password);
  }
  // EraseWifiConfiguration
  else if (command.indexOf("EraseWifiConfiguration") >= 0) {
    String ssid = ""; // Extract the SSID from the command
    _wifiManager.removeWiFiConfiguration(ssid);
  }
  // connecttoWifi
  else if (command.indexOf("ConnectTo") >= 0) {

    _wifiManager.loadJsonString(command);
    _wifiManager.InitWifiConnections();
    response = _wifiManager.getJsonString();
  }
  // BuscaSSID
  else if (command.indexOf("BuscaSSID") >= 0) {
    response = _wifiManager.BuscaSSID();
  }
  // GetJsonString
  else if (command.indexOf("GetJsonString") >= 0) {
    response = _wifiManager.getJsonString();
  }
  // GetNumberOfWifis
  else if (command.indexOf("GetNumberOfWifis") >= 0) {
    response = String(_wifiManager.getNumberOfWifis());
  }
  // LoadJsonFromFile
  else if (command.indexOf("LoadJsonFromFile") >= 0) {
    String filename = ""; // Extract the filename from the command
    //_wifiManager.loadJsonFromFile(filename);
  }
  // ConnectionStatus
  else if (command.indexOf("ConnectionStatus") >= 0) {
    response = _wifiManager.ConnectionStatus();
  }
  // ListFiles
  else if (command.indexOf("ListFiles") >= 0) {
    String directory = "/"; // Extract the directory from the command (if needed)
    response = _memoryManager.listFiles(directory);
  }
  // ListDir
  else if (command.indexOf("ListDir") >= 0) {
    String directory = "/"; // Extract the directory from the command (if needed)
    uint8_t levels = 1; // Extract the levels from the command (if needed)
    response = _memoryManager.listDir(directory.c_str(), levels);
  }
  // OpenFile
  else if (command.indexOf("OpenFile") >= 0) {
    String filename = ""; // Extract the filename from the command
    String mode = ""; // Extract the mode from the command (e.g., "r", "w", "a")
    _memoryManager.openFile(filename, mode);
  }
  // AppendString
  else if (command.indexOf("AppendString") >= 0) {
    String filename = ""; // Extract the filename from the command
    String content = ""; // Extract the content to append from the command
    _memoryManager.appendString(filename, content);
  }
  // ReadLine
  else if (command.indexOf("ReadLine") >= 0) {
    String filename = ""; // Extract the filename from the command
    size_t lineIndex = 0; // Extract the line index from the command
    response = _memoryManager.readLine(filename, lineIndex);
  }
  // ReadLines
  else if (command.indexOf("ReadLines") >= 0) {
    String filename = ""; // Extract the filename from the command
    size_t startLine = 0; // Extract the start line from the command
    size_t numLines = 0; // Extract the number of lines from the command
    std::vector<String> lines;
    _memoryManager.readLines(filename, startLine, numLines, lines);

    // Format the response with the lines read
    for (size_t i = 0; i < lines.size(); ++i) {
      response += lines[i];
      if (i < lines.size() - 1) {
        response += ",";
      }
    }
  }
  // CreatePlaylist
  else if (command.indexOf("CreatePlaylist") >= 0) {
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.createPlaylist(playlistName);
  }
  // Addfiletoplaylist
  else if (command.indexOf("Addfiletoplaylist") >= 0) {
    String fileName = ""; // Extract the file name from the command
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.addFileToPlaylist(fileName, playlistName);
  }
  // RemovefileFromPlaylist
  else if (command.indexOf("RemovefileFromPlaylist") >= 0) {
    String fileName = ""; // Extract the file name from the command
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.removeFileFromPlaylist(fileName, playlistName);
  }
  // LikeFile
  else if (command.indexOf("LikeFile") >= 0) {
    String fileName = ""; // Extract the file name from the command
    _reproductionLibrary.likeFile(fileName);
  }
  // unlikeFile
  else if (command.indexOf("unlikeFile") >= 0) {
    String fileName = ""; // Extract the file name from the command
    _reproductionLibrary.unlikeFile(fileName);
  }
  // GetFileList
  else if (command.indexOf("GetFileList") >= 0) {
    response = _reproductionLibrary.getFileList();
  }
  // GetPlaylistFileList
  else if (command.indexOf("GetPlaylistFileList") >= 0) {
    String playlistName = ""; // Extract the playlist name from the command
    response = _reproductionLibrary.getPlaylistFileList(playlistName);
  }
  // GetLikedFileList
  else if (command.indexOf("GetLikedFileList") >= 0) {
    response = _reproductionLibrary.getLikedFileList();
  }
  // GetFileProperties
  else if (command.indexOf("GetFileProperties") >= 0) {
    String fileName = ""; // Extract the file name from the command
    FileProperties fileProperties = _reproductionLibrary.getFileProperties(fileName);
    // Format the response with the file properties
    // Replace this with the desired format
    response = "Name: " + fileProperties.name + ", Date: " + fileProperties.date + ", Author: " + fileProperties.author + ", Length: " + String(fileProperties.length);
  }
  // Play
  else if (command.indexOf("Play") >= 0) {
    String fileName = ""; // Extract the file name from the command (if needed)
    _reproductionLibrary.play(fileName);
  }
  // Pause
  else if (command.indexOf("Pause") >= 0) {
    _reproductionLibrary.pause();
  }
  // forward
  else if (command.indexOf("forward") >= 0) {
    _reproductionLibrary.forward();
  }
  // Backward
  else if (command.indexOf("Backward") >= 0) {
    _reproductionLibrary.backward();
    String filename = "";
    size_t startLine = 0; // Extract the start line from the command
    size_t numLines = 0; // Extract the number of lines from the command
    std::vector<String> lines;
    _memoryManager.readLines(filename, startLine, numLines, lines);

    // Format the response with the lines read
    for (size_t i = 0; i < lines.size(); ++i) {
      response += lines[i];
      if (i < lines.size() - 1) {
        response += ",";
      }
    }
  }
  // CreatePlaylist
  else if (command.indexOf("CreatePlaylist") >= 0) {
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.createPlaylist(playlistName);
  }
  // Addfiletoplaylist
  else if (command.indexOf("Addfiletoplaylist") >= 0) {
    String fileName = ""; // Extract the file name from the command
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.addFileToPlaylist(fileName, playlistName);
  }
  // RemovefileFromPlaylist
  else if (command.indexOf("RemovefileFromPlaylist") >= 0) {
    String fileName = ""; // Extract the file name from the command
    String playlistName = ""; // Extract the playlist name from the command
    _reproductionLibrary.removeFileFromPlaylist(fileName, playlistName);
  }
  // LikeFile
  else if (command.indexOf("LikeFile") >= 0) {
    String fileName = ""; // Extract the file name from the command
    _reproductionLibrary.likeFile(fileName);
  }
  // unlikeFile
  else if (command.indexOf("unlikeFile") >= 0) {
    String fileName = ""; // Extract the file name from the command
    _reproductionLibrary.unlikeFile(fileName);
  }
  // GetFileList
  else if (command.indexOf("GetFileList") >= 0) {
    response = _reproductionLibrary.getFileList();
  }
  // GetPlaylistFileList
  else if (command.indexOf("GetPlaylistFileList") >= 0) {
    String playlistName = ""; // Extract the playlist name from the command
    response = _reproductionLibrary.getPlaylistFileList(playlistName);
  }
  // GetLikedFileList
  else if (command.indexOf("GetLikedFileList") >= 0) {
    response = _reproductionLibrary.getLikedFileList();
  }
  // GetFileProperties
  else if (command.indexOf("GetFileProperties") >= 0) {
    String fileName = ""; // Extract the file name from the command
    FileProperties fileProperties = _reproductionLibrary.getFileProperties(fileName);
    // Format the response with the file properties
    // Replace this with the desired format
    response = "Name: " + fileProperties.name + ", Date: " + fileProperties.date + ", Author: " + fileProperties.author + ", Length: " + String(fileProperties.length);
  }
  // Play
  else if (command.indexOf("Play") >= 0) {
    String fileName = ""; // Extract the file name from the command (if needed)
    _reproductionLibrary.play(fileName);
  }
  // Pause
  else if (command.indexOf("Pause") >= 0) {
    _reproductionLibrary.pause();
  }
  // forward
  else if (command.indexOf("forward") >= 0) {
    _reproductionLibrary.forward();
  }
  // Backward
  else if (command.indexOf("Backward") >= 0) {
    _reproductionLibrary.backward();
  }
  // SetRandom
  else if (command.indexOf("SetRandom") >= 0) {
    bool enabled = false; // Extract the enabled flag from the command
    _reproductionLibrary.setRandom(enabled);
  }
  // SetBucle
  else if (command.indexOf("SetBucle") >= 0) {
    bool enabled = false; // Extract the enabled flag from the command
    _reproductionLibrary.setBucle(enabled);
  }
  // Estado del reproductor
  else if(command.indexOf("RepStatus") >= 0) {
    response = _reproductionLibrary.getPlayerStatus();
  }
  // GetCurrentFileToPlay
  else if (command.indexOf("GetCurrentFileToPlay") >= 0) {
    response = _reproductionLibrary.getCurrentFile();
  }
  //Comienza la SD
  else if (command.indexOf("GetSDStatus") >= 0) {
    response = _memoryManager.statusCheck();
  }
  // GetNextFileToPlay
  else if (command.indexOf("GetNextFileToPlay") >= 0) {
    response = _reproductionLibrary.getNextFile();
  }
  // GetPreviousFileToPlay
  else if (command.indexOf("GetPreviousFileToPlay") >= 0) {
    response = _reproductionLibrary.getPreviousFile();
  }
    if ( command.indexOf("upload-request") >= 0){
    //It will depend on the request if it is a file or it is an Image
      int index = command.indexOf(':') + 2;
      String type = command.substring(index, command.indexOf('"', index));

      index = command.indexOf(':', index) + 2;
      String filename = command.substring(index, command.indexOf('"', index));

      index = command.indexOf(':', index) + 2;
      String date = command.substring(index, command.indexOf('"', index));

      index = command.indexOf(':', index) + 2;
      String author = command.substring(index, command.indexOf('"', index));

      index = command.indexOf(':', index) + 1;
      filesize = command.substring(index, command.indexOf(',', index)).toInt();

      index = command.indexOf(':', index) + 1;
      bool reversible = command.substring(index, command.indexOf(',', index)).toInt() == 1;

      index = command.indexOf(':', index) + 1;
      bool origin = command.substring(index, command.indexOf(',', index)).toInt() == 1;

      index = command.indexOf(':', index) + 2;
      String fileExtension = command.substring(index, command.indexOf('"', index));

      // Print the values to Serial for debugging
      Serial.print("type: ");
      Serial.println(type);
      Serial.print("filename: ");
      Serial.println(filename);
      Serial.print("filesize: ");
      Serial.println(filesize);
      Serial.print("date: ");
      Serial.println(date);
      Serial.print("author: ");
      Serial.println(author);
      Serial.print("reversible: ");
      Serial.println(reversible);
      Serial.print("origin: ");
      Serial.println(origin);
      Serial.print("filetype: ");
      Serial.println(fileExtension);

      FileProperties fileProperties;

      // Set the member variables
      fileProperties.name = filename;
      fileProperties.date = date;
      fileProperties.author = author;
      fileProperties.length = filesize;
      fileProperties.reversible = reversible;
      fileProperties.origin = origin;
      fileProperties.ext = fileExtension;
      
      Serial.println("Objeto adquirido: ");
      Serial.println(fileProperties.toString());
      _reproductionLibrary.uploadFileToBase(fileProperties);
      AcumuladoUpload = 0;
      
    }
    else if (command.indexOf("NameAllFilesPL") >= 0) {
       // response = "{\"type\":\"allFilesPL\",\"payload\":\"";
        response += _reproductionLibrary.getFileList();
        //response += "\"}";
    }
    else if (command.indexOf("NameAllLikedPL") >= 0) {
        response = _reproductionLibrary.getLikedFileList();
    }
    else if (command.indexOf("NameAllPlaylists") >= 0) {
        response = _reproductionLibrary.listPlaylists();
    }
    else if (command.indexOf("SetSpeed") >= 0) {
       // response = "SetSpeed";
        int index = command.indexOf(':') + 2;
        String type = command.substring(index, command.indexOf('"', index));
        int Speed = strtol(type.substring(0, 2).c_str(), nullptr, 16);
        _reproductionLibrary.SetSpeedRM(Speed);
    }
    else if (command.indexOf("SetBrillo") >= 0) {
        int index = command.indexOf(':') + 2;
        String type = command.substring(index, command.indexOf('"', index));
        Brillo= strtol(type.substring(0, 2).c_str(), nullptr, 16)*2;
    }
    else if (command.indexOf("ColorSelected") >= 0) {
       // response = command;
      int index = command.indexOf('#') + 1;
      String Color = command.substring(index, command.indexOf('"', index));
      response = Color;
      // Convertir los valores hexadecimales a enteros
      uint8_t r = strtol(Color.substring(0, 2).c_str(), nullptr, 16);
      uint8_t g = strtol(Color.substring(2, 4).c_str(), nullptr, 16);
      uint8_t b = strtol(Color.substring(4, 6).c_str(), nullptr, 16);
      rgbOut = (r << 16) | (g << 8) | b;
    }
  return response;
}

String CommandManager::UploadHandle(const String &data) {
    _reproductionLibrary.handleUploadFileToBase( data );

    AcumuladoUpload += data.length();
    String jsonString = "{\"type\":\"upload-progress\",\"loaded\":" + String(AcumuladoUpload) + ",\"total\":" + String(filesize) + "}";
    
    if (AcumuladoUpload == filesize){
        jsonString = "{\"type\":\"upload-complete\"}";
        AcumuladoUpload = 0;
        _memoryManager.closeFile();
    }
    return jsonString;
}