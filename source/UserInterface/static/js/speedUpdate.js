document.addEventListener('DOMContentLoaded', function() {
    const updateBtn = document.getElementById('update-btn');
    const value1Input = document.getElementById('maxSpeed');
    const value2Input = document.getElementById('acceleration');
    const displayValue1 = document.getElementById('display-maxSpeed');
    const displayValue2 = document.getElementById('display-acceleration');

    updateBtn.addEventListener('click', function() {
        // Получаем значения из полей ввода
        const val1 = value1Input.value;
        const val2 = value2Input.value;
        
        // Отображаем значения на странице
        displayValue1.textContent = val1;
        displayValue2.textContent = val2;
        
        // Очищаем поля ввода
        value1Input.value = '';
        value2Input.value = '';
        
        // Отправляем данные на сервер
        fetch('/update-values', {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify({
                value1: val1,
                value2: val2
            })
        })
        .then(response => response.json())
        .then(data => {
            console.log('Success:', data);
        })
        .catch((error) => {
            console.error('Error:', error);
        });
    });
});
