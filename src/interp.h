#if !defined(__INTERP__)
#define __INTERP__
#include <Arduino.h>
#include "relay_cs.h"

class Interp
{
public:
    bool valid;
    bool reconf;
    RelayServer *server;

    Interp() {
        reconf = false;
        valid  = false;
    }

    void command_interp(String command, Config &config);
    void setServer(RelayServer* );
};

#endif // __INTERP__
