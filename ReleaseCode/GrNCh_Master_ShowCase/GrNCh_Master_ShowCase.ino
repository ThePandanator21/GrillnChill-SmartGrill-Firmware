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
//short rotB = 11973;
//bool clockWise = true;
//bool nClockWise = false;
//bool turning = false;
int switchVal;
//bool timing = false;
bool isHome = false;
unsigned long previousMillis = 0;
unsigned long previousTempMillis = 5000;
unsigned long previousFlipMillis = 5000;
unsigned long tempInterval = 5000;
unsigned long flipInterval = 5000;
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

  //Serial.println(currentMillis);
  
  if ((currentMillis - previousTempMillis) >= tempInterval) //Time to update temperatures.
  {
    Serial.println("In if statement"); 
    ambTemp = getTemp(ambProbe);
    Serial.println(ambTemp);
    previousTempMillis = currentMillis; 
  }

  if ((currentMillis - previousFlipMillis) >= flipInterval) //Time to flip. Logic heavily pending.
  {
    if (isHome)
    {
      
    }
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
  //Serial.println("Enter rotHome");
  bool homeSwVal = digitalRead(HOMESWITCH);
  while (!homeSwVal)
  {
    md.setM1Speed(-mSpeed);
    homeSwVal = digitalRead(HOMESWITCH);
  }
  md.setM1Speed(0);
  //Serial.println("Exit rotHome");
  isHome = true;
  return;
}

void flipBasket()
{
  long newEncVal = encVal.read();
  //Motor starting code
  while (newEncVal < rotF)      // If not turned 180 degrees
  {
    md.setM1Speed(mSpeed);
    encVal.write(0);            // Clear value I guess?
    newEncVal = encVal.read();  // Grab new value
  }
  
  //Motor stopping code
  md.setM1Speed(0);
  encVal.write(0);
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
