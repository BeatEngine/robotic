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
#define AP_CHANNEL 1
#define AP_MAX_CON 4
#define AP_HIDDEN false

volatile float windSpeedMpS = 0.0;

void updateWindInput()
{
  // D4 (2) to A0 12 == 1 m/s
  int inputFromOtherESP = analogRead(A0);
  if(inputFromOtherESP > 10)
  {
    inputFromOtherESP -= 10;
  }
  else
  {
    inputFromOtherESP = 0;
  }
  windSpeedMpS = inputFromOtherESP / 12.0f;
}

IPAddress localhost;

String endline = "\r\n";

int seconds = 0;

WiFiServer webserver(80);

String generateResponseHead(int contentLength = 0)
{
  
  return String("HTTP/1.1 200 OK\r\nContent-Language: de\r\nContent-Type: text/html; Content-Length: "+String(contentLength)+"; charset=utf-8\r\n\r\n");  
}

String htmlSite = "<html> <head> <title> Wetterstation</title> <style> .backgroundsite { background-color: darkgray; } .value-box { font-size: 20pt; text-align: left; } </style> <script type=\"text/javascript\"> function ajaxRequest(method, url, body = '', onfinish = function() { } ) { var rq = new XMLHttpRequest; rq.open(method, url); rq.onload = onfinish; rq.send(body); } function setup() { setInterval(function() { ajaxRequest(\"GET\", \"/windspeed\", \"\",function() { var wbox = document.getElementById(\"wind\"); if(isNaN(parseInt(this.responseText)) == false) { wbox.innerText = \"Windspeed: \" + this.responseText + \" m/s\"; } } ); } , 100); } </script> </head> <body onload=\"setup();\" class=\"backgroundsite\"> <div id=\"site\"> <div id=\"info\" style=\"margin-left: 20%;\"> <div id=\"wind\" class=\"value-box\">Windspeed: loading...</div> </div> </div> </body> </html>";

void index(WiFiClient& wclient)
{
  wclient.print(generateResponseHead(htmlSite.length()) + htmlSite); 
}

void setup() {
    //Interrupt Pin D4 == 2
    pinMode(A0, INPUT);
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password, AP_CHANNEL, AP_HIDDEN, AP_MAX_CON);
    wifi_station_set_hostname("wetter");
    delay(1500);
    localhost = WiFi.localIP();
    webserver.begin();
    delay(10);
    Serial.begin(9600);
}

static void getWindSpeed(WiFiClient& wclient)
{ 
  wclient.println(windSpeedMpS);
  wclient.flush();
  wclient.stop();
}

void loop()
{
  //updates windSpeedMpS 
  
  updateWindInput();
  
  String requestHeader = "";
  Serial.println("UB-fix");
  WiFiClient webclient = webserver.available();
  
  if(webclient)
  {
    
    int c = 0;
    
    while(!webclient.available());
    while (webclient.available()) {
      if(c < 255)
      {
        requestHeader += (char)(webclient.read());
        c++;
      }
      else
      {
        webclient.read();
      }
      updateWindInput();
    }
    //Serial.println(requestHeader.substring(0, 15));
    if(requestHeader.startsWith("GET /windspeed"))
    {
      getWindSpeed(webclient);
    }
    else if(requestHeader.startsWith("GET /"))
    {
      index(webclient);
      webclient.flush();
      webclient.stop();
    }
      
  }
} 
