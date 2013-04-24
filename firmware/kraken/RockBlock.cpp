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
char receivedCmd[100];
int receivedIdx;
bool ring;
bool sendQueued;
bool sendStatus;
int rcvStatus;
int rcvLength;
int rcvQueue;
int netQueue;


void rockblock_init() {
    pinMode(RB_SLEEP, OUTPUT);
    rockblock_off();
}

void rockblock_on() {
    rb.begin(19200);

    // Set variables
    receivedIdx = 0;
    ring = false;
    sendQueued = false;
    rcvQueue = 0;
    netQueue = 0;
    rcvLength = 0;

    // Wait 5secs to ensure modem is fully powered down before turning on again
    unsigned long starttime = millis();
    while(millis() < starttime + 5000){}

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
        Serial.println("RB: Turned on");

}

void rockblock_off() {
    sendCommandandExpectPrefix("AT*F", "OK", 10000);
    digitalWrite(RB_SLEEP, LOW);
    if(SERIAL_EN)
        Serial.println("RB: Turned off");
}

bool initiateSession(){
    
    if(!isSatAvailable() || checkSignal() < minimumSignalRequired) return false;
    if(SERIAL_EN)
        Serial.println("RB: Satellite Available");

    bool result = false;
    if(ring){
        result = sendCommandandExpectPrefix("AT+SBDIXA", "+SBDIX:", responseLost);
        ring = false;
    }else{
        result = sendCommandandExpectPrefix("AT+SBDIX", "+SBDIX:", responseLost);
    }
    
    if(result){
        if(SERIAL_EN)
            Serial.println(receivedCmd);
        parseSBDIX();
    }

    return result;
}

bool loadMessage(unsigned char *msg, int length){
    for(uint8_t tries = 0; tries < 5; tries++){
        unsigned long checksum = 0;
        char buf[15];
        for(int i = 0; i < length; i++){
            checksum += msg[i];
        }
        byte checksumHighByte = highByte(checksum);
        byte checksumLowByte = lowByte(checksum);
        snprintf(buf, sizeof(buf), "AT+SBDWB=%d", length);
        if(!sendCommandandExpectPrefix(buf, "READY", responseLost)) continue;

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

int readMessage(unsigned char *msg, int maxLength){
    
    sendCommand("AT+SBDRB");

    unsigned long endtime = millis() + responseLost;
    while(!rb.available()){
        if (millis() > endtime) return -1;
    }

    unsigned char inChar = 0;
    int bytesRead = 0;
    unsigned long checksum = 0;
    unsigned long rcvdChecksum = 0;
    unsigned long msgLength = 0;

    msgLength = (inChar = rb.read()) << 8;
    msgLength = (inChar |= rb.read());

    if (msgLength > maxLength) return -1;

    while(bytesRead < msgLength){
        inChar = rb.read();
        msg[bytesRead++] = inChar;
        checksum += inChar;
    }

    rcvdChecksum = (inChar = rb.read()) << 8;
    rcvdChecksum |= (inChar = rb.read());
    if(rcvdChecksum != checksum) return -1;
        
    sendCommandandExpectPrefix("AT+SBDD1", "OK", responseLost);
    rcvLength = 0;
    rcvQueue--;

    return bytesRead;
}

#define NEXT_VAL(v) \
do { \
        int __tmp;                                              \
        /* skip white spaces */                                 \
        while(*p && *p == ' ') p++;                             \
        /* convert MO status to int, n point to first char */   \
        /* after the number */                                  \
        v = strtol(p, &n, 10);                                  \
        /* if p == n then no number was found */                \
        if (p == n) /* no number */                             \
            return;                                             \
        p = n;                                                  \
        /* have to be at EOL or at a ',' */                     \
        if (*p != ',' && *p != '\0') return;                    \
        /* p should point at start of next number or space */   \
        p++;                                                    \
} while (0)

void parseSBDIX(){
    char *p, *n = NULL;
    p = receivedCmd + 7; // skip +SBDIX:
    /* <MO status>,<MOMSN>,<MT status>,<MTMSN>,<MT length>,<MT queued> */
    int mo_st = -1, mt_st, mt_len, mt_q;
    NEXT_VAL(mo_st);

    p = strchr(p, ',');
    p++;

    NEXT_VAL(mt_st);
    rcvStatus = mt_st;
    p = strchr(p, ',');
    p++;

    NEXT_VAL(mt_len);

    NEXT_VAL(mt_q);

    netQueue = mt_q;

    if(mt_st == 1){
        rcvQueue = mt_q + 1;
        rcvLength = mt_len;
    }
    
   expectResponse("OK", responseLost); 
   
    if(sendQueued && mo_st >= 0 && mo_st <= 4){
        sendCommandandExpectPrefix("AT+SBDD0", "OK", responseLost);
        sendStatus = true;
    }else{
        sendStatus = false;
    }
    sendQueued = false;
}



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
        if(strncmp(receivedCmd, response, strlen(response)) == 0) return true;
    
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

bool messageSent(){
    return sendStatus;
}

int messagesToReceive(){
    return rcvQueue;
}

int messagesWaitingOnNetwork(){
    return netQueue;
}

bool messageAvailableToRead(){
    if(rcvStatus == 1){
        return true;
    }else{
        return false;
    }
}
