#include "utils.h"
#include <EEPROM.h>

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

void make_packet(Config &config)
{
    static int prev = 0;
    if (count % 16 == 0)
    {
        Serial.printf("%c\b", "/-\\|"[(count / 16) % 4]);
    }
    if (count++ < 64)
    {
        int val = analogRead(26);
        Serial.println(val);
        sum += abs(val - prev);
        prev = val;
        config.packet.temp = analogRead(sensorPin);
        config.packet.lux = analogRead(luxPin);
        config.packet.hop = 0;
        config.packet.rssi = -65;
    }
    else
    {
        count = 0;
        config.packet.sound = sum / 64;
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
