#ifndef MODULE_HPP_
#define MODULE_HPP_

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdlib.h>
#include <thread>

using uchar = unsigned char;

class IModule
{
public:
    virtual ~IModule() = default;

    virtual bool connect(const int) = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() const = 0;
    virtual std::vector<std::string> listComs() const = 0;

    virtual void setBaudRate(const int) = 0;
    virtual int getBaudRate() = 0;
    virtual void setUSBParameters(const int, const int) = 0;
    virtual void setCharacteristics(const uchar, const uchar, const uchar) = 0;
    virtual void waitWriteSuccess() = 0;
    virtual size_t checkRXChannel() const = 0;
    virtual void writeData(const std::vector<uchar>& data) = 0;
    virtual void readData(std::vector<uchar>& data) = 0;
    virtual std::vector<uchar> read(const size_t timeout) = 0;

    virtual explicit operator bool() const = 0;
};

#endif // MODULE_HPP_
