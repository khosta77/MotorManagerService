#include "socket.hpp"

size_t Socket::write(int fd, const void* buf, size_t count)
{
    return ::write(fd, buf, count);
}

size_t Socket::read(int fd, void* buf, size_t count)
{
    return ::read(fd, buf, count);
}
