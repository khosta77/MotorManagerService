#ifndef I_SOCKET_HPP_
#define I_SOCKET_HPP_

#include <cstdlib>

class ISocket
{
public:
    virtual ~ISocket() = default;
    virtual size_t write(int fd, const void* buf, size_t count) = 0;
    virtual size_t read(int fd, void* buf, size_t count) = 0;
};

#endif // I_SOCKET_HPP_
