//Team Grill n Chill Firmware
//Major contributions to firmware are credited to Son A. and Steadfast D.
//Bluetooth components are credited to Ma A. and Nguyen J.
//Team Members: Andrew Son, Andy Ma, Colton Russel, Daniel Martin, Dylan Steadfast, Jacob Rohlman,  and Jesse Nguyen.

//Libraries
#include "max6675.h"
#include "DualMC33926MotorShield.h"
#include <Encoder.h>

//Pin Definitions
int ENC_A = 2;
int ENC_B = 3;
int BUZZ = 6; //PWM pin for buzzer
int ESTOP = 21; //Estop Switch, rigged to HW interrupt
int pSCK = 52; //Serial Clock 23?
int pMISO = 50; //Slave out 25?
int PROBE_0 = 26; //Ambient Probe Chip Select
int PROBE_1 = 27; //Meat Probe Chip Select
int PROBE_2 = 28; //System Probe Chip Select
int HOMESWITCH = 30;
int STARTSWITCH = 31;
int STOPSWITCH = 32;
int FANMODULE = 40;

//Component Declarations
DualMC33926MotorShield md;
Encoder motorEnc(ENC_A,ENC_B);
MAX6675 ambProbe(pSCK, PROBE_0, pMISO);
MAX6675 metProbe(pSCK, PROBE_1, pMISO);
MAX6675 sysProbe(pSCK, PROBE_2, pMISO);

//Logic Variables
const int GLOBAL_ERROR_LIMIT = 15;
int GLOBAL_ERROR_COUNT = 0;

int mSpeed = 400;
const short rotF = -11972; //needs to be negative because it works better than trying to use the abs() function.
int switchVal;
bool isHome = false;

unsigned long previousMillis = 0;
unsigned long previousTempMillis = 0;
unsigned long previousFlipMillis = 0;
unsigned long tempInterval = 5000;
unsigned long flipInterval = 5000;
unsigned long breakInterval = 4000;

double ambTemp;
double meatTemp;

double highTemp = 375;
double lowTemp = 350;
bool blowing = false;

bool startCooking = false;
bool startBtnState = false;
bool stopBtnState = false;

void setup()
{
  Serial.begin(9600);
  pinMode(ESTOP, INPUT);
  pinMode(HOMESWITCH, INPUT);
  pinMode(STARTSWITCH, INPUT);
  pinMode(STOPSWITCH, INPUT);
  pinMode(FANMODULE, OUTPUT);
  motorEnc.write(0);
  md.init();
  motorEnc.write(0);
  rotHome();
}

void loop()
{
  unsigned long currentMillis = millis();
  //Serial.println(currentMillis); //Debug print.

  startBtnState = digitalRead(STARTSWITCH);
  stopBtnState = digitalRead(STOPSWITCH);

  if (startBtnState)
  {
    startCooking = true;
  }
  if (stopBtnState)
  {
    if (startCooking)
    {
      rotHome();
    }
    
    startCooking = false;
  }

  //Serial.print("StartButton value = "); Serial.println(startBtnState);
  //Serial.print("StopButton value = "); Serial.println(stopBtnState);

  //startCooking = true;
  
  if ((currentMillis - previousTempMillis) >= tempInterval) //Time to update temperatures.
  {
    //Serial.println("In if statement"); //Debug print.
    //Serial.println(ambProbe.readFarenheit());
    ambTemp = getTemp(ambProbe);
    Serial.println(ambTemp); //Debug print.
    meatTemp = getTemp(metProbe);
    Serial.println(meatTemp); //Debug print.
    previousTempMillis = currentMillis; 
  }

  if ((ambTemp < lowTemp) && startCooking)
  {
    blowing = true;
    //Serial.println("OOO WE BLOWING");
    digitalWrite(FANMODULE, HIGH);
  }
  else if ((ambTemp > highTemp) && startCooking)
  {
    //Serial.println("NO BLOW ZONE");
    blowing = false;
    digitalWrite(FANMODULE, LOW);
  }

  //Serial.println(blowing);

  if (((currentMillis - previousFlipMillis) >= flipInterval) && startCooking) //Time to flip. Logic heavily pending.
  {
    //Serial.print("The value of isHome = "); //Debug print.
    //Serial.println(isHome);
    
    if (isHome) //Export this to a function flipTime();
    {
      rotBasket();
      previousFlipMillis = currentMillis;
    }
    else
    {
      rotHome();
      previousFlipMillis = currentMillis;
    }
  }

  if (GLOBAL_ERROR_COUNT >= GLOBAL_ERROR_LIMIT)
  {
    stopIfFault();
  }
}

double getTemp(MAX6675 probe)
{

  //Serial.println(probe.readFarenheit());
  double temp = probe.readFarenheit();

  if (temp < 0)//MAX6675 tends to skew to here if there is a connection error.
  {
    GLOBAL_ERROR_COUNT++;
  }
  
  return temp;
}

void rotHome()
{
  Serial.println("Enter rotHome"); //Debug print.
  
  long startMillis = millis();
  long curHomeMillis = 0;
  
  bool homeSwVal = digitalRead(HOMESWITCH);
  Serial.print("HomeSwitch value = "); Serial.println(homeSwVal);
  
  while ((!homeSwVal))
  {
    curHomeMillis = millis();
    md.setM1Speed(-mSpeed);
    homeSwVal = digitalRead(HOMESWITCH);

    //Serial.print("HomeSwitch value = "); Serial.println(homeSwVal);

    if(((curHomeMillis - startMillis) > breakInterval))
    {
      break;
    }
  }
  md.setM1Speed(0);
  Serial.println("Exit rotHome"); //Debug print.
  isHome = true;
  motorEnc.write(0);
  return;
}

void rotBasket()
{
  /*Define a new long to contain the motor encoder's current value.
    While the encoders value is less than that of our rot limit, drive motor.
    Else, stop motor, set isHome to false, reset enc value, and then return.
  */
  Serial.println("rotBasket function called"); //Debug print.
  long newEncVal = motorEnc.read();
  //Serial.println(newEncVal);
  while (newEncVal > rotF)//while  not turned 180 degrees
  {
    //Motor Driving Code
    md.setM1Speed(mSpeed);
    newEncVal = motorEnc.read();//Grab new value
    //Serial.println(newEncVal);//Debug print
  }
  //Motor Stopping code
  md.setM1Speed(0);
  motorEnc.write(0);
  isHome = false;
  Serial.println("rotBasket function exit"); //Debug print.
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
