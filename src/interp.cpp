#include <Arduino.h>
#include "config.h"
#include "utils.h"
#include "interp.h"

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
        Serial.println("id <id>      : set id of this module");
        Serial.println("rightid <id> : set rightside id of this module");
        Serial.println("leftid <id>  : set leftside id of this module");
        Serial.println("status       : show settings");
        Serial.println("save         : save settings to EEPROM and start connect");
        Serial.println("invalidate   : invalidate EEPROM settings");
        Serial.println("hold         : hold values of whole sensor net");
        Serial.println("get <id>     : retrieve <id>'s sensor values");
        Serial.println("[dataformat]");
        Serial.println("temperature, brightness, sound, hop, rssi");
    }
    else if (command.startsWith("rightid "))
    {
        config.right = command.substring(8).toInt();
        Serial.println("OK");
    }
    else if (command.startsWith("leftid "))
    {
        config.left = command.substring(7).toInt();
        Serial.println("OK");
    }
    else if (command.startsWith("get "))
    {
        uint8_t value = command.substring(4).toInt();
        if (value == config.id)
        {
            Serial.println("hoo");
            // ore
            //memcpy(NULL, &config.packet, sizeof(pbuf));
        }
        else
        {
            //     leftCh->writeValue((uint8_t*)&value, sizeof(value));
            //     rightCh->writeValue((uint8_t*)&value, sizeof(value));
            //     std::string mes;
            //     while (1) {
            //       ret_message = leftCh->readValue();
            //       if (ret_message.c_str()[0] != 0xff) break;
            //     }

            //     if (ret_message.c_str()[10] != 0xfe) {//found
            //       memcpy(&pbuf, ret_message.c_str(), sizeof(pbuf));
            //     } else {
            //       while (1) {
            //         ret_message = rightCh->readValue();
            //         if (ret_message.c_str()[0] != 0xff) break;
            //       }
            //       if (ret_message.c_str()[10] != 0xfe) {//found
            //         memcpy(&pbuf, ret_message.c_str(), sizeof(pbuf));
            //         pbuf.hop +=1;
            //       }
            //     }
            //   }
            //   Serial.printf("%d,%d,%d,%d,%d\r\n", pbuf.temp, pbuf.lux, pbuf.sound, pbuf.hop, pbuf.rssi);
        }
    }
    else if (command.startsWith("hold"))
    {
        Serial.println("OK");
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
