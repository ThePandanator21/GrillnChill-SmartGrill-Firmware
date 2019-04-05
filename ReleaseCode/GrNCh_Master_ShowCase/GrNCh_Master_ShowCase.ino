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
int HOMESWITCH = 30;

//Component Declarations
DualMC33926MotorShield md;
Encoder encVal(ENC_A,ENC_B);
MAX6675 ambProbe(pSCK, PROBE_0, pMISO);
MAX6675 metProbe(pSCK, PROBE_1, pMISO);
MAX6675 sysProbe(pSCK, PROBE_2, pMISO);

//Logic Variables
const int GLOBAL_ERROR_LIMIT = 15;
int GLOBAL_ERROR_COUNT = 0;

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
unsigned long tempInterval = 5000;
unsigned long interval = 5000;
double ambTemp;

void setup()
{
  Serial.begin(9600);
  pinMode(ESTOP, INPUT_PULLUP);
  pinMode(HOMESWITCH, INPUT_PULLUP);
  encVal.write(0);
  md.init();
  rotHome();
}

void loop()
{
  unsigned long currentMillis = millis();
  long newEncVal = encVal.read();

  //Serial.println(currentMillis);
  
  if ((currentMillis - previousTempMillis) >= tempInterval)
  {
    Serial.println("In if statement"); 
    ambTemp = getTemp(ambProbe);
    Serial.println(ambTemp);
    previousTempMillis = currentMillis;
  }

//  if ((switchVal == LOW) && ((currentMillis - previousMillis) >= interval))
//  {
//    turning = true;
//    previousMillis = currentMillis;
//  }
//  else
//  {
//    turning = false;
//  }

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

  if (GLOBAL_ERROR_COUNT >= GLOBAL_ERROR_LIMIT)
  {
    stopIfFault();
  }
  
}

double getTemp(MAX6675 probe)
{
  double temp = probe.readFarenheit();

  if (temp < 0)//MAX6675 tends to skew to here if there is a connection error.
  {
    GLOBAL_ERROR_COUNT++;
  }
  
  return temp;
}

void rotHome()
{
  Serial.println("Enter rotHome");
  bool homeSwVal = digitalRead(HOMESWITCH);
  while (!homeSwVal)
  {
    md.setM1Speed(-mSpeed);
    homeSwVal = digitalRead(HOMESWITCH);
  }
  md.setM1Speed(0);
  Serial.println("Exit rotHome");
  return;
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
