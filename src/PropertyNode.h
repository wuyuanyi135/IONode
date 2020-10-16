//
// Created by wuyua on 9/25/2020.
//

#ifndef IOPROPERTYNODE_H
#define IOPROPERTYNODE_H

#include "IONode.h"
#include <sstream>

template<typename T>
class PropertyNode : public Node {
protected:
    T prop;
    bool readOnly{false};
    std::function<bool(T new_value)> validator;
    std::function<void(T, T)> updateCallback;
    std::function<bool()> getCallback;
    bool publishOnRegistered{false};

public:
    int register_interface(IOInterface &interface) override;

public:
    ///
    /// \param name
    /// \param prop
    /// \param readOnly
    /// \param publishOnRegistered announce the initial topic and value when registered.
    PropertyNode(const std::string &name, T prop, bool readOnly = false, bool publishOnRegistered = false)
            : Node(name), prop(prop), readOnly(readOnly), publishOnRegistered(publishOnRegistered) {}

    void set_validator(const std::function<bool(T)> &validator);

    void set_update_callback(const std::function<void(T, T)> &cb);

    ///
    /// \param cb should return true if the cached property should be sent to the
    /// interfaced. Otherwise, it will be suppressed, and it is user's duty to
    /// send the GET responses by using push_command
    void set_get_callback(const std::function<bool()> &cb);

    T &get_value();

    /// Set value from API interface. Do not trigger update.
    /// \param newValue
    void set_value(const T &newValue, bool triggerCallback = false);

    /// Notify async GET task done
    void notify_get_request_completed();

    void notify_get_request_completed(T &val);

    /// Return the underlying pointer of the props. Will not trigger any callback.
    /// \return
    T *data();

protected:
    void on_command(std::vector<std::string> &args) override;

    virtual bool on_validation(T &newValue);

    virtual void on_property_updated(T &oldValue, T &newValue);

    virtual bool on_property_get();

    virtual void on_get();

    virtual void on_set(std::string newValue);

    virtual void on_error(std::string error);

private:
    static std::string to_string(T val);

    static T from_string(std::string &str);
};

template<typename T>
void PropertyNode<T>::on_command(std::vector<std::string> &args) {
    unsigned int size = args.size();
    switch (size) {
        case 0:
            on_get();
            break;
        case 1:
            on_set(args[0]);
            break;
        default:
            on_error("Argument number should be 0 or 1\n");
            break;
    }
}

template<typename T>
T &PropertyNode<T>::get_value() {
    return prop;
}

template<typename T>
void PropertyNode<T>::set_value(const T &newValue, bool triggerCallback) {
    auto old = prop;
    prop = newValue;
    if (triggerCallback) {
        on_property_updated(old, prop);
    }
}

template<typename T>
void PropertyNode<T>::on_get() {
    if (on_property_get()) {
        push_message("GET", to_string(prop));
    }
}

template<typename T>
void PropertyNode<T>::on_set(std::string newValue) {
    if (readOnly) {
        // Read only Node should ignore the topic update to prevent publishing echo.
        return;
    }
    T v = from_string(newValue);
    if (on_validation(v)) {
        T oldValue = prop;
        prop = v;
        on_property_updated(oldValue, v);
    } else {
        on_error("Validation failed");
    }
}

template<typename T>
void PropertyNode<T>::on_error(std::string error) {
    push_message("ERROR", error);
}

template<typename T>
void PropertyNode<T>::on_property_updated(T &oldValue, T &newValue) {
    if (updateCallback) {
        updateCallback(oldValue, newValue);
    } else {
        push_message("SET", to_string(newValue));
    }
}

template<typename T>
bool PropertyNode<T>::on_validation(T &newValue) {
    if (validator) {
        return validator(newValue);
    } else {
        return true;
    }
}

template<typename T>
void PropertyNode<T>::set_update_callback(const std::function<void(T, T)> &cb) {
    updateCallback = cb;
}

template<typename T>
void PropertyNode<T>::set_validator(const std::function<bool(T)> &validator) {
    this->validator = validator;
}

template<typename T>
std::string PropertyNode<T>::to_string(T value) {
    std::stringstream convert;
    convert << value;
    return convert.str();
}

template<typename T>
T PropertyNode<T>::from_string(std::string &str) {
    std::stringstream convert(str);
    T value;
    convert >> value;
    return value;
}

template<typename T>
bool PropertyNode<T>::on_property_get() {
    if (getCallback) {
        return getCallback();
    } else {
        return prop;
    }
}

template<typename T>
void PropertyNode<T>::set_get_callback(const std::function<bool()> &cb) {
    getCallback = cb;
}

template<typename T>
T *PropertyNode<T>::data() {
    return &prop;
}

template<typename T>
void PropertyNode<T>::notify_get_request_completed(T &val) {
    push_message("GET", to_string(val));
}

template<typename T>
void PropertyNode<T>::notify_get_request_completed() {
    notify_get_request_completed(prop);
}

template<typename T>
int PropertyNode<T>::register_interface(IOInterface &interface) {
    if (publishOnRegistered) {
        interface.push_message(name, "GET", to_string(prop));
    }
    return Node::register_interface(interface);
}

#endif// IOPROPERTYNODE_H
