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
volatile bool IS_ERROR = false;

int mSpeed = 400;
const short rotF = -11972; //needs to be negative because it works better than trying to use the abs() function.
int switchVal;
bool isHome = false;
bool ventShut = true;

unsigned long previousMillis = 0;
unsigned long previousTempMillis = 0;
unsigned long previousFlipMillis = 0;
unsigned long tempInterval = 2500;
unsigned long flipInterval = 120000;
unsigned long breakInterval = 4000;

double ambTemp;
double meatTemp;
double targetTemp = 165;
double degreeOffset = 10; //This is used because we arent sure of how our probe is reading sometimes.
double donessRatio = 0;

double highTemp = 375;
double lowTemp = 350;
bool blowing = false;

bool startCooking = false;
bool startBtnState = false;
bool stopBtnState = false;

int btValue;

void setup()
{
  Serial.begin(9600);
  Serial1.begin(9600);
  //pinMode(ESTOP, INPUT);
  attachInterrupt(digitalPinToInterrupt(ESTOP), ERROR_BUTTON, HIGH); 
  pinMode(HOMESWITCH, INPUT);
  pinMode(STARTSWITCH, INPUT);
  pinMode(STOPSWITCH, INPUT);
  pinMode(FANMODULE, OUTPUT);
  pinMode(BUZZ, OUTPUT);
  motorEnc.write(0);
  md.init();
  motorEnc.write(0);
  openVent();
  rotHome();
}

void loop()
{
  unsigned long currentMillis = millis();
  //Serial.println(currentMillis); //Debug print.

  if (Serial1.available() > 0)
  {
    btValue = Serial1.parseInt();
    Serial.print("btValue = "); Serial.println(btValue);
    switch (btValue)
    {
      case 0: //General case because sometimes the phone sends this
        break;
        
      case -500: //Emergency Stop from phone.
        break;
        
      case 100: //Chicken
        targetTemp = 165;
        break;
        
      case 200: //Steak
        targetTemp = 140;
        break;
        
      case 300: //Borgar
        targetTemp = 160;
        break;
        
      default:
        targetTemp = 165;
        break;
    }
  }

  //Serial.print("Targ Temp Value = ");Serial.println(targetTemp); //Debug print.

  startBtnState = digitalRead(STARTSWITCH);
  stopBtnState = digitalRead(STOPSWITCH);

  if ((meatTemp > (targetTemp + degreeOffset)) && startCooking)//Cooking Done ish
  {
    buzzNow();
    buzzNow();
    buzzNow();
  }

  if (startBtnState)
  {
    if (!startCooking) //If we were not already cooking...
    {
      previousMillis = millis();
      buzzNow();
    }
    startCooking = true;
  }
  if (stopBtnState || (meatTemp > (targetTemp + degreeOffset)))
  {
    if (startCooking) //If we were already cooking...
    {
      digitalWrite(FANMODULE, LOW); //Turn off the blower.
      rotHome();
      shutVent();
    }
    startCooking = false;
  }
  
  if ((currentMillis - previousTempMillis) >= tempInterval) //Time to update temperatures.
  {
    //Serial.println(ambProbe.readFarenheit());
    ambTemp = getTemp(ambProbe);
    Serial.print("Ambi Probe Value = ");Serial.println(ambTemp); //Debug print.
    meatTemp = getTemp(metProbe);
    Serial.print("Meat Probe Value = ");Serial.println(meatTemp); //Debug print.
    previousTempMillis = currentMillis;

    donessRatio = meatTemp/targetTemp;
  }

  if ((ambTemp < highTemp) && startCooking)
  {
    openVent();
  }
  if ((ambTemp > (highTemp + 25)) && startCooking)
  {
    shutVent();
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

  if (((currentMillis - previousFlipMillis) >= flipInterval) && startCooking) //Time to flip. Logic heavily pending.
  {
    //Serial.print("The value of isHome = "); Serial.println(isHome);
    if (isHome)
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

  if ((GLOBAL_ERROR_COUNT >= GLOBAL_ERROR_LIMIT))
  {
     ERROR_BUTTON();
  }

  if (Serial1.available() > 0)
  {
    String phoneData;
    String ambTempThing;
    String metTempThing;

    ambTempThing = String(int(ambTemp));
    metTempThing = String(int(meatTemp));

    phoneData = ambTempThing + ',' + metTempThing + ',' + '0' + ',' + '0';

    Serial1.print(phoneData);
    //Serial.println(phoneData);
    //Serial.println(char(ambTemp) + ',' + char(meatTemp) + ',' + '0' + ',' + '0');
  }
}

//---------------FUNCTIONS---------------//

void beginCooking()
{

}

void endCooking()
{

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

void buzzNow()
{
  digitalWrite(BUZZ, HIGH);
  delay(100);
  digitalWrite(BUZZ, LOW);
}

void rotHome()
{
  //Serial.println("Enter rotHome"); //Debug print.
  
  long startMillis = millis();
  long curHomeMillis = 0;
  
  bool homeSwVal = digitalRead(HOMESWITCH);
  //Serial.print("HomeSwitch value = "); Serial.println(homeSwVal);
  
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
  //Serial.println("Exit rotHome"); //Debug print.
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
  //Serial.println("rotBasket function called"); //Debug print.
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
  //Serial.println("rotBasket function exit"); //Debug print.
  return;
}

void shutVent()
{
  if (!ventShut)
  {
  md.setM2Speed(-400);
  delay(5500);
  md.setM2Speed(0);
  ventShut = true;
  }
}

void openVent()
{
  if (ventShut)
  {
    md.setM2Speed(400);
    delay(5500);
    md.setM2Speed(0);
    ventShut = false;
  }
}

void ERROR_BUTTON()
{
  //IS_ERROR = true;
  shutVent();
  stopIfFault();
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
