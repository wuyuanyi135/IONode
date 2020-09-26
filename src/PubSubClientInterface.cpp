//
// Created by wuyua on 9/25/2020.
//

#include "PubSubClientInterface.h"
#include "sstream"
#include <utility>

PubSubClientInterface::PubSubClientInterface(PubSubClient &client, std::string prefix, bool prefixWithID)
        : _client(client), _prefix(std::move(prefix)), _prefixWithID(prefixWithID) {
    _client.setCallback([&](char *topic, uint8_t *payload, unsigned int length) {
        auto topicStr = std::string(topic);
        if (_cbMap.find(topicStr) == _cbMap.end()) {
            // Not found, this topic does not belong to me.
            return;
        }
        auto &cb = _cbMap[topicStr];
        std::string arg(payload, payload + length);
        std::vector<std::string> args({arg});
        cb(args);
    });
}

int PubSubClientInterface::push_message(const std::string &name, const std::string &label, const std::string &message) {
    if (_client.connected()) {
        if (label == "ERROR") {
            return _client.publish(get_topic(name + "_err").c_str(), message.c_str());
        } else {
            return _client.publish(get_topic(name).c_str(), message.c_str());
        }

    }
    return false;
}

int PubSubClientInterface::subscribe(std::string identifier, IOInterfaceCallback cb) {
    auto topic = get_topic(identifier);
    _cbMap[topic] = cb;
    return _client.subscribe(topic.c_str());
}

void PubSubClientInterface::unsubscribe(std::string identifier) {
    _client.unsubscribe(get_topic(identifier).c_str());
}

std::string PubSubClientInterface::get_topic(std::string topic) {
    if (!_topicCached) {
        if (_prefixWithID) {
            std::stringstream convert;
            convert << ESP.getChipId();
            _prefix = convert.str() + "/";
        }
        _topicCached = true;
    }
    return _prefix + topic;
}


