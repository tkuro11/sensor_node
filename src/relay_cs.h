#if !defined(__CLIENT_SERVER)
#define __CLIENT_SERVER
#ifdef RELAYDEBUG
#include <Arduino.h>
#define LOG(value) Serial.println(value)
#define LOGF(value, arg) Serial.printf((value), (arg))
#else
#define LOG(value)
#define LOGF(value, arg)
#endif
#include "config.h"
#include <BLEDevice.h>
#include <BLE2902.h>
// Service and Characteristic UUID retrieved by uuidgen(1).
//   For service UUID, using this UUID as a seed of final UUID,
//   final UUID is obtained by substituting last 2 hex-digit to
//   its ID (in hex)
//                           1234567890123456789012345678901234567890
#define SERVICE_UUID_SEED "4615B05D-06A9-4F44-AA73-FDE0DFDE8AD5"
#define CHARACTERISTIC_UUID "DD0242B7-A5EB-47D9-8925-1868BD500826"

class RelayClient
{
    BLEClient *pClient;
    BLERemoteService *service;
    BLERemoteCharacteristic *ch;
    BLEScan *scan;
protected:
    class ConnectCallback : public BLEClientCallbacks
    {
    public:
        bool connected = false;
        void onConnect(BLEClient *pClient)
        {
            connected = true;
        }
        void onDisconnect(BLEClient *pClient)
        {
            connected = false;
        }
    };

    class ScanCallback : public BLEAdvertisedDeviceCallbacks
    {
    public:
        BLEAddress *dst_addr = NULL;
        BLEUUID target_UUID;
        void onResult(BLEAdvertisedDevice advertisedDevice)
        {
            if (advertisedDevice.haveServiceUUID())
            {
                LOG(advertisedDevice.getServiceUUID().toString().c_str());
                if (advertisedDevice.getServiceUUID().equals(target_UUID))
                {
                    LOG("found: ");
                    dst_addr = new BLEAddress(advertisedDevice.getAddress());
                    LOG(dst_addr->toString().c_str());
                    advertisedDevice.getScan()->stop();
                }
            }
        } 
    };
    ScanCallback *scan_cb;

public:
    ConnectCallback *conn;
    bool failed = false;
    int id;
    RelayClient(int child);
    void connect_to();
    Packet* search(int id);
    void write(int val);
    Packet *read();
    void make_current() {
        BLEDevice::setClient(pClient);
    }
    int getRssi() {
        int rssi = pClient->getRssi();
        return rssi;
    }
};

// server class
class RelayServer
{
    BLEServer *p_server;
    BLEService *p_svc;
    BLECharacteristic *p_ch;
    BLEUUID service_UUID;
    std::vector<RelayClient *> clients;
    Packet tmp;

    int id;
    char deviceName[16];
    class Callback : public BLECharacteristicCallbacks
    {
    public:
        volatile bool asserted = false;
        int id;
        int dst_addr;
        BLECharacteristic *p_ch;
        Packet packet;
        Packet holdbuffer;
        Packet tempbuffer;
        Callback() {
            holdbuffer.valid = 0;
            tempbuffer.valid = 0;
        }
        void onWrite(BLECharacteristic *ch)
        {
            dst_addr = *((uint8_t *)ch->getValue().c_str());
            LOGF("accessing to %d\r\n", dst_addr);
            if (dst_addr == 0xff) { // hold
                LOG("hold");
                holdbuffer = packet;
                holdbuffer.valid = 0xffff;
                asserted = true;
            }
            else if (id == dst_addr) {
                LOG("hit");
                if (holdbuffer.valid == 0xffff)
                {
                    tempbuffer = holdbuffer;
                    holdbuffer.valid = 0;
                } else {
                    tempbuffer = packet;
                    tempbuffer.valid = 0xffff;
                }
            } else {
                asserted = true;
            }
        }
        void onRead(BLECharacteristic *ch)
        {
            LOG("read");
            int size;
            if (tempbuffer.valid == 0) size = sizeof(tempbuffer.valid);
            else size = sizeof(Packet);
            tempbuffer.hop++;
            ch->setValue((uint8_t *)&tempbuffer, size);
            Packet *p= &tempbuffer;
            Serial.printf("- %d,%d,%d,%d,%d\r\n", p->valid, p->lux, p->sound, p->hop, p->rssi);

            tempbuffer.valid = 0;
        }
    };
    ///////////////////
    Callback *p_callback;
    class ConnectCallback : public BLEServerCallbacks
    {
    public:
        bool connected = false;
        void onConnect(BLEClient *pClient)
        {
            connected = true;
        }
        void onDisconnect(BLEClient *pClient)
        {
            connected = false;
        }
    };
public:
    ConnectCallback *conn;
    int rssi;

    RelayServer(int id);
    void add(RelayClient *p); // add a child node
    void start();             // service and advertising start
    bool tick(int);           //
    void advertise();         //
    Packet *search(int);
    Packet *get_packet();     //
};

#endif // __CLIENT_SERVER
