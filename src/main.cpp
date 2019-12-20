#include <Arduino.h>
#include <EEPROM.h>
#include <BLEDevice.h>
#include <BLE2902.h>

#include "config.h"
#include "utils.h"
#include "interp.h"
#include "relay_cs.h"

Config config;
Interp interp;

void init_pins()
{
    pinMode(4, INPUT_PULLDOWN); // init pin
    if (digitalRead(4))
    {
        interp.reconf = false; // force invalidate
    }

    pinMode(GPIO_NUM_2, OUTPUT);
    digitalWrite(GPIO_NUM_2, HIGH);

    pinMode(GPIO_NUM_7, OUTPUT); //AR
    pinMode(GPIO_NUM_8, OUTPUT); //GAIN
    digitalWrite(7, LOW);
    digitalWrite(8, LOW);
}

RelayClient *client;
RelayServer *server;

void setup()
{
    Serial.begin(115200);

    // put your setup code here, to run once:
    init_EEPROM();
    interp.reconf = 
    interp.valid = restore_from_EEPROM(config);
    init_pins();

    BLEDevice::init("");
    server = new RelayServer(config.id);
    interp.setServer(server);
    if (config.left) {
        auto client = new RelayClient(config.left);
        server->add(client);
    }
    if (config.right) {
        auto client = new RelayClient(config.right);
        server->add(client);
    }
    server->start();
}

void loop()
{
    static int count = 0;
    // put your main code here, to run repeatedly:
    if (Serial.available() > 0)
    {
        String cmd = readline();
        interp.command_interp(cmd, config);
        Serial.read(); // skip trailing CR
    }
    if (interp.reconf) {
        count++;
        if (count >= 12000) count = 0;
        if (count % 3000 == 0) {
            Serial.printf("%c\b", "/-\\|"[count / 3000]);
        }
        server->tick();
    }
}