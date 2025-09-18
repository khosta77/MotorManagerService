#ifndef SOURCE_MODULE_FT232RL_HPP_
#define SOURCE_MODULE_FT232RL_HPP_

#include <cassert>
#include <chrono>
#include <cmath>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <mutex>
#include <stdlib.h>
#include <thread>

extern "C"
{
#include "ftd2xx.h"
};

#include "Exceptions.hpp"

class ModuleFT232RL
{
    unsigned int device_id_;
    int baudrate_;

    // http://microsin.net/programming/pc/ftdi-d2xx-functions-api.html - ds
    FT_HANDLE ftHandle;
    //[[ maybe_unused ]] FT_STATUS ftStatus;
    //[[ maybe_unused ]] DWORD EventDWord;
    //[[ maybe_unused ]] DWORD TxBytes;
    //[[ maybe_unused ]] DWORD RxBytes;
    DWORD BytesReceived;
    DWORD BytesWritten;

    struct DeviceInfo
    {
        DWORD flags_;
        DWORD id_;
        DWORD type_;
        DWORD locId_;

        char serialNumber_[16];
        char description_[64];

        FT_HANDLE ftHandleTemp_;
    } device_info_[3];

    mutable std::mutex mutex_;

    void getDeviceInfo()
    {
        DWORD numDevs;

        if ( FT_STATUS code = FT_CreateDeviceInfoList( &numDevs );
             code != FT_OK )
            throw ModuleFT2xxException( code );

        if ( ( ( device_id_ < 0 ) and ( device_id_ >= numDevs ) ) )
            throw ModuleFT2xxException( FT_DEVICE_NOT_FOUND );

        FT_STATUS code = FT_GetDeviceInfoDetail(
            device_id_, &device_info_[device_id_].flags_,
            &device_info_[device_id_].type_, &device_info_[device_id_].id_,
            &device_info_[device_id_].locId_,
            device_info_[device_id_].serialNumber_,
            device_info_[device_id_].description_,
            &device_info_[device_id_].ftHandleTemp_ );

        if ( code != FT_OK )
            throw ModuleFT2xxException( code );
    }

  public:
    ModuleFT232RL( const int dev_id = 0, const int baudrate = 9600 )
        : device_id_( dev_id ), baudrate_( baudrate )
    {
        getDeviceInfo();

        if ( FT_STATUS code = FT_Open( device_id_, &ftHandle ); code != FT_OK )
            throw ModuleFT2xxException( code );

        this->setBaudRate( baudrate_ );
        this->setUSBParametrs( 256, 256 );

        // Настройка таймаутов
        FT_SetTimeouts( ftHandle, 1000, 1000 ); // read, write в ms

        // Использование событий (Linux/macOS)
        FT_SetEventNotification( ftHandle, FT_EVENT_RXCHAR, 0 );
    }

    ~ModuleFT232RL() { FT_Close( ftHandle ); }

    /* @brief BaudRate
     * */
    void setBaudRate( const int baudrate = 9600 )
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        if ( FT_STATUS code = FT_SetBaudRate( ftHandle, baudrate );
             code != FT_OK )
            throw ModuleFT2xxException( code );

        baudrate_ = baudrate;
    }

    /* @brief Установить параметры USB, Увеличение буферов (перед открытием
     * устройства)
     * @param dwRxSize - размер буфера на прием
     * @param dwTxSize - размер буфера на отдачу
     * */
    void setUSBParametrs( const int dwRxSize = 65536,
                          const int dwTxSize = 65536 )
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        if ( FT_STATUS code =
                 FT_SetUSBParameters( ftHandle, dwRxSize, dwTxSize );
             code != FT_OK )
            throw ModuleFT2xxException( code );
    }

    /* @brief setCharacteristics - задание характеристик передачи
     * @param wordlenght - количество бит на слово (фрейм) - тут должно быть
     * значение FT_BITS_8 или FT_BITS_7
     * @param stopbit - количество стоп-бит - должно быть FT_STOP_BITS_1 или
     * FT_STOP_BITS_2
     * @param parity - четность - должно быть FT_PARITY_NONE, FT_PARITY_ODD,
     * FT_PARITY_EVEN, \ FT_PARITY_MARK или FT_PARITY SPACE
     * */
    void setCharacteristics( const UCHAR &wordlenght = FT_BITS_8,
                             const UCHAR &stopbit = FT_STOP_BITS_1,
                             const UCHAR &parity = FT_PARITY_NONE )
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        FT_STATUS code =
            FT_SetDataCharacteristics( ftHandle, wordlenght, stopbit, parity );
        if ( code != FT_OK )
            throw ModuleFT2xxException( code );
    }

    /* @brief функция отправляет данные на устройство. В примере использования
     * FT_Read были еще статусы какие-то и задержки, в общем они не работали
     * @param frame_ - массив, который отправляем
     * */
    template <typename T> void writeData( const std::vector<T> &frame )
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        const size_t b_size = sizeof( T ) * frame.size();

        // Создаем буфер нужного размера
        std::vector<uint8_t> buffer( b_size );

        // Копируем данные в буфер
        std::memcpy( buffer.data(), frame.data(), b_size );

        size_t count = 128;
        for ( size_t i = 0; i < b_size; i += 128 )
        {
            if ( count > ( b_size - i ) )
                count = b_size - i;

            FT_STATUS code =
                FT_Write( ftHandle, buffer.data() + i, count, &BytesWritten );
            if ( code != FT_OK )
                throw ModuleFT2xxException( code );
            std::cout << BytesWritten << std::endl;
            std::this_thread::sleep_for( std::chrono::milliseconds( 5000 ) );
            // FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
        }
    }

    void waitWriteSuccess()
    {
        // std::lock_guard<std::mutex> lock( mutex_ );
        DWORD RxBytes = 1, TxBytes = 1, SxBytes = 1;
        while ( TxBytes != 0 )
        {
            if ( FT_STATUS code =
                     FT_GetStatus( ftHandle, &RxBytes, &TxBytes, &SxBytes );
                 code != FT_OK )
                throw ModuleFT2xxException( code );
            std::cout << RxBytes << ' ' << TxBytes << ' ' << SxBytes << ' '
                      << BytesWritten << std::endl;
        }
    }

    /* @brief функция читает данные с устройства. В примере использования
     * FT_Read были еще статусы какие-то и задержки, в общем они не работали
     * @param frame_ - массив, который считываем. НАДО ЗАРАНЕЕ ЗНАТЬ ЕГО РАЗМЕР
     * */
    template <typename T> void readData( std::vector<T> &frame_ )
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        const size_t SIZE = ( sizeof( T ) * frame_.size() );
        std::vector<uint8_t> buffer_( SIZE );

        if ( FT_STATUS code =
                 FT_Read( ftHandle, buffer_.data(), SIZE, &BytesReceived );
             code != FT_OK )
            throw ModuleFT2xxException( code );

        std::memcpy( frame_.data(), buffer_.data(), SIZE );
    }

    size_t checkRXChannel() const
    {
        std::lock_guard<std::mutex> lock( mutex_ );
        DWORD RxBytes;
        if ( FT_STATUS code = FT_GetQueueStatus( ftHandle, &RxBytes );
             code != FT_OK )
            throw ModuleFT2xxException( code );
        return static_cast<size_t>( RxBytes );
    }

    template <typename T> std::vector<T> read( const size_t timeout = 1000 )
    {
        while ( checkRXChannel() == 0 )
        {
            // Это очень ужасно, но выбора нет, надо вставить мин задержку
            // иначе, другие не раздуплятся. В целом эта задержка ресурсы не
            // стирает, а для модуля она слишком быстрая и он разницы не
            // заметит. Возможно надо будет увиличить
            std::this_thread::sleep_for( std::chrono::microseconds( 10 ) );
        }

        size_t newCurrentValue = checkRXChannel();
        for ( size_t i = 0; i < timeout; ++i )
        {
            size_t buffer = checkRXChannel();
            if ( buffer != newCurrentValue )
            {
                newCurrentValue = buffer;
                i = 0;
            }
        }

        std::vector<T> message_( ( newCurrentValue / sizeof( T ) ), 0 );
        readData<T>( message_ );
        return message_;
    }

    // friend std::ostream &operator<<( std::ostream &, const ModuleFT232RL & );
};

#if 0
std::ostream &operator<<( std::ostream &os, const ModuleFT232RL &ft_ )
{
    os << std::dec << "Dev " << ft_.device_id_ << ":" << std::endl;
    os << std::hex << "\tFlags = 0x" << ft_.device_info_[ft_.device_id_].flags_
       << std::endl;
    os << std::hex << "\tType = 0x" << ft_.device_info_[ft_.device_id_].type_
       << std::endl;
    os << std::hex << "\tID = 0x" << ft_.device_info_[ft_.device_id_].id_
       << std::endl;
    os << std::hex << "\tLocId = 0x" << ft_.device_info_[ft_.device_id_].locId_
       << std::endl;
    os << std::dec
       << "\tSerialNumber = " << ft_.device_info_[ft_.device_id_].serialNumber_
       << std::endl;
    os << std::dec
       << "\tDescription = " << ft_.device_info_[ft_.device_id_].description_
       << std::endl;
    return os;
}
#endif

#endif // SOURCE_MODULE_FT232RL_HPP_
