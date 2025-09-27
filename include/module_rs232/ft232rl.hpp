#ifndef FT232RL_HPP_
#define FT232RL_HPP_

#include "i_module.hpp"
#include "exceptions.hpp"

extern "C"
{
#include "ftd2xx.h"
};

class FT232RL : public IModule
{
public:
    FT232RL(const int baudrate = 9600);
    ~FT232RL() override;

    bool connect(const int) override;
    void disconnect() override;
    bool isConnected() const override;
    std::vector<std::string> listComs() const override;
    explicit operator bool() const override
    {
        return isConnected();
    }

    /* @brief BaudRate
     * */
    void setBaudRate(const int) override;

    /* @brief Установить параметры USB, Увеличение буферов (перед открытием
     * устройства)
     * @param dwRxSize - размер буфера на прием
     * @param dwTxSize - размер буфера на отдачу
     * */
    void setUSBParameters(const int, const int) override;

    /* @brief setCharacteristics - задание характеристик передачи
     * @param wordlenght - количество бит на слово (фрейм) - тут должно быть
     * значение FT_BITS_8 или FT_BITS_7
     * @param stopbit - количество стоп-бит - должно быть FT_STOP_BITS_1 или
     * FT_STOP_BITS_2
     * @param parity - четность - должно быть FT_PARITY_NONE, FT_PARITY_ODD,
     * FT_PARITY_EVEN, \ FT_PARITY_MARK или FT_PARITY SPACE
     * */
    void setCharacteristics(const uchar, const uchar, const uchar) override;

    void waitWriteSuccess() override;
    size_t checkRXChannel() const override;

    /* @brief функция отправляет данные на устройство
     * @param frame - массив байтов, который отправляем
     * */
    void writeData(const std::vector<uchar> &frame) override;

    /* @brief функция читает данные с устройства. В примере использования
     * FT_Read были еще статусы какие-то и задержки, в общем они не работали
     * @param frame_ - массив, который считываем. НАДО ЗАРАНЕЕ ЗНАТЬ ЕГО РАЗМЕР
     * */
    void readData(std::vector<uchar> &frame) override;

    /* @brief функция читает данные с устройства с таймаутом
     * @param timeout - время ожидания в миллисекундах
     * @return вектор байтов с полученными данными
     * */
    std::vector<uchar> read(const size_t timeout = 1000) override;

    friend std::ostream &operator<<(std::ostream &, const FT232RL &);

private:
    unsigned int m_deviceId;
    int baudrate_;
    bool connected_;

    // http://microsin.net/programming/pc/ftdi-d2xx-functions-api.html - ds
    FT_HANDLE ftHandle;
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

    std::mutex mutex_;

    void getDeviceInfo(const int);
    static std::vector<FT_DEVICE_LIST_INFO_NODE> getDeviceList();
};

std::ostream &operator<<(std::ostream &os, const FT232RL &ft_);

#endif // FT232RL_HPP_
