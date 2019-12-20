#include <Arduino.h>
#include "config.h"
#include "utils.h"
#include "interp.h"

void Interp::setServer(RelayServer* server) 
{
    this->server = server;
}

void Interp::command_interp(String command, Config &config)
{
    if (command.startsWith("id "))
    {
        config.id = command.substring(3).toInt();
        Serial.println("OK");
    }
    else if (command.startsWith("help"))
    {
        Serial.println("help         : display this help message");
        Serial.println("ver          : show version number");
        Serial.println("id <id>      : set the id of this module");
        Serial.println("rightid <id> : set rightside id of this module");
        Serial.println("leftid <id>  : set leftside id of this module");
        Serial.println("status       : show settings");
        Serial.println("save         : save settings to EEPROM and start connection");
        Serial.println("invalidate   : invalidate EEPROM settings");
        Serial.println("hold         : hold values of whole sensor net");
        Serial.println("get <id>     : retrieve <id>'s sensor values");
        Serial.println("p <on|off>   : turn on(default)/off prompt (for lifecheck purpose)");
        Serial.println("[dataformat]");
        Serial.println("temperature, brightness, sound, hop, rssi");
    }
    else if (command.startsWith("rightid "))
    {
        config.right = command.substring(8).toInt();
        Serial.println("OK");
    }
    else if (command.startsWith("ver"))
    {
        Serial.println(VERSION);
    }
    else if (command.startsWith("p "))
    {
        String  sw = command.substring(2);
        if (sw.equals("on")) {
            prompt = true;
        } else {
            prompt = false;
        }
    }
    else if (command.startsWith("leftid "))
    {
        config.left = command.substring(7).toInt();
        Serial.println("OK");
    }
    else if (command.startsWith("get "))
    {
        uint8_t value = command.substring(4).toInt();
        Packet *p;
        if (value == config.id)
        {
            p = server->get_packet();
            p->rssi = -1;
        }
        else
        {
            p = server->search(value);
        }
        if (p->valid == 0xaaaa) {
            Serial.println("not found");
        } else {
            Serial.printf("%d,%d,%d,%d,%d\r\n", p->temp, p->lux, p->sound, p->hop, p->rssi);
        }
    }
    else if (command.startsWith("hold"))
    {
        Packet* p = server->search(0xff);
        if (p->valid == 0xaaaa) 
            Serial.println("OK");
        else
            Serial.println("NG");            
    }
    else if (command.startsWith("status"))
    {
        Serial.printf("id = %d\n\rleft, right = %d, %d",
                      config.id, config.left, config.right);
        Serial.println(" - OK");
    }
    else if (command.startsWith("invalidate"))
    {
        config.checker = 0x0000;
        save_to_EEPROM(config);
        Serial.println("invalidated.");
        //   valid = reconf = false;
    }
    else if (command.startsWith("save"))
    {
        Serial.println("saved.");
        config.checker = MAGIC;
        save_to_EEPROM(config);
        //   valid = reconf = true;
    }
    else if (command.startsWith("start"))
    {
        reconf = true;
    }
    else
    {
        Serial.println("error");
    }
}
