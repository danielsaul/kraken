/**
  * gps.cpp
  *
  * Part of the Kraken Ocean Drifter
  * http://poseidon.sgsphysics.co.uk
  *
  * Greg Brooks & Daniel Saul
  * (C) SGS Poseidon Project 2013
  */

#include "gps.h"
uint8_t CK_A=0x00;  //checksum variables used to configure GPS
uint8_t CK_B=0x00;
uint8_t gps_set_success = 0;
void gps_setup(){

    Serial.println("Starting");
    delay(50);
    Serial.begin(9600);
    Serial.flush();

 //   setSeaMode();
    setOutputUBX();
    setNavSolOff();
    setNavPosLLH();

    if(Serial.available()){
        delay(50);
     char datain;
     char packet[36];
      for (int x=0; x<36; x++){
        datain = Serial.read();
        packet[x]=datain;//read the incoming binary packet into array 'packet'
      }

    Serial.println(packet);
    }



}

bool setNavSolOff(){
uint8_t Buf[]={0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
 CK_A = 0x00, CK_B = 0x00; 
 for(int x=0;x<12;x++)
 {
     CK_A = CK_A + Buf[x];
     CK_B = CK_B + CK_A;
 }
  uint8_t unsetport[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CK_A, CK_B};//don't forget to calculate checksum (page84) then add that to end of array
 unsigned long starttime = millis(); 
    while(!gps_set_success)
    {
        if (millis() - starttime > gps_timeout){
            gps_set_success=0;
            return false;
        }

        sendUBX(unsetport, sizeof(unsetport)/sizeof(uint8_t));
        gps_set_success=getUBX_ACK(unsetport);

    }
    gps_set_success=0;
    return true;


}

bool setNavPosLLH(){

uint8_t buf[]={0x06, 0x01, 0x08, 0x00, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00};
 CK_A = 0x00, CK_B = 0x00; 
 for(int x=0;x<12;x++)
 {
     CK_A = CK_A + buf[x];
     CK_B = CK_B + CK_A;
 }
  uint8_t setport[] = {0xB5, 0x62, 0x06, 0x01, 0x08, 0x00, 0x01, 0x02, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, CK_A, CK_B};//don't forget to calculate checksum (page84) then add that to end of array - done
  unsigned long starttime = millis(); 
    while(!gps_set_success)
    {
        if (millis() - starttime > gps_timeout){
            gps_set_success=0;
            return false;
        }

        sendUBX(setport, sizeof(setport)/sizeof(uint8_t));
        gps_set_success=getUBX_ACK(setport);

    }
    gps_set_success=0;
    return true;
    

}

bool setOutputUBX(){
  uint8_t Buffer[]={0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0xC0, 0x12, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00};
  CK_A = 0x00, CK_B = 0x00;
 for(int I=0;I<24;I++)
 {
     CK_A = CK_A + Buffer[I];
     CK_B = CK_B + CK_A;
 }
  uint8_t setUBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0xC0, 0x12, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, CK_A, CK_B};//don't forget to calculate checksum (page84) then add that to end of array - done
  unsigned long starttime = millis(); 
    while(!gps_set_success)
    {
        if (millis() - starttime > gps_timeout){
            gps_set_success=0;
            return false;
        }

        sendUBX(setUBX, sizeof(setUBX)/sizeof(uint8_t));
        gps_set_success=getUBX_ACK(setUBX);

    }
    gps_set_success=0;
Serial.println("UBX mode set.");
    return true;
}

bool setSeaMode(){
    uint8_t B[]={0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    CK_A = 0x00, CK_B = 0x00;
    for(int I=0;I<40;I++)
    {
        CK_A = CK_A + B[I];
        CK_B = CK_B + CK_A;
    }
    uint8_t setNav[] = {0xB5, 0x62, 0x06, 0x24, 0x24, 0x00, 0xFF, 0xFF, 0x05, 0x01, 0x00, 0x00, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x05, 0x00, 0xFA, 0x00, 0xFA, 0x00, 0x64, 0x00, 0x2C, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, CK_A, CK_B                        };
        
    unsigned long starttime = millis(); 
    while(!gps_set_success)
    {
        /*if (millis() - starttime > gps_timeout){
            gps_set_success=0;
            return false;
        }*/
        sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));
        gps_set_success=getUBX_ACK(setNav);
    }
    gps_set_success=0;
    Serial.println("Sea mode set.");
    return true;
}


void gps_sleep(){

    uint8_t GPSoff[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x08, 0x00, 0x16, 0x74};
    sendUBX(GPSoff, sizeof(GPSoff)/sizeof(uint8_t));

}

void gps_wake(){

 uint8_t GPSon[] = {0xB5, 0x62, 0x06, 0x04, 0x04, 0x00, 0x00, 0x00,0x09, 0x00, 0x17, 0x76};
 sendUBX(GPSon, sizeof(GPSon)/sizeof(uint8_t));

}


// Send a byte array of UBX protocol to the GPS
void sendUBX(uint8_t *MSG, uint8_t len) {
  for(int i=0; i<len; i++) {
    Serial.write(MSG[i]);
  }
}


bool getUBX_ACK(uint8_t *MSG) {
  uint8_t b;
  uint8_t ackByteID = 0;
  uint8_t ackPacket[10];
  unsigned long startTime = millis();
  
 
  // Construct the expected ACK packet    
  ackPacket[0] = 0xB5;  // header
  ackPacket[1] = 0x62;	// header
  ackPacket[2] = 0x05;	// class
  ackPacket[3] = 0x01;	// id
  ackPacket[4] = 0x02;	// length
  ackPacket[5] = 0x00;
  ackPacket[6] = MSG[2];	// ACK class
  ackPacket[7] = MSG[3];	// ACK id
  ackPacket[8] = 0;		// CK_A
  ackPacket[9] = 0;		// CK_B
 
  // Calculate the checksums
  for (uint8_t i=2; i<8; i++) {
    ackPacket[8] = ackPacket[8] + ackPacket[i];
    ackPacket[9] = ackPacket[9] + ackPacket[8];
  }
 
  while (1) {
 
    // Test for success
    if (ackByteID > 9) {
      // All packets in order!
      
      return true;
    }
 
    // Timeout if no valid response in 3 seconds
    if (millis() - startTime > 3000) { 
      
      return false;
    }
 
    // Make sure data is available to read
    if (Serial.available()) {
      b = Serial.read(); 
      // Check that bytes arrive in sequence as per expected ACK packet
      if (b == ackPacket[ackByteID]) { 
        ackByteID++;
        
      } 
      else {
        ackByteID = 0;	// Reset and look again, invalid order
      }
 
    }
  }
}

