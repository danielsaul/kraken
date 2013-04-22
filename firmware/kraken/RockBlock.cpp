/**
 * RockBlock.cpp
 *
 * Part of the Kraken Ocean Drifter
 * http://poseidon.sgsphysics.co.uk
 *
 * Daniel Saul
 * (C) SGS Poseidon Project 2013
 */

#include "RockBlock.h"

SoftwareSerial rb(RB_RX, RB_TX);
Iridium9602 iridium = Iridium9602(rb);

void rockblock_init() {
    pinMode(RB_SLEEP, OUTPUT);
    digitalWrite(RB_SLEEP, HIGH); // Turn RB on so if can be powered off without timing out
    rockblock_off();
}

bool loadMessage(unsigned char *msg, int length){
    while(uint8_t tries = 0; tries < 15; tries++){
        unsigned long checksum = 0;
        char buf[15];
        for(int i = 0; i < length; i++){
            checksum += msg[i];
        }
        byte checksumHighByte = highByte(checksum);
        byte checksumLowByte = lowByte(checksum);
        snprintf(buf, sizeof(buf), "AT+SBDWB=%d", length);
        if(!sendCommandandExpectPrefix("READY", responseLost)) continue;

        checksum = rb.write(msg, length);
        checksum += rb.write(&checksumHighByte, 1);
        checksum += rb.write(&checksumLowByte, 1);
        if(checksum != length+2) continue;
        rb.println();
        if(expectResponse("0", responseLost)){
            if(SERIAL_EN)
                Serial.println("RB: Message loaded");
            sendQueued = true;
            return true;
        }
    }
    return false;
}

bool initiateSession(){
    
    if(!isSatAvailable() || checkSignal() < minimumSignalRequired) return false;
    if(SERIAL_EN)
        Serial.println("RB: Satellite Available");

    bool result = false;
    if(ring){
        result = sendCommandandExpectPrefix("AT+SBDIXA", "+SBDIX:", responseLost);
    }else{
        result = sendCommandandExpectPrefix("AT+SBDIX", "+SBDIX:", responseLost);
    }
    
    if(result){
        parseSBDIX();
    }

    ring = false;
    return result;
}

bool getNextVal(char * p, char * n, int * v){
        int __tmp;                                              
        /* skip white spaces */                                 
        while(*p && *p == ' ') p++;                             
        /* convert MO status to int, n point to first char */   
        /* after the number */                                  
        __tmp = strtol(p, &n, 10);                              
        if ((v))                                                
            *(v) = __tmp;                                   
        /* if p == n then no number was found */                
        if (p == n) /* no number */                             
            return false;                                   
        p = n;                                                  
        /* have to be at EOL or at a ',' */                     
        if (*p != ',' && *p != '\0') return false;              
        /* p should point at start of next number or space */  
        p++;                                                    
        return true;
}

void parseSBDIX(){
    char *p, *n = NULL;
    p = receivedCmd + 7; // skip +SBDIX:
    /* <MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued> */
    int mo_st = -1, mt_st, mt_len, mt_q;
    
    if(!getNextVal(&mo_st)) return;
    if(sendQueued && mo_st >= 0 && mo_st <= 4){
        sendStatus = true;
    }else{
        sendStatus = false;
    }
    sendQueued = false;
    
    p = strchr(p, ',');
    p++;

    if(!getNextVal(&mt_st)) return;
    rcvStatus = mt_st;

    p = strchr(p, ',');
    p++;

    if(!getNextVal(&mt_len)) return;
    if(!getNextVal(&mt_q)) return;
    netQueue = mt_q;

    if(mt_st == 1){
        rcvQueue = mt_q + 1;
        rcvLength = mt_len;
    }
   //expectResponse("OK", responseLost); 
}

void rockblock_on() {
    rb.begin(19200);
    //iridium.powerOn();

    // Set variables
    receivedIdx = 0;

    if(SERIAL_EN)
        Serial.println("RB: Turning Sat Modem On...");

    digitalWrite(RB_SLEEP, HIGH);
    while(!sendCommandandExpectPrefix("AT", "OK", 500)){}

    if(SERIAL_EN)
        Serial.println("RB: Responding");

    sendCommandandExpectPrefix("ATE0", "OK", 1000);     // Serial Character Echo Off
    sendCommandandExpectPrefix("AT&K0", "OK", 1000);    // No flow control handshaking
    sendCommandandExpectPrefix("ATQ0", "OK", 1000);     // Turn responses on
    // Turn off incoming message alert?
    sendCommandandExpectPrefix("AT&W0", "OK", 1000);    // Write defaults to modem & flush to eeprom
    sendCommandandExpectPrefix("AT&Y0", "OK", 1000);    // Set default reset Profile
    // Set indicator reporting to true?
    // Turn on incoming message alert?
    
    if(SERIAL_EN)
        Serial.println("RB: Setup complete");

}

void rockblock_off() {
    //iridium.powerOff();
    sendCommandandExpectPrefix("AT*F", "OK", 10000);
    digitalWrite(RB_SLEEP, LOW);
    if(SERIAL_EN)
        Serial.println("RB: Turned off");
}

/////

bool sendCommandandExpectPrefix(const char * command, const char * response, unsigned long timeout) {
    sendCommand(command);
    return expectResponse(response, timeout);
}

void sendCommand(const char * command){
    rb.print(command);
    rb.print("\r\n");
}

bool expectResponse(const char * response, unsigned long timeout){
    unsigned long starttime = millis();

    do {
        clearReceivedCmd();

        unsigned long timeleft = timeout - (millis() - starttime);
        if (timeleft == 0) timeleft = 1;
        if (!receiveCmdCRLF(timeleft)) continue;
   
        if(strncmp(receivedCmd, response, strlen(response) == 0)) return true;

        // Didn't match, see if it was anything else
        if(strncmp(receivedCmd, "SBDRING", 8) == 0){
            ring = true;
        }

    } while(timeout == 0 || millis() - starttime < timeout);

    return false;
}

// We want an entire line with \n\r at the end
bool receiveCmdCRLF(unsigned long timeout){
    unsigned long endtime = 0;
    char inChar;
    if(timeout > 0)
        endtime = millis() + timeout;

    do{

        while(rb.available()){
            
            if(receivedIdx >= 99) clearReceivedCmd();

            inChar = rb.read();
            if(receivedIdx== 0 && (inChar == '\r' || inChar == '\n')) continue; //Ignore whitespace at start of line
            receivedCmd[receivedIdx++] = (unsigned char)inChar;
            receivedCmd[receivedIdx] = 0;
            if(receivedIdx >= 2 && receivedCmd[receivedIdx - 2] == '\r' && receivedCmd[receivedIdx - 1] == '\n'){
                // We've got out complete line - remove whitespace at end
                while(receivedCmd[receivedIdx-1] == '\r' || receivedCmd[receivedIdx-1] == '\n'){
                    receivedCmd[--receivedIdx] = '\0';
                }
                return true;
            }
        }
    } while(timeout == 0 || millis() < endtime);

    return false;
}

void clearReceivedCmd(){
    receivedIdx = 0;
    receivedCmd[0] = '\0';
}


int checkSignal(){
    if(sendCommandandExpectPrefix("AT+CSQ", "+CSQ", 50000)){
        return receivedCmd[5] - '0';
    }else{
        return '\0';
    }
}

bool isSatAvailable(){
    if(digitalRead(RB_NET) == HIGH){
        return true;
    }else{
        return false;
    }
}
