#ifndef CommCtrlrConfig_h
#define CommCtrlrConfig_h

#include <Arduino.h>

/*******************************
 *     Constants for Sat Modem *
 *******************************/
const unsigned long satResponseTimeout = 1 * 60UL * 1000UL;       // (ms) Timeout used when waiting for response timeouts
const unsigned int satSBDIXResponseLost = 30000;     // (ms) How much time to wait before assuming SBDIX command failed

#endif
