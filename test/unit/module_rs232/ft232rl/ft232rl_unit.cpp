#include "ft232rl.hpp"

#include <gtest/gtest.h>
#include <thread>
#include <chrono>

#if 1
// Тест подключения и отключения
TEST(FT232RLTest, ConnectionManagement)
{
    FT232RL module_;
    EXPECT_FALSE(module_.isConnected());
    EXPECT_FALSE(static_cast<bool>(module_));

    // Подключаем устройство
    EXPECT_TRUE(module_.connect(0));
    EXPECT_TRUE(module_.isConnected());
    EXPECT_TRUE(static_cast<bool>(module_));

    // Повторное подключение должно вернуть true
    EXPECT_TRUE(module_.connect(0));
    EXPECT_FALSE(module_.connect(1));

    // Отключаем устройство
    module_.disconnect();
    EXPECT_FALSE(module_.isConnected());
    EXPECT_FALSE(static_cast<bool>(module_));

    EXPECT_TRUE(module_.connect(1));
    EXPECT_TRUE(module_.isConnected());
    EXPECT_TRUE(static_cast<bool>(module_));

    // Повторное подключение должно вернуть true
    EXPECT_TRUE(module_.connect(1));
    EXPECT_FALSE(module_.connect(0));

    module_.disconnect();
    EXPECT_FALSE(module_.isConnected());
    EXPECT_FALSE(static_cast<bool>(module_));

    EXPECT_TRUE(module_.connect(0));
    EXPECT_TRUE(module_.isConnected());
    EXPECT_TRUE(static_cast<bool>(module_));
    FT232RL modul1_;
    EXPECT_TRUE(modul1_.connect(1));
    EXPECT_TRUE(modul1_.isConnected());
    EXPECT_TRUE(static_cast<bool>(modul1_));
}
#endif

// Тест получения списка устройств
TEST(FT232RLTest, DeviceListing)
{
    FT232RL module_;
    auto devices = module_.listComs();
    EXPECT_GE(devices.size(), 2);
}

// Тест настроек BaudRate
TEST(FT232RLTest, BaudRateSettings)
{
    FT232RL module1;
    FT232RL module2;

    ASSERT_TRUE(module1.connect(0)) << "Failed to connect module1";
    ASSERT_TRUE(module2.connect(1)) << "Failed to connect module2";

    // Тестируем разные скорости
    std::vector<int> baudRates = {9600, 19200, 38400, 57600, 115200};

    for (int baudRate : baudRates)
    {
        EXPECT_NO_THROW(module1.setBaudRate(baudRate));
        EXPECT_NO_THROW(module2.setBaudRate(baudRate));

        EXPECT_EQ(module1.getBaudRate(), baudRate);
        EXPECT_EQ(module2.getBaudRate(), baudRate);
    }

    // Восстанавливаем стандартную скорость
    module1.setBaudRate(9600);
    module2.setBaudRate(9600);

    module1.disconnect();
    module2.disconnect();
}

// Тест параметров USB
TEST(FT232RLTest, USBParameters)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);

    // Тестируем разные размеры буферов
    EXPECT_NO_THROW(module1.setUSBParameters(512, 512));
    EXPECT_NO_THROW(module2.setUSBParameters(1024, 1024));

    // Дополнительные тесты с разными размерами
    EXPECT_NO_THROW(module1.setUSBParameters(256, 512));
    EXPECT_NO_THROW(module2.setUSBParameters(512, 256));

    module1.disconnect();
    module2.disconnect();
}

// Тест характеристик передачи
TEST(FT232RLTest, DataCharacteristics)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);

    // Тестируем разные конфигурации
    EXPECT_NO_THROW(module1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE));
    EXPECT_NO_THROW(module2.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE));

    EXPECT_NO_THROW(module1.setCharacteristics(FT_BITS_7, FT_STOP_BITS_1, FT_PARITY_EVEN));
    EXPECT_NO_THROW(module2.setCharacteristics(FT_BITS_7, FT_STOP_BITS_2, FT_PARITY_ODD));

    // Возвращаем к стандартной конфигурации
    EXPECT_NO_THROW(module1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE));
    EXPECT_NO_THROW(module2.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE));

    module1.disconnect();
    module2.disconnect();
}

// Тест проверки RX канала
TEST(FT232RLTest, RXChannelCheck)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);

    // Проверяем, что функция не бросает исключений
    EXPECT_NO_THROW(module1.checkRXChannel());
    EXPECT_NO_THROW(module2.checkRXChannel());

    // Получаем размеры буферов
    size_t rx1 = module1.checkRXChannel();
    size_t rx2 = module2.checkRXChannel();

    // Это просто проверка что функция работает, не проверяем конкретные значения
    // так как они зависят от состояния устройств
    // std::cout << "Device 0 RX bytes: " << rx1 << std::endl;
    // std::cout << "Device 1 RX bytes: " << rx2 << std::endl;

    module1.disconnect();
    module2.disconnect();
}

// Тест записи данных
TEST(FT232RLTest, DataWrite)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);

    // Тестовые данные для отправки
    std::vector<uchar> testData1 = {0x01, 0x02, 0x03, 0x04, 0x05};
    std::vector<uchar> testData2 = {0xAA, 0xBB, 0xCC, 0xDD, 0xEE};

    // Тестируем запись
    EXPECT_NO_THROW(module1.writeData(testData1));
    EXPECT_NO_THROW(module2.writeData(testData2));

    // Тестируем с пустыми данными
    std::vector<uchar> emptyData;
    EXPECT_NO_THROW(module1.writeData(emptyData));
    EXPECT_NO_THROW(module2.writeData(emptyData));

    // Тестируем с большими данными
    std::vector<uchar> largeData(100, 0x55);
    EXPECT_NO_THROW(module1.writeData(largeData));

    module1.disconnect();
    module2.disconnect();
}

// Тест оператора вывода
TEST(FT232RLTest, OutputOperator)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);
    std::stringstream ss;

    // Проверяем что оператор вывода работает без исключений
    EXPECT_NO_THROW(ss << module1 << std::endl);
    EXPECT_NO_THROW(ss << module2 << std::endl);

    module1.disconnect();
    module2.disconnect();
}

// Тест работы с исключениями при неправильных операциях
TEST(FT232RLTest, ExceptionHandling)
{
    FT232RL module;

    // Попытка установить BaudRate без подключения
    EXPECT_THROW(module.setBaudRate(9600), ModuleFT2xxException);

    // Попытка установить USB параметры без подключения
    EXPECT_THROW(module.setUSBParameters(256, 256), ModuleFT2xxException);

    // Попытка установить характеристики без подключения
    EXPECT_THROW(module.setCharacteristics(), ModuleFT2xxException);

    // Попытка записи без подключения
    std::vector<uchar> testData = {0x01, 0x02};
    EXPECT_THROW(module.writeData(testData), ModuleFT2xxException);

    // Попытка чтения без подключения
    EXPECT_THROW(module.readData(testData), ModuleFT2xxException);

    // Попытка проверки RX канала без подключения
    EXPECT_THROW(module.checkRXChannel(), ModuleFT2xxException);
}

// Тест многопоточного доступа
TEST(FT232RLTest, ThreadSafety)
{
    FT232RL module1;
    FT232RL module2;

    module1.connect(0);
    module2.connect(1);

    // Запускаем несколько потоков для каждого устройства
    auto testDevice = [](FT232RL& module, int deviceId) {
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_NO_THROW(module.setBaudRate(9600 + i * 1000));
            EXPECT_NO_THROW(module.checkRXChannel());

            std::vector<uchar> testData = {static_cast<uchar>(i), static_cast<uchar>(deviceId)};
            EXPECT_NO_THROW(module.writeData(testData));

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::thread thread1(testDevice, std::ref(module1), 0);
    std::thread thread2(testDevice, std::ref(module2), 1);

    thread1.join();
    thread2.join();

    module1.disconnect();
    module2.disconnect();
}

// Тест последовательного подключения/отключения
TEST(FT232RLTest, SequentialConnection)
{
    FT232RL module;

    // Многократное подключение/отключение
    for (int i = 0; i < 5; ++i)
    {
        EXPECT_TRUE(module.connect(0));
        EXPECT_TRUE(module.isConnected());

        // Выполняем некоторые операции
        EXPECT_NO_THROW(module.setBaudRate(9600));
        EXPECT_NO_THROW(module.checkRXChannel());

        module.disconnect();
        EXPECT_FALSE(module.isConnected());
    }
}

// Тест информации об устройствах
TEST(FT232RLTest, DeviceInfo)
{
    FT232RL module1;
    FT232RL module2;

    // Получаем список устройств до подключения
    auto devicesBefore = module1.listComs();
    EXPECT_GE(devicesBefore.size(), 2);

    module1.connect(0);
    module2.connect(1);

    std::stringstream ss;
    // Проверяем что информация об устройствах доступна
    EXPECT_NO_THROW(ss << module1 << std::endl);
    EXPECT_NO_THROW(ss << module2 << std::endl);

    // Получаем список устройств после подключения
    auto devicesAfter = module1.listComs();
    EXPECT_EQ(devicesBefore.size(), devicesAfter.size());

    module1.disconnect();
    module2.disconnect();
}

int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    {
        FT232RL tempModule;
        auto devices = tempModule.listComs();
        assert(devices.size() >= 2);
    }
    return RUN_ALL_TESTS();
}
