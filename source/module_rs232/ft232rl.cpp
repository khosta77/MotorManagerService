#include "ft232rl.hpp"

FT232RL::FT232RL(const int dev_id, const int baudrate) : device_id_(dev_id), baudrate_(baudrate)
{
    getDeviceInfo();
}

FT232RL::~FT232RL()
{
    disconnect();
}

bool FT232RL::connect()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);

        if (connected_)
        {
            return true; // Уже подключен
        }

        if (FT_STATUS code = FT_Open(device_id_, &ftHandle); code != FT_OK)
        {
            std::cerr << "Failed to connect to device " << device_id_ << ", error: " << code << std::endl;
            connected_ = false;
            return false;
        }
    }

    try
    {
        this->setBaudRate(baudrate_);
        this->setUSBParameters(256, 256);
        FT_SetTimeouts(ftHandle, 1000, 1000);
        FT_SetEventNotification(ftHandle, FT_EVENT_RXCHAR, 0);

        connected_ = true;
        std::cout << "Successfully connected to device " << device_id_ << std::endl;
        return true;
    }
    catch (const ModuleFT2xxException &e)
    {
        std::cerr << "Error during device setup: " << e.what() << std::endl;
        FT_Close(ftHandle);
        connected_ = false;
        return false;
    }
}

void FT232RL::disconnect()
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (connected_ && ftHandle)
    {
        FT_Close(ftHandle);
        ftHandle = nullptr;
        connected_ = false;
        std::cout << "Disconnected from device " << device_id_ << std::endl;
    }
}

bool FT232RL::isConnected() const
{
    return connected_;
}

std::vector<std::string> FT232RL::listComs() const
{
    std::vector<std::string> devices;

    DWORD numDevs;
    if (FT_STATUS code = FT_CreateDeviceInfoList(&numDevs); code != FT_OK)
    {
        std::cerr << "Failed to get device list, error: " << code << std::endl;
        return devices;
    }

    if (numDevs > 0)
    {
        std::vector<FT_DEVICE_LIST_INFO_NODE> devInfo(numDevs);
        if (FT_STATUS code = FT_GetDeviceInfoList(devInfo.data(), &numDevs); code == FT_OK)
        {
            for (DWORD i = 0; i < numDevs; ++i)
            {
                std::string deviceInfo =
                    std::format("Device {}: {} - {}", i, devInfo[i].Description, devInfo[i].SerialNumber);
                devices.push_back(deviceInfo);
            }
        }
    }

    return devices;
}

void FT232RL::setBaudRate(const int baudrate = 9600)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    if (FT_STATUS code = FT_SetBaudRate(ftHandle, baudrate); code != FT_OK)
        throw ModuleFT2xxException(code);

    baudrate_ = baudrate;
}

void FT232RL::setUSBParameters(const int dwRxSize = 65536, const int dwTxSize = 65536)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    if (FT_STATUS code = FT_SetUSBParameters(ftHandle, dwRxSize, dwTxSize); code != FT_OK)
        throw ModuleFT2xxException(code);
}


void FT232RL::setCharacteristics(
    const uchar wordlenght = FT_BITS_8,
    const uchar stopbit = FT_STOP_BITS_1,
    const uchar parity = FT_PARITY_NONE)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    FT_STATUS code = FT_SetDataCharacteristics(ftHandle, wordlenght, stopbit, parity);
    if (code != FT_OK)
        throw ModuleFT2xxException(code);
}

void FT232RL::waitWriteSuccess()
{
    // std::lock_guard<std::mutex> lock( mutex_ );
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    DWORD RxBytes = 1, TxBytes = 1, SxBytes = 1;
    while (TxBytes != 0)
    {
        if (FT_STATUS code = FT_GetStatus(ftHandle, &RxBytes, &TxBytes, &SxBytes); code != FT_OK)
            throw ModuleFT2xxException(code);
        std::cout << RxBytes << ' ' << TxBytes << ' ' << SxBytes << ' ' << BytesWritten << std::endl;
    }
}

size_t FT232RL::checkRXChannel() const
{
    // std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    DWORD RxBytes;
    if (FT_STATUS code = FT_GetQueueStatus(ftHandle, &RxBytes); code != FT_OK)
        throw ModuleFT2xxException(code);
    return static_cast<size_t>(RxBytes);
}

void FT232RL::getDeviceInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    DWORD numDevs;

    if (FT_STATUS code = FT_CreateDeviceInfoList(&numDevs); code != FT_OK)
        throw ModuleFT2xxException(code);

    if (((device_id_ < 0) and (device_id_ >= numDevs)))
        throw ModuleFT2xxException(FT_DEVICE_NOT_FOUND);

    FT_STATUS code = FT_GetDeviceInfoDetail(
        device_id_,
        &device_info_[device_id_].flags_,
        &device_info_[device_id_].type_,
        &device_info_[device_id_].id_,
        &device_info_[device_id_].locId_,
        device_info_[device_id_].serialNumber_,
        device_info_[device_id_].description_,
        &device_info_[device_id_].ftHandleTemp_);

    if (code != FT_OK)
        throw ModuleFT2xxException(code);
}

std::ostream &operator<<(std::ostream &os, const FT232RL &ft_)
{
    os << std::dec << "Dev " << ft_.device_id_ << ":" << std::endl;
    os << std::hex << "\tFlags = 0x" << ft_.device_info_[ft_.device_id_].flags_ << std::endl;
    os << std::hex << "\tType = 0x" << ft_.device_info_[ft_.device_id_].type_ << std::endl;
    os << std::hex << "\tID = 0x" << ft_.device_info_[ft_.device_id_].id_ << std::endl;
    os << std::hex << "\tLocId = 0x" << ft_.device_info_[ft_.device_id_].locId_ << std::endl;
    os << std::dec << "\tSerialNumber = " << ft_.device_info_[ft_.device_id_].serialNumber_ << std::endl;
    os << std::dec << "\tDescription = " << ft_.device_info_[ft_.device_id_].description_ << std::endl;
    return os;
}

void FT232RL::writeData(const std::vector<uchar> &frame)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    const size_t b_size = frame.size();
    size_t count = frame.size(); // 128;
    for (size_t i = 0; i < b_size; i += frame.size())
    {
        if (count > (b_size - i))
            count = b_size - i;

        FT_STATUS code = FT_Write(
            ftHandle,
            static_cast<LPVOID>(const_cast<uint8_t *>(frame.data() + i)),
            count,
            &BytesWritten);
        if (code != FT_OK)
            throw ModuleFT2xxException(code);
        std::cout << BytesWritten << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    }
}

void FT232RL::readData(std::vector<uchar> &frame)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_)
        throw ModuleFT2xxException(FT_DEVICE_NOT_OPENED);

    const size_t SIZE = frame.size();

    if (FT_STATUS code = FT_Read(ftHandle, frame.data(), SIZE, &BytesReceived); code != FT_OK)
        throw ModuleFT2xxException(code);
}

std::vector<uchar> FT232RL::read(const size_t timeout)
{
    while (checkRXChannel() == 0)
    {
        // Это очень ужасно, но выбора нет, надо вставить мин задержку
        // иначе, другие не раздуплятся. В целом эта задержка ресурсы не
        // стирает, а для модуля она слишком быстрая и он разницы не
        // заметит. Возможно надо будет увиличить
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    size_t newCurrentValue = checkRXChannel();
    for (size_t i = 0; i < timeout; ++i)
    {
        size_t buffer = checkRXChannel();
        if (buffer != newCurrentValue)
        {
            newCurrentValue = buffer;
            i = 0;
        }
    }

    std::vector<uchar> message_(newCurrentValue, 0);
    readData(message_);
    return message_;
}

