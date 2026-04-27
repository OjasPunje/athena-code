#pragma once

#include "ICanBus.hpp"
#include <memory>
#include <thread>
#include <atomic>
#include <CANAPI.h>
#include <CANAPI_Types.h>

class CSerialCAN;

namespace CANLib {

class SerialCanBus : public ICanBus {
public:
    SerialCanBus();
    ~SerialCanBus() override;

    bool open(const std::string& interface_name, ReceiveCallback callback) override;
    void close() override;
    bool send(const CanFrame& frame) override;

private:
    std::unique_ptr<CSerialCAN> serialCanImpl_;
    ReceiveCallback receiveCallback_;
    std::thread receiveThread_;
    std::atomic<bool> running_;

    void receiveLoop_();
};

}