#define RELAYDEBUG
#include "relay_cs.h"
#include "utils.h"

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

/////////////////////////

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
        int backup_wdt;
        extern int counter_max;
        backup_wdt = counter_max;
        failed = false;
        pClient->disconnect();
        scan->setActiveScan(true);
        scan_cb->dst_addr = NULL;
        LOG("------");
        scan->setAdvertisedDeviceCallbacks(scan_cb);
        int count = 0;
        while (scan_cb->dst_addr == NULL)
        {
            LOG("scan start");
            if (++count > 3) {
                failed = true;
                return;
            }
            scan->start(5);
        }
        delay(800);counter_max = 5;
        pClient->connect(*(scan_cb->dst_addr));
        service = pClient->getService(scan_cb->target_UUID);
        ch = service->getCharacteristic(CHARACTERISTIC_UUID);
        //delay(10000);
    }
}

Packet *RelayClient::search(int id)
{
    static Packet p;
    write(id);
    while ((p = *(read())).valid == 0x0000)
    {
        delay(10);
    } // 0xaaaa not found
    return &p;
}

void RelayClient::write(int addr)
{
    ch->writeValue((uint8_t *)&addr, 1);
}

Packet *RelayClient::read()
{
    return (Packet *)ch->readValue().c_str();
}

///////////// server class
RelayServer::RelayServer(int id) : id(id)
{
    BLEDevice::init("");
    p_server = BLEDevice::createServer();
    service_UUID = makeUUID(id);
    p_svc = p_server->createService(service_UUID);
    p_ch = p_svc->createCharacteristic(CHARACTERISTIC_UUID,
                                       BLECharacteristic::PROPERTY_WRITE |
                                           BLECharacteristic::PROPERTY_READ |
                                           0);
    p_callback = new Callback();
    p_callback->id = id;
    p_callback->p_ch = p_ch;
    p_ch->setCallbacks(p_callback);
    p_ch->setValue(std::string("0"));
    conn = new ConnectCallback();
    p_server->setCallbacks(conn);

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

bool RelayServer::tick(int average_count = 100)
{
    for (RelayClient *cl : clients)
    {
        if (!cl->conn->connected)
        {
            cl->connect_to();
            advertise();
        }
        if (cl->failed) return false;
    }

    make_packet(p_callback->packet, average_count);

    if (p_callback->asserted)
    {
        LOG("asserted");
        if (clients.empty())
        {
            LOG("not found");
            p_callback->tempbuffer.valid = 0xaaaa;
            p_callback->asserted = false;
        }
        else
        {
            bool found = false;
            Packet *p;
            for (RelayClient *cl : clients)
            {
                cl->make_current();
                p = cl->search(p_callback->dst_addr);
                Serial.printf("-- %d\r\n", cl->id);
                if (p->valid != 0xaaaa)
                {
                    found = true;
                    if (cl->id == p_callback->dst_addr)
                    {
                        int rssi = cl->getRssi();
                        p->rssi = rssi;
                    }
                    p_callback->tempbuffer = *p;
                    // p->valid = 0xffff;
                    break;
                }
            }
            if (!found)
                p_callback->tempbuffer = *p;
            Packet *q = &p_callback->tempbuffer;
            Serial.printf("-- %d,%d,%d,%d,%d\r\n", q->valid, q->lux, q->sound, q->hop, q->rssi);
            p_callback->asserted = false;
        }
    }
    return true;
}

Packet *RelayServer::get_packet()
{
    return &p_callback->packet;
}

Packet *RelayServer::search(int dst)
{
    bool found = false;
    Packet *p;
    for (RelayClient *cl : clients)
    {
        if (cl->conn->connected) {
            cl->make_current();
            p = cl->search(dst);
            if (p->valid != 0xaaaa)
            {
                found = true;
                tmp = *p;
                if (cl->id == dst)
                {
                    int rssi = cl->getRssi();
                    tmp.rssi = rssi;
                }
                p->valid = 0xffff;
                break;
            }
        }
    }
    if (!found)
    {
        tmp.valid = 0xaaaa;
        return &tmp;
    }
    else
    {
        return &tmp;
    }
}

void RelayServer::advertise()
{
    BLEAdvertising *adv = p_server->getAdvertising();
    adv->addServiceUUID(service_UUID);
    adv->start();
}
