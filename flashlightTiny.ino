#include <tinySPI.h>
#define NOP __asm__ __volatile__ ("nop\n\t")

//Slave comm on tiny
int CE=1;
int IRQ=9;
int CSN=2;
int LAMP = 3;
int ledPin = 7;

byte addrRequest = 128;

byte newAddr = 0x06;

// Variables relevant to the battery
int battery = 255;
volatile boolean usingBattery = true;

// These are lookup tables for flickers
byte dipTable1[32];
int dipFrame1 = -1;
byte dipTable2[256];
int dipFrame2 = -1;

// These are used to keep timing
long frame = 0;
unsigned long lastMilli;

// These are lookup tables for blackout
volatile int blackoutFrame = 256;
byte boTable[256];

void blackout()
{
  blackoutFrame = -random(20);
}

byte tester = 0;
//temp start addr is 0xE7E706, need to set tx_addr and rx_addr_0 to this

void setup(){
  pinMode(ledPin, OUTPUT);

  // Set up the lookup tables for the flickers  
  for (int i = 0; i < 32; ++i)
  {
    if (i < 16)
      dipTable1[i] = 255-i*15;
    else
      dipTable1[i] = 255-(32-i)*15;
  }
  for (int i = 0; i < 256; ++i)
  {
    if (i < 32)
      dipTable2[i] = 255-i*4;
    else if (i < 224)
      dipTable2[i] = 255-32*4;
    else
      dipTable2[i] = 255-(256-i)*4;
  }
  // Blackout flicker
  for (int i = 0; i < 256; ++i)
  {
    if (i < 8)
      boTable[i] = 255-i*16;
    else if (i < 24)
      boTable[i] = 255-(24-i)*4-64;
    else if (i < 32)
      boTable[i] = 255-64-(i-24)*23;
    else if (i < 256-24)
      boTable[i] = 0;
    else
      boTable[i] = 255-(256-i)*10;
  }
  
  //set up rf
  lastMilli = millis();
  delay(1000);
  pinMode(LAMP, OUTPUT);
  digitalWrite(LAMP, LOW);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  pinMode(IRQ, INPUT);
  pinMode(CSN, OUTPUT);
  digitalWrite(CSN, HIGH);
  pinMode(CE, OUTPUT);
  digitalWrite(CE,LOW);
  SPI.setDataMode(SPI_MODE0);
  SPI.begin();
  digitalWrite(LAMP, HIGH);
  nrfSlavOn();
  digitalWrite(LAMP, LOW);
  delay(5);
  writeReg(0x00,0b00001110);  //powerup
  flushBuffers();
  delay(5);
  digitalWrite(CE, HIGH);
  delay(5);
  transmitSpin(addrRequest);
  setRX(0x06);
  spin();
  newAddr = verifyData()+1;
  clearStatus();
  setTX(newAddr);
  delay(5);
}

void loop(){
  /*handle light levels
  if (millis() - lastMilli >= 15)
  {
    // This stuff does the flickering
    if (dipFrame1 < 0 && random(64) < 1)
      dipFrame1 = 0;
    if (dipFrame2 < 0 && random(256) < 1)
      dipFrame2 = 0;
  
    int brightness = 0;
    if (battery > 45)
      brightness = (battery + 255) / 2;
    else
      brightness = battery*3;
    
    if (dipFrame1 >= 0)
    {
      brightness *= dipTable1[dipFrame1];
      brightness /= 255;
      dipFrame1 += 1;
  
      if (dipFrame1 >= 32)
      {
        dipFrame1 = -1;
      }
    }
    if (dipFrame2 >= 0)
    {
      brightness *= dipTable2[dipFrame2];
      brightness /= 255;
      dipFrame2 += 1;
  
      if (dipFrame2 >= 256)
      {
        dipFrame2 = -1;
      }
    }
    
    if (blackoutFrame < 256)
    {
      if (blackoutFrame >= 0)
      {
        brightness *= boTable[blackoutFrame];
        brightness /= 255;
      }
      blackoutFrame += random(2);
    }
  
    if (usingBattery)
    {
      analogWrite(ledPin, brightness);
      if (frame % 120 == 0 && battery > 0)
      {
        battery -= 1;
        if (battery % 50 == 0)
        {
          // TODO: Transmit the battery level to master
        }
      }
    }
    else
    {
      analogWrite(ledPin, 0);
    }
  
    ++frame;
    lastMilli += 15;
  }
  */
  
//  if(digitalRead(IRQ)==LOW)
//{
////byte statusReg = readReg(0x07);
//  writeReg(0x07, 0b01110000);
//  flushBuffers();
//}

//NEED TO ADD NORMANS serialEvent() FUNCTION FOR COMPLETEs
  if(tester==128){
    tester=129;
  }
  transmitSpin(tester);
  writeReg(0x07, 0b01110000);
  flushBuffers();
  delay(50);
  digitalWrite(LAMP, tester%2);
  tester++;
}

void nrfSlavOn(){
  writeReg(0x00,0b00001100);  //makes tx mode
  writeReg(0x01,0b00000001); //autoACK for all pipes
  writeReg(0x02,0b00000011); //enable all pipes
  writeReg(0x03,0b00000001);  //3 byte addresses
  writeReg(0x04,0b01001111);// retransmit delay, and up to 15 retries
  writeReg(0x11, 0b00000001);  //payload size max
  writeAddr(0x10, 0x06); //set transmit address
  writeAddr(0x0A, 0x06);  //set rx address for ack
  //writeReg(0x50, 0x73);
  //writeReg(0x1D,0b00000100); //enable dynamic payload and ack payload
  //writeReg(0x1C,0b00000001); //enable dynamic payload for pipe0
  delay(5);
}

void transmitSpin(byte data){
  byte flag=0;
  //while(!flag){
    transmit(data);  
    spin();
    if(0b00100000&readReg(0x07)){
      flag=1;
    }
    clearStatus();
  //}
}

byte getSender(){
  return(readReg(0x07)>>1)&0b00000111;
}

void setTX(byte addr){
  digitalWrite(CE, LOW);
  writeAddr(0x10, addr); //set transmit address
  writeAddr(0x0A, addr);  //set rx address for ack
  writeReg(0x00,0b00001110); //powerup
  flushBuffers();
  digitalWrite(CE, HIGH);
  delayMicroseconds(200);
}

void setRX(byte addr){
  digitalWrite(CE, LOW);
  writeAddr(0x0A, addr); //first receive addr
  writeReg(0x00,0b00001111); //powerup
  flushBuffers();
  digitalWrite(CE, HIGH);
  delayMicroseconds(200);
}

void spin(){
  while(digitalRead(IRQ)==HIGH){
    digitalWrite(ledPin, HIGH);
    delay(50);
    digitalWrite(ledPin, LOW);
    delay(50);    
  }
}

int verifyData(){  //clears status and returns byte if good, else returns -1 (without clearing)
  byte statreg = readReg(0x07);
  if(0b01000000&statreg){
    clearStatus();
    return readrf();
  }else{
    return -1;
  }
}

void clearStatus(){
    writeReg(0x07, 0b01110000);
}

byte writeAddr(byte addr, byte index){
  digitalWrite(CSN, LOW);
  digitalWrite(ledPin, HIGH);
  NOP;
  byte incoming = SPI.transfer(writeRegVal(addr)); //first receive addr
  NOP;
  SPI.transfer(index);
  NOP;
  if(addr<0x0C){
    SPI.transfer(0xE7);
    NOP;
    SPI.transfer(0xE7);
    NOP;
  }
  digitalWrite(ledPin, LOW);
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

byte readrf(){
  digitalWrite(CSN, LOW);
  NOP;
  SPI.transfer(0b01100001);
  NOP;
  byte data = SPI.transfer(0xFF);
  NOP;
  digitalWrite(CSN, HIGH);
  NOP;
  writeReg(0x07, 0b01110000);
  return data;
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
