#include "network_serializer.hpp"

#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <future>

class SocketMock : public ISocket
{
public:
    MOCK_METHOD(size_t, write, (int fd, const void* buf, size_t count), (override));
    MOCK_METHOD(size_t, read, (int fd, void* buf, size_t count), (override));
};

//// Тесты для readFromSock

// Отправка 0
TEST(readFromSockTest, ZeroReadTest)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    const std::string test_data = "";

    EXPECT_CALL(*mockPtr, read(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([&test_data]([[maybe_unused]] int fd, void* buf, size_t count) {
            static size_t total_read = 0;
            size_t remaining = test_data.size() - total_read;
            if (remaining == 0)
            {
                total_read = 0;
                return static_cast<ssize_t>(0);
            }

            size_t bytes_to_copy = std::min(remaining, count);
            memcpy(buf, (test_data.data() + total_read), bytes_to_copy);
            total_read += bytes_to_copy;

            return static_cast<ssize_t>(bytes_to_copy);
        }));

    NetworkSerializer server(std::move(mockSocket));

    std::string result = server.readFromSock(0);
    EXPECT_EQ(result, test_data);
}

// Просто отправка сообщения
TEST(readFromSockTest, BasicReadTest)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    const std::string test_data = "test_data\n\n";

    EXPECT_CALL(*mockPtr, read(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([&test_data]([[maybe_unused]] int fd, void* buf, size_t count) {
            static size_t total_read = 0;
            size_t remaining = test_data.size() - total_read;
            if (remaining == 0)
            {
                total_read = 0;
                return static_cast<ssize_t>(0);
            }

            size_t bytes_to_copy = std::min(remaining, count);
            memcpy(buf, (test_data.data() + total_read), bytes_to_copy);
            total_read += bytes_to_copy;

            return static_cast<ssize_t>(bytes_to_copy);
        }));

    NetworkSerializer server(std::move(mockSocket));

    std::string result = server.readFromSock(0);
    EXPECT_EQ(result, test_data);
}

// Тест на отправку сообщения на границе
TEST(readFromSockTest, ReadTestOnBorders)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    std::string test_data = std::string(1024, '0');
    test_data += "\n\n";

    EXPECT_CALL(*mockPtr, read(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Invoke([&test_data]([[maybe_unused]] int fd, void* buf, size_t count) {
            static size_t total_read = 0;
            size_t remaining = test_data.size() - total_read;
            if (remaining == 0)
            {
                total_read = 0;
                return static_cast<ssize_t>(0);
            }

            size_t bytes_to_copy = std::min(remaining, count);
            memcpy(buf, (test_data.data() + total_read), bytes_to_copy);
            total_read += bytes_to_copy;

            return static_cast<ssize_t>(bytes_to_copy);
        }));

    NetworkSerializer server(std::move(mockSocket));

    std::string result = server.readFromSock(0);
    EXPECT_EQ(result, test_data);
}

// Тест на имитацию ошибки
TEST(readFromSockTest, WriteError)
{
    auto mockSocket = std::make_unique<SocketMock>();
    SocketMock* mockPtr = mockSocket.get();

    EXPECT_CALL(*mockPtr, read(testing::_, testing::_, testing::_)).WillOnce(testing::Return(-1));

    NetworkSerializer server(std::move(mockSocket));

    EXPECT_THROW(server.readFromSock(1), ErrorReadingFromSocket);
}

