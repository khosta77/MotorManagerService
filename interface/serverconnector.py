import socket
import json
import random

class ValidationError(Exception):
    pass

class ServerConnector:
    def __init__(self, ip=None, port=None, name=None):
        self.ip = ip
        self.port = port
        self.name = name
        self.socket = None
        if ip and port:
            self.connect(ip, port)
            if name:
                self.send_name(name)
        self.is_connected()

    def connect(self, ip, port):
        self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        self.socket.settimeout(5)  # Увеличиваем таймаут
        try:
            self.socket.connect((ip, port))
            print(f"Connected to {ip}:{port}")
        except (socket.timeout, ConnectionRefusedError, OSError) as e:
            self.close()
            raise ValidationError(f"Connection failed: {e}")

    def is_connected(self):
        """Проверяет, активно ли соединение."""
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')
        try:
            self.socket.getpeername()
        except OSError:
            raise ValidationError('Socket is not connected')

    def send_name(self, name):
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')

        name_to_send = name if name else self.name
        if not name_to_send:
            raise ValidationError(f'Name to send had problem {name_to_send}')

        data = json.dumps({"name": name_to_send}, ensure_ascii=True) + "\n\n"
        self.socket.sendall(data.encode('utf-8'))
        print(f"Sent name: {name_to_send}")

    def send_command(self, command, message):
        """Отправляет команду и возвращает ответ"""
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')

        message_id = random.randint(0, 999999)
        
        # Создаем команду как строку напрямую, избегая двойного JSON encoding
        command_text = f'{{"command": "{command}", "message": "{message}"}}'
        
        # Создаем внешний объект
        outer_command = {"id": message_id, "text": command_text}
        
        # Сериализуем в JSON
        data = json.dumps(outer_command, ensure_ascii=True) + "\n\n"
        
        print(f"Sending command: {command}")
        print(f"Message: {message}")
        print(f"Data: {data}")
        
        # Отправляем как UTF-8
        try:
            self.socket.sendall(data.encode('utf-8'))
        except (ConnectionResetError, BrokenPipeError, OSError) as e:
            raise ValidationError(f'Connection lost during send: {e}')
        
        # Читаем ответ
        response = self.receive_response()
        return response

    def receive_response(self):
        """Читает ответ от сервера"""
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')
        
        response_data = b""
        while True:
            try:
                chunk = self.socket.recv(1024)
                if not chunk:
                    break
                response_data += chunk
                if response_data.endswith(b'\n\n'):
                    break
            except socket.timeout:
                break
        
        if response_data:
            try:
                # Пытаемся декодировать как UTF-8
                response_str = response_data.decode('utf-8').strip()
                print(f"Received response: {response_str}")
                try:
                    return json.loads(response_str)
                except json.JSONDecodeError as e:
                    print(f"JSON decode error: {e}")
                    return {"raw_response": response_str, "error": "Invalid JSON"}
            except UnicodeDecodeError as e:
                print(f"UTF-8 decode error: {e}")
                # Пытаемся декодировать как latin-1 и конвертировать
                try:
                    response_str = response_data.decode('latin-1').strip()
                    print(f"Received response (latin-1): {response_str}")
                    return json.loads(response_str)
                except (json.JSONDecodeError, UnicodeDecodeError):
                    return {"raw_response": response_data.hex(), "error": "Invalid encoding"}
        else:
            return {"error": "No response received"}

    def send_message(self, text):
        """Старый метод для совместимости"""
        return self.send_command("moving", text)

    def close(self):
        if self.socket:
            self.socket.close()
            self.socket = None
