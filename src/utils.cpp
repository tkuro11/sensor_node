#include "utils.h"
#include <EEPROM.h>
#include <Arduino.h>
//////Serial   ///////////

String readline()
{
    String ret;
    while (1)
    {
        while (Serial.available() == 0)
            ;
        char c = Serial.read();
        if (c == 0x0a || c == 0x0d)
            break;
        ret += c;
    }
    return ret;
}

//////packet ///////////
long sum = 0;
int count = 0;

void make_packet(Packet &packet)
{
    static int prev = 0;
    if (count++ < 64)
    {
        int val = analogRead(sndPin);
        sum += abs(val - prev);
        prev = val;
        packet.temp = analogRead(sensorPin);
        packet.lux = analogRead(luxPin);
        packet.hop = 0;
    }
    else
    {
        count = 0;
        packet.sound = sum / 64;
        sum = 0;
    }
}

//////EEPROM   ///////////

bool init_EEPROM()
{
    return EEPROM.begin(sizeof(Config));
}

bool restore_from_EEPROM(Config &config)
{
    uint8_t *p = (uint8_t *)&config;
    for (int i = 0; i < sizeof(config); i++)
    {
        *p++ = EEPROM.read(i);
    }
    if (config.checker == MAGIC)
    {
        //valid
        return true;
    }
    else
    {
        return false;
    }
}

void save_to_EEPROM(Config &config)
{
    uint8_t *p = (uint8_t *)&config;
    for (int i = 0; i < sizeof(config); i++)
    {
        EEPROM.write(i, *(p++));
    }
    EEPROM.commit();
}
