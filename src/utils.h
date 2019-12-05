#if !defined(__EEPROM__)
#define __EEPROM__
#include "config.h"

//////EEPROM   ///////////
bool init_EEPROM();
bool restore_from_EEPROM(Config&);
void save_to_EEPROM(Config&);
String readline();
void make_packet(Config&);

#endif // __EEPROM__
