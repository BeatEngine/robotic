#include <ESP8266WiFi.h>
//#include <WiFiClient.h>

//#include <ESP8266WiFiMulti.h> 
//#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>

//ESP-Module-Pins
#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

unsigned long ticktimer;
// 20 ticks/s <=> 1.75 m/s
volatile byte windTicks = 0;
volatile byte lastINTpin = LOW;
//Interrupt routine
void __receiveWindSignalISR()
{
  if(windTicks < 255)
  {
    windTicks++;
  }
}

/*void fakeInterruptOnChange()
{
  const byte tmp = digitalRead(D5);
  if(tmp != lastINTpin)
  {
    lastINTpin = tmp;
    __receiveWindSignalISR();
  }
}*/

float windSpeedMpS = 0.0;

void updateWindInput()
{
  // Values in 2 Seconds
  const unsigned int tdiff = (unsigned int)(millis()-ticktimer);

  //Client side:
  // D4 (2) to A0 12 == 1 m/s
  //int inputFromOtherESP = analogRead(A0);
  //windSpeedMpS = inputFromOtherESP / 12.0f;
  
  if(tdiff > 1998)
  {
    
    const float tINs = (float)(tdiff)/1000.0;
    
    windSpeedMpS = 0.0875*(float)(windTicks)/tINs;
    const int valToSend = 12*windSpeedMpS;
    if(valToSend < 1024)
    {
      analogWrite(D4, valToSend);
    }
    windTicks = 0;
    ticktimer = millis();
    
  }
}
//const byte interruptPin = 14; // D5

void setup() {
    windTicks = 0;
    //Interrupt Pin D4 == 2
    pinMode(D5, OUTPUT);
    pinMode(D4, OUTPUT);
    digitalWrite(D5, HIGH);
    delay(10);
    digitalWrite(D5, LOW);
    analogWrite(D4, 0);
    delay(10);
    //pinMode(D5, INPUT_PULLUP);
    delay(300);
    //Connected D4 to A0 to send these sensor values
    ticktimer = millis();
    attachInterrupt(digitalPinToInterrupt(D5), __receiveWindSignalISR, CHANGE);
    
    
    //digitalWrite(D4, LOW);
    
    
}



void loop()
{
  //updates windSpeedMpS 
  delay(5);
  updateWindInput();
  delay(5);
} 
