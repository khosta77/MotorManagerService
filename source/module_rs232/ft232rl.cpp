#include "ft232rl.hpp"

FT232RL::FT232RL(const int dev_id, const int baudrate) : device_id_(dev_id), baudrate_(baudrate)
{
    getDeviceInfo();

    if (FT_STATUS code = FT_Open(device_id_, &ftHandle); code != FT_OK)
        throw ModuleFT2xxException(code);

    this->setBaudRate(baudrate_);
    this->setUSBParameters(256, 256);

    FT_SetTimeouts(ftHandle, 1000, 1000); // read, write в ms
    FT_SetEventNotification(ftHandle, FT_EVENT_RXCHAR, 0);
}

FT232RL::~FT232RL()
{
    std::lock_guard<std::mutex> lock(mutex_);
    FT_Close(ftHandle);
}

void FT232RL::setBaudRate(const int baudrate = 9600)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (FT_STATUS code = FT_SetBaudRate(ftHandle, baudrate); code != FT_OK)
        throw ModuleFT2xxException(code);

    baudrate_ = baudrate;
}

void FT232RL::setUSBParameters(const int dwRxSize = 65536, const int dwTxSize = 65536)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (FT_STATUS code = FT_SetUSBParameters(ftHandle, dwRxSize, dwTxSize); code != FT_OK)
        throw ModuleFT2xxException(code);
}


void FT232RL::setCharacteristics(
    const uchar wordlenght = FT_BITS_8,
    const uchar stopbit = FT_STOP_BITS_1,
    const uchar parity = FT_PARITY_NONE)
{
    std::lock_guard<std::mutex> lock(mutex_);
    FT_STATUS code = FT_SetDataCharacteristics(ftHandle, wordlenght, stopbit, parity);
    if (code != FT_OK)
        throw ModuleFT2xxException(code);
}

void FT232RL::waitWriteSuccess()
{
    // std::lock_guard<std::mutex> lock( mutex_ );
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
    DWORD RxBytes;
    if (FT_STATUS code = FT_GetQueueStatus(ftHandle, &RxBytes); code != FT_OK)
        throw ModuleFT2xxException(code);
    return static_cast<size_t>(RxBytes);
}

void FT232RL::getDeviceInfo()
{
    std::lock_guard<std::mutex> lock(mutex_);
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

