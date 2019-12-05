#define RELAYDEBUG
#include "relay_cs.h"

// Service and Characteristic UUID retrieved by uuidgen(1).
//   For service UUID, using this UUID as a seed of final UUID,
//   final UUID is obtained by substituting last 2 hex-digit to
//   its ID (in hex)
//                           1234567890123456789012345678901234567890
#define SERVICE_UUID_SEED "4615B05D-06A9-4F44-AA73-FDE0DFDE8AD5"
#define CHARACTERISTIC_UUID "DD0242B7-A5EB-47D9-8925-1868BD500826"

// Final Service UUID maker function
static BLEUUID makeUUID(int id)
{
    char buf[36 + 4] = SERVICE_UUID_SEED;
    sprintf(buf + 34, "%02x", id);
    return BLEUUID(buf);
}

static void notify(BLERemoteCharacteristic *c, uint8_t *d, size_t length, bool isNotify)
{
    LOG("notify");
    LOG((char*)d);
    //  for (int i = 0; i< length; i++) {
    //    Serial.printf("%02x ", d[i]);
    //  }
    //  Serial.printf("Notify %s\n", isNotify? "true" : "false");
    c->readValue();
}

RelayClient::RelayClient(int child) : id(child)
{
    pClient = BLEDevice::createClient(); // create client
    conn = new ConnectCallback();
    pClient->setClientCallbacks(conn);

    scan_cb = new ScanCallback();                // making scan callback
    scan = BLEDevice::getScan();                 // get scan I/F
    scan->setAdvertisedDeviceCallbacks(scan_cb); // set callback
    scan_cb->target_UUID = makeUUID(child);
}

void RelayClient::connect_to()
{ // search child
    if (!conn->connected)
    {
        scan->setActiveScan(true);
        scan_cb->dst_addr = NULL;
        LOG("------");
        LOG(scan_cb->target_UUID.toString().c_str());
        while (scan_cb->dst_addr == NULL)
        {
            LOG("scan start");
            scan->start(10);
        }
        pClient->connect(*(scan_cb->dst_addr));
        //delay(100);
        service = pClient->getService(scan_cb->target_UUID);
        ch = service->getCharacteristic(CHARACTERISTIC_UUID);
        notify(ch, NULL, 0, false);
        ch->registerForNotify(notify);
        //delay(10000);
    }
}

std::string RelayClient::search(int id)
{
    return "";
}

void RelayClient::write(int val)
{
    ch->writeValue((uint8_t *)&val, 1);
}

// server class
RelayServer::RelayServer(int id) : id(id)
{
    BLEDevice::init("");
    p_server = BLEDevice::createServer();
    service_UUID = makeUUID(id);
    p_svc = p_server->createService(service_UUID);
    p_ch = p_svc->createCharacteristic(CHARACTERISTIC_UUID,
                                       BLECharacteristic::PROPERTY_NOTIFY |
                                           BLECharacteristic::PROPERTY_WRITE |
                                           BLECharacteristic::PROPERTY_READ |
                                           0);
    p_callback = new Callback();
    p_callback->id = id;
    p_callback->p_ch = p_ch;
    p_ch->setCallbacks(p_callback);
    p_ch->addDescriptor(new BLE2902());
    p_ch->setValue(std::string("hoge"));
}

void RelayServer::add(RelayClient *p)
{
    clients.push_back(p);
}

void RelayServer::start()
{
    p_svc->start();
    advertise();
}

void RelayServer::tick()
{
    for (RelayClient *cl : clients)
    {
        if (!cl->conn->connected)
        {
            LOG(cl->id);
            cl->connect_to();
        }
    }

    if (p_callback->asserted)
    {
        LOG("asserted");
        if (clients.empty())
        {
            LOG("empty");
        }
        for (RelayClient *cl : clients)
        {
            if (cl->id)
            {
                cl->write(p_callback->dst_addr);
            }
        }
        p_callback->asserted = false;
    }
}

void RelayServer::advertise()
{

    BLEAdvertising *adv = p_server->getAdvertising();
    adv->addServiceUUID(service_UUID);
    adv->start();
}
