#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLE2902.h>

#define SERVICE_UUID        "efe25867-61de-45d6-8e32-6fa4aba888a2"
#define CHARACTERISTIC_UUID "75f630fc-db35-4115-81fd-a8b5a02ae3ba"

BLEServer* pServer = NULL;
BLECharacteristic* pCharacteristic = NULL;
bool deviceConnected = false;

class MyServerCallbacks: public BLEServerCallbacks {

    // These could be used to enable or disable sensor readings. For example, if we have no clients connected
    // then there is no need to spend energy sampling a value if there is no-one there to read it.
    
    void onConnect(BLEServer* pServer) {
      M5.Lcd.println("connect");
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      M5.Lcd.println("disconnect");
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
  void onRead(BLECharacteristic *pCharacteristic) {
    M5.Lcd.println("read");
    pCharacteristic->setValue("Hello World!!!!!!");
  }
  
  void onWrite(BLECharacteristic *pCharacteristic) {  // don't think we even need this as the gateway will never tell the node what the humidity is
    M5.Lcd.println("write");
    std::string value = pCharacteristic->getValue();
    M5.Lcd.println(value.c_str());
  }
};

void setup() {
//  Serial.begin(115200);
  M5.begin();
  M5.Power.begin();
  M5.Lcd.println("BLE Humidity Sensor Starting");

  BLEDevice::init("HumidityReader");
  BLEAddress address = BLEDevice::getAddress();
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
//                                         BLECharacteristic::PROPERTY_NOTIFY |
//                                         BLECharacteristic::PROPERTY_INDICATE
                                       );
  pCharacteristic->setCallbacks(new MyCallbacks());
  pCharacteristic->addDescriptor(new BLE2902());

  M5.Lcd.printf("Using address: %s \n", BLEDevice::getAddress().toString().c_str());


  pService->start();
  
  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->start();
}

void loop() {

  if (deviceConnected) {
    int humidity = random(0, 100);
    M5.Lcd.printf("Sending: %d \n", humidity);

    char buf [4];
    itoa(humidity, buf, 10);
    
    pCharacteristic->setValue(buf); // set the value of the characteristic so that the gateway can read the value
    pCharacteristic->notify();
  }

  delay(3000);
  
  M5.update();
}
