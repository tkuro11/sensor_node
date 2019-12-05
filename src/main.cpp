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

    pinMode(GPIO_NUM_25, OUTPUT); //AR
    pinMode(GPIO_NUM_26, INPUT);  // Out
    pinMode(GPIO_NUM_15, OUTPUT); //Gain
    pinMode(GPIO_NUM_14, OUTPUT); //Vdd
    pinMode(GPIO_NUM_12, OUTPUT); //GND
    digitalWrite(25, HIGH);
    digitalWrite(15, HIGH);
    digitalWrite(14, HIGH);
    digitalWrite(12, LOW);
}

RelayClient *client;
RelayServer *server;

void setup()
{
    // put your setup code here, to run once:
    init_EEPROM();
    init_pins();
    interp.valid = restore_from_EEPROM(config);

    Serial.begin(115200);
    BLEDevice::init("");

    server = new RelayServer(config.id);
    if (config.left) {
        auto client = new RelayClient(config.left);
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
        Serial.print("cmd: ");
        Serial.println(cmd);
        interp.command_interp(cmd, config);
        Serial.read(); // skip trailing CR
    }
    if (interp.reconf) {
        count++;
        if (count > 120) count = 0;
        if (count % 30 == 0) {
            Serial.printf("%c\b", "/-\\|"[count / 100]);
        }

        server->tick();
    }
}