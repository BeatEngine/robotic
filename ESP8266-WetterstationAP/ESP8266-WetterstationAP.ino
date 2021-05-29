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

//      WLAN access point
#define ssid "Wetterstation"
#define password "12345"

unsigned long ticktimer;
// 20 ticks/s <=> 1.75 m/s
volatile unsigned short windTicks = 0;
volatile byte lastINTpin = LOW;
//Interrupt routine
void __receiveWindSignalISR()
{
  windTicks++;
}

void fakeInterruptOnChange()
{
  const byte tmp = digitalRead(D5);
  if(tmp != lastINTpin)
  {
    lastINTpin = tmp;
    __receiveWindSignalISR();
  }
}

float windSpeedMpS = 0.0;

void updateWindInput()
{
  // Values in one Second
  const unsigned int tdiff = (unsigned int)(millis()-ticktimer);
  fakeInterruptOnChange();
  if(tdiff > 998)
  {
    fakeInterruptOnChange();
    const float tINs = (float)(tdiff)/1000.0;
    fakeInterruptOnChange();
    windSpeedMpS = 0.0875*(float)(windTicks)/tINs;
    windTicks = 0;
    ticktimer = millis();
    fakeInterruptOnChange();
  }
}

IPAddress localhost;

String endline = "\r\n";

int seconds = 0;

bool stopper = false;

//Fritzbox:

WiFiServer webserver(80);

String generateResponseHead(int contentLength = 0)
{
  fakeInterruptOnChange();
  return String("HTTP/1.1 200 OK\r\nContent-Language: de\r\nContent-Type: text/html; Content-Length: "+String(contentLength)+"; charset=utf-8\r\n\r\n");  
}

String htmlSite = "<html> <head> <title> Wetterstation</title> <style> .backgroundsite { background-color: darkgray; } .value-box { font-size: 20pt; text-align: left; } </style> <script type=\"text/javascript\"> function ajaxRequest(method, url, body = '', onfinish = function() { } ) { var rq = new XMLHttpRequest; rq.open(method, url); rq.onload = onfinish; rq.send(body); } function setup() { setInterval(function() { ajaxRequest(\"GET\", \"/windspeed\", \"\",function() { var wbox = document.getElementById(\"wind\"); if(isNaN(parseInt(this.responseText)) == false) { wbox.innerText = \"Windspeed: \" + this.responseText + \" m/s\"; } } ); } , 100); } </script> </head> <body onload=\"setup();\" class=\"backgroundsite\"> <div id=\"site\"> <div id=\"info\" style=\"margin-left: 20%;\"> <div id=\"wind\" class=\"value-box\">Windspeed: loading...</div> </div> </div> </body> </html>";

void index(WiFiClient& wclient)
{
  fakeInterruptOnChange();
  wclient.print(generateResponseHead(htmlSite.length()) + htmlSite);
  fakeInterruptOnChange();
}

//const byte interruptPin = 14; // D5

void setup() {
    
    //Interrupt Pin D4 == 2
    pinMode(D5, OUTPUT);
    digitalWrite(D5, HIGH);
    delay(10);
    digitalWrite(D5, LOW);
    delay(10);
    pinMode(D5, INPUT);
    //digitalWrite(D4, LOW);
    //attachInterrupt(digitalPinToInterrupt(interruptPin), __receiveWindSignalISR, CHANGE);
    Serial.begin(9600);
    //Serial.println();
    //Serial.print("Configuring access point...\n");
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    wifi_station_set_hostname("wetter");
    //while (WiFi.status() != WL_CONNECTED) {
     //delay(100);
     //Serial.print(".");
    //}
    
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
    
    localhost = WiFi.localIP();

    webserver.begin();
    ticktimer = millis();
    fakeInterruptOnChange();
}

static void getWindSpeed(WiFiClient& wclient)
{ 
  fakeInterruptOnChange();
  wclient.println(windSpeedMpS);
  fakeInterruptOnChange();
  wclient.flush();
  fakeInterruptOnChange();
  wclient.stop();
  fakeInterruptOnChange();
}

void loop()
{
  //updates windSpeedMpS 
  fakeInterruptOnChange();
  updateWindInput();
  fakeInterruptOnChange();
  String requestHeader = "";
  fakeInterruptOnChange();
  
  WiFiClient webclient = webserver.available();
  fakeInterruptOnChange();
  if(webclient)
  {
    fakeInterruptOnChange();
    //Serial.println("Handle client!");
    int c = 0;
    fakeInterruptOnChange();
    while(!webclient.available())
    {
      fakeInterruptOnChange();
      updateWindInput();
    }
    while (webclient.available()) {
      fakeInterruptOnChange();
      updateWindInput();
      if(c < 255)
      {
        fakeInterruptOnChange();
        requestHeader += (char)(webclient.read());
        fakeInterruptOnChange();
        c++;
        fakeInterruptOnChange();
      }
      else
      {
        fakeInterruptOnChange();
        webclient.read();
        fakeInterruptOnChange();
      }
    }
    //Serial.println(requestHeader.substring(0, 15));
    if(requestHeader.startsWith("GET /windspeed"))
    {
      fakeInterruptOnChange();
      getWindSpeed(webclient);
      fakeInterruptOnChange();
    }
    else if(requestHeader.startsWith("GET /"))
    {
      fakeInterruptOnChange();
      index(webclient);
      webclient.flush();
      fakeInterruptOnChange();
      webclient.stop();
      fakeInterruptOnChange();
    }
      fakeInterruptOnChange();
    }
} 
