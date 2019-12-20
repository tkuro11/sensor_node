#if !defined(__INTERP__)
#define __INTERP__
#include <Arduino.h>
#include "relay_cs.h"

class Interp
{
public:
    bool valid;
    bool reconf;
    bool prompt;
    RelayServer *server;

    Interp() {
        reconf = false;
        valid  = false;
        prompt = true;
    }

    void command_interp(String command, Config &config);
    void setServer(RelayServer* );
};

#endif // __INTERP__
