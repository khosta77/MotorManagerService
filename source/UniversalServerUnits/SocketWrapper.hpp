#ifndef SOCKETWRAPPER_HPP_
#define SOCKETWRAPPER_HPP_

#include "SocketInterface.hpp"

#include <unistd.h>

class SocketWrapper : public SocketInterface
{
public:
    size_t write(int fd, const void* buf, size_t count) override;
    size_t read(int fd, void* buf, size_t count) override;
};

#endif // SOCKETWRAPPER_HPP_
