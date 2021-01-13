//
// Created by wuyua on 1/12/2021.
//
#include "OTA.h"
#include <ESP8266mDNS.h>
#include <sstream>

OTA::OTA(int port, String name) : port(port), httpServer(port), name(name) {
  if (name.isEmpty()) {
    std::stringstream ss;
    ss << ESP.getChipId();
    this->name = ss.str().c_str();
  }
}
void OTA::begin() {
  httpServer.onNotFound([&]() {
    httpServer.send(
        200,
        "text/html",
        "<!DOCTYPE html>"
        "<html><head><title>" + name + "</title></head><body>"
        "<h1>" + name + " Firmware Update</h1>"
        "<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">"
        "      Firmware:<br>"
            "<input type=\"file\" accept=\".bin,.bin.gz\" name=\"firmware\">"
            "<input type=\"submit\" value=\"Update Firmware\">"
        "</form>"
        "<form method=\"POST\" action=\"/update\" enctype=\"multipart/form-data\">"
        "      Filesystem:<br>"
        "<input type=\"file\" accept=\".bin,.bin.gz\" name=\"filesystem\">"
        "<input type=\"submit\" value=\"Update Filesystem\">"
        "</form>"
        "</body></html>"
        );
  });

  MDNS.begin((("update_" + name).c_str()));
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
