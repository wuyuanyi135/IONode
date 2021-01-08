//
// Created by wuyua on 10/13/2020.
//

#include "ESP8266Init.h"


ESP8266Init::ESP8266Init(
        const char *ssid,
        const char *password,
        const char *mqttServer,
        int mqttPort,
        const char *description,
        int wifiIntervalMS,
        int mqttIntervalMS,
        int wifiTimeoutMS,
        int mqttTimeoutMS) : ssid(ssid),
                             password(password),
                             mqttServer(mqttServer),
                             mqttPort(mqttPort),
                             description(description),
                             wifiIntervalMS(wifiIntervalMS),
                             mqttIntervalMS(mqttIntervalMS),
                             wifiTimeoutMS(wifiTimeoutMS),
                             mqttTimeoutMS(mqttTimeoutMS) {
    std::stringstream convert;
    convert << ESP.getChipId();
    espID = convert.str();
}

ESP8266Init::State ESP8266Init::async_init() {
    ulong now;
    switch (state) {
        case INIT:
            WiFi.persistent(false);
            WiFi.mode(WIFI_STA);
            WiFi.begin(ssid, password);
            state = WIFI_CONNECTING;

            starting_time = millis();
            Serial.print("Starting connecting WiFi: SSID=");
            Serial.println(ssid);
            break;

        case WIFI_CONNECTING:
            now = millis();
            if (now - last_time >= wifiIntervalMS) {
                if (!WiFi.isConnected()) {
                    if (now - starting_time >= wifiTimeoutMS) {
                        Serial.println("Failed to connect WiFi: timeout");
                        state = WIFI_FAILED;
                    } else {
                        Serial.println("Connecting WiFi");
                        last_time = now;
                    }
                } else {
                    Serial.println("Connected to WiFi");
                    state = WIFI_CONNECTED;
                }
            }
            break;

        case WIFI_CONNECTED:
            client.setServer(mqttServer, mqttPort);

            client.connect(espID.c_str(), (espID + "/status").c_str(), 0, true, "off");
            last_time = starting_time = millis();
            state = MQTT_CONNECTING;
            break;

        case MQTT_CONNECTING:
            now = millis();
            if (now - last_time >= mqttIntervalMS) {
                if (!client.connected()) {
                    if (now - starting_time >= mqttTimeoutMS) {
                        Serial.println("Failed to connect MQTT: timeout");
                        state = MQTT_FAILED;
                    } else {
                        Serial.println("Connecting MQTT");
                        last_time = now;
                    }
                } else {
                    Serial.println("Connected to MQTT");
                    state = MQTT_CONNECTED_;
                }
            }

            break;

        case MQTT_CONNECTED_:
            client.publish((espID + "/desc").c_str(), description, true);
            client.publish((espID + "/status").c_str(), "on", true);
            client.publish((espID + "/ip").c_str(), WiFi.localIP().toString().c_str(), true);
            state = FINISHED;
            break;

        case FINISHED:
            state = OVERRUN;
            break;

        case OVERRUN:
        case WIFI_FAILED:
        case MQTT_FAILED:
        default:
            return state;
    }
    return state;
}
ESP8266Init::State ESP8266Init::blocking_init() {
    State ret;
    while (true) {
        yield(); // Yield to WiFi related process
        ret = async_init();
        switch (ret) {
            case INIT:
            case WIFI_CONNECTING:
            case WIFI_CONNECTED:
            case MQTT_CONNECTING:
            case MQTT_CONNECTED_:
                continue;
            case FINISHED:
            case OVERRUN:
            case WIFI_FAILED:
            case MQTT_FAILED:
                return ret;
        }
    }
}
