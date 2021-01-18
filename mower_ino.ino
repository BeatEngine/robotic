
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
    analogWrite(10, (int)(255)*speedL*(1 / (1 + f*exp(-0.01*(millis()-start)))));
    analogWrite(5, (int)(255)*speedR*(1 / (1 + f*exp(-0.01*(millis()-start)))));
   }
   digitalWrite(10, LOW); 
   digitalWrite(5, LOW);
  }
  else if(y >= 0.0)
  {
   unsigned long start = millis();
   while(millis()-start < duration && analogRead(A0) > 400)
   {
     analogWrite(9, (int)(255)*speedL*(1 / (1 + f*exp(-0.01*(millis()-start)))));
     analogWrite(6, (int)(255)*speedR*(1 / (1 + f*exp(-0.01*(millis()-start)))));
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
    analogWrite(6, (int)(255)*(1 / (1 + f*exp(-0.01*(millis()-start)))));
    analogWrite(10, (int)(255)*(1 / (1 + f*exp(-0.01*(millis()-start)))));
    }
    digitalWrite(6, LOW); 
    digitalWrite(10, LOW);
  }
  else
  {
    unsigned long start = millis();
    while(millis()-start < duration)
    {
     analogWrite(9, (int)(255)*(1 / (1 + f*exp(-0.01*(millis()-start)))));
     analogWrite(5, (int)(255)*(1 / (1 + f*exp(-0.01*(millis()-start)))));
    }
    digitalWrite(9, LOW); 
    digitalWrite(5, LOW);
  }
   return analogRead(A0) < 401;
}


void setup()
{
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(9, OUTPUT);
  pinMode(10, OUTPUT);

  pinMode(11, OUTPUT);
  digitalWrite(11, HIGH);
  analogRead(A0);
  //Serial.begin(9600);
}
int counter = 0;
void loop()
{
  // A1 11 cm                 vb                       
  float L = 0.8;
  float R = 0.8;
  //Serial.println(analogRead(A0));
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
    if(cnt < 1)
    {
      cnt ++;
    }
  }
    
}
