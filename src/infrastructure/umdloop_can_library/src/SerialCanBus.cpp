#include "umdloop_can_library/SerialCanBus.hpp"
#include <SerialCAN.h>
#include <iostream>
#include <cstring>

namespace CANLib {

namespace {
    void mapCanFrameToMessage(const CanFrame& frame, ::CANAPI_Message_t& msg) {
        std::memset(&msg, 0, sizeof(msg));
        msg.id = frame.id;
        msg.dlc = frame.dlc;
        msg.xtd = frame.is_extended ? 1 : 0;
        msg.rtr = frame.is_rtr ? 1 : 0;
        std::copy(frame.data.begin(), frame.data.begin() + frame.dlc, msg.data);
    }

    void mapMessageToCanFrame(const ::CANAPI_Message_t& msg, CanFrame& frame) {
        frame.id = msg.id;
        frame.dlc = std::min(msg.dlc, static_cast<uint8_t>(8));
        frame.is_extended = msg.xtd == 1;
        frame.is_rtr = msg.rtr == 1;
        frame.data.fill(0);
        std::copy(msg.data, msg.data + frame.dlc, frame.data.begin());
    }
}

SerialCanBus::SerialCanBus() : running_(false) {
    serialCanImpl_ = std::make_unique<CSerialCAN>();
}

SerialCanBus::~SerialCanBus() {
    close();
}

bool SerialCanBus::open(const std::string& interface_name, ReceiveCallback callback) {
    if (running_) return false;

    receiveCallback_ = callback;

    ::CANAPI_OpMode_t opMode = {};
    opMode.byte = CANMODE_DEFAULT;
    
    auto ret = serialCanImpl_->InitializeChannel(interface_name.c_str(), opMode);
    if (ret != CCanApi::NoError) {
        std::cerr << "Failed to open CAN interface: " << interface_name << std::endl;
        return false;
    }

    ::CANAPI_Bitrate_t bitrate = {};
    bitrate.index = CANBTR_INDEX_500K;
    
    ret = serialCanImpl_->StartController(bitrate);
    if (ret != CCanApi::NoError) {
        std::cerr << "Failed to start CAN controller" << std::endl;
        serialCanImpl_->TeardownChannel();
        return false;
    }

    running_ = true;
    receiveThread_ = std::thread(&SerialCanBus::receiveLoop_, this);
    return true;
}

void SerialCanBus::close() {
    if (running_) {
        running_ = false;
        if (receiveThread_.joinable()) {
            receiveThread_.join();
        }
        serialCanImpl_->TeardownChannel();
    }
}

bool SerialCanBus::send(const CanFrame& frame) {
    if (!running_) return false;

    ::CANAPI_Message_t msg;
    mapCanFrameToMessage(frame, msg);
    return serialCanImpl_->WriteMessage(msg) == CCanApi::NoError;
}

void SerialCanBus::receiveLoop_() {
    ::CANAPI_Message_t msg;
    while (running_) {
        auto ret = serialCanImpl_->ReadMessage(msg, 100);
        if (ret == CCanApi::NoError && receiveCallback_) {
            CanFrame frame;
            mapMessageToCanFrame(msg, frame);
            receiveCallback_(frame);
        } else if (ret != CCanApi::Timeout && ret != CCanApi::ReceiverEmpty) {
            if (ret == CCanApi::NotInitialized || ret == CCanApi::ControllerOffline) {
                break;
            }
        }
    }
}

} // namespace CANLib