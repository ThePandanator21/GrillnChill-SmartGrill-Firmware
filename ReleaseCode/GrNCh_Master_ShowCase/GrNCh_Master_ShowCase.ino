//Team Grill n Chill Firmware
//Major contributions to firmware are credited to Son A. and Steadfast D.
//Bluetooth components are credited to Ma A. and Nguyen J.

//Libraries
#include "max6675.h"
#include "DualMC33926MotorShield.h"
#include <Encoder.h>

//Pin Definitions
int ktcSO = 25; //Slave out
int ktcCS = 5; //Chip Select
int ktcCLK = 6; //Serial Clock
int switchPin = 30;

//Component Declarations
DualMC33926MotorShield md;
Encoder encVal(2,3);
MAX6675 ktc(ktcCLK, ktcCS, ktcSO);

//Logic Variables
int motorSpeed = 400;
short halfTurn = 11973;
bool clockWise = true;
bool nClockWise = false;
bool turning = false;
int switchVal;
bool timing = false;
unsigned long previousMillis = 0;
unsigned long previousTempMillis = 500;
unsigned long tempInterval = 500;
unsigned long interval = 5000;

//Motor Fault Code
void stopIfFault()
{
  if (md.getFault())
  {
    Serial.println("fault");
    while(1);
  }
}

void setup()
{
  Serial.begin(9600);
  pinMode(switchPin, INPUT_PULLUP);
  encVal.write(0);
  md.init();
}

void loop()
{
//Data Read Block
  //Motor 1 Encoder
  long newEncVal = encVal.read();
  //Serial.println(newEncVal);
  //Program time
  unsigned long currentMillis = millis();
  //Serial.println(currentMillis);
  //Ambient Temp

  switchVal = digitalRead(switchPin);
  //Serial.print("Switch State ");
  //Serial.println(switchVal);

  if ((currentMillis - previousTempMillis) >= tempInterval)
  {
    long ambientTemp = ktc.readFahrenheit();
    Serial.print("Deg F = ");
    Serial.println(ambientTemp);
    previousTempMillis = currentMillis;
  }

  if ((switchVal == LOW) && ((currentMillis - previousMillis) >= interval))
  {
    turning = true;
    previousMillis = currentMillis;
  }
  else
  {
    turning = false;
  }

  //Serial.println(turning);
  
  //Motor direction code
  if ( turning && clockWise && !nClockWise )//&& timing
  {
    md.setM1Speed(-motorSpeed);
    //timing = false;
  }
  else if (turning && !clockWise && nClockWise )//&& timing
  {
    md.setM1Speed(motorSpeed);
    //timing = false;
  }

  //Motor stopping code
  if (newEncVal <= -halfTurn)//if direction is clockwise and needs to stop
  {
    md.setM1Speed(0);
    encVal.write(0);
    clockWise = false;
    nClockWise = true;
    turning = false;
    //Serial.println("Stop Neg Called");
    //previousMillis = currentMillis;
  }
  else if (newEncVal >= halfTurn)
  {
    md.setM1Speed(0);
    encVal.write(0);
    clockWise = true;
    nClockWise = false;
    turning = false;
    //Serial.println("Stop Pos Called");
    //previousMillis = currentMillis;
  }
}
