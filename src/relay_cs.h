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
                    advertisedDevice.getScan()->stop();
                }
            }
        }
    };
    ScanCallback *scan_cb;

public:
    ConnectCallback *conn;
    int id;
    RelayClient(int child);
    void connect_to();
    std::string search(int id);
    void write(int val);
};

// server class
class RelayServer
{
    BLEServer *p_server;
    BLEService *p_svc;
    BLECharacteristic *p_ch;
    BLEUUID service_UUID;
    std::vector<RelayClient *> clients;

    int id;
    char deviceName[16];
    class Callback : public BLECharacteristicCallbacks
    {
    public:
        volatile bool asserted = false;
        int id;
        int dst_addr;
        BLECharacteristic *p_ch;
        void onWrite(BLECharacteristic *ch)
        {
            dst_addr = *((uint8_t *)ch->getValue().c_str());
            LOGF("accessing to %d\r\n", dst_addr);
            if (id == dst_addr)
            {
                LOG("hit");
                p_ch->setValue((uint8_t *)"hogehoge", 4);
                p_ch->notify();
            }
            else
            {
                asserted = true;
            }
        }
        void onRead(BLECharacteristic *ch)
        {
            ch->setValue((uint8_t *)"nonsense", 8);
        }
    };
    ///////////////////
    Callback *p_callback;

public:
    RelayServer(int id);
    void add(RelayClient *p); // add a child node
    void start();             // service and advertising start
    void tick();              // what?
    void advertise();         //
};

#endif // __CLIENT_SERVER
