#include <ESP8266WiFi.h>
#include <WiFiClient.h>

//#include <ESP8266WiFiMulti.h> 
//#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>

//      WLAN access point
#define ssid "Markisensteuerung"
#define password "12345"
#define AP_CHANNEL 1
#define AP_MAX_CON 4
#define AP_HIDDEN false

//      WLAN access to the weather-station
#define w_ssid "Wetterstation"
#define w_password "12345"
String w_hostname = "wetter";

IPAddress w_host;

bool w_connected = false;
WiFiClient w_client;

String endline = "\r\n";

int seconds = 0;

bool stopper = false;

//Detects not any use-case
bool isInside = true;

//Fritzbox:

WiFiServer webserver(80);

String generateResponseHead(int contentLength = 0)
{
  return String("HTTP/1.1 200 OK\r\nContent-Language: de\r\nContent-Type: text/html; Content-Length: "+String(contentLength)+"; charset=utf-8\r\n\r\n");  
}

String htmlSite = "<html> <head>     <title>Markisensteuerung</title>     <style>         .btn         {             width: 90%;             margin-top: 30px;             text-align: center;             font-size: 28pt;             height: 80px;             color: white;             background-color: rgb(255, 153, 0);             border: black solid 2px;             border-radius: 6px;         }         .btn::hover         {             border-color: lime;         }         .btn.btn::active         {             background-color: darkgreen;         }         .progress         {             height: 50px;             width: 90%;         }     </style>     <script type=\"text/javascript\">          function ajaxRequest(method, url, body = '', onfinish = function(){})         {             var rq = new XMLHttpRequest;             rq.open(method, url);             rq.onload = onfinish;             rq.send(body);         }          function ausfahren()         {             ajaxRequest(\"POST\", \"/ausfahren\", \"\",function()             {                              });         }         function einfahren()         {             ajaxRequest(\"POST\", \"/einfahren\", \"\",function()             {                              });         }         function anhalten()         {             ajaxRequest(\"POST\", \"/stop\", \"\",function()             {                              });         }          function setup()         {             document.getElementById(\"mkin\").onclick = einfahren;             document.getElementById(\"mkout\").onclick = ausfahren;             document.getElementById(\"mkstop\").onclick = anhalten;         }          setInterval(function()         {             ajaxRequest(\"GET\", \"/state\", \"\",function()             {                 var pgb = document.getElementById(\"progress\");                 if(isNaN(parseInt(this.responseText)) == false)                 {                     pgb.setAttribute(\"aria-valuenow\", parseInt(this.responseText));                     pgb.style.width=parseInt(this.responseText)+ \"%\";                     pgb.innerText = parseInt(this.responseText) + \"%\";                 }             });         }, 100);      </script> </head> <body onload=\"setup();\">     <div id=\"site\">         <div id=\"buttons\">             <button class=\"btn\" id=\"mkin\">                 Einfahren             </button>             <button class=\"btn\" id=\"mkstop\">                 Anhalten             </button>             <button class=\"btn\" id=\"mkout\">                 Ausfahren             </button>         </div>         <div id=\"info\">             <div class=progress><div id=\"progress\" class=\"progress-bar\" role=\"progressbar\" style=\"width: 0%;\" aria-valuenow=\"0\" aria-valuemin=\"0\" aria-valuemax=\"100\">0%</div>         </div>     </div> </body> </html> ";

void index(WiFiClient& wclient)
{
  wclient.print(generateResponseHead(htmlSite.length()) + htmlSite);
}



void setup() {
    pinMode(4, OUTPUT);
    pinMode(5, OUTPUT);
    digitalWrite(5, LOW);
    digitalWrite(4, LOW);
    //Serial.begin(9600);
    //Serial.println();
    //Serial.print("Configuring access point...\n");
    WiFi.mode(WIFI_AP_STA);
    WiFi.softAP(ssid, password, AP_CHANNEL, AP_HIDDEN, AP_MAX_CON);
    wifi_station_set_hostname("markise");
    WiFi.begin(w_ssid, w_password);
    //while (WiFi.status() != WL_CONNECTED) {
     //delay(100);
     //Serial.print(".");
    //}
    
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
    
    delay(1500);
    w_host = WiFi.localIP();
    webserver.begin();
    if(w_client.connect(w_host, 80))
    {
      w_connected = true;
    }
}



long maxTimeAction = 27000;
bool action = false;
long actionBegin = 0;

static float getWindSpeedFromStation()
{
  w_client.println("GET /windspeed HTTP/1.1");
  w_client.println("Host: markise");
  w_client.println("Connection: close");
  w_client.println();
  unsigned short et = 0;
  while(!w_client.available() && et < 2000)
  {
    et++;
  }
  String num = "";
  bool nan = false;
  while(w_client.available())
  {
    char c = w_client.read();
    if(c != ' ')
    {
      if(c != '.' && (c < '0' || c > '9'))
      {
        nan = true;
      }
      num += c;
    }
  }
  float result = 0.0;
  if(!nan)
  {
    result = num.toFloat();
    if(result > 35) //Filter currently Error in Wetherstation
    {
      result = 0.0;
    }
  }
  return result;
}

static void getState(WiFiClient& wclient)
{
  seconds = (millis()-actionBegin)/1000;
  if(seconds < 0 || !action)
  {
    seconds = 0;
  }
  wclient.println((int)((100*seconds/48.0f)));
  wclient.flush();
  wclient.stop();
}

void stopEverything()
{
  digitalWrite(5, LOW);
  digitalWrite(4, LOW);
}

byte lazyChecker = 0;

void loop()
{
  String requestHeader = "";

  if(action)
  {
    if(millis()-actionBegin >= maxTimeAction)
    {
      stopEverything();
      action = false;
    }
  }
  
  WiFiClient webclient = webserver.available();
  if(webclient)
  {
    //Serial.println("Handle client!");
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
    }
    //Serial.println(requestHeader.substring(0, 15));
    if(requestHeader.startsWith("POST /einfahren"))
    {
      if(!action)
      {
        action = true;
        digitalWrite(5, LOW);
        digitalWrite(4, HIGH);
        actionBegin = millis();
      }
      webclient.flush();
      webclient.stop();
    }else if(requestHeader.startsWith("POST /ausfahren"))
    {
      if(!action)
      {
        isInside = false;
        action = true;
        digitalWrite(4, LOW);
        digitalWrite(5, HIGH);
        actionBegin = millis();
      }
      webclient.flush();
      webclient.stop();
    }
    else if(requestHeader.startsWith("GET /state"))
    {
      getState(webclient);
    }
    else if(requestHeader.startsWith("POST /stop"))
    {
      stopEverything();
      action = false;
      webclient.flush();
      webclient.stop();
    }
    else if(requestHeader.startsWith("GET /"))
    {
      index(webclient);
      webclient.flush();
      webclient.stop();
    }
    
    }
    if(w_connected && !isInside)
    {
      //const float wisp = getWindSpeedFromStation();
      //Serial.println(wisp);
      if(lazyChecker > 2)
      {
        if(getWindSpeedFromStation() > 3.0)
        {
          stopEverything();
          action = true;
          delay(10);
          digitalWrite(5, LOW);
          digitalWrite(4, HIGH);
          actionBegin = millis();
          isInside = true;
        }
        lazyChecker = 0;
      }
      lazyChecker++;
    }
} 
