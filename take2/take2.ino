//#include <tinySPI.h>
#define NOP __asm__ __volatile__ ("nop\n\t")
//Slave comm on tiny
int CE=1;
int IRQ=9;
int CSN=2;
int ledPin = 3;
int LAMP=7;

byte tester = 0;

byte addrRequest = 128;

byte newAddr = 0x06;
//temp start addr is 0xE7E706, need to set tx_addr and rx_addr_0 to this


//====================NORMAN's CODE======================
// Variables relevant to the battery
int battery = 255;
boolean usingBattery = false;

// These are lookup tables for flickers
//byte dipTable1[32];
int dipFrame1 = -1;
//byte dipTable2[256];
int dipFrame2 = -1;
static const uint8_t dipTable1[33] ={255,240,225,210,195,180,165,150,135,120,105,90,75,60,45,30,15,30,45,60,75,90,105,120,135,150,165,180,195,210,225,240};
static const uint8_t dipTable2[256] ={255,251,247,243,239,235,231,227,223,219,215,211,207,203,199,195,191,187,183,179,175,171,167,163,159,155,151,147,143,139,135,131,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,127,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,195,199,203,207,211,215,219,223,227,231,235,239,243,247,251};
// These are used to keep timing
unsigned long lastMilli;

// These are lookup tables for blackout
//int blackoutFrame = 256;
//byte boTable[256];

//static const uint8_t boTable[256] ={255,239,223,207,191,175,159,143,127,131,135,139,143,147,151,155,159,163,167,171,175,179,183,187,191,168,145,122,99,76,53,30,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,15,25,35,45,55,65,75,85,95,105,115,125,135,145,155,165,175,185,195,205,215,225,235,245};
//=====================END NORMANS CODE=============================


void setup(){
  
  //lastMilli = millis();
  
  
delay(100);
pinMode(ledPin, OUTPUT);
digitalWrite(ledPin, HIGH);
pinMode(LAMP, OUTPUT);/*
pinMode(IRQ, INPUT);
pinMode(CSN, OUTPUT);
digitalWrite(CSN, HIGH);
pinMode(CE, OUTPUT);
digitalWrite(CE,LOW);
SPI.setDataMode(SPI_MODE0);
SPI.begin();
nrfSlavOn();
delay(5);
writeReg(0x00,0b00001110); //powerup
flushBuffers();
delay(5);
digitalWrite(CE, HIGH);
delay(5);*/
}
void loop(){/*
if(digitalRead(IRQ)==LOW)
{
//byte statusReg = readReg(0x07);
writeReg(0x07, 0b01110000);
flushBuffers();
}
transmit(tester);
delay(50);
digitalWrite(ledPin, tester%2);
tester++;
if(tester==addrRequest)
  tester++;
analogWrite(LAMP, tester);
*/
//===============================NORMANS STUFF===========================================
  //if (millis() - lastMilli >= 15)
  //{
    // This stuff does the flickering
    if (dipFrame1 < 0 && random(64) < 1)
      dipFrame1 = 0;
    if (dipFrame2 < 0 && random(255) < 1)
      dipFrame2 = 0;
  
    byte brightness = 255;

    
    if (dipFrame1 >= 0)
    {
      brightness = dipTable1[dipFrame1];
      dipFrame1 += 1;
  
      if (dipFrame1 >= 32)
      {
        dipFrame1 = -1;
      }
    }
    if (dipFrame2 >= 0)
    {
      brightness = dipTable2[dipFrame2];
      dipFrame2 += 1;
  
      if (dipFrame2 >= 255)
      {
        dipFrame2 = -1;
      }
    }
    
  
     if(digitalRead(0)){
       analogWrite(LAMP, brightness);
     }else{
       analogWrite(0);
     }
     delay(1);
    //lastMilli += 15;
  //}
  
  //========================END NORMANS STUFF=========================
}

/*
void nrfSlavOn(){
writeReg(0x00,0b00001100); //makes tx mode
writeReg(0x01,0b00000001); //autoACK for all pipes
writeReg(0x02,0b00000011); //enable all pipes
writeReg(0x03,0b00000001); //3 byte addresses
writeReg(0x04,0b01001111);// retransmit delay, and up to 15 retries
writeReg(0x11, 0b00000001); //payload size max
writeAddr(0x10, 0x06); //set transmit address
writeAddr(0x0A, 0x06); //set rx address for ack
//writeReg(0x50, 0x73);
//writeReg(0x1D,0b00000100); //enable dynamic payload and ack payload
//writeReg(0x1C,0b00000001); //enable dynamic payload for pipe0
delay(5);
}
byte writeAddr(byte addr, byte index){
digitalWrite(CSN, LOW);
NOP;
byte incoming = SPI.transfer(writeRegVal(addr)); //first receive addr
NOP;
SPI.transfer(index);
NOP;
SPI.transfer(0xE7);
NOP;
SPI.transfer(0xE7);
NOP;
digitalWrite(CSN, HIGH);
NOP;
return incoming;
}
byte writeReg(byte addr, byte data){
digitalWrite(CSN, LOW);
NOP;
byte incoming = SPI.transfer(0b00100000|(0b00011111&addr));
NOP;
SPI.transfer(data);
NOP;
digitalWrite(CSN, HIGH);
NOP;
return incoming;
}
byte writeRegVal(byte addr){
return (0b00100000|(0b00011111&addr));
}
byte readReg(byte addr){
digitalWrite(CSN, LOW);
NOP;
SPI.transfer(0b00011111&addr);
NOP;
byte incoming = SPI.transfer(0xFF);
NOP;
digitalWrite(CSN, HIGH);
NOP;
return incoming;
}
byte transmit(byte data){
digitalWrite(CSN, LOW);
NOP;
SPI.transfer(0b10100000);
NOP;
SPI.transfer(data);
NOP;
digitalWrite(CSN, HIGH);
}
void flushBuffers(){
digitalWrite(CSN, LOW);
NOP;
SPI.transfer(0b11100001);
NOP;
digitalWrite(CSN, HIGH);
NOP;
digitalWrite(CSN, LOW);
NOP;
SPI.transfer(0b11100010);
NOP;
digitalWrite(CSN, HIGH);
NOP;
}
*/

