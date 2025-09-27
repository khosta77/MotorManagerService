#include "ft232rl.hpp"
#include <iostream>

/*
 * Человеку, что будет читать данный пример(возможно я из будущего...):
 * * При отладке я столкнулся с тем, что оно все работает корректно.
 * * Но почему то когда я запускал с устройство, то оно не корректно работало...
 *
 * * Если на устройстве возникнут баги, возможно контроллер не качественный, потому что при отладке
 * * работает все.
 */

#if 0
int main()
{
    try
    {
        FT232RL module_(0, 9600);
    }
    catch (const std::exception& e)
    {
        std::cout << e.what() << std::endl;
    }
    std::cout << "yyy\n";
    // if (module_.connect())
    //   std::cout << "xxx\n";
    // std::cout << module_;
    std::cout << std::string(80, '=') << std::endl;
    // module_.setBaudRate(115200);
    // module_.writeData(std::vector<uchar>(1000, 0));
    return 0;
}
#endif
#include <iostream>

int main()
{
    FT_STATUS ftStatus;
    DWORD numDevices;

    // Получаем количество устройств
    ftStatus = FT_CreateDeviceInfoList(&numDevices);
    if (ftStatus != FT_OK)
    {
        std::cout << "Ошибка получения списка устройств: " << ftStatus << std::endl;
        return -1;
    }

    std::cout << "Найдено устройств FTDI: " << numDevices << std::endl;

    if (numDevices > 0)
    {
        // Выделяем память для информации об устройствах
        FT_DEVICE_LIST_INFO_NODE *devInfo = new FT_DEVICE_LIST_INFO_NODE[numDevices];

        // Получаем информацию об устройствах
        ftStatus = FT_GetDeviceInfoList(devInfo, &numDevices);
        if (ftStatus == FT_OK)
        {
            for (DWORD i = 0; i < numDevices; i++)
            {
                std::cout << "Устройство " << i << ":" << std::endl;
                std::cout << "  Flags: 0x" << std::hex << devInfo[i].Flags << std::dec << std::endl;
                std::cout << "  Type: " << devInfo[i].Type << std::endl;
                std::cout << "  ID: 0x" << std::hex << devInfo[i].ID << std::dec << std::endl;
                std::cout << "  LocId: " << devInfo[i].LocId << std::endl;
                std::cout << "  ftHandle: " << devInfo[i].ftHandle << std::endl;
                std::cout << std::endl;
            }
        }

        delete[] devInfo;
    }

    return 0;
}
