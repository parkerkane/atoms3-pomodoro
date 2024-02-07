//
// Created by Timo on 23/12/2023.
//

#include "pomodoro.h"

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>

#define SERVICE_UUID "ae088d12-a31e-4d8c-8b2b-84dd292883f4"
#define HEARTHBEAT_CHAR_UUID "49a380be-0591-4bb0-978b-fab6cc055f3f"

namespace ble {

BLEServer* pServer = NULL;
bool hearthbeatState = false;
volatile unsigned long lastHeartbeatTime = 0;

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

        printf("Received hearthbeat: %s\r\n", rxValue.c_str());
        hearthbeatState = true;
        lastHeartbeatTime = millis();
    }
};

void setup()
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

void clearHearthbeatState() 
{
    hearthbeatState = false;
}

bool getHerthbeatState()
{
    return hearthbeatState;
}

unsigned long getHerthbeatTime() {
    return lastHeartbeatTime;
}

/* namespace ble */
}