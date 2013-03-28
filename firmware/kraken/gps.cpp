/**
  * gps.cpp
  *
  * Part of the Kraken Ocean Drifter
  * http://poseidon.sgsphysics.co.uk
  *
  * Various Authors
  * (C) SGS Poseidon Project 2013
  */

#include "gps.h"

void gps_setup(){

    Serial.println("GPS: Starting up");
    delay(50);
   
    Serial.begin(9600);
    Serial.flush();

    setSeaMode();
    setOutputUBX();

}



uint8_t getLocation(int32_t* lat, int32_t* lon, int32_t* alt)
{
  // Request a NAV-POSLLH message from the GPS
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x02, 0x00, 0x00, 0x03,
        0x0A};
  
    sendUBX(request, 8);

    while(Serial.available())
      Serial.read();
    Serial.flush();
    uint8_t buf[36];
    if (!getBytes(36,buf))
      return 1;
  
  //for (int i =0; i < 36; i++){
  //  Serial.print(buf[i],HEX);
  // Serial.print(", "); 
  //}

    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 )
        return 2;
    if( buf[2] != 0x01 || buf[3] != 0x02 )
        return 3;

    // 4 bytes of longitude (1e-7)
    *lon = (int32_t)buf[10] | (int32_t)buf[11] << 8 | 
        (int32_t)buf[12] << 16 | (int32_t)buf[13] << 24;
    
    // 4 bytes of latitude (1e-7)
    *lat = (int32_t)buf[14] | (int32_t)buf[15] << 8 | 
        (int32_t)buf[16] << 16 | (int32_t)buf[17] << 24;
    
    // 4 bytes of altitude above MSL (mm)
    *alt = (int32_t)buf[22] | (int32_t)buf[23] << 8 | 
        (int32_t)buf[24] << 16 | (int32_t)buf[25] << 24;
        
    return 0;
}

uint8_t gps_get_time(uint8_t* hour, uint8_t* minute, uint8_t* second)
{
    // Send a NAV-TIMEUTC message to the receiver
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x21, 0x00, 0x00,
        0x22, 0x67};
        
    sendUBX(request, 8);
    
    while(Serial.available())
      Serial.read();
    Serial.flush();
    uint8_t buf[28];
    if (!getBytes(28,buf))
      return 1;


    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 )
        return 2;
    if( buf[2] != 0x01 || buf[3] != 0x21 )
        return 3;

    *hour = buf[22];
    *minute = buf[23];
    *second = buf[24];

    return 0;
}

uint8_t gps_check_lock(uint8_t* lock, uint8_t* sats)
{
    // Construct the request to the GPS
    uint8_t request[8] = {0xB5, 0x62, 0x01, 0x06, 0x00, 0x00,
        0x07, 0x16};
         
    sendUBX(request, 8);
    
    while(Serial.available())
      Serial.read();
    Serial.flush();
    uint8_t buf[60];
    if (!getBytes(60,buf))
      return 1;

    // Verify the sync and header bits
    if( buf[0] != 0xB5 || buf[1] != 0x62 )
        return 2;
    if( buf[2] != 0x01 || buf[3] != 0x06 )
        return 3;

  
    // Return the value if GPSfixOK is set in 'flags'
    if( buf[17] & 0x01 )
        *lock = buf[16];
    else
        *lock = 0;

    *sats = buf[53];
    
    return 0;
}

uint8_t CK_A=0x00;  //checksum variables used to configure GPS
uint8_t CK_B=0x00;
uint8_t gps_set_success = 0;

bool setOutputUBX(){
  uint8_t setUBX[] = {0xB5, 0x62, 0x06, 0x00, 0x14, 0x00, 0x01, 0x00, 0x00, 0x00, 0xD0, 0x08, 0x00, 0x00, 0x80, 0x25, 0x00, 0x00, 0x03, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x89};
   // while(!gps_set_success)
   // {
        sendUBX(setUBX, sizeof(setUBX)/sizeof(uint8_t));
   //   gps_set_success=getUBX_ACK(setUBX);
   // }
   //gps_set_success=0;
    
    Serial.println("GPS: UBX mode set.");
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
        sendUBX(setNav, sizeof(setNav)/sizeof(uint8_t));
        gps_set_success=getUBX_ACK(setNav);
    }
    gps_set_success=0;
    Serial.println("GPS: Sea mode set.");
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

bool getBytes(uint8_t count, uint8_t* buff)
{
 unsigned long startTime = millis();
  uint8_t t = 0;
  uint8_t b;
  bool start_found = false;
  while (1) {
 
    // Test for success
    if (t >= count) {
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

      if (!start_found){
        if (b == 0xB5){
          start_found = true;
          t++;
          *buff = b;
          buff++;
        }
      }
      else{
        t++;
        *buff = b;
        buff++;   
      } 
    }    
  }
  return true;
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

