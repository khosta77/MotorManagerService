#include <gtest/gtest.h>
#include "ft232rl.hpp"
#include <thread>
#include <chrono>


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
}

#if 0
// Тест получения списка устройств
TEST_F(FT232RLTest, DeviceListing)
{
    auto devices = module1.listComs();
    EXPECT_GE(devices.size(), 2); // Должно быть как минимум 2 устройства

    // Выводим информацию об устройствах для отладки
    std::cout << "Found " << devices.size() << " devices:" << std::endl;
    for (const auto& device : devices)
    {
        std::cout << "  " << device << std::endl;
    }
}

// Тест одновременной работы с двумя устройствами
TEST_F(FT232RLTest, MultipleDevices)
{
    // Подключаем оба устройства
    ASSERT_TRUE(module1.connect());
    ASSERT_TRUE(module2.connect());

    // Проверяем, что оба подключены
    EXPECT_TRUE(module1.isConnected());
    EXPECT_TRUE(module2.isConnected());

    // Устанавливаем разные параметры для каждого устройства
    module1.setBaudRate(9600);
    module2.setBaudRate(115200);

    module1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    module2.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_EVEN);
}

// Тест обработки ошибок
TEST_F(FT232RLTest, ErrorHandling)
{
    // Попытка работы с неподключенным устройством должна бросать исключение
    EXPECT_THROW(module1.setBaudRate(9600), ModuleFT2xxException);
    EXPECT_THROW(module1.writeData({0x01, 0x02}), ModuleFT2xxException);

    // Подключаем и проверяем нормальную работу
    ASSERT_TRUE(module1.connect());
    EXPECT_NO_THROW(module1.setBaudRate(9600));
}

// Тест передачи данных между устройствами
TEST_F(FT232RLTest, DataTransfer)
{
    // Этот тест требует, чтобы устройства были соединены между собой
    // (TX одного подключен к RX другого и наоборот)

    ASSERT_TRUE(module1.connect());
    ASSERT_TRUE(module2.connect());

    // Устанавливаем одинаковые параметры
    module1.setBaudRate(9600);
    module2.setBaudRate(9600);
    module1.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);
    module2.setCharacteristics(FT_BITS_8, FT_STOP_BITS_1, FT_PARITY_NONE);

    // Тестовые данные
    std::vector<uchar> testData = {0x48, 0x65, 0x6C, 0x6C, 0x6F}; // "Hello"

    // Отправляем данные с первого устройства
    EXPECT_NO_THROW(module1.writeData(testData));

    // Ждем немного для передачи
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // Проверяем, что данные получены на втором устройстве
    size_t availableBytes = module2.checkRXChannel();
    if (availableBytes > 0)
    {
        std::vector<uchar> receivedData = module2.read(500);
        EXPECT_EQ(receivedData.size(), testData.size());
        if (receivedData.size() == testData.size())
        {
            EXPECT_EQ(receivedData, testData);
        }
    }
}

// Тест таймаутов
TEST_F(FT232RLTest, TimeoutHandling)
{
    ASSERT_TRUE(module1.connect());

    // Чтение при отсутствии данных должно вернуть пустой вектор
    auto result = module1.read(100); // Таймаут 100 мс
    EXPECT_TRUE(result.empty());
}

// Тест потокобезопасности
TEST_F(FT232RLTest, ThreadSafety)
{
    ASSERT_TRUE(module1.connect());

    auto worker = [](FT232RL& module, int threadId) {
        for (int i = 0; i < 10; ++i)
        {
            EXPECT_NO_THROW(module.setBaudRate(9600 + threadId * 100 + i));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    };

    std::thread t1(worker, std::ref(module1), 1);
    std::thread t2(worker, std::ref(module1), 2);

    t1.join();
    t2.join();
}

// Тест оператора приведения к bool
TEST_F(FT232RLTest, BoolConversion)
{
    FT232RL module(0);

    // До подключения
    if (module)
    {
        FAIL() << "Module should not be connected";
    }

    // После подключения
    module.connect();
    if (!module)
    {
        FAIL() << "Module should be connected";
    }

    // После отключения
    module.disconnect();
    if (module)
    {
        FAIL() << "Module should not be connected after disconnect";
    }
}
#endif
int main(int argc, char** argv)
{
    testing::InitGoogleTest(&argc, argv);
    {
        FT232RL tempModule(0);
        auto devices = tempModule.listComs();
        assert(devices.size() >= 2);
    }
    return RUN_ALL_TESTS();
}
