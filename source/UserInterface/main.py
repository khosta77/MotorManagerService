import json
from datetime import datetime
from serverconnector import ServerConnector
from flask import Flask, render_template, request, jsonify

IP="127.0.0.1"
PORT=38000
MYNAME="UserInterface"

app = Flask(__name__)

# Конфигурация кнопок
BUTTON_LAYOUT = [
    [10, -100,  '', -100, '', 100, 10],
    [ 1,   '', -10,  -10, 10,  '',  1],
    [ 0, -100, -10,    0, 10, 100,  0],
    [-1,   '',  10,   10, 10,  '', -1],
    [-10,  100,  '',  100, '', 100, -10]
]

DEFAULT_VALUES = {
    'maxSpeed': 20000,
    'acceleration': 10000
}

@app.route('/')
def index():
    return render_template('index.html', button_layout=BUTTON_LAYOUT, default_values=DEFAULT_VALUES)

def pushMotor(mnum, step):
    return {
        "motor" : mnum,
        "acceleration" : int(DEFAULT_VALUES['acceleration']),
        "maxSpeed" : int(DEFAULT_VALUES['maxSpeed']),
        "step" : step
    }

def pushMotors(mnums : list, step):
    return {
        "motors" : mnums,
        "acceleration" : int(DEFAULT_VALUES['acceleration']),
        "maxSpeed" : int(DEFAULT_VALUES['maxSpeed']),
        "step" : step
    }

@app.route('/button_click', methods=['POST'])
def handle_click():
    data = request.json
    value = int(data['value'])
    row = int(data['row'])
    col = int(data['col'])

    message = {
        "configureAllEngines": [],
        "startSimultaneously": [],
        "startImmediately": []
    }

    if col== 0: # Z
        message["startImmediately"].append(pushMotor(1, value))
    elif col == 6: # E
        message["startImmediately"].append(pushMotor(2, value))
    elif ((row == 2) and (col == 3)): # absloution zero
        message["configureAllEngines"].append(pushMotors([1, 2, 3, 4], -100000))
    elif row == 2: # X
        message["startImmediately"].append(pushMotor(3, value))
    elif col == 3: # Y
        message["startImmediately"].append(pushMotor(4, value))
    elif abs(row - 2) == abs(col - 3):
        '''
        Тут возможно ошибка, я криво это перемещаю по диагонали
        '''
        message["startSimultaneously"].append(pushMotor(3, value))
        message["startSimultaneously"].append(pushMotor(4, value))
    else:
        print(f"some problem {row}-{col}")

    json_message = json.dumps(message)
    message = ''

    try:
        connector_ = ServerConnector(IP, PORT, MYNAME)
        connector_.send_message(json_message)
        connector_.close()
    except:
        message = str(json_message)
    else:
        message = f'{datetime.now().strftime("[%H:%M:%S]")} Json send to {IP}:{PORT} from {MYNAME}\n'

    return jsonify({
        'status': 'success',
        'message': message,
    })

@app.route('/update-values', methods=['POST'])
def update_values():
    data = request.get_json()
    DEFAULT_VALUES['acceleration'] = data.get('value1', '')
    DEFAULT_VALUES['maxSpeed'] = data.get('value2', '')

    return jsonify({
        'status': 'success',
        'message': 'Значения обновлены',
        'maxSpeed': DEFAULT_VALUES['maxSpeed'],
        'acceleration': DEFAULT_VALUES['acceleration']
    })

if __name__ == '__main__':
    app.run(port=8000)
