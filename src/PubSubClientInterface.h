//
// Created by wuyua on 9/25/2020.
//

#ifndef MQTTINTERFACE_H
#define MQTTINTERFACE_H

#include <unordered_map>
#include "IOInterface.h"
#include "PubSubClient.h"

class PubSubClientInterface : public IOInterface {
protected:
    PubSubClient &_client;
    std::string _prefix;
    bool _prefixWithID;
    bool _topicCached = false;
    std::unordered_map<std::string, IOInterfaceCallback> _cbMap;
protected:
    virtual std::string get_topic(std::string topic);

public:
    explicit PubSubClientInterface(PubSubClient &client, std::string prefix = "", bool prefixWithID = true);

    int push_message(const std::string &name, const std::string &label, const std::string &message) override;

    int subscribe(std::string identifier, IOInterfaceCallback cb) override;

    void unsubscribe(std::string identifier) override;
};


#endif //MQTTINTERFACE_H
