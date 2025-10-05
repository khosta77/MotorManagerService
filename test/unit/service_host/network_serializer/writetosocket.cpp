#include "network_serializer.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

class SocketMock : public ISocket
{
public:
    MOCK_METHOD(size_t, write, (int fd, const void* buf, size_t count), (override));
    MOCK_METHOD(size_t, read, (int fd, void* buf, size_t count), (override));
};

// Тест на отправку
TEST(WriteToSockTest, Success)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    EXPECT_CALL(*mockPtr, write(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(10))
        .WillOnce(testing::Return(5));

    NetworkSerializer server(std::move(mockSocket));

    EXPECT_NO_THROW(server.writeToSock(1, "123456789"));
}

// Сообщение уже содержит \n\n - должно бросить исключение
TEST(WriteToSockTest, InvalidMessage)
{
    NetworkSerializer server;
    EXPECT_THROW(server.writeToSock(1, "invalid\n\nmessage"), NotCorrectMessageToSend);
}

// Эмулируем ошибку записи
TEST(WriteToSockTest, WriteToSock_WriteError)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    EXPECT_CALL(*mockPtr, write(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    NetworkSerializer server(std::move(mockSocket));

    EXPECT_THROW(server.writeToSock(1, "test"), ErrorWritingToSocket);
}

// Проверяем что в конце буфера есть \n\n и сообщение пришло корректно
TEST(WriteToSockTest, CorrectBuffer)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    const void* savedBuf = nullptr;
    EXPECT_CALL(*mockPtr, write(testing::_, testing::_, testing::_))
        .WillOnce(testing::DoAll(testing::SaveArg<1>(&savedBuf), testing::Return(15)));

    NetworkSerializer server(std::move(mockSocket));

    std::string msg = "123456789";
    server.writeToSock(1, msg);

    msg += "\n\n";
    std::string buf(static_cast<const char*>(savedBuf));
    EXPECT_EQ(buf, msg);
}

