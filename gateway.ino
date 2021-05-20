/**
 * Base code for gateway node development in NWEN439
 * You will need to modify this code to implement the necessary
 * features specified in the handout.
 * 
 * Important: The compiled code for this module is quite
 * big so you will need to change the partition scheme to 
 * "No OTA (Large App)". To do this, click "Tools" on 
 * Arduino Studio, then select "No OTA (Large App)" from 
 * Partition Scheme.
 */

#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <coap-simple.h>
#include "time.h"

const std::string TEMPERATURE_SERVICE_UUID = "4fafc201-1fb5-459e-8fcc-c5c9c331914b";
const std::string TEMPERATURE_CHARACTERISTIC_UUID = "beb5483e-36e1-4688-b7f5-ea07361b26a8";

/**
 * SSID and password of the WIFI network
 * TODO: Change these parameters based your WIFI configuration.
 * Note that M5Stack only supports PSK, and EAP and other
 * complicated authentication mechanisms
 */
 
const char* WIFI_SSID = "Ben";
const char* WIFI_PASSWORD = "00000000";

/**
 * Time synchronization related stuff
 */
const char* ntpServer1 = "oceania.pool.ntp.org";
const char* ntpServer2 = "time.nist.gov";
const long  gmtOffset_sec = (13)*60*60;
const int   daylightOffset_sec = 3600;

/**
 * Unix timestamp
 * Once gateway is time synced, you can use this timestamp
 */
RTC_DATA_ATTR time_t timestamp = 0;

/**
 * BLE scanner object
 */
BLEScan* scanner;

/**
 * IP address, UDP and Coap instances
 * Must be global since they are used by callbacks
 */
IPAddress wifi_ip; 
WiFiUDP udp;
Coap coap(udp);  

/**
 * Example CoAP server callback for /temp URL query
 * You can use this as basis for writing your own callback
 * to implement your design.
 */
void callback_temp(CoapPacket &packet, IPAddress ip, int port) {
  M5.Lcd.println("CoAP /temp query received");

  // Send response
  char *payload = "temp=25";
  int ret = coap.sendResponse(ip, port, packet.messageid, payload, strlen(payload), 
    COAP_CONTENT, COAP_TEXT_PLAIN, packet.token, packet.tokenlen);

  if(ret) 
    M5.Lcd.println("Sent response");
  else
    M5.Lcd.println("Failed to send response");
}

class ScanCallback: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    BLEUUID service = advertisedDevice.getServiceUUID();

    if (service.equals(BLEUUID(TEMPERATURE_SERVICE_UUID))) {
      M5.Lcd.printf("Found UUID %s with address %s and name %s \n", advertisedDevice.getServiceUUID().toString().c_str(), advertisedDevice.getAddress().toString().c_str(), advertisedDevice.getName());

      BLEClient* temperatureClient = BLEDevice::createClient();
      temperatureClient->connect(advertisedDevice.getAddress());  // Get's stuck here from some reason
      
      BLERemoteService* pMyRemoteService = temperatureClient->getService(service);
      BLERemoteCharacteristic* pMyRemoteCharacteristic = pMyRemoteService->getCharacteristic(BLEUUID(TEMPERATURE_CHARACTERISTIC_UUID));

      std::string myValue = pMyRemoteCharacteristic->readValue();


//      advertisedDevice.getScan()->stop();
    }
  }
};


/**
 * Write code that will be run once during startup
 * in setup() function.
 */
void setup() {
  
  // Initialize the M5Stack object
  M5.begin();

  // Power
  M5.Power.begin();

  // Clear screen and lower brightness to save power
  M5.Lcd.clear(); 
  M5.Lcd.setBrightness(30);
  
  M5.Lcd.println("Gateway node starting...");

  // Initialize BLE 
  BLEDevice::init("m5-gateway");
  // Create BLE scanner
  scanner = BLEDevice::getScan();
  
  scanner->setAdvertisedDeviceCallbacks(new ScanCallback());
  scanner->start(5);

  // Initialize WIFI
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) 
    delay(1000);
  M5.Lcd.print("Connected to WIFI with IP address ");
  wifi_ip = WiFi.localIP();
  M5.Lcd.println(wifi_ip);

  // Do time sync. Make sure gateway can connect to 
  // Internet (i.e., your WiFi AP must be connected to Internet)
  configTime(gmtOffset_sec, daylightOffset_sec, ntpServer1, ntpServer2);
  delay(2000);
  time(&timestamp);  
  M5.Lcd.println("Current time is " + String(timestamp));
    
  // Initialize CoAP
  // TODO: Add server url endpoints
  // You can add multiple endpoint urls, examples:
  //   coap.server(callback_switch, "switch");
  //   coap.server(callback_temp, "temp");
  //   coap.server(callback_humidity, "humidity");

  // Example url/callback
  coap.server(callback_temp, "temp");

  // start coap server/client
  coap.start();  
}

/**
 * Write code that will be run repeatedly in loop()
 */
void loop() {
  delay(1000);
  coap.loop();  
}
