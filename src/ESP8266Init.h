//
// Created by wuyua on 10/13/2020.
//

#ifndef IONODE_ESP8266INIT_H
#define IONODE_ESP8266INIT_H

#include "Arduino.h"
#include "ESP8266WiFi.h"
#include <PubSubClient.h>
#include <sstream>


class ESP8266Init {
public:
    enum State {
        INIT,
        WIFI_CONNECTING,
        WIFI_CONNECTED,
        MQTT_CONNECTING,
        MQTT_CONNECTED_,
        FINISHED,
        OVERRUN,
        WIFI_FAILED = -1,
        MQTT_FAILED = -2
    };

protected:
    const char *ssid;
    const char *password;
    const char *mqttServer;
    int mqttPort;
    const char *description;
    ulong wifiIntervalMS;
    ulong mqttIntervalMS;
    ulong wifiTimeoutMS;
    ulong mqttTimeoutMS;
    std::string espID;

public:
    ESP8266Init(
            const char *ssid,
            const char *password,
            const char *mqttServer,
            int mqttPort,
            const char *description,
            // Optional arguments,
            int wifiIntervalMS = 1000,
            int mqttIntervalMS = 1000,
            int wifiTimeoutMS = 30000,
            int mqttTimeoutMS = 30000);

    State async_init();

    State blocking_init();

public:
    WiFiClient wifiClient{};
    PubSubClient client{wifiClient};

private:
    State state{INIT};
    ulong last_time{0};
    ulong starting_time{0};
};

void wifi_init(
        const char *ssid,
        const char *password,
        const char *mqttServer,
        int mqttPort,
        const char *description);

#endif//IONODE_ESP8266INIT_H
