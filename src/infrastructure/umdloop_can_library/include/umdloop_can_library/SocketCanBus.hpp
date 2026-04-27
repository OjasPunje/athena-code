#pragma once

#include "ICanBus.hpp"
#include <memory>
#include <thread>
#include <atomic>

namespace CANLib {

class SocketCanBus : public ICanBus {
public:
    SocketCanBus();
    ~SocketCanBus() override;

    bool open(const std::string& interface_name, ReceiveCallback callback) override;
    void close() override;
    bool send(const CanFrame& frame) override;

private:
    int socketFd_;
    ReceiveCallback receiveCallback_;
    std::thread receiveThread_;
    std::atomic<bool> running_;
    std::string interfaceName_;

    void receiveLoop_();
};

} // namespace CANLib