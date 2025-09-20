#ifndef FT232RL_HPP_
#define FT232RL_HPP_

#include "i_module.hpp"
#include "exceptions.hpp"

extern "C"
{
#include "ftd2xx.h"
};

class FT232RL : public IModule<FT232RL>
{
public:
    FT232RL(const int dev_id = 0, const int baudrate = 9600);
    ~FT232RL() override;

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

    /* @brief функция отправляет данные на устройство. В примере использования
     * FT_Read были еще статусы какие-то и задержки, в общем они не работали
     * @param frame_ - массив, который отправляем
     * */
    template <typename T>
    void writeDataImpl(const std::vector<T> &frame)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const size_t b_size = sizeof(T) * frame.size();
        std::cout << std::format("b_size:= {}\n", b_size);

        // Создаем буфер нужного размера
        std::vector<uint8_t> buffer(b_size);

        // Копируем данные в буфер
        std::memcpy(buffer.data(), frame.data(), b_size);

        size_t count = frame.size(); // 128;
        for (size_t i = 0; i < b_size; i += frame.size())
        {
            if (count > (b_size - i))
                count = b_size - i;

            FT_STATUS code = FT_Write(ftHandle, buffer.data() + i, count, &BytesWritten);
            if (code != FT_OK)
                throw ModuleFT2xxException(code);
            std::cout << BytesWritten << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(5000));
            // FT_Purge( ftHandle, FT_PURGE_TX | FT_PURGE_RX );
        }
    }

    /* @brief функция читает данные с устройства. В примере использования
     * FT_Read были еще статусы какие-то и задержки, в общем они не работали
     * @param frame_ - массив, который считываем. НАДО ЗАРАНЕЕ ЗНАТЬ ЕГО РАЗМЕР
     * */
    template <typename T>
    void readDataImpl(std::vector<T> &frame_)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        const size_t SIZE = (sizeof(T) * frame_.size());
        std::vector<uint8_t> buffer_(SIZE);

        if (FT_STATUS code = FT_Read(ftHandle, buffer_.data(), SIZE, &BytesReceived); code != FT_OK)
            throw ModuleFT2xxException(code);

        std::memcpy(frame_.data(), buffer_.data(), SIZE);
    }

    template <typename T>
    std::vector<T> readImpl(const size_t timeout = 1000)
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

        std::vector<T> message_((newCurrentValue / sizeof(T)), 0);
        readData<T>(message_);
        return message_;
    }

    friend std::ostream &operator<<(std::ostream &, const FT232RL &);

private:
    unsigned int device_id_;
    int baudrate_;

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

    void getDeviceInfo();
};

std::ostream &operator<<(std::ostream &os, const FT232RL &ft_);

#endif // FT232RL_HPP_
