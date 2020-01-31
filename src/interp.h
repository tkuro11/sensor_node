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
    int average_count;
    RelayServer *server;

    Interp() {
        reconf = false;
        valid  = false;
        prompt = true;
        average_count = 100;
    }

    void command_interp(String , Config &);
    void setServer(RelayServer* );
};

#endif // __INTERP__
