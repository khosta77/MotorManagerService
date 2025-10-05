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
        self.socket.settimeout(1)  # Таймаут на подключение
        try:
            self.socket.connect((ip, port))
            #print(f"Connected to {ip}:{port}")
        except (socket.timeout, ConnectionRefusedError, OSError) as e:
            self.close()
            raise ValidationError(f"Connection failed: {e}")

    def is_connected(self):
        """Проверяет, активно ли соединение."""
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')

        self.socket.getpeername()

    def send_name(self, name):
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')

        name_to_send = name if name else self.name
        if not name_to_send:
            raise ValidationError(f'Name to send had problem {name_to_send}')

        data = json.dumps({"name": name_to_send}) + "\n\n"
        self.socket.sendall(data.encode('utf-8'))

    def send_message(self, text):
        if not self.socket:
            raise ValidationError(f'Not socket {self.socket}')

        message_id = random.randint(0, 999999)  # Случайное 6-значное число (как в C++)
        data = json.dumps({"id": message_id, "text": text}) + "\n\n"
        print(text)
        print(str(data))
        self.socket.sendall(data.encode('utf-8'))

    def close(self):
        if self.socket:
            self.socket.close()
            self.socket = None
