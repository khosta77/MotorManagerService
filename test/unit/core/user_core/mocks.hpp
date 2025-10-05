#pragma once

#include "user_core.hpp"
#include "i_module.hpp"
#include "i_socket.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <memory>

using ::testing::Return;
using ::testing::NiceMock;
using ::testing::Invoke;
using ::testing::_;

class MockModule : public IModule
{
public:
    MOCK_METHOD(bool, connect, (const int), (override));
    MOCK_METHOD(void, disconnect, (), (override));
    MOCK_METHOD(bool, isConnected, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, listComs, (), (const, override));

    MOCK_METHOD(void, setBaudRate, (const int), (override));
    MOCK_METHOD(int, getBaudRate, (), (override));
    MOCK_METHOD(void, setUSBParameters, (const int, const int), (override));
    MOCK_METHOD(void, setCharacteristics, (const uchar, const uchar, const uchar), (override));
    MOCK_METHOD(void, waitWriteSuccess, (), (override));
    MOCK_METHOD(size_t, checkRXChannel, (), (const, override));
    MOCK_METHOD(void, writeData, (const std::vector<uchar>& data), (override));
    MOCK_METHOD(void, readData, (std::vector<uchar> & data), (override));
    MOCK_METHOD(std::vector<uchar>, read, (const size_t timeout), (override));

    explicit operator bool() const override
    {
        return isConnected();
    }
};

class MockSocket : public ISocket
{
public:
    MOCK_METHOD(size_t, write, (int fd, const void* buf, size_t count), (override));
    MOCK_METHOD(size_t, read, (int fd, void* buf, size_t count), (override));
};

struct TestRig
{
    NiceMock<MockModule>* module;
    NiceMock<MockSocket>* socket;
    std::unique_ptr<UserCore> core;
    std::shared_ptr<std::string> lastWrite;
};

inline TestRig makeRig()
{
    TestRig rig{};
    std::unique_ptr<IModule> modulePtr{rig.module = new NiceMock<MockModule>()};
    std::unique_ptr<ISocket> socketPtr{rig.socket = new NiceMock<MockSocket>()};

    rig.lastWrite = std::make_shared<std::string>();
    ON_CALL(*rig.socket, write(_, _, _))
        .WillByDefault(Invoke([w = rig.lastWrite](int, const void* buf, size_t count) {
            w->assign(static_cast<const char*>(buf), count);
            return count;
        }));

    ON_CALL(*rig.module, isConnected()).WillByDefault(Return(true));

    rig.core = std::make_unique<UserCore>(std::move(modulePtr), std::move(socketPtr));
    return rig;
}

