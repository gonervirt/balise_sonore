let config = null;

// Fetch and update configuration
async function fetchConfig() {
    try {
        const response = await fetch('/api/config');
        config = await response.json();
        updateAllTabs();
    } catch (error) {
        console.error('Error fetching config:', error);
    }
}

function updateAllTabs() {
    updateHomeTab();
    updateWifiTab();
    updateMessageTab();
    updateSystemTab();
}

function updateHomeTab() {
    const content = document.getElementById('homeContent');
    content.innerHTML = `
        <div class="config-item">
            <div class="config-label">WiFi Mode</div>
            <div class="config-value">${config.wifi.mode ? 'Access Point' : 'Station'}</div>
        </div>
        <div class="config-item">
            <div class="config-label">SSID</div>
            <div class="config-value">${config.wifi.ssid}</div>
        </div>
        <div class="config-item">
            <div class="config-label">Active Message</div>
            <div class="config-value">${config.message.active}</div>
        </div>
        <div class="config-item">
            <div class="config-label">Volume</div>
            <div class="config-value">${config.volume}</div>
        </div>
    `;
}

function updateWifiTab() {
    document.querySelector('#wifiStatus').innerHTML = `
        <div class="config-item">
            <div class="config-label">Current Mode</div>
            <div class="config-value">${config.wifi.mode ? 'Access Point' : 'Station'}</div>
        </div>
        <div class="config-item">
            <div class="config-label">SSID</div>
            <div class="config-value">${config.wifi.ssid}</div>
        </div>
        <div class="config-item">
            <div class="config-label">Channel</div>
            <div class="config-value">${config.wifi.channel}</div>
        </div>
        <div class="config-item">
            <div class="config-label">Hidden</div>
            <div class="config-value">${config.wifi.hidden ? 'Yes' : 'No'}</div>
        </div>
    `;
}

function updateMessageTab() {
    const messageList = document.getElementById('messageList');
    messageList.innerHTML = config.messages.map(msg => `
        <div class="msg-container">
            <input type="radio" class="msg-radio" name="messageNum" value="${msg.id}" 
                   id="msg${msg.id}" ${msg.id === config.message.active ? 'checked' : ''}>
            <label class="msg-label" for="msg${msg.id}">Message ${msg.id}: ${msg.text}</label>
        </div>
    `).join('');
    
    document.getElementById('volume').value = config.volume;
}

function updateSystemTab() {
    document.getElementById('systemInfo').innerHTML = `
        <div class="config-item">
            <div class="config-label">Active Messages</div>
            <div class="config-value">${config.message.count}</div>
        </div>
        <div class="config-item">
            <div class="config-label">WiFi Mode</div>
            <div class="config-value">${config.wifi.mode ? 'Access Point' : 'Station'}</div>
        </div>
    `;
}

// Tab management
function showTab(tabName) {
    document.querySelectorAll('.tab-content').forEach(tab => {
        tab.classList.remove('active');
    });
    document.getElementById(tabName).classList.add('active');
}

// API interactions
async function handleWifiSubmit(event) {
    event.preventDefault();
    const formData = new FormData(event.target);
    const data = {
        ssid: formData.get('ssid'),
        password: formData.get('password'),
        mode: parseInt(formData.get('mode')),
        channel: parseInt(formData.get('channel')),
        hidden: formData.get('hidden') === 'on'
    };
    
    try {
        const response = await fetch('/api/wifi', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(data)
        });
        if (response.ok) {
            await fetchConfig();
        }
    } catch (error) {
        console.error('Error saving WiFi config:', error);
    }
}

async function setActiveMessage() {
    const selectedMessage = document.querySelector('input[name="messageNum"]:checked');
    if (!selectedMessage) return;
    
    const data = {
        action: 'setActive',
        messageNum: parseInt(selectedMessage.value)
    };
    
    try {
        await fetch('/api/message', {
            method: 'POST',
            headers: {'Content-Type': 'application/json'},
            body: JSON.stringify(data)
        });
        await fetchConfig();
    } catch (error) {
        console.error('Error setting active message:', error);
    }
}

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    fetchConfig();
    // Refresh config every 5 seconds
    setInterval(fetchConfig, 5000);
});
