from flask import Flask, jsonify, request
from flask_cors import CORS

app = Flask(__name__)
CORS(app)

# Mock data
config = {
    "wifi": {
        "mode": 1,
        "ssid": "TestAP",
        "channel": 1,
        "hidden": False
    },
    "message": {
        "active": 1,
        "count": 2
    },
    "messages": [
        {"id": 1, "text": "Test Message 1"},
        {"id": 2, "text": "Test Message 2"}
    ],
    "volume": 50
}

@app.route('/api/config', methods=['GET'])
def get_config():
    return jsonify(config)

@app.route('/api/message', methods=['POST'])
def handle_message():
    data = request.get_json()
    action = data.get('action')
    
    if action == 'add':
        new_id = len(config['messages']) + 1
        config['messages'].append({"id": new_id, "text": f"New Message {new_id}"})
    elif action == 'remove':
        msg_id = data.get('messageNum')
        config['messages'] = [m for m in config['messages'] if m['id'] != msg_id]
    elif action == 'update':
        msg_id = data.get('messageNum')
        new_text = data.get('text')
        for msg in config['messages']:
            if msg['id'] == msg_id:
                msg['text'] = new_text
    elif action == 'setActive':
        config['message']['active'] = data.get('messageNum')
    
    return jsonify({"status": "success"})

@app.route('/api/volume', methods=['POST'])
def handle_volume():
    data = request.get_json()
    config['volume'] = data.get('volume', 50)
    return jsonify({"status": "success"})

@app.route('/api/wifi', methods=['POST'])
def handle_wifi():
    data = request.get_json()
    config['wifi'].update(data)
    return jsonify({"status": "success"})

@app.route('/api/status', methods=['GET'])
def get_status():
    return jsonify({"status": "ok"})

if __name__ == '__main__':
    app.run(host='0.0.0.0', port=5000)
