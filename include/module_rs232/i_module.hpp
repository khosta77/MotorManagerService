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

class IModuleBase
{
public:
    virtual ~IModuleBase() = default;

    virtual void setBaudRate(const int) = 0;
    virtual void setUSBParameters(const int, const int) = 0;
    virtual void setCharacteristics(const uchar, const uchar, const uchar) = 0;
    virtual void waitWriteSuccess() = 0;
    virtual size_t checkRXChannel() const = 0;
};

template <typename Derived>
class IModule : public IModuleBase
{
public:
    template <typename T>
    void writeData(const std::vector<T>& data)
    {
        static_cast<Derived*>(this)->writeDataImpl(data);
    }

    template <typename T>
    void readData(std::vector<T>& data)
    {
        static_cast<Derived*>(this)->readDataImpl(data);
    }

    template <typename T>
    std::vector<T> read(const size_t timeout)
    {
        return static_cast<Derived*>(this)->template readImpl<T>(timeout);
    }
};

#endif // MODULE_HPP_
