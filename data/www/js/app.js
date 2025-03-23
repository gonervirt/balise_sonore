let config = null;

// Fetch and update configuration
async function fetchConfig() {
    try {
        const response = await fetch('/api/config', {
            headers: {
                'Accept': 'application/json'
            }
        });
        if (!response.ok) {
            throw new Error(`HTTP error! status: ${response.status}`);
        }
        config = await response.json();
        updateAllTabs();
    } catch (error) {
        console.error('Error fetching config:', error);
        // Maybe show an error to the user
        alert('Failed to fetch configuration. Please refresh the page.');
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
async function handleApiCall(url, data) {
    try {
        const response = await fetch(url, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
                'Accept': 'application/json'
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            const errorText = await response.text();
            throw new Error(errorText || `HTTP error! status: ${response.status}`);
        }
        
        await fetchConfig(); // Refresh data after successful call
        return true;
    } catch (error) {
        console.error('API call failed:', error);
        alert('Operation failed: ' + error.message);
        return false;
    }
}

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
    
    await handleApiCall('/api/wifi', data);
}

async function setActiveMessage() {
    const selectedMessage = document.querySelector('input[name="messageNum"]:checked');
    if (!selectedMessage) return;
    
    await handleApiCall('/api/message', {
        action: 'setActive',
        messageNum: parseInt(selectedMessage.value)
    });
}

async function handleVolumeSubmit() {
    const volume = document.getElementById('volume').value;
    await handleApiCall('/api/volume', {
        volume: parseInt(volume)
    });
}

async function addNewMessage() {
    await handleApiCall('/api/message', {
        action: 'add'
    });
}

async function removeSelectedMessage() {
    const selectedMessage = document.querySelector('input[name="messageNum"]:checked');
    if (!selectedMessage) {
        alert('Please select a message to remove');
        return;
    }
    
    await handleApiCall('/api/message', {
        action: 'remove',
        messageNum: parseInt(selectedMessage.value)
    });
}

async function saveMessageText() {
    const selectedMessage = document.querySelector('input[name="messageNum"]:checked');
    if (!selectedMessage) {
        alert('Please select a message to edit');
        return;
    }
    
    const newText = prompt('Enter new message text:', '');
    if (newText === null || newText.trim() === '') {
        return; // User cancelled or entered empty text
    }

    await handleApiCall('/api/message', {
        action: 'update',
        messageNum: parseInt(selectedMessage.value),
        text: newText.trim()
    });
}

// Initialize on page load
document.addEventListener('DOMContentLoaded', () => {
    fetchConfig();
    // Refresh config every 5 seconds
    setInterval(fetchConfig, 5000);

    // Add event listeners with null checks
    const saveVolumeBtn = document.getElementById('saveVolume');
    if (saveVolumeBtn) saveVolumeBtn.addEventListener('click', handleVolumeSubmit);

    const addMessageBtn = document.getElementById('addMessage');
    if (addMessageBtn) addMessageBtn.addEventListener('click', addNewMessage);

    const removeMessageBtn = document.getElementById('removeMessage');
    if (removeMessageBtn) removeMessageBtn.addEventListener('click', removeSelectedMessage);

    const saveMessageBtn = document.getElementById('saveMessage');
    if (saveMessageBtn) saveMessageBtn.addEventListener('click', saveMessageText);

    // Add error logging if buttons are missing
    if (!addMessageBtn) console.error('Add Message button not found');
});
