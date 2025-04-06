#include "bluetooth_manager.h"

BluetoothManager::BluetoothManager(Config& config)
    : config(config) {
}

void BluetoothManager::begin() {
    Serial.println("Initializing Bluetooth...");
    if (!BLEDevice::getInitialized()) {
        BLEDevice::init("BaliseSonore");
    }
    BLEServer *server = BLEDevice::createServer();
    BLEService *service = server->createService(SERVICE_UUID);

    // Create characteristics for message number and volume
    messageChar = service->createCharacteristic(
        MESSAGE_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );
    volumeChar = service->createCharacteristic(
        VOLUME_CHAR_UUID,
        BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE
    );

    // Set initial values using local variables
    int messageNum = config.getNumeroMessage();
    int volume = config.getVolume();
    messageChar->setValue(messageNum);
    volumeChar->setValue(volume);

    // Set callbacks for write events
    messageChar->setCallbacks(new MessageCallback(config));
    volumeChar->setCallbacks(new VolumeCallback(config));

    service->start();
    BLEAdvertising *advertising = BLEDevice::getAdvertising();
    advertising->addServiceUUID(SERVICE_UUID);
    advertising->start();

    Serial.println("Bluetooth initialized and advertising started");
}

BluetoothManager::~BluetoothManager() {
    BLEDevice::deinit();
    Serial.println("Bluetooth stopped");
}

// Callback for message number characteristic
void MessageCallback::onWrite(BLECharacteristic *characteristic) {
    int newMessageNum = characteristic->getValue().c_str()[0] - '0'; // Convert char to int
    Serial.printf("Received new message number: %d\n", newMessageNum);

    if (newMessageNum >= 1 && newMessageNum <= config.getMessageCount()) {
        config.setNumeroMessage(newMessageNum);
        config.saveConfig();
        Serial.println("Message number updated successfully");
    } else {
        Serial.println("Invalid message number received");
    }
}

// Callback for volume characteristic
void VolumeCallback::onWrite(BLECharacteristic *characteristic) {
    int newVolume = characteristic->getValue().c_str()[0] - '0'; // Convert char to int
    Serial.printf("Received new volume: %d\n", newVolume);

    if (newVolume >= 0 && newVolume <= 30) {
        config.setVolume(newVolume);
        config.saveConfig();
        Serial.println("Volume updated successfully");
    } else {
        Serial.println("Invalid volume received");
    }
}