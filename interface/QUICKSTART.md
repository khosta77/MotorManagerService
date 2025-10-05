# 🚀 Быстрый старт MotorControlService

## 📦 Установка (один раз)

```bash
cd interface
./start_system.sh --install
```

## 🎮 Запуск системы

### Автоматический запуск (рекомендуется)
```bash
cd interface
./start_system.sh
```

### Ручной запуск
```bash
# Активация виртуального окружения
cd interface
source venv/bin/activate

# Терминал 1: MockMCU
cd ../mock-mcu/build && ./mock-mcu -d 1 -s

# Терминал 2: MotorControlService  
cd ../../build && ./source/universal_server

# Терминал 3: Веб-интерфейс
cd ../interface && python main.py
```

## 🌐 Использование

1. Откройте браузер: `http://127.0.0.1:8000`
2. Нажмите кнопки для управления моторами
3. Настройте скорость и ускорение

## 🧪 Тестирование

```bash
source venv/bin/activate
python test_interface.py
```

## 🛑 Остановка

```bash
./start_system.sh --stop
```

## ❓ Помощь

```bash
./start_system.sh --help
```

## 🔧 Устранение неполадок

### Ошибка "externally-managed-environment"
```bash
# Используйте виртуальное окружение
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
```

### Ошибка подключения к серверу
- Убедитесь, что MotorControlService запущен
- Проверьте порт 38000: `lsof -i :38000`
- Перезапустите систему: `./start_system.sh --stop && ./start_system.sh`
