class MotorControlApp {
    constructor() {
        this.serverIP = '127.0.0.1';
        this.serverPort = 38000;
        this.isConnected = false;
        this.deviceId = 0;
        this.deviceName = 'Не подключено';
        this.deviceVersion = 'Неизвестно';
        this.devices = [];
        this.motors = [];
        
        // Флаги для предотвращения множественных подключений
        this.isConnecting = false;
        this.isDisconnecting = false;
        this.isConnectingDevice = false;
        
        this.init();
    }

    init() {
        this.bindEvents();
        this.createMotorsTable();
        this.checkSystemStatus();
        this.updateUI();
    }

    bindEvents() {
        // Подключение к серверу
        document.getElementById('connectBtn').addEventListener('click', () => {
            this.connectToServer();
        });

        // Обновление устройств
        document.getElementById('refreshDevices').addEventListener('click', () => {
            this.refreshDevices();
        });

        // Подключение к устройству
        document.getElementById('connectDevice').addEventListener('click', () => {
            this.connectToDevice();
        });

        // Отключение от устройства
        document.getElementById('disconnectDevice').addEventListener('click', () => {
            this.disconnectFromDevice();
        });

        // Получение версии
        document.getElementById('getVersion').addEventListener('click', () => {
            this.getVersion();
        });

        // Глобальные команды
        document.getElementById('moveSelectedSync').addEventListener('click', () => {
            this.moveSelectedMotors('synchronous');
        });

        document.getElementById('moveSelectedAsync').addEventListener('click', () => {
            this.moveSelectedMotors('asynchronous');
        });

        document.getElementById('stopAll').addEventListener('click', () => {
            this.stopAllMotors();
        });

        // Очистка логов
        document.getElementById('clearLogs').addEventListener('click', () => {
            this.clearLogs();
        });

        // Изменение IP/PORT
        document.getElementById('serverIP').addEventListener('change', (e) => {
            this.serverIP = e.target.value;
        });

        document.getElementById('serverPort').addEventListener('change', (e) => {
            this.serverPort = parseInt(e.target.value);
        });
    }

    createMotorsTable() {
        const tbody = document.getElementById('motorsTableBody');
        tbody.innerHTML = '';

        for (let i = 1; i <= 10; i++) {
            const row = document.createElement('tr');
            row.innerHTML = `
                <td>
                    <input type="checkbox" class="motor-checkbox" id="motor${i}Check" data-motor="${i}">
                </td>
                <td>
                    <span class="motor-number">Мотор ${i}</span>
                </td>
                <td>
                    <input type="number" class="motor-input" id="motor${i}Acceleration" value="2000" min="1" max="10000">
                </td>
                <td>
                    <input type="number" class="motor-input" id="motor${i}MaxSpeed" value="5000" min="1" max="20000">
                </td>
                <td>
                    <input type="number" class="motor-input" id="motor${i}Step" value="0" min="-10000" max="10000">
                </td>
                <td>
                    <button class="btn btn-primary motor-move-btn" onclick="motorControlApp.moveSingleMotor(${i})">
                        <i class="fas fa-play"></i> Запустить
                    </button>
                </td>
            `;
            tbody.appendChild(row);
        }
    }

    async connectToServer() {
        if (this.isConnecting) {
            this.showNotification('Подключение уже выполняется...', 'warning');
            return;
        }
        
        this.isConnecting = true;
        try {
            this.log('Попытка подключения к серверу...', 'info');
            
            const response = await fetch('/api/system/connect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    ip: this.serverIP,
                    port: this.serverPort
                })
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.isConnected = true;
                this.log('Подключение к серверу установлено', 'success');
                this.showNotification('Подключение к серверу установлено', 'success');
            } else {
                throw new Error(result.message || 'Ошибка подключения');
            }
        } catch (error) {
            this.log(`Ошибка подключения к серверу: ${error.message}`, 'error');
            this.showNotification(`Ошибка подключения: ${error.message}`, 'error');
        } finally {
            this.isConnecting = false;
        }
        
        this.updateUI();
    }

    async refreshDevices() {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        try {
            this.log('Обновление списка устройств...', 'info');
            
            const response = await fetch('/api/system/devices');
            const result = await response.json();
            
            if (result.status === 'success') {
                this.devices = result.devices || [];
                this.log(`Найдено устройств: ${this.devices.length}`, 'success');
                this.showNotification(`Найдено устройств: ${this.devices.length}`, 'success');
                
                // Обновляем отображение устройств
                this.updateDeviceDisplay();
            } else {
                throw new Error(result.message || 'Ошибка получения устройств');
            }
        } catch (error) {
            this.log(`Ошибка получения устройств: ${error.message}`, 'error');
            this.showNotification(`Ошибка получения устройств: ${error.message}`, 'error');
        }
    }

    updateDeviceDisplay() {
        // Создаем выпадающий список устройств
        const deviceSelect = document.createElement('select');
        deviceSelect.id = 'deviceSelect';
        deviceSelect.innerHTML = '<option value="">Выберите устройство...</option>';
        
        this.devices.forEach((device, index) => {
            const option = document.createElement('option');
            option.value = index;
            option.textContent = `ID: ${index} - ${this.cleanDeviceName(device)}`;
            deviceSelect.appendChild(option);
        });

        // Заменяем старый элемент или добавляем новый
        const existingSelect = document.getElementById('deviceSelect');
        if (existingSelect) {
            existingSelect.parentNode.replaceChild(deviceSelect, existingSelect);
        } else {
            const deviceActions = document.querySelector('.device-actions');
            deviceActions.insertBefore(deviceSelect, deviceActions.firstChild);
        }

        // Добавляем обработчик изменения выбора
        deviceSelect.addEventListener('change', (e) => {
            this.deviceId = parseInt(e.target.value);
            this.updateUI();
        });
    }

    cleanDeviceName(deviceName) {
        // Очищаем имя устройства от непечатаемых символов
        return deviceName.replace(/[\x00-\x1F\x7F-\x9F]/g, '').substring(0, 50);
    }

    async connectToDevice() {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        if (this.isConnectingDevice) {
            this.showNotification('Подключение к устройству уже выполняется...', 'warning');
            return;
        }

        this.isConnectingDevice = true;
        try {
            this.log(`Подключение к устройству ID: ${this.deviceId}`, 'info');
            
            const response = await fetch('/api/system/connect-device', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    deviceId: this.deviceId
                })
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.deviceName = result.deviceName || 'Подключено';
                this.log(`Подключено к устройству: ${this.deviceName}`, 'success');
                this.showNotification(`Подключено к устройству: ${this.deviceName}`, 'success');
            } else {
                throw new Error(result.message || 'Ошибка подключения к устройству');
            }
        } catch (error) {
            this.log(`Ошибка подключения к устройству: ${error.message}`, 'error');
            this.showNotification(`Ошибка подключения к устройству: ${error.message}`, 'error');
        } finally {
            this.isConnectingDevice = false;
        }
        
        this.updateUI();
    }

    async disconnectFromDevice() {
        if (this.isDisconnecting) {
            this.showNotification('Отключение от устройства уже выполняется...', 'warning');
            return;
        }

        this.isDisconnecting = true;
        try {
            this.log('Отключение от устройства...', 'info');
            
            const response = await fetch('/api/system/disconnect-device', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                }
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.deviceName = 'Не подключено';
                this.deviceId = 0;
                this.deviceVersion = 'Неизвестно';
                this.log('Отключено от устройства', 'success');
                this.showNotification('Отключено от устройства', 'success');
            } else {
                throw new Error(result.message || 'Ошибка отключения');
            }
        } catch (error) {
            this.log(`Ошибка отключения: ${error.message}`, 'error');
            this.showNotification(`Ошибка отключения: ${error.message}`, 'error');
        } finally {
            this.isDisconnecting = false;
        }
        
        this.updateUI();
    }

    async getVersion() {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        try {
            this.log('Получение версии MCU...', 'info');
            
            const response = await fetch('/api/system/version');
            const result = await response.json();
            
            if (result.status === 'success') {
                // Извлекаем версию из ответа
                let version = 'Неизвестно';
                if (result.response && result.response.subMessage) {
                    try {
                        const versionData = JSON.parse(result.response.subMessage);
                        if (versionData.version) {
                            version = versionData.version;
                        }
                    } catch (e) {
                        // Если не удалось распарсить, используем raw ответ
                        version = result.response.subMessage;
                    }
                }
                
                this.deviceVersion = version;
                this.log(`Версия MCU: ${version}`, 'success');
                this.showNotification(`Версия MCU: ${version}`, 'success');
        } else {
                // Ошибка от сервера - показываем реальное сообщение об ошибке
                this.log(`Ошибка получения версии: ${result.message}`, 'error');
                this.showNotification(`Ошибка получения версии: ${result.message}`, 'error');
                this.deviceVersion = 'Ошибка';
            }
        } catch (error) {
            this.log(`Ошибка получения версии: ${error.message}`, 'error');
            this.showNotification(`Ошибка получения версии: ${error.message}`, 'error');
            this.deviceVersion = 'Ошибка';
        }
        
        this.updateUI();
    }

    async moveSingleMotor(motorNumber) {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        const acceleration = parseInt(document.getElementById(`motor${motorNumber}Acceleration`).value);
        const maxSpeed = parseInt(document.getElementById(`motor${motorNumber}MaxSpeed`).value);
        const step = parseInt(document.getElementById(`motor${motorNumber}Step`).value);

        try {
            this.log(`Движение мотора ${motorNumber}: шаг=${step}, скорость=${maxSpeed}, ускорение=${acceleration}`, 'info');
            
            const response = await fetch('/api/motor/move', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    motorNumber: motorNumber,
                    step: step,
                    maxSpeed: maxSpeed,
                    acceleration: acceleration
                })
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.log(`Мотор ${motorNumber} запущен`, 'success');
                this.showNotification(`Мотор ${motorNumber} запущен`, 'success');
            } else {
                throw new Error(result.message || 'Ошибка движения мотора');
            }
        } catch (error) {
            this.log(`Ошибка движения мотора ${motorNumber}: ${error.message}`, 'error');
            this.showNotification(`Ошибка движения мотора ${motorNumber}: ${error.message}`, 'error');
        }
    }

    async moveSelectedMotors(mode) {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        const selectedMotors = [];
        
        for (let i = 1; i <= 10; i++) {
            const checkbox = document.getElementById(`motor${i}Check`);
            if (checkbox.checked) {
                const acceleration = parseInt(document.getElementById(`motor${i}Acceleration`).value);
                const maxSpeed = parseInt(document.getElementById(`motor${i}MaxSpeed`).value);
                const step = parseInt(document.getElementById(`motor${i}Step`).value);
                
                selectedMotors.push({
                    number: i,
                    acceleration: acceleration,
                    maxSpeed: maxSpeed,
                    step: step
                });
            }
        }

        if (selectedMotors.length === 0) {
            this.showNotification('Выберите хотя бы один мотор', 'warning');
            return;
        }

        try {
            this.log(`Движение ${selectedMotors.length} моторов в ${mode} режиме`, 'info');
            
            const response = await fetch('/api/motor/move-multiple', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    mode: mode,
                    motors: selectedMotors
                })
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.log(`Выполнено движение ${selectedMotors.length} моторов в ${mode} режиме`, 'success');
                this.showNotification(`Выполнено движение ${selectedMotors.length} моторов`, 'success');
            } else {
                throw new Error(result.message || 'Ошибка движения моторов');
            }
        } catch (error) {
            this.log(`Ошибка движения моторов: ${error.message}`, 'error');
            this.showNotification(`Ошибка движения моторов: ${error.message}`, 'error');
        }
    }

    async stopAllMotors() {
        if (!this.isConnected) {
            this.showNotification('Сначала подключитесь к серверу', 'warning');
            return;
        }

        try {
            this.log('Остановка всех моторов...', 'info');
            
            const response = await fetch('/api/motor/stop-all', {
                method: 'POST'
            });

            const result = await response.json();
            
            if (result.status === 'success') {
                this.log('Все моторы остановлены', 'success');
                this.showNotification('Все моторы остановлены', 'success');
            } else {
                throw new Error(result.message || 'Ошибка остановки моторов');
            }
        } catch (error) {
            this.log(`Ошибка остановки моторов: ${error.message}`, 'error');
            this.showNotification(`Ошибка остановки моторов: ${error.message}`, 'error');
        }
    }

    async checkSystemStatus() {
        try {
            const response = await fetch('/api/system/status');
            const result = await response.json();
            
            if (result.status === 'success') {
                this.isConnected = result.connected;
                document.getElementById('serverStatusText').textContent = result.server || 'Неизвестно';
                document.getElementById('mcuStatus').textContent = result.mcu || 'Неизвестно';
                document.getElementById('deviceStatusText').textContent = result.device || 'Неизвестно';
            }
        } catch (error) {
            this.log(`Ошибка проверки статуса: ${error.message}`, 'error');
        }
        
        this.updateUI();
    }

    updateUI() {
        // Обновление статуса подключения к серверу
        const serverStatus = document.getElementById('serverStatus');
        const connectBtn = document.getElementById('connectBtn');
        
        if (this.isConnected) {
            serverStatus.innerHTML = '<i class="fas fa-circle"></i> Подключен';
            serverStatus.className = 'status-indicator connected';
            connectBtn.innerHTML = '<i class="fas fa-unlink"></i> Отключиться';
            connectBtn.onclick = () => this.disconnectFromServer();
        } else {
            serverStatus.innerHTML = '<i class="fas fa-circle"></i> Отключен';
            serverStatus.className = 'status-indicator';
            connectBtn.innerHTML = '<i class="fas fa-plug"></i> Подключиться';
            connectBtn.onclick = () => this.connectToServer();
        }

        // Блокируем кнопку подключения во время операций
        connectBtn.disabled = this.isConnecting || this.isDisconnecting;

        // Обновление информации об устройстве
        document.getElementById('deviceId').textContent = this.deviceId;
        document.getElementById('deviceName').textContent = this.deviceName;
        document.getElementById('deviceVersion').textContent = this.deviceVersion;

        // Обновление статусов
        const serverStatusText = document.getElementById('serverStatusText');
        const deviceStatusText = document.getElementById('deviceStatusText');
        
        if (this.isConnected) {
            serverStatusText.textContent = 'Подключен';
            serverStatusText.className = 'status-value connected';
        } else {
            serverStatusText.textContent = 'Отключен';
            serverStatusText.className = 'status-value disconnected';
        }

        if (this.deviceName !== 'Не подключено') {
            deviceStatusText.textContent = 'Подключено';
            deviceStatusText.className = 'status-value connected';
        } else {
            deviceStatusText.textContent = 'Отключено';
            deviceStatusText.className = 'status-value disconnected';
        }

        // Обновление доступности кнопок
        const refreshBtn = document.getElementById('refreshDevices');
        const getVersionBtn = document.getElementById('getVersion');
        const connectDeviceBtn = document.getElementById('connectDevice');
        const disconnectDeviceBtn = document.getElementById('disconnectDevice');
        
        refreshBtn.disabled = !this.isConnected || this.isConnecting || this.isDisconnecting;
        getVersionBtn.disabled = !this.isConnected || this.isConnecting || this.isDisconnecting;
        connectDeviceBtn.disabled = !this.isConnected || this.isConnecting || this.isDisconnecting || this.isConnectingDevice;
        disconnectDeviceBtn.disabled = !this.isConnected || this.deviceName === 'Не подключено' || this.isConnecting || this.isDisconnecting;
    }

    async disconnectFromServer() {
        if (this.isDisconnecting) {
            this.showNotification('Отключение уже выполняется...', 'warning');
            return;
        }
        
        this.isDisconnecting = true;
        try {
            this.log('Отключение от сервера...', 'info');
            
            const response = await fetch('/api/system/disconnect', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                }
            });

            const result = await response.json();

            if (result.status === 'success') {
                this.isConnected = false;
                this.deviceName = 'Не подключено';
                this.deviceId = 0;
                this.deviceVersion = 'Неизвестно';
                this.devices = [];
                this.log('Отключено от сервера', 'success');
                this.showNotification('Отключено от сервера', 'success');
            } else {
                throw new Error(result.message || 'Ошибка отключения от сервера');
            }
        } catch (error) {
            this.log(`Ошибка отключения от сервера: ${error.message}`, 'error');
            this.showNotification(`Ошибка отключения: ${error.message}`, 'error');
        } finally {
            this.isDisconnecting = false;
        }
        
        this.updateUI();
    }

    clearLogs() {
        const logsContainer = document.getElementById('logsContainer');
        logsContainer.innerHTML = `
            <div class="log-entry info">
                <span class="log-time">[${new Date().toLocaleTimeString()}]</span>
                <span class="log-level">INFO</span>
                <span class="log-message">Логи очищены</span>
            </div>
        `;
    }

    log(message, level = 'info') {
        const logsContainer = document.getElementById('logsContainer');
        const logEntry = document.createElement('div');
        logEntry.className = `log-entry ${level}`;
        logEntry.innerHTML = `
            <span class="log-time">[${new Date().toLocaleTimeString()}]</span>
            <span class="log-level">${level.toUpperCase()}</span>
            <span class="log-message">${message}</span>
        `;

        logsContainer.appendChild(logEntry);
        logsContainer.scrollTop = logsContainer.scrollHeight;
    }

    showNotification(message, type = 'info') {
        const container = document.getElementById('notifications');
        const notification = document.createElement('div');
        notification.className = `notification ${type}`;
        notification.textContent = message;
        
        container.appendChild(notification);
        
        setTimeout(() => {
            notification.remove();
        }, 5000);
    }
}

// Инициализация приложения
document.addEventListener('DOMContentLoaded', () => {
    window.motorControlApp = new MotorControlApp();
});