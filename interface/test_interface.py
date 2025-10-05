#!/usr/bin/env python3
"""
Тестовый скрипт для проверки работы обновленного интерфейса
"""

import requests
import json
import time

BASE_URL = "http://127.0.0.1:8000"

def test_connection():
    """Тестирует подключение к веб-интерфейсу"""
    try:
        response = requests.get(BASE_URL)
        if response.status_code == 200:
            print("✅ Веб-интерфейс доступен")
            return True
        else:
            print(f"❌ Веб-интерфейс недоступен: {response.status_code}")
            return False
    except Exception as e:
        print(f"❌ Ошибка подключения к веб-интерфейсу: {e}")
        return False

def test_version():
    """Тестирует получение версии MCU"""
    try:
        response = requests.post(f"{BASE_URL}/version")
        data = response.json()
        print(f"📋 Версия MCU: {data}")
        return data['status'] == 'success'
    except Exception as e:
        print(f"❌ Ошибка получения версии: {e}")
        return False

def test_list_devices():
    """Тестирует получение списка устройств"""
    try:
        response = requests.post(f"{BASE_URL}/list_devices")
        data = response.json()
        print(f"📋 Список устройств: {data}")
        return data['status'] == 'success'
    except Exception as e:
        print(f"❌ Ошибка получения списка устройств: {e}")
        return False

def test_motor_movement():
    """Тестирует движение моторов"""
    try:
        # Тест движения по X-оси (кнопка в центре)
        response = requests.post(f"{BASE_URL}/button_click", 
                               json={"value": 100, "row": 2, "col": 2})
        data = response.json()
        print(f"🎮 Движение моторов: {data}")
        return data['status'] == 'success'
    except Exception as e:
        print(f"❌ Ошибка движения моторов: {e}")
        return False

def main():
    print("🧪 Тестирование обновленного интерфейса MotorControlService")
    print("=" * 60)
    
    # Проверяем доступность веб-интерфейса
    if not test_connection():
        print("❌ Веб-интерфейс недоступен. Убедитесь, что он запущен на порту 8000")
        return
    
    print("\n📡 Тестирование команд сервера:")
    
    # Тест получения версии
    print("\n1. Тест получения версии MCU:")
    test_version()
    
    # Тест получения списка устройств
    print("\n2. Тест получения списка устройств:")
    test_list_devices()
    
    # Тест движения моторов
    print("\n3. Тест движения моторов:")
    test_motor_movement()
    
    print("\n✅ Тестирование завершено!")

if __name__ == "__main__":
    main()
