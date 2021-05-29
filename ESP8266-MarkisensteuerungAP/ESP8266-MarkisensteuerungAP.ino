#include <ESP8266WiFi.h>
//#include <WiFiClient.h>

//#include <ESP8266WiFiMulti.h> 
//#include <ESP8266mDNS.h>
//#include <ESP8266WebServer.h>

//      WLAN access point
#define ssid "Markisensteuerung"
#define password "12345"

IPAddress localhost;

String nashostname = "fritz.box";

String endline = "\r\n";

int seconds = 0;

bool stopper = false;

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
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid, password);
    wifi_station_set_hostname("markise");
    //while (WiFi.status() != WL_CONNECTED) {
     //delay(100);
     //Serial.print(".");
    //}
    
    //Serial.println("WiFi connected");
    //Serial.println("IP address: ");
    //Serial.println(WiFi.localIP());
    
    localhost = WiFi.localIP();

    webserver.begin();
}

long maxTimeAction = 27000;
bool action = false;
long actionBegin = 0;

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
} 
