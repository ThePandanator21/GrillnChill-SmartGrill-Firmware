//Team Grill n Chill Firmware
//Major contributions to firmware are credited to Son A. and Steadfast D.
//Bluetooth components are credited to Ma A. and Nguyen J.

//Libraries
#include "max6675.h"
#include "DualMC33926MotorShield.h"
#include <Encoder.h>

//Pin Definitions
int ENC_A = 2;
int ENC_B = 3;
int BUZZ = 6; //PWM pin for buzzer
int ESTOP = 21; //Estop Switch, rigged to HW interrupt
int pSCK = 23; //Serial Clock
int pMISO = 25; //Slave out
int PROBE_0 = 26; //Ambient Probe Chip Select
int PROBE_1 = 27; //Meat Probe Chip Select
int PROBE_2 = 28; //System Probe Chip Select


//Component Declarations
DualMC33926MotorShield md;
Encoder encVal(ENC_A,ENC_B);
MAX6675 ambProbe(pSCK, PROBE_0, pMISO);
MAX6675 metProbe(pSCK, PROBE_1, pMISO);
MAX6675 sysProbe(pSCK, PROBE_2, pMISO);

//Logic Variables
int mSpeed = 400;
short rotF = 11972;
short rotB = 11973;
bool clockWise = true;
bool nClockWise = false;
bool turning = false;
int switchVal;
bool timing = false;
unsigned long previousMillis = 0;
unsigned long previousTempMillis = 500;
unsigned long tempInterval = 500;
unsigned long interval = 5000;
int ambTemp;
const int numReadings = 10;
int readIndex = 0;
int readings[numReadings];


void setup()
{
  Serial.begin(9600);
  pinMode(ESTOP, INPUT_PULLUP);
  encVal.write(0);
  md.init();
  resetReadings();
}

void loop()
{
  unsigned long currentMillis = millis();
  long newEncVal = encVal.read();
    
  if ((currentMillis - previousTempMillis) >= tempInterval)
  {
    long ambientTemp = ambProbe.readFahrenheit();
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
    md.setM1Speed(-mSpeed);
    //timing = false;
  }
  else if (turning && !clockWise && nClockWise )//&& timing
  {
    md.setM1Speed(mSpeed);
    //timing = false;
  }

  //Motor stopping code
  if (newEncVal <= -rotB)//if direction is clockwise and needs to stop
  {
    md.setM1Speed(0);
    encVal.write(0);
    clockWise = false;
    nClockWise = true;
    turning = false;
    //Serial.println("Stop Neg Called");
    //previousMillis = currentMillis;
  }
  else if (newEncVal >= rotF)
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

int getTemp(int temp)
{
  
}

void resetReadings()
{
  for (int x = 0; x < numReadings; x++)
  {
    readings[x] = 0;
  }

  readIndex = 0;
}

//Motor Fault Code
void stopIfFault()
{
  if (md.getFault())
  {
    Serial.println("fault");
    while(1);
  }
}
