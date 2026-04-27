#pragma once

#include <string>
#include <functional>
#include "CanFrame.hpp"

namespace CANLib {

// Abstract interface for a CAN bus
class ICanBus {
public:
    using ReceiveCallback = std::function<void(const CanFrame&)>;

    virtual ~ICanBus() = default;

    // Initializes and opens the CAN bus
    virtual bool open(const std::string& interface_name, ReceiveCallback callback) = 0;

    // Closes the CAN bus
    virtual void close() = 0;

    // Sends a CAN frame
    virtual bool send(const CanFrame& frame) = 0;
};

} // namespace CANLib