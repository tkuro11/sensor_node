#if !defined(__INTERP__)
#define __INTERP__
#include <Arduino.h>

class Interp
{
public:
    bool valid;
    bool reconf;

    Interp() {
        reconf = false;
        valid  = false;
    }

    void command_interp(String command, Config &config);
};

#endif // __INTERP__
