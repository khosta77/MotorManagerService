document.addEventListener('DOMContentLoaded', function() {
    const buttons = document.querySelectorAll('.grid-button');
    const logContainer = document.getElementById('logContainer');
    
    buttons.forEach(button => {
        button.addEventListener('click', function() {
            const value = this.dataset.value;
            const row = this.dataset.row;
            const col = this.dataset.col;
            
            // Отправка данных на сервер
            fetch('/button_click', {
                method: 'POST',
                headers: {
                    'Content-Type': 'application/json',
                },
                body: JSON.stringify({
                    value: value,
                    row: row,
                    col: col
                })
            })
            .then(response => response.json())
            .then(data => {
                // Добавляем запись в лог
                const logEntry = document.createElement('div');
                logEntry.className = 'log-entry';
                logEntry.textContent = data.message;
                logContainer.appendChild(logEntry);
                logContainer.scrollTop = logContainer.scrollHeight;
            })
            .catch(error => {
                console.error('Error:', error);
            });
        });
    });
});
