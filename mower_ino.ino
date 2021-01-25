
unsigned char cnt = 0;
bool haspower = true;

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

#define inpX A3
#define inpY A4
#define inpZ A5

bool updater();


bool drive(double x, double y, double speedR, double speedL, double duration, bool beginning)
{
  double f = 1.0;
  if(!beginning)
  {
    f = 0.0;
  }
  if(x < 0.0 && y >= 0.0)
  {
   x -= 1.0; 
   unsigned long start = millis();
   
   while(millis()-start < duration)
   {
    analogWrite(10, (int)(255)*speedL*(1 / (1 + f*exp(-0.005*(millis()-start)))));
    analogWrite(5, (int)(255)*speedR*(1 / (1 + f*exp(-0.005*(millis()-start)))));
   }
   digitalWrite(10, LOW); 
   digitalWrite(5, LOW);
  }
  else if(y >= 0.0)
  {
   unsigned long start = millis();
   while(millis()-start < duration && analogRead(A0) > 400 && updater())
   {
     analogWrite(9, (int)(255)*speedL*(1 / (1 + f*exp(-0.005*(millis()-start)))));
     analogWrite(6, (int)(255)*speedR*(1 / (1 + f*exp(-0.005*(millis()-start)))));
   }
   digitalWrite(9, LOW); 
   digitalWrite(6, LOW);
  }
  else if(x < 0.0)
  {
    x -= 1.0; 
    unsigned long start = millis();
    
    while(millis()-start < duration)
    {
    analogWrite(6, (int)(255)*(1 / (1 + f*exp(-0.005*(millis()-start)))));
    analogWrite(10, (int)(255)*(1 / (1 + f*exp(-0.005*(millis()-start)))));
    }
    digitalWrite(6, LOW); 
    digitalWrite(10, LOW);
  }
  else
  {
    unsigned long start = millis();
    while(millis()-start < duration)
    {
     analogWrite(9, (int)(255)*(1 / (1 + f*exp(-0.005*(millis()-start)))));
     analogWrite(5, (int)(255)*(1 / (1 + f*exp(-0.005*(millis()-start)))));
    }
    digitalWrite(9, LOW); 
    digitalWrite(5, LOW);
  }
   return analogRead(A0) < 401 || !updater();
}

long timeBegin;
void setup()
{
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  pinMode(A0, INPUT);
  analogRead(A0);

  //Acceleration sensors
  pinMode(2, OUTPUT); //GND
  pinMode(4, OUTPUT); //VCC
  analogWrite(2, 0); //GND
  analogWrite(4, 3.3/5.0*255); //VCC 3.3
  pinMode(inpX, INPUT);
  pinMode(inpY, INPUT);
  pinMode(inpZ, INPUT);
  analogRead(inpX);
  analogRead(inpY);
  analogRead(inpZ);
  delay(800);
  //Serial.begin(115200);
  timeBegin = 0;
  //while(!updater());
}
int counter = 0;

double posX = 0;
double posY = 0;
double posZ = 0;

double ax = 0.0;
double ay = 0.0;
double az = 0.0;

double vx = 0.0;
double vy = 0.0;
double vz = 0.0;
#define SENSORCONSTANT_X 364
#define SENSORCONSTANT_Y 360
#define SENSORCONSTANT_Z 433
#define positive(vx) sqrt(vx*vx)
#define threshold 1.1

double acceleration(int pin)
{
  int value = 0;
  for(int i = 0; i < 2; i++)
  {
    value += analogRead(pin);
  }
  return (((double)(value))/2-357)/75.83*9.81;
}

double filteredSpeed(double cmPerSecond)
{
  if(positive(cmPerSecond) < 0.01)
  {
    return 0.0;
  }
  return cmPerSecond;
}

//double middle = 0;
//long cnm = 1;

/** Returns true if not crashed! */
bool updater()
{
  ax = acceleration(inpX);
  ay = acceleration(inpY)-0.47;
  az = acceleration(inpZ);
  /*Serial.print(ax);
  Serial.print(",");
  Serial.print(ay);
  Serial.print(",");
  Serial.println(az);*/
  return positive(ay) >= 5.0;
}

void loop()
{
  // A1 11 cm                                    
  float L = 0.8;
  float R = 0.8;

  /*if(timeBegin == 0)
  {
    timeBegin = millis();
    ax1 = acceleration(inpX);
    ay1 = acceleration(inpY);
    az1 = acceleration(inpZ);
  }
  else
  {
    double relativeTime = (double)(millis()-timeBegin);
    double ax2, ay2, az2;
    ax2 = acceleration(inpX);
    ay2 = acceleration(inpY);
    az2 = acceleration(inpZ);
    vx = relativeTime*0.5*(ax1+ax2); //cm per second
    vy = relativeTime*0.5*(ay1+ay2);
    //vz = filteredSpeed(relativeTime*0.5*(az1+az2));
    //posX += vx * relativeTime;
    posY += vy * relativeTime;
    //posZ += vz * relativeTime;
    
    
    Serial.print((ay1-ay2));
    Serial.print(",");
    Serial.print(vy);
    Serial.print(",");
    Serial.println(posY/10);
    //Serial.print(",");
    //middle += acceleration(inpZ);
    //Serial.println(middle/(double)cnm);
    //cnm++;
    timeBegin = 0;
  }*/

  

  

  //posX += (double)(analogRead(inpX))/1024.0*relativeTime*relativeTime;
  //posY += (double)(analogRead(inpY))/1024.0*relativeTime*relativeTime;
  //posZ += (double)(analogRead(inpZ))/1024.0*relativeTime*relativeTime;
  if(analogRead(A1) < 401)
  {
    R = 0.75;
    counter = 0;
  }
  else
  {
    L = 0.75 - 0.05*counter;
    if(counter < 1)
    {
      counter++;
    }
    
  }
  //haspower = false;
  if(haspower)
  {
    if(drive(0.0, 0.0, R, L, 400, cnt < 1))
    {
      cnt = 0;
      delay(300);
      while(drive(-1.0, 0.0, R, L, 200, cnt < 1))
      {
        
      }
      delay(100);
      drive(0.0, -1.0, R, L, 400, cnt < 1);
      delay(200);
    }
    else
    {
      if(cnt < 1)
      {
        cnt ++;
      }
    }
  }
    
}
