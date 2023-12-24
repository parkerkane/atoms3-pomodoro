//
// Created by Timo on 23/12/2023.
//

#include "pomodoro.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID "ae088d12-a31e-4d8c-8b2b-84dd292883f4"
#define HEARTHBEAT_CHAR_UUID "49a380be-0591-4bb0-978b-fab6cc055f3f"

BLEServer* pServer = NULL;
unsigned long lastHeartbeatTime = -15 * 60 * 1000;

class MyServerCallbacks : public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) override
    {
        BLEDevice::startAdvertising();
    };

    void onDisconnect(BLEServer* pServer) override
    {
    }
};

class HearthbeatCallbacks : public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic* pCharacteristic) override
    {
        std::string rxValue = pCharacteristic->getValue();

        printf("BLE Received: %s\r\n", rxValue.c_str());
        lastHeartbeatTime = millis();
    }
};

void bleSetup()
{
    BLEDevice::init("Pomodoro Timer");

    pServer = BLEDevice::createServer();
    pServer->setCallbacks(new MyServerCallbacks());

    BLEService* pService = pServer->createService(SERVICE_UUID);

    BLECharacteristic* startCharacteristic = pService->createCharacteristic(
        HEARTHBEAT_CHAR_UUID,
        BLECharacteristic::PROPERTY_WRITE);

    startCharacteristic->setCallbacks(new HearthbeatCallbacks());

    pService->start();

    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    pAdvertising->addServiceUUID(SERVICE_UUID);
    pAdvertising->setScanResponse(true);
    pAdvertising->setMinPreferred(0x0); // set value to 0x00 to not advertise this parameter
    BLEDevice::startAdvertising();
}

bool bleIsDesktopActive()
{
    unsigned long now = millis();

    return now - lastHeartbeatTime < 1 * 5 * 1000;
}