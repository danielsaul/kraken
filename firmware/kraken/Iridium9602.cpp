
#if (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#include <avr/pgmspace.h>
#include <string.h>

#include "CommCtrlrConfig.h"
#include "RockBlock.h"
#include "Iridium9602.h"
#include <SoftwareSerial.h>

/* define to enable debug messages */
//#define _WS_DEBUG 1

/* from CommCtrlArduino.ino */
void initIridiumNetworkInterrupt();

extern volatile int NetworkAvailableJustChanged;
extern volatile int SatelliteNetworkAvailable;

Iridium9602::Iridium9602(SoftwareSerial& sPort) : _SoftwareSerial(sPort), _rcvIdx(0)
{
}



void Iridium9602::initModem()
{
        _rcvIdx = 0;
        _bRing = false;
        _sessionInitiated = false;
        _MOQueued = false;
        _MTQueued = 0;
        _MTMsgLen = 0;
        _lastSessionTime = 0;
        _lastSessionResult = 1;
        //Wait for modem to be fully powered down - this is important delay!
        { //Delay without the delay command
                unsigned long millistart = millis();
                while ( millis() < millistart + 5000 ) {
                }
        }

        //delay(2500);
        //Turn modem on
        digitalWrite(RB_SLEEP, HIGH);
        /*
        //Check for DSR line going high to indicate boot has finished
        { 
                unsigned long millistart = millis();
                while ( (millis() < (millistart + 60000)) ) //Delay without the delay command
                {
                        //if (HIGH == digitalRead(pinDSR)) 
                        //{	
                        //        break; 
                        //}
                }
        }
        //if (HIGH == digitalRead(pinDSR)) 
        //{
                //Setup NetworkAvailable Pinchange interrupt 
                //initIridiumNetworkInterrupt();
        //}*/

        while(!sendCommandandExpectPrefix(F("AT"), F("OK"), 500)) {
        }
        if (SERIAL_EN)
            Serial.println("RockBlock: Responding");

        //Set Serial Character Echo Off 
        sendCommandandExpectPrefix(F("ATE0"), F("OK"), 1000);
        if (SERIAL_EN)
            Serial.println("RockBlock: Echo Off");

        //Set modem to not use flow control handshaking during SBD messaging âˆš
        sendCommandandExpectPrefix(F("AT&K0"), F("OK"), 1000);
        if (SERIAL_EN)
            Serial.println("RockBlock: Flow control off");
        //Set response quiet mode - 0 = responses ARE sent to arduino from modem âˆš
        sendCommandandExpectPrefix(F("ATQ0"), F("OK"), 1000);
        if (SERIAL_EN)
            Serial.println("RockBlock: Responses on");

        setIncommingMsgAlert(false);

        // Write the defaults to the modem and use default and flush to eeprom âˆš
        sendCommandandExpectPrefix(F("AT&W0"), F("OK"), 1000);
        if (SERIAL_EN)
            Serial.println("RockBlock: Write defaults");
        //Designate Default Reset Profile
        sendCommandandExpectPrefix(F("AT&Y0"), F("OK"), 1000);
        if (SERIAL_EN)
            Serial.println("RockBlock: Default reset profile");
        
        setIndicatorReporting(true);
        setIncommingMsgAlert(true);
}

void Iridium9602::clearIncomingMsg(void)
{
        _rcvIdx = 0;
        _receivedCmd[0] = '\0';
}

void Iridium9602::flushIncomingMsg(void)
{
        clearIncomingMsg();
        while (_SoftwareSerial.available()) _SoftwareSerial.read();
}

#define NEXT_VAL(v) \
do { \
        int __tmp;                                              \
        /* skip white spaces */                                 \
        while(*p && *p == ' ') p++;                             \
        /* convert MO status to int, n point to first char */   \
        /* after the number */                                  \
        __tmp = strtol(p, &n, 10);                              \
        if ((v))                                                \
                *(v) = __tmp;                                   \
        /* if p == n then no number was found */                \
        if (p == n) /* no number */                             \
                goto err_out;                                   \
        p = n;                                                  \
        /* have to be at EOL or at a ',' */                     \
        if (*p != ',' && *p != '\0') goto err_out;              \
        /* p should poimnt at start of next number or space */  \
        p++;                                                    \
} while (0) 

static bool parseSBDIXResponse(char * buf, int * mo_st, int * mt_st,
                               int * mt_len, int * mt_queued)
{
        /* <MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued> */
        char * p, * n = NULL;

        p = buf + 7;

        /* get MO status */
        NEXT_VAL(mo_st);

        /* skip MOMSN */
        p = strchr(p, ',');
        if (*p == '\0') goto err_out;
        p++;

        NEXT_VAL(mt_st);
        /* skip MTMSN */

        p = strchr(p, ',');
        if (!*p) goto err_out;
        p++;

        NEXT_VAL(mt_len);
        NEXT_VAL(mt_queued);

        /* if we get here parse was good */
        return true;

err_out:
        return false;
}

#undef NEXT_VAL


void Iridium9602::parseUnsolicitedResponse(char * cmd)
{
        /* check time session lost timeout */
		//wdtrst();
        if (strncmp_P(_receivedCmd, PSTR("+SBDIX:"), 7) == 0) {
                //DebugMsg::msg_P("SAT", 'D', PSTR("Match SBDIX"));
                int mo_st = -1, mt_st, mt_len, mt_q;
                if (parseSBDIXResponse(_receivedCmd, &mo_st, &mt_st, &mt_len, &mt_q)) {
                    //wdtrst();
                    if (mt_st == 1) {  // Received message OK if 1
                    	_MTQueued = mt_q + 1;
                    	_MTMsgLen = mt_len;	
                    } else if ((mt_st == 2)) {
                    	//MT message didn't transfer properly, but this is taken care of by reading the MT status externally
                        
                    }
                    
                    /* update count stored at GSS */
                    _GSSQueued = mt_q;
                    _MTStatus = mt_st;
                }


                clearIncomingMsg();
                //wdtrst();
                expectPrefix(F("OK"), satResponseTimeout);
                //wdtrst();
                _sessionInitiated = false;
    

                if (_MOQueued && 
                    (mo_st >= 0) && (mo_st <= 4)) {   //4 or less indicates sent success
                        _lastSessionResult = 1;
                       // wdtrst();
                        /* clear out the MO queue since message was sent */
                        sendCommandandExpectPrefix("AT+SBDD0", "OK", satResponseTimeout);
                        _MOQueued = false;
                } else  {
                        _lastSessionResult = -mo_st;
                }
        } else if (strncmp_P(_receivedCmd, PSTR("SBDRING"), 8) == 0) {
                //DebugMsg::msg_P("SAT", 'D', PSTR("Match RING"));
                _bRing = true;
        } else if (strncmp_P(_receivedCmd, PSTR("+CIEV:"), 6) == 0) {
                //DebugMsg::msg("SAT", 'D', "Match CIEV");
                if (_receivedCmd[6] == '0') {
                        /* signal level 0 - 5 */
                        _signal = _receivedCmd[8] - '0';
                } else {
                        /* network available */
                        if (_receivedCmd[0] == '1')  {
                                _networkAvailable = true;
                                _networkStateChanged = true;
                        } else {
                                _networkAvailable = false;
                                _signal = 0;
                                _networkStateChanged = true;
                        }
                }
        }
}

/* implements the loop used to poll for data from the modem
 * and call chcker_func() when ever full response is received. If the 
 * function return true the loop will terminate, otherwise the response
 * will be handed over to parseMassage
 */
bool Iridium9602::expectLoop(const void * response,
                             unsigned long timeout,
                             bool clear_received,
                             bool (*checker_func)(const Iridium9602 &, const void *))
{
        unsigned long starttime = millis();

#if _WS_DEBUG
        if (_rcvIdx != 0) {
                DebugMsg::msg_P("SAT", 'D', PSTR("_rcvIdx is not 0 at start of %s"), __func__);
        }
#endif
		//wdtrst();
        /* always run at least one loop iteration */
        do {
                /* time left */
                unsigned long to = timeout - (millis() - starttime);
                /* make sure that we don't pass in 0 or we'll block */
                if (to == 0) to = 1; /* use 1 ms */

#if 0
                DebugMsg::msg_P("SAT", 'I', PSTR("%s: timeout: %d to: %d st: %d ml: %d"), 
                                __func__, timeout, to, starttime, millis());
#endif
                if (!checkIncomingCRLF(to)) {
                        continue;
                }


                if (checker_func(*this, response)) {
                        if (clear_received) {
                                clearIncomingMsg();
                        }
                        return true;
                }

                /* did not match the desired reponse, parse for asynchronous stuff */
                parseUnsolicitedResponse(_receivedCmd);
                clearIncomingMsg();
        } while(timeout == 0 || millis() - starttime < timeout);

        return false;
}

static bool __prefixCheck(const Iridium9602 & sat, const void * data)
{

        /* If we are here then _receivedCmd should have the response */
        if (strncmp(sat.get_receivedCmd(), (const char *)data, strlen((const char *)data)) == 0) {
                return true;
        }

        return false;
}

bool Iridium9602::expectPrefix(const char * response,
                               unsigned long timeout,
                               bool clear_received)
{

        return expectLoop(response, timeout, clear_received, __prefixCheck);
}

static bool __prefixCheck_P(const Iridium9602 & sat, const void * data)
{
        /* If we are here then _receivedCmd should have the response */
        if (strcmp_P(sat.get_receivedCmd(), (PGM_P)data) >= 0) {
                return true;
        }

        return false;
}

bool Iridium9602::expectPrefix(const __FlashStringHelper * response,
                               unsigned long timeout,
                               bool clear_received)
{
        return expectLoop(response, timeout, clear_received, __prefixCheck_P);
}

void Iridium9602::sendCommand(const char * command)
{
        _SoftwareSerial.print(command);
        _SoftwareSerial.print("\r\n");
}

void Iridium9602::sendCommand(const __FlashStringHelper * command)
{

        _SoftwareSerial.print(command);
        _SoftwareSerial.print("\r\n");
}

bool Iridium9602::sendCommandandExpectPrefix(const char * command,
                                             const char * response,
                                             unsigned long timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::sendCommandandExpectPrefix(const __FlashStringHelper * command,
                                             const __FlashStringHelper * response,
                                             unsigned long timeout,
                                             bool clear_received)
{
        sendCommand(command);
        return expectPrefix(response, timeout, clear_received);
}

bool Iridium9602::checkIncomingCRLF(unsigned long timeout)
{
        unsigned long endtime = 0;
        char inChar;

        /* calculate end time for our loop */
        if (timeout > 0) {
                endtime = millis() + timeout;
        }

        /* execute at leat once */
        do 
        {
                /* this is here to keep on reading as long as modem says that 
                 * data is available with out caring for the timeout
                 */
quick_restart:
                /* want to make sure that we have enough space in the buffer to
                 * null terminate it
                */
                if (_rcvIdx >= MAX_RECV_BUFFER - 1) {
                        /* at this point the things are screwed up enough that a reboot is best */
#if _WS_DEBUG
                        _receivedCmd[_rcvIdx] = '\0';
#endif
                        clearIncomingMsg();
                }

                if (_SoftwareSerial.available())
                {
                        inChar = _SoftwareSerial.read();
                        /* ignore white space chars at start of line */
                        if (_rcvIdx == 0 && (inChar == '\r' || inChar == '\n')) continue;
                        _receivedCmd[_rcvIdx++] = (unsigned char )inChar;
                        _receivedCmd[_rcvIdx] = 0;
                        if (_rcvIdx >= 2 && _receivedCmd[_rcvIdx - 2] == '\r' && _receivedCmd[_rcvIdx - 1] == '\n') {
                                /* get rid of trailing white spaces */
                                while (_receivedCmd[_rcvIdx-1] == '\r' || _receivedCmd[_rcvIdx-1] == '\n') {
                                        _receivedCmd[--_rcvIdx] = '\0';
                                }
                                return true;
                        }
                        goto quick_restart;
                }
        /*
         * second compound statement will only execute if
         * timeout != 0.
         *
         * Will loop as long as timeout is 0 or endtime
         * is less than current time (millis())
         */
        } while(timeout == 0 || millis() < endtime);

        return false;
}

int Iridium9602::checkSignal()
{
/*XXXX */
#if 1
        if (sendCommandandExpectPrefix("AT+CSQ", "+CSQ", 50000, false)) {
                _signal = (_receivedCmd[5] - '0');  //Convert ASCII number to integer
                
        } else {
                _signal = '\0'; // Return a null if unable to get a response
        }
#else
	_signal = 5;
#endif

        return _signal;
}



bool Iridium9602::setIncommingMsgAlert(bool bEnable)
{

        if (bEnable)
        {
                return sendCommandandExpectPrefix(F("AT+SBDMTA=1"), F("OK"), satResponseTimeout);
        } else {
                return sendCommandandExpectPrefix(F("AT+SBDMTA=0"), F("OK"), satResponseTimeout);
        }
}

bool Iridium9602::setIndicatorReporting(bool bEnable)
{
        if (bEnable) {
                return sendCommandandExpectPrefix(F("AT+CIER=1,1,1"), F("OK"), satResponseTimeout);
        } else {
                return sendCommandandExpectPrefix(F("AT+CIER=0,0,0"), F("OK"), satResponseTimeout);
        }
}

int Iridium9602::getMessageWaitingCount(void)
{
        return _GSSQueued;
}

int Iridium9602::getRecentMTStatus(void)
{
        return _MTStatus;
}


unsigned char Iridium9602::wait_read(void)
{

        while (!_SoftwareSerial.available()) 
                ;
                
       return _SoftwareSerial.read();
}

int Iridium9602::loadMTMessage(unsigned char * msg, int msg_sz)
{
        unsigned char inChar = 0;
        int br = 0; /* bytes read */
        unsigned long checksum = 0;
        unsigned long checksum_from_modem = 0;
        unsigned long msgLen = 0;

        sendCommand("AT+SBDRB");

        msgLen = (inChar = wait_read()) << 8;
        msgLen = (inChar |= _SoftwareSerial.read());

        while (br < msgLen) {
                inChar = _SoftwareSerial.read();
                /* ignore white space chars at start of line */
                msg[br++] = inChar;
                checksum += inChar;
        }



        checksum_from_modem = (inChar = _SoftwareSerial.read()) << 8;

        checksum_from_modem |= (inChar = _SoftwareSerial.read());


        sendCommandandExpectPrefix("AT+SBDD1", "OK", satResponseTimeout);
        
        if (checksum_from_modem != checksum) {

                br = 0;
        }

        _MTMsgLen = 0;
        _MTQueued--; 
        return br;
}

bool Iridium9602::isSatAvailable(void)
{
/*XXXX */
#if 1
        if (digitalRead(RB_NET) == HIGH) 
        {
                return true;
        } 
#else
	return true;
#endif

        return false;
}

void Iridium9602::powerOff(void)
{
        sendCommandandExpectPrefix(F("AT+*F"), F("OK"), satResponseTimeout);      //Make sat modem prepare for poweroff
        //Wait until OK for up to 10 seconds
        digitalWrite(RB_SLEEP,LOW);  //Power modem off.
}

void Iridium9602::powerOn(void)
{
        Iridium9602::initModem();
}

bool Iridium9602::isModemOn(void)
{
        //digitalRead(pinDSR) == LOW)  //Low == 9602 is powered on
        //{
        //        return true;
        //}
        //return false;
}

bool Iridium9602::isSimulatorPresent(void)
{
        for (int i = 0; i < 10; i++) {
                if (sendCommandandExpectPrefix(F("RUASIM?"), F("YES"), satResponseTimeout))      //Ask if modem is really a simulator
                {
                        return true;
                }
        }

        return false;
}



//FUNCTION: Write binary data to sat modem MO Queue 
bool Iridium9602::loadMOMessage(unsigned char* messageArray, int messageLength) 
{
        //Compute Checksum
        unsigned long checksum = 0;
        char buf[15];
        byte checksumHighByte;
        byte checksumLowByte;
        //Checksum is 2-byte summation of entire SBD message, high order byte first.  
        for (int i = 0; i < messageLength; i++){
                checksum += messageArray[i]; 
        }
        checksumHighByte = highByte(checksum);
        checksumLowByte = lowByte(checksum);
        snprintf(buf, sizeof(buf), "AT+SBDWB=%d", messageLength);
        sendCommand(buf);

        if (!expectPrefix(F("READY"), satResponseTimeout)) return false;



        /* write message length */
        checksum = _SoftwareSerial.write(messageArray, messageLength);
        /* write checksum bytes */
        checksum += _SoftwareSerial.write(&checksumHighByte, 1);
        checksum += _SoftwareSerial.write(&checksumLowByte, 1);

        /* check that we wrote expected number of bytes write calls above failed */
        if (checksum != messageLength + 2) return false;



        /* XXX Don't know if new line is needed */
        _SoftwareSerial.println();
        if (!expectPrefix(F("0"), satResponseTimeout)) return false;
        /* we hope that we get it, 
         * but I've seen sometime no OK.
         * */
        if (!expectPrefix(F("OK"), satResponseTimeout)) {
                /* XXX */
        }

        _MOQueued = true;
        return true;
}

bool Iridium9602::initiateSBDSession(unsigned long timeout)
{
#if 0
        _SoftwareSerial.println("AT+SBDD0");
        checkIncomingCRLF(2000);
#endif
        bool ret = false;
        

		
        if (_sessionInitiated) goto out;

        if (_bRing) {
        	ret = sendCommandandExpectPrefix(F("AT+SBDIXA"), F("OK"), timeout);
        } else {
        	ret = sendCommandandExpectPrefix(F("AT+SBDIX"), F("OK"), timeout);
        }
        _sessionInitiated = true;
        _bRing = false;

out:
        return ret;
}

bool Iridium9602::pollUnsolicitedResponse(unsigned long timeout)
{
        unsigned long starttime = millis();

        do {
                unsigned long to = timeout - (millis() - starttime);
                if (to == 0) to = 1;
                if (checkIncomingCRLF(to)) {

                        parseUnsolicitedResponse(_receivedCmd);
                        clearIncomingMsg();
                }
        } while(timeout == 0 || millis() - starttime < timeout);

        if (millis() - _lastSessionTime > satSBDIXResponseLost) {
                _sessionInitiated = false;
                _lastSessionResult = 0;
        }

        return false;
}


