import json
from datetime import datetime
from serverconnector import ServerConnector
from flask import Flask, render_template, request, jsonify

IP = "127.0.0.1"
PORT = 38000
MYNAME = "UserInterface"

app = Flask(__name__)

# Глобальный коннектор для переиспользования соединения
connector = None

def log_json(title, data):
    """Логирование JSON данных в терминал"""
    timestamp = datetime.now().strftime("%H:%M:%S.%f")[:-3]
    print(f"\n[{timestamp}] {title}")
    print("=" * 50)
    print(json.dumps(data, indent=2, ensure_ascii=False))
    print("=" * 50)

@app.route('/')
def index():
    return render_template('index.html')

def ensure_connection():
    """Обеспечивает подключение к серверу"""
    global connector
    try:
        if connector is None:
            connector = ServerConnector(IP, PORT, MYNAME)
            # Подключаемся к FT232RL устройству
            connector.send_command("listconnect", "")
            connector.send_command("reconnect", json.dumps({"deviceId": 0}))
        return True
    except Exception as e:
        connector = None
        raise Exception(f"Ошибка подключения: {e}")

@app.route('/api/motor/move', methods=['POST'])
def move_motor():
    """Движение одного мотора"""
    try:
        data = request.get_json()
        log_json("ПОЛУЧЕН JSON (движение мотора)", data)
        
        motor_number = data.get('motor')
        step = data.get('step')
        mode = data.get('mode', 'synchronous')
        
        if not motor_number or step is None:
            error_response = {
                'status': 'error',
                'message': 'Не указан номер мотора или шаг'
            }
            log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
            return jsonify(error_response), 400

        ensure_connection()
        
        motor_command = {
            "number": int(motor_number),
            "acceleration": data.get('acceleration', 2000),
            "maxSpeed": data.get('maxSpeed', 5000),
            "step": int(step)
        }

        command = {
            "command": "moving",
            "message": json.dumps({
                "mode": mode,
                "motors": [motor_command]
            })
        }
        
        log_json("ОТПРАВЛЯЕМ КОМАНДУ В MOTORCONTROLSERVICE", command)
        response = connector.send_command(command["command"], command["message"])
        log_json("ПОЛУЧЕН ОТВЕТ ОТ MOTORCONTROLSERVICE", response)
        
        success_response = {
            'status': 'success',
            'message': f'Мотор {motor_number} перемещен на {step} шагов',
            'response': response
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/motor/move-multiple', methods=['POST'])
def move_multiple_motors():
    """Движение нескольких моторов"""
    try:
        data = request.get_json()
        log_json("ПОЛУЧЕН JSON (движение нескольких моторов)", data)
        
        motors = data.get('motors', [])
        mode = data.get('mode', 'synchronous')
        
        if not motors:
            error_response = {
                'status': 'error',
                'message': 'Не указаны моторы для движения'
            }
            log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
            return jsonify(error_response), 400

        ensure_connection()
        
        motor_commands = []
        for motor in motors:
            motor_commands.append({
                "number": int(motor['number']),
                "acceleration": motor.get('acceleration', 2000),
                "maxSpeed": motor.get('maxSpeed', 5000),
                "step": int(motor['step'])
            })

        command = {
            "command": "moving",
            "message": json.dumps({
                "mode": mode,
                "motors": motor_commands
            })
        }
        
        log_json("ОТПРАВЛЯЕМ КОМАНДУ В MOTORCONTROLSERVICE", command)
        response = connector.send_command(command["command"], command["message"])
        log_json("ПОЛУЧЕН ОТВЕТ ОТ MOTORCONTROLSERVICE", response)
        
        success_response = {
            'status': 'success',
            'message': f'Выполнено движение {len(motors)} моторов',
            'response': response
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/system/connect', methods=['POST'])
def connect_to_server():
    """Подключение к серверу"""
    try:
        log_json("ПОЛУЧЕН JSON (подключение к серверу)", request.get_json() or {})
        
        ensure_connection()
        
        success_response = {
            'status': 'success',
            'message': 'Подключение к серверу установлено'
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/system/disconnect', methods=['POST'])
def disconnect_from_server():
    """Отключение от сервера"""
    global connector
    try:
        log_json("ПОЛУЧЕН JSON (отключение от сервера)", request.get_json() or {})
        
        if connector:
            connector.send_command("disconnect", "")
            connector.close()
            connector = None
        
        success_response = {
            'status': 'success',
            'message': 'Отключение от сервера выполнено'
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/system/version', methods=['GET'])
def get_version():
    """Получение версии MCU"""
    try:
        log_json("ПОЛУЧЕН ЗАПРОС (получение версии MCU)", {})
        
        ensure_connection()
        response = connector.send_command("version", "")
        log_json("ПОЛУЧЕН ОТВЕТ ОТ MOTORCONTROLSERVICE", response)
        
        success_response = {
            'status': 'success',
            'message': 'Версия получена',
            'response': response
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/system/devices', methods=['GET'])
def get_devices():
    """Получение списка устройств"""
    try:
        log_json("ПОЛУЧЕН ЗАПРОС (получение списка устройств)", {})
        
        ensure_connection()
        response = connector.send_command("listconnect", "")
        log_json("ПОЛУЧЕН ОТВЕТ ОТ MOTORCONTROLSERVICE", response)
        
        success_response = {
            'status': 'success',
            'message': 'Список устройств получен',
            'response': response
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/system/status', methods=['GET'])
def get_system_status():
    """Получение статуса системы"""
    global connector
    
    log_json("ПОЛУЧЕН ЗАПРОС (статус системы)", {})
    
    is_connected = connector is not None
    
    success_response = {
        'status': 'success',
        'connected': is_connected,
        'server': 'MotorControlService',
        'mcu': 'MockMCU' if is_connected else 'Disconnected',
        'device': 'FT232RL' if is_connected else 'Disconnected'
    }
    log_json("ОТПРАВЛЕН JSON (статус системы)", success_response)
    return jsonify(success_response)

@app.route('/api/settings/save', methods=['POST'])
def save_settings():
    """Сохранение настроек"""
    try:
        data = request.get_json()
        log_json("ПОЛУЧЕН JSON (сохранение настроек)", data)
        
        # Здесь можно сохранить настройки в файл или базу данных
        # Пока что просто возвращаем успех
        
        success_response = {
            'status': 'success',
            'message': 'Настройки сохранены',
            'settings': data
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

@app.route('/api/logs/export', methods=['GET'])
def export_logs():
    """Экспорт логов"""
    try:
        log_json("ПОЛУЧЕН ЗАПРОС (экспорт логов)", {})
        
        # Здесь можно реализовать экспорт логов
        logs = [
            f"[{datetime.now().strftime('%H:%M:%S')}] INFO: Система инициализирована",
            f"[{datetime.now().strftime('%H:%M:%S')}] INFO: Ожидание подключения..."
        ]
        
        success_response = {
            'status': 'success',
            'message': 'Логи экспортированы',
            'logs': logs
        }
        log_json("ОТПРАВЛЕН JSON (успех)", success_response)
        return jsonify(success_response)

    except Exception as e:
        error_response = {
            'status': 'error',
            'message': str(e)
        }
        log_json("ОТПРАВЛЕН JSON (ошибка)", error_response)
        return jsonify(error_response), 500

# Обработка ошибок
@app.errorhandler(404)
def not_found(error):
    return jsonify({
        'status': 'error',
        'message': 'Endpoint не найден'
    }), 404

@app.errorhandler(500)
def internal_error(error):
    return jsonify({
        'status': 'error',
        'message': 'Внутренняя ошибка сервера'
    }), 500

if __name__ == '__main__':
    print("Запуск MotorControlService Web Interface")
    print("Сервер: http://127.0.0.1:8000")
    print("API: http://127.0.0.1:8000/api/")
    print("Документация: README.md")
    app.run(port=8000, debug=True)
