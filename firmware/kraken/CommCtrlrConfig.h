#ifndef CommCtrlrConfig_h
#define CommCtrlrConfig_h

#include <Arduino.h>

//************* Watchdog Setup ****************
extern unsigned long wdResetTime;
#define TIMEOUTPERIODs 120
#define TIMEOUTPERIOD (TIMEOUTPERIODs* 1000UL)             // You can make this time as long as you want,
                                       // it's not limited to 8 seconds like the normal
                                       // watchdog
#define wdtrst() wdResetTime = millis();  // This macro will reset the timer

/*
Global Configuration Information
*/

/*******************************
 *    Pin declarations         *
 *******************************/
#define pinRI 8
#define pinNA 9
#define pinModemPowerSwitch 7


/*******************************
 *     Constants for Sat Modem *
 *******************************/
const byte satMinimumSignalRequired = 2;	//Minimum signal bars number required to do a normal SBD session - a Forced session will ignore this.
const byte satIncomingPackLenLimit = 70;             //Used to define length of buffer arrays for packet data
const unsigned int satPowerOffMinimumTime = 2000;    //Probably 2000 millis for iridium
const unsigned int maxTelemLenConst = 340;          //Maximum acceptable length of telemetry packet FROM EEPROM, set by Iridium MO max message size
#define satNetworkNotAvailable = 255;
const int LongMsgQueueLen = 20;             // Number of messages that can be in the queue to send out sat modem
const unsigned long satResponseTimeout = 1 * 60UL * 1000UL;       // (ms) Timeout used when waiting for response timeouts
const unsigned int satSBDIXResponseLost = 30000;     // (ms) How much time to wait before assuming SBDIX command failed

/*******************************
 *   Constants for message payload processing (set these based on headers of sat modem provider)
********************************/
const byte packetPayloadStartIndex = 0;  // Message content starts here in a received packet from sat modem 6 for orbcomm, may be 0 for Iridium
const byte satIncomingMessageHeaderLength = 0;  //Length of inbound message headers, 15 for orbcomm, may be 0 for Iridium
const byte i2cRetryLimit = 10;
const unsigned int satMessageCharBufferSize = 340;  //Char array size for loading messages from eeprom into

/*******************************
 *   Internal EEPROM Locations         *
 *******************************/
const int EPLOCcmdCounterArrayStart = 2; 
const int EPLENcmdCounterArray = 76;  //76 byte array to store used received command counter numbers.
const int EPLOCAtcReportPairArrayStart = 80;  // Arduino EEPROM location to store the latest ATC report pair
const int EPLOCAtcReportPairArray = 12;
const int EPLOCI2CRebootCount = 100;
const int EPLOCLastMaxSBDDelayStart = 101;




#endif


