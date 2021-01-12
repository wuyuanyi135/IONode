//
// Created by wuyua on 1/12/2021.
//

#ifndef IONODE_OTA_H
#define IONODE_OTA_H

#include <ESP8266HTTPUpdateServer.h>
#include <ESP8266WebServer.h>
class OTA {
public:
  explicit OTA(int port = 80);
  void begin();
  void loop();
private:
  bool initialized{false};
  int port;
  ESP8266WebServer httpServer;
  ESP8266HTTPUpdateServer httpUpdater;
};
#endif // IONODE_OTA_H
