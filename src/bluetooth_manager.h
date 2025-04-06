#ifndef BLUETOOTH_MANAGER_H
#define BLUETOOTH_MANAGER_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include "config.h"

#define SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define MESSAGE_CHAR_UUID "abcd1234-5678-1234-5678-123456789abc"
#define VOLUME_CHAR_UUID "abcd5678-1234-5678-1234-123456789abc"

class BluetoothManager {
public:
    BluetoothManager(Config& config);
    void begin();
    ~BluetoothManager();

private:
    Config& config;
    BLECharacteristic *messageChar;
    BLECharacteristic *volumeChar;
};

class MessageCallback : public BLECharacteristicCallbacks {
public:
    MessageCallback(Config& config) : config(config) {}
    void onWrite(BLECharacteristic *characteristic) override;

private:
    Config& config;
};

class VolumeCallback : public BLECharacteristicCallbacks {
public:
    VolumeCallback(Config& config) : config(config) {}
    void onWrite(BLECharacteristic *characteristic) override;

private:
    Config& config;
};

#endif // BLUETOOTH_MANAGER_H