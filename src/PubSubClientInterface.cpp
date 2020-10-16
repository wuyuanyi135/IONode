//
// Created by wuyua on 9/25/2020.
//

#include "PubSubClientInterface.h"
#include <utility>
#include "sstream"

PubSubClientInterface::PubSubClientInterface(PubSubClient &client,
                                             std::string prefix,
                                             bool prefixWithID,
                                             bool retained)
        : _client(client),
          _prefix(std::move(prefix)),
          _prefixWithID(prefixWithID),
          _retained(retained) {
    _client.setCallback([&](char *topic, uint8_t *payload, unsigned int length) {
        auto topicStr = std::string(topic);
        auto found = _cbMap.find(topicStr);
        if (found == _cbMap.end()) {
            // Not found, this topic does not belong to me.
            return;
        }

        auto foundPublishAt = publishedAt.find(topicStr);
        if (foundPublishAt == publishedAt.end()) {
            // Not published by me. Not possible to echo
        } else {
            if (sys_now() - foundPublishAt->second < SAME_TOPIC_ECHO_DEBOUNCE_TIME) {
                // I published and received immediately, just ignore to prevent echo!
                return;
            }
        }
        auto &cb = found->second;

        if (length == 0) {
            // Read instruction
            std::vector<std::string> args({});
            cb(args);
        } else {
            // Write instruction
            std::string arg(payload, payload + length);
            std::vector<std::string> args({arg});
            cb(args);
        }
    });
}

int PubSubClientInterface::push_message(const std::string &name,
                                        const std::string &label,
                                        const std::string &message) {
    if (_client.connected()) {
        std::string t;
        if (label == "ERROR") {
            t = get_topic(name + "_err");
            publishedAt[t] = sys_now();
            return _client.publish(t.c_str(), message.c_str());
        } else {
            t = get_topic(name);
            publishedAt[t] = sys_now();
            return _client.publish(t.c_str(), message.c_str());
        }
    }
    return false;
}

int PubSubClientInterface::subscribe(std::string identifier,
                                     IOInterfaceCallback cb) {
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
