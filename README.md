# MotorManagerService

Микросервис для управления шаговыми двигателями через TCP-сокеты с веб-интерфейсом. Поддерживает подключение к MCU через FT232RL модуль и предоставляет RESTful API для управления моторами.

## Возможности

- Управление до 10 шаговыми двигателями
- Синхронное и асинхронное движение моторов
- Веб-интерфейс для управления через браузер
- RESTful API для интеграции
- Подключение к MCU через FT232RL
- Мониторинг состояния системы в реальном времени

## Сборка & Запуск

Проект собирается и компилируется! Проверил на `mac OS 15`.

1. Работа с Json реализована через [nlohmann/json](https://github.com/nlohmann/json?ysclid=m9h6e6grnw955784922)
 просто:

```bash
git clone git@github.com:nlohmann/json.git
```

2. Надо проверить зависимости, должен быть `cmake>=3.28`, `clang++>=16.0`, `gtest` и `boost` (На `mac OS`
установлен через homebrew) и еще

* cmake `cmake --version`

* make `make --version`

* lcov `lcov --version`

* genhtml `genhtml --version`

* clang++ `clang++ --version`

3. Сборка & скомпилировать все

```bash
cmake -DENABLE_COVERAGE=OFF -B build
cd build
make
```

* Если собирать без маркера `-DENABLE_COVERAGE=ON` - будет треш в терминале

4. Подготовка файлов покрытия, если `-DENABLE_COVERAGE=ON`. 

```bash
make coverage
```

5. Вызов программы:

```bash
./source/universal_server
```

## Веб-интерфейс

### Быстрый запуск

```bash
cd interface
./start_system.sh
```

Откройте браузер: `http://127.0.0.1:8000`

### Ручной запуск

1. **Сборка MockMCU** (заглушка для тестирования):
```bash
cd mock-mcu
mkdir -p build
cd build
cmake ..
make
```

2. **Запуск MockMCU**:
```bash
cd mock-mcu/build && ./mock-mcu -d 1 -s
```

3. **MotorManagerService**:
```bash
cd build && ./source/universal_server
```

4. **Создание виртуального окружения**:
```bash
cd interface
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

5. **Веб-интерфейс**:
```bash
cd interface
source venv/bin/activate
python main.py
```

Подробная документация: [interface/README.md](interface/README.md)

## TODO, коды с задачи, которые в бэклоге.

* [ ] #001 Прописать коды ошибок при передаче

* [ ] #002 Возвращать ошибку, если пришла не корректная команда

* [ ] #003 Проблемы с тестом, нужно исправить
