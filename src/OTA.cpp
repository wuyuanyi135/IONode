//
// Created by wuyua on 1/12/2021.
//
#include "OTA.h"
#include <ESP8266mDNS.h>
#include <sstream>

OTA::OTA(int port) : port(port), httpServer(port) {}
void OTA::begin() {
  std::stringstream ss;
  ss << ESP.getChipId();

  MDNS.begin((("update_" + ss.str()).c_str()));
  httpUpdater.setup(&httpServer);
  httpServer.begin();
  MDNS.addService("http", "tcp", port);
  initialized = true;
}
void OTA::loop() {
  if (!initialized) {
    return;
  }
  httpServer.handleClient();
  MDNS.update();
}
