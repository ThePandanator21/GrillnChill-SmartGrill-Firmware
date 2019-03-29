//Team Grill n Chill Firmware Rev 0.6
//Major contributions to firmware are credited to Son A. and Steadfast D.
//Bluetooth components are credited to Ma A. and Nguyen J.


//Libraries [to be cited]
#include "max6675.h"
#include "DualMC33926MotorShield.h"

//Motor Declarations
DualMC33926MotorShield md; //motor driver md


//Logic Declarations

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

int switchPin = 30; //was 23

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
  md.init();
}

void loop()
{
  md.setM2Speed(400);
  Serial.println(md.getM2CurrentMilliamps());
}
