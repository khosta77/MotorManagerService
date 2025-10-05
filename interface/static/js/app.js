// MotorControlService Web Interface
// JavaScript для управления интерфейсом

class MotorControlApp {
    constructor() {
        this.isConnected = false;
        this.currentSettings = {
            maxSpeed: 5000,
            acceleration: 2000,
            mode: 'synchronous'
        };
        this.logs = [];
        
        this.init();
    }

    init() {
        this.bindEvents();
        this.updateStatusIndicators();
        this.addLog('Система инициализирована. Ожидание подключения...', 'info');
        this.checkSystemStatus();
    }

    bindEvents() {
        // Кнопки движения отдельных моторов
        document.querySelectorAll('.move-btn').forEach(btn => {
            btn.addEventListener('click', (e) => {
                const motorRow = e.target.closest('.motor-row');
                const motorNumber = motorRow.dataset.motor;
                const checkbox = motorRow.querySelector('.motor-checkbox');
                
                if (!checkbox.checked) {
                    this.addLog(`Мотор ${motorNumber} отключен`, 'warning');
                    return;
                }
                
                const speed = parseInt(motorRow.querySelector('.speed-input').value);
                const acceleration = parseInt(motorRow.querySelector('.acceleration-input').value);
                const steps = parseInt(motorRow.querySelector('.steps-input').value);
                
                this.moveMotor(motorNumber, steps, speed, acceleration);
            });
        });

        // Групповые кнопки
        document.getElementById('move-selected-btn').addEventListener('click', () => {
            this.moveSelectedMotors();
        });

        document.getElementById('move-all-btn').addEventListener('click', () => {
            this.moveAllMotors();
        });

        document.getElementById('calibrate-btn').addEventListener('click', () => {
            this.calibrateMotors();
        });

        // Обработка изменений в полях ввода
        document.querySelectorAll('.speed-input, .acceleration-input, .steps-input').forEach(input => {
            input.addEventListener('change', () => {
                this.validateMotorSettings(input.closest('.motor-row'));
            });
        });

        // Сохранение настроек
        document.getElementById('save-settings').addEventListener('click', () => {
            this.saveSettings();
        });

        // Системные кнопки
        document.getElementById('connect-btn').addEventListener('click', () => {
            this.connectToServer();
        });

        document.getElementById('disconnect-btn').addEventListener('click', () => {
            this.disconnectFromServer();
        });

        document.getElementById('version-btn').addEventListener('click', () => {
            this.getVersion();
        });

        document.getElementById('devices-btn').addEventListener('click', () => {
            this.getDevices();
        });

        // Управление логами
        document.getElementById('clear-logs').addEventListener('click', () => {
            this.clearLogs();
        });

        document.getElementById('export-logs').addEventListener('click', () => {
            this.exportLogs();
        });

        // Модальные окна
        document.getElementById('modal-close').addEventListener('click', () => {
            this.closeModal();
        });

        // Закрытие модального окна по клику вне его
        document.getElementById('status-modal').addEventListener('click', (e) => {
            if (e.target.id === 'status-modal') {
                this.closeModal();
            }
        });

        // Обработка изменений настроек
        document.getElementById('max-speed').addEventListener('change', (e) => {
            this.currentSettings.maxSpeed = parseInt(e.target.value);
        });

        document.getElementById('acceleration').addEventListener('change', (e) => {
            this.currentSettings.acceleration = parseInt(e.target.value);
        });

        document.querySelectorAll('input[name="mode"]').forEach(radio => {
            radio.addEventListener('change', (e) => {
                this.currentSettings.mode = e.target.value;
            });
        });
    }

    async checkSystemStatus() {
        try {
            const response = await fetch('/api/system/status');
            const data = await response.json();
            
            if (data.status === 'success') {
                this.isConnected = data.connected;
                this.updateStatusIndicators();
            }
        } catch (error) {
            console.error('Ошибка проверки статуса:', error);
        }
    }

    async moveMotor(motorNumber, step, speed = 5000, acceleration = 2000) {
        this.addLog(`Движение мотора ${motorNumber} на ${step} шагов (скорость: ${speed}, ускорение: ${acceleration})`, 'info');

        try {
            const response = await fetch('/api/motor/move', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    motor: parseInt(motorNumber),
                    step: step,
                    mode: this.currentSettings.mode,
                    acceleration: acceleration,
                    maxSpeed: speed
                })
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.addLog(`Мотор ${motorNumber} успешно перемещен`, 'success');
                this.showNotification(`Мотор ${motorNumber} перемещен на ${step} шагов`, 'success');
            } else {
                this.addLog(`Ошибка движения мотора ${motorNumber}: ${data.message}`, 'error');
                this.showNotification(`Ошибка: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка связи: ${error.message}`, 'error');
            this.showNotification('Ошибка связи с сервером', 'error');
        }
    }

    async moveSelectedMotors() {
        const selectedMotors = [];
        
        document.querySelectorAll('.motor-row').forEach(row => {
            const checkbox = row.querySelector('.motor-checkbox');
            if (checkbox.checked) {
                const motorNumber = row.dataset.motor;
                const speed = parseInt(row.querySelector('.speed-input').value);
                const acceleration = parseInt(row.querySelector('.acceleration-input').value);
                const steps = parseInt(row.querySelector('.steps-input').value);
                
                selectedMotors.push({
                    number: parseInt(motorNumber),
                    acceleration: acceleration,
                    maxSpeed: speed,
                    step: steps
                });
            }
        });

        if (selectedMotors.length === 0) {
            this.addLog('Не выбрано ни одного мотора для движения', 'warning');
            return;
        }

        try {
            this.addLog(`Движение ${selectedMotors.length} выбранных моторов...`, 'info');
            
            const response = await fetch('/api/motor/move-multiple', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    motors: selectedMotors,
                    mode: this.currentSettings.mode
                })
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.addLog(`Успешно выполнено движение ${selectedMotors.length} моторов`, 'success');
                this.showNotification(`Движение ${selectedMotors.length} моторов выполнено`, 'success');
            } else {
                this.addLog(`Ошибка группового движения: ${result.message}`, 'error');
                this.showNotification(`Ошибка: ${result.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка при групповом движении: ${error.message}`, 'error');
            this.showNotification(`Ошибка: ${error.message}`, 'error');
        }
    }

    async moveAllMotors() {
        // Включаем все моторы
        document.querySelectorAll('.motor-checkbox').forEach(checkbox => {
            checkbox.checked = true;
        });
        
        // Выполняем движение всех моторов
        await this.moveSelectedMotors();
    }

    validateMotorSettings(motorRow) {
        const speed = parseInt(motorRow.querySelector('.speed-input').value);
        const acceleration = parseInt(motorRow.querySelector('.acceleration-input').value);
        const steps = parseInt(motorRow.querySelector('.steps-input').value);
        
        // Валидация значений
        if (speed < 1 || speed > 99999) {
            motorRow.querySelector('.speed-input').style.borderColor = '#f56565';
            this.addLog('Скорость должна быть от 1 до 99999', 'warning');
        } else {
            motorRow.querySelector('.speed-input').style.borderColor = '#38b2ac';
        }
        
        if (acceleration < 1 || acceleration > 99999) {
            motorRow.querySelector('.acceleration-input').style.borderColor = '#f56565';
            this.addLog('Ускорение должно быть от 1 до 99999', 'warning');
        } else {
            motorRow.querySelector('.acceleration-input').style.borderColor = '#ed8936';
        }
        
        if (isNaN(steps)) {
            motorRow.querySelector('.steps-input').style.borderColor = '#f56565';
            this.addLog('Количество шагов должно быть числом', 'warning');
        } else {
            motorRow.querySelector('.steps-input').style.borderColor = '#4299e1';
        }
    }

    async moveMotorsDiagonal(motors, steps) {
        const motorCommands = motors.map((motor, index) => ({
            number: parseInt(motor),
            acceleration: this.currentSettings.acceleration,
            maxSpeed: this.currentSettings.maxSpeed,
            step: parseInt(steps[index])
        }));

        this.addLog(`Диагональное движение: моторы ${motors.join(', ')}`, 'info');

        try {
            const response = await fetch('/api/motor/move-multiple', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    motors: motorCommands,
                    mode: 'asynchronous' // Диагональные движения всегда асинхронные
                })
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.addLog(`Диагональное движение выполнено`, 'success');
                this.showNotification('Диагональное движение выполнено', 'success');
            } else {
                this.addLog(`Ошибка диагонального движения: ${data.message}`, 'error');
                this.showNotification(`Ошибка: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка связи: ${error.message}`, 'error');
            this.showNotification('Ошибка связи с сервером', 'error');
        }
    }

    async calibrateMotors() {
        this.addLog('Начало калибровки (абсолютный ноль)', 'info');

        try {
            const response = await fetch('/api/motor/move-multiple', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    motors: [
                        { number: 1, acceleration: this.currentSettings.acceleration, maxSpeed: this.currentSettings.maxSpeed, step: -100000 },
                        { number: 2, acceleration: this.currentSettings.acceleration, maxSpeed: this.currentSettings.maxSpeed, step: -100000 },
                        { number: 3, acceleration: this.currentSettings.acceleration, maxSpeed: this.currentSettings.maxSpeed, step: -100000 },
                        { number: 4, acceleration: this.currentSettings.acceleration, maxSpeed: this.currentSettings.maxSpeed, step: -100000 }
                    ],
                    mode: 'synchronous'
                })
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.addLog('Калибровка завершена успешно', 'success');
                this.showNotification('Калибровка завершена', 'success');
            } else {
                this.addLog(`Ошибка калибровки: ${data.message}`, 'error');
                this.showNotification(`Ошибка калибровки: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка связи: ${error.message}`, 'error');
            this.showNotification('Ошибка связи с сервером', 'error');
        }
    }

    async connectToServer() {
        try {
            this.addLog('Подключение к серверу...', 'info');
            
            const response = await fetch('/api/system/connect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                }
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.isConnected = true;
                this.updateStatusIndicators();
                this.addLog('Подключение к серверу установлено', 'success');
                this.showNotification('Подключение установлено', 'success');
            } else {
                this.addLog(`Ошибка подключения: ${data.message}`, 'error');
                this.showNotification(`Ошибка подключения: ${data.message}`, 'error');
            }

        } catch (error) {
            this.addLog(`Ошибка подключения: ${error.message}`, 'error');
            this.showNotification('Ошибка подключения', 'error');
        }
    }

    async disconnectFromServer() {
        try {
            this.addLog('Отключение от сервера...', 'info');
            
            const response = await fetch('/api/system/disconnect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                }
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.isConnected = false;
                this.updateStatusIndicators();
                this.addLog('Отключение от сервера выполнено', 'info');
                this.showNotification('Отключение выполнено', 'info');
            } else {
                this.addLog(`Ошибка отключения: ${data.message}`, 'error');
                this.showNotification(`Ошибка отключения: ${data.message}`, 'error');
            }

        } catch (error) {
            this.addLog(`Ошибка отключения: ${error.message}`, 'error');
        }
    }

    async getVersion() {
        try {
            const response = await fetch('/api/system/version');
            const data = await response.json();

            if (data.status === 'success') {
                const version = data.response?.what || 'Неизвестно';
                this.showModal('Версия MCU', `Версия прошивки: ${version}`);
                this.addLog(`Версия MCU: ${version}`, 'info');
            } else {
                this.addLog(`Ошибка получения версии: ${data.message}`, 'error');
                this.showNotification(`Ошибка получения версии: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка получения версии: ${error.message}`, 'error');
            this.showNotification('Ошибка получения версии', 'error');
        }
    }

    async getDevices() {
        try {
            const response = await fetch('/api/system/devices');
            const data = await response.json();

            if (data.status === 'success') {
                const devices = data.response?.subMessage ? JSON.parse(data.response.subMessage) : {};
                const deviceList = devices.listConnect?.join(', ') || 'Не найдено';
                this.showModal('Список устройств', `Доступные устройства: ${deviceList}`);
                this.addLog(`Список устройств: ${deviceList}`, 'info');
            } else {
                this.addLog(`Ошибка получения списка устройств: ${data.message}`, 'error');
                this.showNotification(`Ошибка получения списка устройств: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка получения списка устройств: ${error.message}`, 'error');
            this.showNotification('Ошибка получения списка устройств', 'error');
        }
    }

    async saveSettings() {
        try {
            const response = await fetch('/api/settings/save', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify(this.currentSettings)
            });

            const data = await response.json();

            if (data.status === 'success') {
                this.addLog('Настройки сохранены', 'success');
                this.showNotification('Настройки сохранены', 'success');
            } else {
                this.addLog(`Ошибка сохранения настроек: ${data.message}`, 'error');
                this.showNotification(`Ошибка сохранения настроек: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка сохранения настроек: ${error.message}`, 'error');
            this.showNotification('Ошибка сохранения настроек', 'error');
        }
    }

    updateStatusIndicators() {
        const serverStatus = document.querySelector('#server-status .status-dot');
        const mcuStatus = document.querySelector('#mcu-status .status-dot');
        const deviceStatus = document.querySelector('#device-status .status-dot');

        if (this.isConnected) {
            serverStatus.className = 'status-dot online';
            mcuStatus.className = 'status-dot online';
            deviceStatus.className = 'status-dot online';
        } else {
            serverStatus.className = 'status-dot offline';
            mcuStatus.className = 'status-dot offline';
            deviceStatus.className = 'status-dot offline';
        }
    }

    addLog(message, level = 'info') {
        const timestamp = new Date().toLocaleTimeString();
        const logEntry = {
            time: timestamp,
            level: level,
            message: message
        };

        this.logs.push(logEntry);

        const logsContainer = document.getElementById('logs-container');
        const logElement = document.createElement('div');
        logElement.className = `log-entry ${level}`;
        logElement.innerHTML = `
            <span class="log-time">[${timestamp}]</span>
            <span class="log-level">${level.toUpperCase()}</span>
            <span class="log-message">${message}</span>
        `;

        logsContainer.appendChild(logElement);
        logsContainer.scrollTop = logsContainer.scrollHeight;

        // Ограничиваем количество логов
        if (this.logs.length > 100) {
            this.logs.shift();
            logsContainer.removeChild(logsContainer.firstChild);
        }
    }

    clearLogs() {
        this.logs = [];
        document.getElementById('logs-container').innerHTML = '';
        this.addLog('Логи очищены', 'info');
    }

    async exportLogs() {
        try {
            const response = await fetch('/api/logs/export');
            const data = await response.json();

            if (data.status === 'success') {
                const logText = this.logs.map(log => 
                    `[${log.time}] ${log.level.toUpperCase()}: ${log.message}`
                ).join('\n');

                const blob = new Blob([logText], { type: 'text/plain' });
                const url = URL.createObjectURL(blob);
                const a = document.createElement('a');
                a.href = url;
                a.download = `motor-control-logs-${new Date().toISOString().split('T')[0]}.txt`;
                document.body.appendChild(a);
                a.click();
                document.body.removeChild(a);
                URL.revokeObjectURL(url);

                this.addLog('Логи экспортированы', 'success');
                this.showNotification('Логи экспортированы', 'success');
            } else {
                this.addLog(`Ошибка экспорта логов: ${data.message}`, 'error');
                this.showNotification(`Ошибка экспорта логов: ${data.message}`, 'error');
            }
        } catch (error) {
            this.addLog(`Ошибка экспорта логов: ${error.message}`, 'error');
            this.showNotification('Ошибка экспорта логов', 'error');
        }
    }

    showModal(title, content) {
        document.getElementById('modal-title').textContent = title;
        document.getElementById('modal-body').innerHTML = `<p>${content}</p>`;
        document.getElementById('status-modal').classList.add('show');
    }

    closeModal() {
        document.getElementById('status-modal').classList.remove('show');
    }

    showNotification(message, type = 'info') {
        const notification = document.createElement('div');
        notification.className = `notification ${type}`;
        notification.innerHTML = `
            <div style="display: flex; align-items: center; gap: 10px;">
                <i class="fas fa-${this.getNotificationIcon(type)}"></i>
                <span>${message}</span>
            </div>
        `;

        document.getElementById('notifications').appendChild(notification);

        // Автоматическое удаление через 5 секунд
        setTimeout(() => {
            if (notification.parentNode) {
                notification.parentNode.removeChild(notification);
            }
        }, 5000);
    }

    getNotificationIcon(type) {
        const icons = {
            success: 'check-circle',
            error: 'exclamation-circle',
            warning: 'exclamation-triangle',
            info: 'info-circle'
        };
        return icons[type] || 'info-circle';
    }
}

// Инициализация приложения при загрузке страницы
document.addEventListener('DOMContentLoaded', () => {
    window.motorControlApp = new MotorControlApp();
});

// Обработка ошибок
window.addEventListener('error', (e) => {
    console.error('Ошибка JavaScript:', e.error);
    if (window.motorControlApp) {
        window.motorControlApp.addLog(`JavaScript ошибка: ${e.error.message}`, 'error');
    }
});

// Обработка необработанных промисов
window.addEventListener('unhandledrejection', (e) => {
    console.error('Необработанная ошибка Promise:', e.reason);
    if (window.motorControlApp) {
        window.motorControlApp.addLog(`Promise ошибка: ${e.reason}`, 'error');
    }
});
