#!/bin/bash

# Скрипт для запуска всей системы MotorControlService
# Автор: MotorControlService Setup
# Версия: 1.0

set -e

# Цвета для вывода
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Функции для вывода
print_header() {
    echo -e "${BLUE}================================${NC}"
    echo -e "${BLUE}$1${NC}"
    echo -e "${BLUE}================================${NC}"
}

print_success() {
    echo -e "${GREEN}✓ $1${NC}"
}

print_error() {
    echo -e "${RED}✗ $1${NC}"
}

print_warning() {
    echo -e "${YELLOW}WARNING: $1${NC}"
}

print_info() {
    echo -e "${BLUE}ℹ $1${NC}"
}

# Активация виртуального окружения
activate_venv() {
    if [ -d "venv" ]; then
        print_info "Активируем виртуальное окружение..."
        source venv/bin/activate
        print_success "Виртуальное окружение активировано"
    else
        print_warning "Виртуальное окружение не найдено. Создаем..."
        python3 -m venv venv
        source venv/bin/activate
        pip install -r requirements.txt
        print_success "Виртуальное окружение создано и зависимости установлены"
    fi
}

# Проверка зависимостей
check_dependencies() {
    print_header "ПРОВЕРКА ЗАВИСИМОСТЕЙ"
    
    # Проверка Python
    if command -v python3 &> /dev/null; then
        print_success "Python3 установлен: $(python3 --version)"
    else
        print_error "Python3 не найден. Установите Python 3.7+"
        exit 1
    fi
    
    # Активация виртуального окружения
    activate_venv
    
    # Проверка Flask
    if python -c "import flask" 2>/dev/null; then
        print_success "Flask установлен"
    else
        print_warning "Flask не найден. Устанавливаем зависимости..."
        pip install -r requirements.txt
    fi
    
    # Проверка исполняемых файлов
    if [ -f "../build/source/universal_server" ]; then
        print_success "MotorControlService собран"
    else
        print_error "MotorControlService не собран. Запустите сборку в ../build/"
        exit 1
    fi
    
    if [ -f "../mock-mcu/build/mock-mcu" ]; then
        print_success "MockMCU собран"
    else
        print_warning "MockMCU не собран. Соберите его в ../mock-mcu/build/"
    fi
}

# Установка зависимостей
install_dependencies() {
    print_header "УСТАНОВКА ЗАВИСИМОСТЕЙ"
    
    if [ -f "requirements.txt" ]; then
        print_info "Создаем виртуальное окружение..."
        python3 -m venv venv
        source venv/bin/activate
        print_info "Устанавливаем Python зависимости..."
        pip install -r requirements.txt
        print_success "Зависимости установлены"
    else
        print_error "Файл requirements.txt не найден"
        exit 1
    fi
}

# Запуск MockMCU
start_mock_mcu() {
    print_header "ЗАПУСК MOCKMCU"
    
    if [ -f "../mock-mcu/build/mock-mcu" ]; then
        print_info "Запускаем MockMCU в фоновом режиме..."
        cd ../mock-mcu/build
        ./mock-mcu -d 1 -s &
        MOCKMCU_PID=$!
        cd ../../interface
        print_success "MockMCU запущен (PID: $MOCKMCU_PID)"
        echo $MOCKMCU_PID > mockmcu.pid
    else
        print_warning "MockMCU не найден. Пропускаем запуск."
    fi
}

# Запуск MotorControlService
start_motor_service() {
    print_header "ЗАПУСК MOTORCONTROLSERVICE"
    
    if [ -f "../build/source/universal_server" ]; then
        print_info "Запускаем MotorControlService в фоновом режиме..."
        cd ../build
        ./source/universal_server &
        SERVICE_PID=$!
        cd ../interface
        print_success "MotorControlService запущен (PID: $SERVICE_PID)"
        echo $SERVICE_PID > service.pid
        
        # Ждем запуска сервера
        print_info "Ожидаем запуска сервера..."
        sleep 2
    else
        print_error "MotorControlService не найден. Соберите проект в ../build/"
        exit 1
    fi
}

# Запуск веб-интерфейса
start_web_interface() {
    print_header "ЗАПУСК ВЕБ-ИНТЕРФЕЙСА"
    
    print_info "Запускаем веб-интерфейс..."
    print_success "Веб-интерфейс доступен по адресу: http://127.0.0.1:8000"
    print_info "Нажмите Ctrl+C для остановки всех сервисов"
    
    # Запуск Flask в foreground
    python main.py
}

# Остановка всех сервисов
stop_services() {
    print_header "ОСТАНОВКА СЕРВИСОВ"
    
    # Остановка веб-интерфейса
    if [ -f "service.pid" ]; then
        SERVICE_PID=$(cat service.pid)
        if kill -0 $SERVICE_PID 2>/dev/null; then
            kill $SERVICE_PID
            print_success "MotorControlService остановлен"
        fi
        rm -f service.pid
    fi
    
    # Остановка MockMCU
    if [ -f "mockmcu.pid" ]; then
        MOCKMCU_PID=$(cat mockmcu.pid)
        if kill -0 $MOCKMCU_PID 2>/dev/null; then
            kill $MOCKMCU_PID
            print_success "MockMCU остановлен"
        fi
        rm -f mockmcu.pid
    fi
    
    print_success "Все сервисы остановлены"
}

# Обработка сигналов
trap stop_services EXIT INT TERM

# Главная функция
main() {
    print_header "ЗАПУСК СИСТЕМЫ MOTORCONTROLSERVICE"
    
    # Проверка аргументов
    case "${1:-}" in
        --install)
            install_dependencies
            exit 0
            ;;
        --stop)
            stop_services
            exit 0
            ;;
        --help)
            echo "Использование: $0 [ОПЦИИ]"
            echo ""
            echo "Опции:"
            echo "  --install    Установить зависимости"
            echo "  --stop       Остановить все сервисы"
            echo "  --help       Показать эту справку"
            echo ""
            echo "Без опций: запустить всю систему"
            exit 0
            ;;
    esac
    
    # Основной процесс запуска
    check_dependencies
    start_mock_mcu
    start_motor_service
    start_web_interface
}

# Запуск
main "$@"
