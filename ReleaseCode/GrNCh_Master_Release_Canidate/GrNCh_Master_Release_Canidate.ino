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
const short rotOut = -11975; //Needs to be negative because it works better than trying to use the abs() function.
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

double highTemp = 500;
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
  pinMode(ESTOP, INPUT);
  //attachInterrupt(digitalPinToInterrupt(ESTOP), ERROR_BUTTON, RISING); 
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

  bool flipNow = false;

  startBtnState = digitalRead(STARTSWITCH);
  stopBtnState = digitalRead(STOPSWITCH);
  IS_ERROR = digitalRead(ESTOP);

  if (Serial1.available() > 0)
  {
    btValue = Serial1.parseInt();
    Serial.print("BlueTooth Value Received = "); Serial.println(btValue);
    switch (btValue)
    {
      case 0: //General case because sometimes the phone sends this
        break;
        
      case -500: //Stop from phone.
        stopBtnState = true;
        break;

      case -200: //Start / Flip from phone
        startBtnState = true;
        break;
        
      default:
        if (btValue < 0)
        {
          targetTemp = 165;
        }
        else
        {
          targetTemp = btValue;
        }
        break;
    }
    Serial.print("Targ Temp Value = ");Serial.println(targetTemp); //Debug print.
  }
  
  /*  Meat Done Alert */
  if ((meatTemp > (targetTemp + degreeOffset)) && startCooking)
  {
    buzzNow();
  }

  /*  Front Panel Logic */
  if (startBtnState)
  {
    if (!startCooking) //If we were not already cooking...
    {
      previousMillis = millis();
      buzzNow();
    }
    else if (startCooking)
    {
      flipNow = true;
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

  /*  Temperature Sampling and BT Send*/
  if ((currentMillis - previousTempMillis) >= tempInterval) //Time to update temperatures.
  {
    //Serial.println(ambProbe.readFarenheit());
    ambTemp = getTemp(ambProbe);
    Serial.print("Ambi Probe Value = ");Serial.println(ambTemp); //Debug print.
    meatTemp = getTemp(metProbe);
    Serial.print("Meat Probe Value = ");Serial.println(meatTemp); //Debug print.
    previousTempMillis = currentMillis;

    float donenessRatio = meatTemp/targetTemp;
    
    /* Bluetooth Send on Update */
    String phoneData;
    String ambTempThing;
    String metTempThing;

    bool doneness = false;

    if (donenessRatio > .9)
    {
      doneness = true;
    }

    ambTempThing = String(int(ambTemp));
    metTempThing = String(int(meatTemp));

    phoneData = ambTempThing + ',' + metTempThing + ',' + String(doneness) + ',' + String(IS_ERROR);

    Serial1.print(phoneData);
    Serial1.flush();
    Serial.print("String Sent to Phone = "); Serial.println(phoneData);
  }

  /* Vent Control */
  if ((ambTemp < highTemp) && startCooking)
  {
    if (ventShut)
    {
      Serial.println("Too Cold, Opening Vent");
    }
    openVent();
  }
  if ((ambTemp > (highTemp + 100)) && startCooking) // If Amb > 600
  {
    if (!ventShut)
    {
      Serial.println("Too Hot, Opening Vent");
    }
    shutVent();
  }

  /* Fan Blower Control */
  if ((ambTemp < lowTemp) && startCooking)
  {
    if (blowing == false)
    {
      Serial.println("Blower Activated"); 
    }
    blowing = true;
    digitalWrite(FANMODULE, HIGH);
  }
  else if ((ambTemp > (highTemp - 100)) && startCooking) //If Amb > 400
  {
    if (blowing == true)
    {
      Serial.println("Blower Shutdown"); 
    }
    blowing = false;
    digitalWrite(FANMODULE, LOW);
  }

  /* Flip Logic */
  if ((((currentMillis - previousFlipMillis) >= flipInterval) || flipNow) && startCooking) //If cooking and time to flip or flipoverride
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

    flipNow = false;
  }

  /* Error Logic */
  if ((GLOBAL_ERROR_COUNT >= GLOBAL_ERROR_LIMIT) || IS_ERROR || (ambTemp > (highTemp + 200))) //amb > 700
  {
     Serial.println("ERRORS DETECTED");
     Serial1.print("0,0,0,1");
     digitalWrite(FANMODULE, LOW);
     md.setM1Speed(0);
     buzzBad();
     shutVent();
     stopIfFault();
     while(1);
  }
}

//--------------------------------------------- FUNCTIONS ---------------------------------------------//

//Call to get probe temp, adds to global error count if bad temp like 0;
double getTemp(MAX6675 probe)
{
  double temp = probe.readFarenheit();

  if ((temp < 0) || (temp > 1000))
  {
    GLOBAL_ERROR_COUNT++;
  }
  
  return temp;
}

//short beep for gentle alerts
void buzzNow()
{
  digitalWrite(BUZZ, HIGH);
  delay(100);
  digitalWrite(BUZZ, LOW);
}

//beeps for a 5 seconds when there is an issue.
void buzzBad()
{
  digitalWrite(BUZZ, HIGH);
  delay(1000);
  digitalWrite(BUZZ, LOW);
}

//Uses switch value to rotate basket to home position, and breaks out if rotating too long.
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

//Uses encoder readings to rotate the basket out 180 degrees
void rotBasket()
{
  //Serial.println("rotBasket function called"); //Debug print.
  long startMillis = millis();
  long curRotMillis = 0;
  
  long newEncVal = motorEnc.read();
  //Serial.println(newEncVal);
  while (newEncVal > rotOut)//while  not turned 180 degrees
  {
    //Motor Driving Code
    md.setM1Speed(mSpeed);
    newEncVal = motorEnc.read();//Grab new value
    //Serial.println(newEncVal);//Debug print
    curRotMillis = millis();
    if(((curRotMillis - startMillis) > breakInterval))
    {
      break;
    }
  }
  //Motor Stopping code
  md.setM1Speed(0);
  motorEnc.write(0);
  isHome = false;
  //Serial.println("rotBasket function exit"); //Debug print.
  return;
}

//closes smoke stack only if open
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

//opens smoke stack only if closed
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

//Estop ISR
void ERROR_BUTTON()
{
  IS_ERROR = true;
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
